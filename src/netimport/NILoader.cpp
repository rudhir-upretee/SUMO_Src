/****************************************************************************/
/// @file    NILoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
/// @version $Id: NILoader.cpp 13109 2012-12-02 14:49:47Z behrisch $
///
// Perfoms network import
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

#include <string>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <netbuild/NBTypeCont.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOSAXReader.h>
#include <netimport/NIXMLEdgesHandler.h>
#include <netimport/NIXMLNodesHandler.h>
#include <netimport/NIXMLTrafficLightsHandler.h>
#include <netimport/NIXMLTypesHandler.h>
#include <netimport/NIXMLConnectionsHandler.h>
#include <netimport/NIImporter_DlrNavteq.h>
#include <netimport/NIImporter_VISUM.h>
#include <netimport/vissim/NIImporter_Vissim.h>
#include <netimport/NIImporter_ArcView.h>
#include <netimport/NIImporter_SUMO.h>
#include <netimport/NIImporter_RobocupRescue.h>
#include <netimport/NIImporter_OpenStreetMap.h>
#include <netimport/NIImporter_OpenDrive.h>
#include <netimport/NIImporter_MATSim.h>
#include <netimport/NIImporter_ITSUMO.h>
#include <utils/xml/XMLSubSys.h>
#include "NILoader.h"
#include <utils/common/TplConvert.h>
#include <utils/geom/GeoConvHelper.h>

#ifdef HAVE_INTERNAL
#include <internal/HeightMapper.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
NILoader::NILoader(NBNetBuilder& nb)
    : myNetBuilder(nb) {}


NILoader::~NILoader() {}


void
NILoader::load(OptionsCont& oc) {
    // build the projection
    if (!GeoConvHelper::init(oc)) {
        throw ProcessError("Could not build projection!");
    }
    // load types first
    NIXMLTypesHandler* handler =
        new NIXMLTypesHandler(myNetBuilder.getTypeCont());
    loadXMLType(handler, oc.getStringVector("type-files"), "types");
    // try to load height data so it is ready for use by other importers
#ifdef HAVE_INTERNAL
    HeightMapper::loadIfSet(oc);
#endif
    // try to load using different methods
    NIImporter_SUMO::loadNetwork(oc, myNetBuilder);
    NIImporter_RobocupRescue::loadNetwork(oc, myNetBuilder);
    NIImporter_OpenStreetMap::loadNetwork(oc, myNetBuilder);
    NIImporter_VISUM::loadNetwork(oc, myNetBuilder);
    NIImporter_ArcView::loadNetwork(oc, myNetBuilder);
    NIImporter_Vissim::loadNetwork(oc, myNetBuilder);
    NIImporter_DlrNavteq::loadNetwork(oc, myNetBuilder);
    NIImporter_OpenDrive::loadNetwork(oc, myNetBuilder);
    NIImporter_MATSim::loadNetwork(oc, myNetBuilder);
    NIImporter_ITSUMO::loadNetwork(oc, myNetBuilder);
    loadXML(oc);
    // check the loaded structures
    if (myNetBuilder.getNodeCont().size() == 0) {
        throw ProcessError("No nodes loaded.");
    }
    if (myNetBuilder.getEdgeCont().size() == 0) {
        throw ProcessError("No edges loaded.");
    }
    // report loaded structures
    WRITE_MESSAGE(" Import done:");
    if (myNetBuilder.getDistrictCont().size() > 0) {
        WRITE_MESSAGE("   " + toString(myNetBuilder.getDistrictCont().size()) + " districts loaded.");
    }
    WRITE_MESSAGE("   " + toString(myNetBuilder.getNodeCont().size()) + " nodes loaded.");
    if (myNetBuilder.getTypeCont().size() > 0) {
        WRITE_MESSAGE("   " + toString(myNetBuilder.getTypeCont().size()) + " types loaded.");
    }
    WRITE_MESSAGE("   " + toString(myNetBuilder.getEdgeCont().size()) + " edges loaded.");
    if (myNetBuilder.getEdgeCont().getNoEdgeSplits() > 0) {
        WRITE_MESSAGE("The split of edges was performed " + toString(myNetBuilder.getEdgeCont().getNoEdgeSplits()) + " times.");
    }
    if (GeoConvHelper::getProcessing().usingGeoProjection()) {
        WRITE_MESSAGE("Proj projection parameters used: '" + GeoConvHelper::getProcessing().getProjString() + "'.");
    }
}


/* -------------------------------------------------------------------------
 * file loading methods
 * ----------------------------------------------------------------------- */
void
NILoader::loadXML(OptionsCont& oc) {
    // load nodes
    loadXMLType(new NIXMLNodesHandler(myNetBuilder.getNodeCont(),
                                      myNetBuilder.getTLLogicCont(), oc),
                oc.getStringVector("node-files"), "nodes");
    // load the edges
    loadXMLType(new NIXMLEdgesHandler(myNetBuilder.getNodeCont(),
                                      myNetBuilder.getEdgeCont(),
                                      myNetBuilder.getTypeCont(),
                                      myNetBuilder.getDistrictCont(), oc),
                oc.getStringVector("edge-files"), "edges");
    // load the connections
    loadXMLType(new NIXMLConnectionsHandler(myNetBuilder.getEdgeCont()),
                oc.getStringVector("connection-files"), "connections");
    // load traffic lights (needs to come last, references loaded edges and connections)
    loadXMLType(new NIXMLTrafficLightsHandler(
                    myNetBuilder.getTLLogicCont(), myNetBuilder.getEdgeCont()),
                oc.getStringVector("tllogic-files"), "traffic lights");
}


void
NILoader::loadXMLType(SUMOSAXHandler* handler, const std::vector<std::string>& files,
                      const std::string& type) {
    // build parser
    std::string exceptMsg = "";
    // start the parsing
    try {
        for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
            if (!FileHelpers::exists(*file)) {
                WRITE_ERROR("Could not open " + type + "-file '" + *file + "'.");
                exceptMsg = "Process Error";
                continue;
            }
            PROGRESS_BEGIN_MESSAGE("Parsing " + type + " from '" + *file + "'");
            XMLSubSys::runParser(*handler, *file);
            PROGRESS_DONE_MESSAGE();
        }
    } catch (const XERCES_CPP_NAMESPACE::XMLException& toCatch) {
        exceptMsg = TplConvert::_2str(toCatch.getMessage())
                    + "\n  The " + type  + " could not be loaded from '" + handler->getFileName() + "'.";
    } catch (const ProcessError& toCatch) {
        exceptMsg = std::string(toCatch.what()) + "\n  The " + type  + " could not be loaded from '" + handler->getFileName() + "'.";
    } catch (...) {
        exceptMsg = "The " + type  + " could not be loaded from '" + handler->getFileName() + "'.";
    }
    delete handler;
    if (exceptMsg != "") {
        throw ProcessError(exceptMsg);
    }
}


bool
NILoader::transformCoordinates(Position& from, bool includeInBoundary, GeoConvHelper* from_srs) {
    Position orig(from);
    bool ok = GeoConvHelper::getProcessing().x2cartesian(from, includeInBoundary);
#ifdef HAVE_INTERNAL
    if (ok) {
        const HeightMapper& hm = HeightMapper::get();
        if (hm.ready()) {
            if (from_srs != 0 && from_srs->usingGeoProjection()) {
                from_srs->cartesian2geo(orig);
            }
            SUMOReal z = hm.getZ(orig);
            from = Position(from.x(), from.y(), z);
        }
    }
#else
    UNUSED_PARAMETER(from_srs);
#endif
    return ok;
}


bool
NILoader::transformCoordinates(PositionVector& from, bool includeInBoundary, GeoConvHelper* from_srs) {
    const SUMOReal maxLength = OptionsCont::getOptions().getFloat("geometry.max-segment-length");
    if (maxLength > 0 && from.size() > 1) {
        // transformation to cartesian coordinates must happen before we can check segment length
        PositionVector copy = from;
        for (int i = 0; i < (int) from.size(); i++) {
            transformCoordinates(copy[i], false);
        }
        // check lengths and insert new points where needed (in the original
        // coordinate system)
        int inserted = 0;
        for (int i = 0; i < (int)copy.size() - 1; i++) {
            Position start = from[i + inserted];
            Position end = from[i + inserted + 1];
            SUMOReal length = copy[i].distanceTo(copy[i + 1]);
            const Position step = (end - start) * (maxLength / length);
            int steps = 0;
            while (length > maxLength) {
                length -= maxLength;
                steps++;
                from.insertAt(i + inserted + 1, start + (step * steps));
                inserted++;
            }
        }
        // now perform the transformation again so that height mapping can be
        // performed for the new points
    }
    bool ok = true;
    for (int i = 0; i < (int) from.size(); i++) {
        ok = ok && transformCoordinates(from[i], includeInBoundary, from_srs);
    }
    return ok;
}
/****************************************************************************/
