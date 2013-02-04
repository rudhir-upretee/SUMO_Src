/****************************************************************************/
/// @file    GUISettingsHandler.cpp
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @date    Fri, 24. Apr 2009
/// @version $Id: GUISettingsHandler.cpp 13001 2012-11-16 15:12:43Z behrisch $
///
// The dialog to change the view (gui) settings.
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/common/RGBColor.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/FileHelpers.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/xml/SUMOSAXReader.h>
#include "GUISettingsHandler.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
GUISettingsHandler::GUISettingsHandler(const std::string& content, bool isFile)
    : SUMOSAXHandler(content), myDelay(-1), myZoom(-1), myXPos(-1), myYPos(-1), myCurrentColorer(SUMO_TAG_NOTHING), myCurrentScheme(0) {
    if (isFile) {
        XMLSubSys::runParser(*this, content);
    } else {
        setFileName("registrySettings");
        SUMOSAXReader* reader = XMLSubSys::getSAXReader(*this);
        reader->parseString(content);
        delete reader;
    }
}


GUISettingsHandler::~GUISettingsHandler() {
}


void
GUISettingsHandler::myStartElement(int element,
                                   const SUMOSAXAttributes& attrs) {
    bool ok = true;
    switch (element) {
        case SUMO_TAG_DELAY:
            myDelay = attrs.getOptSUMORealReporting(SUMO_ATTR_VALUE, 0, ok, myDelay);
            break;
        case SUMO_TAG_VIEWPORT:
            myZoom = attrs.getOptSUMORealReporting(SUMO_ATTR_ZOOM, 0, ok, myZoom);
            myXPos = attrs.getOptSUMORealReporting(SUMO_ATTR_X, 0, ok, myXPos);
            myYPos = attrs.getOptSUMORealReporting(SUMO_ATTR_Y, 0, ok, myYPos);
            break;
        case SUMO_TAG_SNAPSHOT: {
            bool ok = true;
            std::string file = attrs.getStringReporting(SUMO_ATTR_FILE, 0, ok);
            if (file != "" && !FileHelpers::isAbsolute(file)) {
                file = FileHelpers::getConfigurationRelative(getFileName(), file);
            }
            mySnapshots[attrs.getOptSUMOTimeReporting(SUMO_ATTR_TIME, file.c_str(), ok, 0)] = file;
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_SCHEME: {
            bool ok = true;
            mySettings.name = attrs.getOptStringReporting(SUMO_ATTR_NAME, 0, ok, mySettings.name);
            if (gSchemeStorage.contains(mySettings.name)) {
                mySettings = gSchemeStorage.get(mySettings.name);
            }
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_OPENGL:
            mySettings.antialiase = TplConvert::_2bool(attrs.getStringSecure("antialiase", toString(mySettings.antialiase)).c_str());
            mySettings.dither = TplConvert::_2bool(attrs.getStringSecure("dither", toString(mySettings.dither)).c_str());
            break;
        case SUMO_TAG_VIEWSETTINGS_BACKGROUND: {
            bool ok = true;
            mySettings.backgroundColor = RGBColor::parseColorReporting(attrs.getStringSecure("backgroundColor", toString(mySettings.backgroundColor)), "background", 0, true, ok);
            mySettings.showGrid = TplConvert::_2bool(attrs.getStringSecure("showGrid", toString(mySettings.showGrid)).c_str());
            mySettings.gridXSize = TplConvert::_2SUMOReal(attrs.getStringSecure("gridXSize", toString(mySettings.gridXSize)).c_str());
            mySettings.gridYSize = TplConvert::_2SUMOReal(attrs.getStringSecure("gridYSize", toString(mySettings.gridYSize)).c_str());
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_EDGES: {
            int laneEdgeMode = TplConvert::_2int(attrs.getStringSecure("laneEdgeMode", "0").c_str());
            mySettings.laneShowBorders = TplConvert::_2bool(attrs.getStringSecure("laneShowBorders", toString(mySettings.laneShowBorders)).c_str());
            mySettings.showLinkDecals = TplConvert::_2bool(attrs.getStringSecure("showLinkDecals", toString(mySettings.showLinkDecals)).c_str());
            mySettings.showRails = TplConvert::_2bool(attrs.getStringSecure("showRails", toString(mySettings.showRails)).c_str());
            mySettings.edgeName = parseTextSettings("edgeName", attrs, mySettings.edgeName);
            mySettings.internalEdgeName = parseTextSettings("internalEdgeName", attrs, mySettings.internalEdgeName);
            mySettings.streetName = parseTextSettings("streetName", attrs, mySettings.streetName);
            mySettings.hideConnectors = TplConvert::_2bool(attrs.getStringSecure("hideConnectors", toString(mySettings.hideConnectors)).c_str());
            myCurrentColorer = element;
#ifdef HAVE_INTERNAL
            mySettings.edgeColorer.setActive(laneEdgeMode);
#endif
            mySettings.laneColorer.setActive(laneEdgeMode);
        }
        break;
        case SUMO_TAG_COLORSCHEME:
            myCurrentScheme = 0;
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_EDGES) {
                myCurrentScheme = mySettings.laneColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
#ifdef HAVE_INTERNAL
                if (myCurrentScheme == 0) {
                    myCurrentScheme = mySettings.edgeColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
                }
#endif
            }
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_VEHICLES) {
                myCurrentScheme = mySettings.vehicleColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
            }
            if (myCurrentScheme && !myCurrentScheme->isFixed()) {
                bool ok = true;
                myCurrentScheme->setInterpolated(attrs.getOptBoolReporting(SUMO_ATTR_INTERPOLATED, 0, ok, false));
                myCurrentScheme->clear();
            }
            break;
        case SUMO_TAG_ENTRY:
            if (myCurrentScheme) {
                bool ok = true;
                RGBColor color = attrs.getColorReporting(0, ok);
                if (myCurrentScheme->isFixed()) {
                    myCurrentScheme->setColor(attrs.getStringSecure(SUMO_ATTR_NAME, ""), color);
                } else {
                    myCurrentScheme->addColor(color,
                                              attrs.getSUMORealReporting(SUMO_ATTR_THRESHOLD, 0, ok));
                }
            }
            break;
        case SUMO_TAG_VIEWSETTINGS_VEHICLES:
            mySettings.vehicleColorer.setActive(TplConvert::_2int(attrs.getStringSecure("vehicleMode", "0").c_str()));
            mySettings.vehicleQuality = TplConvert::_2int(attrs.getStringSecure("vehicleQuality", toString(mySettings.vehicleQuality)).c_str());
            mySettings.minVehicleSize = TplConvert::_2SUMOReal(attrs.getStringSecure("minVehicleSize", toString(mySettings.minVehicleSize)).c_str());
            mySettings.vehicleExaggeration = TplConvert::_2SUMOReal(attrs.getStringSecure("vehicleExaggeration", toString(mySettings.vehicleExaggeration)).c_str());
            mySettings.showBlinker = TplConvert::_2bool(attrs.getStringSecure("showBlinker", toString(mySettings.showBlinker)).c_str());
            mySettings.vehicleName = parseTextSettings("vehicleName", attrs, mySettings.vehicleName);
            myCurrentColorer = element;
            break;
        case SUMO_TAG_VIEWSETTINGS_JUNCTIONS:
            mySettings.junctionMode = TplConvert::_2int(attrs.getStringSecure("junctionMode", toString(mySettings.junctionMode)).c_str());
            mySettings.drawLinkTLIndex = TplConvert::_2bool(attrs.getStringSecure("drawLinkTLIndex", toString(mySettings.drawLinkTLIndex)).c_str());
            mySettings.drawLinkJunctionIndex = TplConvert::_2bool(attrs.getStringSecure("drawLinkJunctionIndex", toString(mySettings.drawLinkJunctionIndex)).c_str());
            mySettings.junctionName = parseTextSettings("junctionName", attrs, mySettings.junctionName);
            mySettings.internalJunctionName = parseTextSettings("internalJunctionName", attrs, mySettings.internalJunctionName);
            mySettings.showLane2Lane = TplConvert::_2bool(attrs.getStringSecure("showLane2Lane", toString(mySettings.showLane2Lane)).c_str());
            break;
        case SUMO_TAG_VIEWSETTINGS_ADDITIONALS:
            mySettings.addMode = TplConvert::_2int(attrs.getStringSecure("addMode", toString(mySettings.addMode)).c_str());
            mySettings.minAddSize = TplConvert::_2SUMOReal(attrs.getStringSecure("minAddSize", toString(mySettings.minAddSize)).c_str());
            mySettings.addExaggeration = TplConvert::_2SUMOReal(attrs.getStringSecure("addExaggeration", toString(mySettings.addExaggeration)).c_str());
            mySettings.addName = parseTextSettings("addName", attrs, mySettings.addName);
            break;
        case SUMO_TAG_VIEWSETTINGS_POIS:
            mySettings.poiExaggeration = TplConvert::_2SUMOReal(attrs.getStringSecure("poiExaggeration", toString(mySettings.poiExaggeration)).c_str());
            mySettings.minPOISize = TplConvert::_2SUMOReal(attrs.getStringSecure("minPOISize", toString(mySettings.minPOISize)).c_str());
            mySettings.poiName = parseTextSettings("poiName", attrs, mySettings.poiName);
            break;
        case SUMO_TAG_VIEWSETTINGS_LEGEND:
            mySettings.showSizeLegend = TplConvert::_2bool(attrs.getStringSecure("showSizeLegend", toString(mySettings.showSizeLegend)).c_str());
            break;
        case SUMO_TAG_VIEWSETTINGS_DECAL: {
            GUISUMOAbstractView::Decal d;
            d.filename = attrs.getStringSecure("filename", d.filename);
            if (d.filename != "" && !FileHelpers::isAbsolute(d.filename)) {
                d.filename = FileHelpers::getConfigurationRelative(getFileName(), d.filename);
            }
            d.centerX = TplConvert::_2SUMOReal(attrs.getStringSecure("centerX", toString(d.centerX)).c_str());
            d.centerY = TplConvert::_2SUMOReal(attrs.getStringSecure("centerY", toString(d.centerY)).c_str());
            d.width = TplConvert::_2SUMOReal(attrs.getStringSecure("width", toString(d.width)).c_str());
            d.height = TplConvert::_2SUMOReal(attrs.getStringSecure("height", toString(d.height)).c_str());
            d.rot = TplConvert::_2SUMOReal(attrs.getStringSecure("rotation", toString(d.rot)).c_str());
            d.layer = TplConvert::_2SUMOReal(attrs.getStringSecure("layer", toString(d.layer)).c_str());
            d.initialised = false;
            myDecals.push_back(d);
        }
        break;
        default:
            break;
    }
}


GUIVisualizationTextSettings
GUISettingsHandler::parseTextSettings(
    const std::string& prefix, const SUMOSAXAttributes& attrs,
    GUIVisualizationTextSettings defaults) {
    bool ok = true;
    return GUIVisualizationTextSettings(
               TplConvert::_2bool(attrs.getStringSecure(prefix + "_show", toString(defaults.show)).c_str()),
               TplConvert::_2SUMOReal(attrs.getStringSecure(prefix + "_size", toString(defaults.size)).c_str()),
               RGBColor::parseColorReporting(attrs.getStringSecure(prefix + "_color", toString(defaults.color)), "edges", 0, true, ok));
}


std::string
GUISettingsHandler::addSettings(GUISUMOAbstractView* view) const {
    if (mySettings.name != "") {
        gSchemeStorage.add(mySettings);
        if (view) {
            FXint index = view->getColoringSchemesCombo().appendItem(mySettings.name.c_str());
            view->getColoringSchemesCombo().setCurrentItem(index);
            view->setColorScheme(mySettings.name);
        }
    }
    return mySettings.name;
}


void
GUISettingsHandler::setViewport(GUISUMOAbstractView* view) const {
    if (myZoom > 0) {
        view->setViewport(myZoom, myXPos, myYPos);
    }
}


void
GUISettingsHandler::setViewport(SUMOReal& zoom, SUMOReal& xoff, SUMOReal& yoff) const {
    zoom = myZoom;
    xoff = myXPos;
    yoff = myYPos;
}


void
GUISettingsHandler::setSnapshots(GUISUMOAbstractView* view) const {
    if (!mySnapshots.empty()) {
        view->setSnapshots(mySnapshots);
    }
}


bool
GUISettingsHandler::hasDecals() const {
    return !myDecals.empty();
}


const std::vector<GUISUMOAbstractView::Decal>&
GUISettingsHandler::getDecals() const {
    return myDecals;
}


SUMOReal
GUISettingsHandler::getDelay() const {
    return myDelay;
}


/****************************************************************************/

