/****************************************************************************/
/// @file    GUISUMOAbstractView.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Andreas Gaubatz
/// @date    Sept 2002
/// @version $Id: GUISUMOAbstractView.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// The base class for a view
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

#include <iostream>
#include <utility>
#include <cmath>
#include <cassert>
#include <limits>
#include <fxkeys.h>
#include <foreign/polyfonts/polyfonts.h>
#include <foreign/gl2ps/gl2ps.h>
#include <utils/foxtools/FXSingleEventThread.h>
#include <utils/foxtools/MFXCheckableButton.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/common/RGBColor.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/shapes/PointOfInterest.h>
#include <utils/gui/globjects/GUIPointOfInterest.h>
#include <utils/gui/globjects/GUIPolygon.h>
#include <utils/gui/windows/GUIDialog_ViewSettings.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GUISUMOAbstractView.h"
#include "GUIMainWindow.h"
#include "GUIGlChildWindow.h"
#include "GUIDanielPerspectiveChanger.h"
#include "GUIDialog_EditViewport.h"

#ifdef WIN32
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// member method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * GUISUMOAbstractView - FOX callback mapping
 * ----------------------------------------------------------------------- */
FXDEFMAP(GUISUMOAbstractView) GUISUMOAbstractViewMap[] = {
    FXMAPFUNC(SEL_CONFIGURE,           0,                 GUISUMOAbstractView::onConfigure),
    FXMAPFUNC(SEL_PAINT,               0,                 GUISUMOAbstractView::onPaint),
    FXMAPFUNC(SEL_LEFTBUTTONPRESS,     0,                 GUISUMOAbstractView::onLeftBtnPress),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE,   0,                 GUISUMOAbstractView::onLeftBtnRelease),
    FXMAPFUNC(SEL_RIGHTBUTTONPRESS,    0,                 GUISUMOAbstractView::onRightBtnPress),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,  0,                 GUISUMOAbstractView::onRightBtnRelease),
    FXMAPFUNC(SEL_MOUSEWHEEL,          0,                 GUISUMOAbstractView::onMouseWheel),
    FXMAPFUNC(SEL_MOTION,              0,                 GUISUMOAbstractView::onMouseMove),
    FXMAPFUNC(SEL_LEAVE,               0,                 GUISUMOAbstractView::onMouseLeft),
    FXMAPFUNC(SEL_KEYPRESS,            0,                 GUISUMOAbstractView::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE,          0,                 GUISUMOAbstractView::onKeyRelease),

};


FXIMPLEMENT_ABSTRACT(GUISUMOAbstractView, FXGLCanvas, GUISUMOAbstractViewMap, ARRAYNUMBER(GUISUMOAbstractViewMap))


/* -------------------------------------------------------------------------
 * GUISUMOAbstractView - methods
 * ----------------------------------------------------------------------- */
GUISUMOAbstractView::GUISUMOAbstractView(FXComposite* p,
        GUIMainWindow& app,
        GUIGlChildWindow* parent,
        const SUMORTree& grid,
        FXGLVisual* glVis, FXGLCanvas* share)
    : FXGLCanvas(p, glVis, share, p, MID_GLCANVAS,
                 LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0),
    myApp(&app),
    myParent(parent),
    myGrid(&((SUMORTree&)grid)),
    myChanger(0),
    myMouseHotspotX(app.getDefaultCursor()->getHotX()),
    myMouseHotspotY(app.getDefaultCursor()->getHotY()),
    myPopup(0),
    myUseToolTips(false),
    myAmInitialised(false),
    myViewportChooser(0),
    myVisualizationChanger(0) {
    setTarget(this);
    enable();
    flags |= FLAG_ENABLED;
    myInEditMode = false;
    // show the middle at the beginning
    myChanger = new GUIDanielPerspectiveChanger(*this, *myGrid);
    myVisualizationSettings = &gSchemeStorage.getDefault();
    myVisualizationSettings->gaming = myApp->isGaming();
    gSchemeStorage.setViewport(this);
}


GUISUMOAbstractView::~GUISUMOAbstractView() {
    gSchemeStorage.setDefault(myVisualizationSettings->name);
    gSchemeStorage.saveViewport(myChanger->getXPos(), myChanger->getYPos(), myChanger->getZoom());
    delete myPopup;
    delete myChanger;
    delete myViewportChooser;
    delete myVisualizationChanger;
    // cleanup decals
    for (std::vector<GUISUMOAbstractView::Decal>::iterator it = myDecals.begin(); it != myDecals.end(); ++it) {
        delete it->image;
    }
}


bool
GUISUMOAbstractView::isInEditMode() {
    return myInEditMode;
}


void
GUISUMOAbstractView::updateToolTip() {
    if (!myUseToolTips) {
        return;
    }
    update();
}


Position
GUISUMOAbstractView::getPositionInformation() const {
    Boundary bound = myChanger->getViewport();
    SUMOReal x = bound.xmin() + bound.getWidth()  * myWindowCursorPositionX / getWidth();
    // cursor origin is in the top-left corner
    SUMOReal y = bound.ymin() + bound.getHeight() * (getHeight() - myWindowCursorPositionY) / getHeight();
    return Position(x, y);
}


void
GUISUMOAbstractView::updatePositionInformation() const {
    Position pos = getPositionInformation();
    std::string text = "x:" + toString(pos.x()) + ", y:" + toString(pos.y());
    myApp->getCartesianLabel().setText(text.c_str());
    GeoConvHelper::getFinal().cartesian2geo(pos);
    if (GeoConvHelper::getFinal().usingGeoProjection()) {
        text = "lat:" + toString(pos.y(), GEO_OUTPUT_ACCURACY) + ", lon:" + toString(pos.x(), GEO_OUTPUT_ACCURACY);
    } else {
        text = "x:" + toString(pos.x()) + ", y:" + toString(pos.y());
    }
    myApp->getGeoLabel().setText(text.c_str());
}


Boundary
GUISUMOAbstractView::getVisibleBoundary() const {
    return myChanger->getViewport();
}

void
GUISUMOAbstractView::paintGL() {
    if (getWidth() == 0 || getHeight() == 0) {
        return;
    }

    if (getTrackedID() > 0) {
        centerTo(getTrackedID(), false);
    }

    unsigned int id = 0;
    if (myUseToolTips) {
        id = getObjectUnderCursor();
    }

    // draw
    glClearColor(
        myVisualizationSettings->backgroundColor.red(),
        myVisualizationSettings->backgroundColor.green(),
        myVisualizationSettings->backgroundColor.blue(),
        1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (myVisualizationSettings->dither) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
    if (myVisualizationSettings->antialiase) {
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
    } else {
        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
    }

    applyGLTransform();
    doPaintGL(GL_RENDER, myChanger->getViewport());
    if (myVisualizationSettings->showSizeLegend) {
        displayLegend();
    }
    // check whether the select mode /tooltips)
    //  shall be computed, too
    if (myUseToolTips && id != 0) {
        showToolTipFor(id);
    }
    swapBuffers();
}


GUIGlID
GUISUMOAbstractView::getObjectUnderCursor() {
    return getObjectAtPosition(getPositionInformation());
}


GUIGlID
GUISUMOAbstractView::getObjectAtPosition(Position pos) {
    const SUMOReal SENSITIVITY = 0.1; // meters
    Boundary selection;
    selection.add(pos);
    selection.grow(SENSITIVITY);
    const std::vector<GUIGlID> ids = getObjectsInBoundary(selection);
    // Interpret results
    unsigned int idMax = 0;
    SUMOReal maxLayer = std::numeric_limits<SUMOReal>::min();
    for (std::vector<GUIGlID>::const_iterator it = ids.begin(); it != ids.end(); it++) {
        GUIGlID id = *it;
        GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        if (o == 0) {
            continue;
        }
        if (o->getGlID() == 0) {
            continue;
        }
        //std::cout << "point selection hit " << o->getMicrosimID() << "\n";
        GUIGlObjectType type = o->getType();
        if (type != 0) {
            SUMOReal layer = (SUMOReal)type;
            // determine an "abstract" layer for shapes
            //  this "layer" resembles the layer of the shape
            //  taking into account the stac of other objects
            if (type == GLO_POI || type == GLO_POLYGON) {
                layer = dynamic_cast<Shape*>(o)->getLayer();
            }
            // check whether the current object is above a previous one
            if (layer > maxLayer) {
                idMax = id;
                maxLayer = layer;
            }
        }
        GUIGlObjectStorage::gIDStorage.unblockObject(id);
    }
    return idMax;
}


std::vector<GUIGlID>
GUISUMOAbstractView::getObjectsInBoundary(const Boundary& bound) {
    const int NB_HITS_MAX = 1024 * 1024;
    // Prepare the selection mode
    static GUIGlID hits[NB_HITS_MAX];
    static GLint nb_hits = 0;
    glSelectBuffer(NB_HITS_MAX, hits);
    glInitNames();

    Boundary oldViewPort = myChanger->getViewport(false); // backup the actual viewPort
    myChanger->setViewport(bound);
    applyGLTransform(false);

    // paint in select mode
    int hits2 = doPaintGL(GL_SELECT, bound);
    // Get the results
    nb_hits = glRenderMode(GL_RENDER);
    if (nb_hits == -1) {
        myApp->setStatusBarText("Selection in boundary failed. Try to select fewer than " + toString(hits2) + " items");
    }
    std::vector<GUIGlID> result;
    for (int i = 0; i < nb_hits; ++i) {
        assert(i * 4 + 3 < NB_HITS_MAX);
        result.push_back(hits[i * 4 + 3]);
    }
    // switch viewport back to normal
    myChanger->setViewport(oldViewPort);
    return result;
}


void
GUISUMOAbstractView::showToolTipFor(unsigned int id) {
    if (id != 0) {
        GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        if (object != 0) {
            Position pos = getPositionInformation();
            pos.add(0, p2m(15));
            GLHelper::drawTextBox(object->getFullName(), pos, GLO_MAX - 1, p2m(20), RGBColor(0, 0, 0), RGBColor(1, 0.7, 0));
            GUIGlObjectStorage::gIDStorage.unblockObject(id);
        }
    }
}


void
GUISUMOAbstractView::paintGLGrid() {
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1);

    SUMOReal xmin = myGrid->xmin();
    SUMOReal ymin = myGrid->ymin();
    SUMOReal ypos = ymin;
    SUMOReal xpos = xmin;
    SUMOReal xend = myGrid->xmax();
    SUMOReal yend = myGrid->ymax();

    glTranslated(0, 0, .55);
    glColor3d(0.5, 0.5, 0.5);
    // draw horizontal lines
    glBegin(GL_LINES);
    for (; ypos < yend;) {
        glVertex2d(xmin, ypos);
        glVertex2d(xend, ypos);
        ypos += myVisualizationSettings->gridYSize;
    }
    // draw vertical lines
    for (; xpos < xend;) {
        glVertex2d(xpos, ymin);
        glVertex2d(xpos, yend);
        xpos += myVisualizationSettings->gridXSize;
    }
    glEnd();
    glTranslated(0, 0, -.55);
}


void
GUISUMOAbstractView::displayLegend() {
    // compute the scale bar length
    size_t length = 1;
    const std::string text("10000000000");
    size_t noDigits = 1;
    size_t pixelSize = (size_t) m2p((SUMOReal) length);
    while (pixelSize <= 20) {
        length *= 10;
        noDigits++;
        if (noDigits > text.length()) {
            return;
        }
        pixelSize = (size_t) m2p((SUMOReal) length);
    }
    SUMOReal lineWidth = 1.0;
    glLineWidth((SUMOReal) lineWidth);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // draw the scale bar
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    SUMOReal len = (SUMOReal) pixelSize / (SUMOReal)(getWidth() - 1) * (SUMOReal) 2.0;
    glColor3d(0, 0, 0);
    double o = double(15) / double(getHeight());
    double o2 = o + o;
    double oo = double(5) / double(getHeight());
    glBegin(GL_LINES);
    // vertical
    glVertex2d(-.98, -1. + o);
    glVertex2d(-.98 + len, -1. + o);
    // tick at begin
    glVertex2d(-.98, -1. + o);
    glVertex2d(-.98, -1. + o2);
    // tick at end
    glVertex2d(-.98 + len, -1. + o);
    glVertex2d(-.98 + len, -1. + o2);
    glEnd();

    SUMOReal w = SUMOReal(35) / SUMOReal(getWidth());
    SUMOReal h = SUMOReal(35) / SUMOReal(getHeight());
    pfSetPosition(SUMOReal(-0.99), SUMOReal(1. - o2 - oo));
    pfSetScaleXY(w, h);
    glRotated(180, 1, 0, 0);
    pfDrawString("0m");
    glRotated(-180, 1, 0, 0);

    pfSetPosition(SUMOReal(-.99 + len), SUMOReal(1. - o2 - oo));
    glRotated(180, 1, 0, 0);
    pfDrawString((text.substr(0, noDigits) + "m").c_str());
    glRotated(-180, 1, 0, 0);

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


SUMOReal
GUISUMOAbstractView::m2p(SUMOReal meter) const {
    return  meter * getWidth() / myChanger->getViewport().getWidth();
}


SUMOReal
GUISUMOAbstractView::p2m(SUMOReal pixel) const {
    return pixel * myChanger->getViewport().getWidth() / getWidth();
}


void
GUISUMOAbstractView::recenterView() {
    myChanger->setViewport(*myGrid);
}


void
GUISUMOAbstractView::centerTo(GUIGlID id, bool applyZoom, SUMOReal zoomDist) {
    GUIGlObject* o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
    if (o != 0 && dynamic_cast<GUIGlObject*>(o) != 0) {
        if (applyZoom && zoomDist < 0) {
            myChanger->setViewport(o->getCenteringBoundary());
        } else {
            myChanger->centerTo(o->getCenteringBoundary().getCenter(), zoomDist, applyZoom);
        }
        update();
    }
    GUIGlObjectStorage::gIDStorage.unblockObject(id);
}


void
GUISUMOAbstractView::centerTo(const Boundary& bound) {
    myChanger->setViewport(bound);
    update();
}

/*
bool
GUISUMOAbstractView::allowRotation() const
{
    return myParent->allowRotation();
}
*/

void
GUISUMOAbstractView::setWindowCursorPosition(FXint x, FXint y) {
    myWindowCursorPositionX = x + myMouseHotspotX;
    myWindowCursorPositionY = y + myMouseHotspotY;
}


FXbool
GUISUMOAbstractView::makeCurrent() {
    FXbool ret = FXGLCanvas::makeCurrent();
    return ret;
}


long
GUISUMOAbstractView::onConfigure(FXObject*, FXSelector, void*) {
    if (makeCurrent()) {
        glViewport(0, 0, getWidth() - 1, getHeight() - 1);
        glClearColor(
            myVisualizationSettings->backgroundColor.red(),
            myVisualizationSettings->backgroundColor.green(),
            myVisualizationSettings->backgroundColor.blue(),
            1);
        doInit();
        myAmInitialised = true;
        makeNonCurrent();
        checkSnapshots();
    }
    return 1;
}


long
GUISUMOAbstractView::onPaint(FXObject*, FXSelector, void*) {
    if (!isEnabled() || !myAmInitialised) {
        return 1;
    }
    if (makeCurrent()) {
        paintGL();
        makeNonCurrent();
    }
    return 1;
}


void
GUISUMOAbstractView::destroyPopup() {
    delete myPopup;
    myPopup = 0;
}


long
GUISUMOAbstractView::onLeftBtnPress(FXObject*, FXSelector , void* data) {
    destroyPopup();
    FXEvent* e = (FXEvent*) data;
    // check whether the selection-mode is activated
    if (e->state & CONTROLMASK) {
        // try to get the object-id if so
        if (makeCurrent()) {
            unsigned int id = getObjectUnderCursor();
            if (id != 0) {
                gSelected.toggleSelection(id);
            }
            makeNonCurrent();
            if (id != 0) {
                // possibly, the selection-colouring is used,
                //  so we should update the screen again...
                update();
            }
        }
    }
    myChanger->onLeftBtnPress(data);
    grab();
    return 1;
}


long
GUISUMOAbstractView::onLeftBtnRelease(FXObject*, FXSelector , void* data) {
    destroyPopup();
    myChanger->onLeftBtnRelease(data);
    if (myApp->isGaming()) {
        onGamingClick(getPositionInformation());
    }
    ungrab();
    return 1;
}


long
GUISUMOAbstractView::onRightBtnPress(FXObject*, FXSelector , void* data) {
    destroyPopup();
    myChanger->onRightBtnPress(data);
    grab();
    return 1;
}


long
GUISUMOAbstractView::onRightBtnRelease(FXObject*, FXSelector , void* data) {
    destroyPopup();
    if (!myChanger->onRightBtnRelease(data) && !myApp->isGaming()) {
        openObjectDialog();
    }
    ungrab();
    return 1;
}


long
GUISUMOAbstractView::onMouseWheel(FXObject*, FXSelector , void* data) {
    myChanger->onMouseWheel(data);
    return 1;
}


long
GUISUMOAbstractView::onMouseMove(FXObject*, FXSelector , void* data) {
    SUMOReal xpos = myChanger->getXPos();
    SUMOReal ypos = myChanger->getYPos();
    SUMOReal zoom = myChanger->getZoom();
    if (myViewportChooser == 0 || !myViewportChooser->haveGrabbed()) {
        myChanger->onMouseMove(data);
    }
    if (myViewportChooser != 0 &&
            (xpos != myChanger->getXPos() || ypos != myChanger->getYPos() || zoom != myChanger->getZoom())) {

        myViewportChooser->setValues(
            myChanger->getZoom(), myChanger->getXPos(), myChanger->getYPos());

    }
    updatePositionInformation();
    return 1;
}


long
GUISUMOAbstractView::onMouseLeft(FXObject*, FXSelector , void* /*data*/) {
    return 1;
}


void
GUISUMOAbstractView::openObjectDialog() {
    ungrab();
    if (!isEnabled() || !myAmInitialised) {
        return;
    }
    if (makeCurrent()) {
        // initialise the select mode
        unsigned int id = getObjectUnderCursor();
        GUIGlObject* o = 0;
        if (id != 0) {
            o = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
        } else {
            o = GUIGlObjectStorage::gIDStorage.getNetObject();
        }
        if (o != 0) {
            myPopup = o->getPopUpMenu(*myApp, *this);
            int x, y;
            FXuint b;
            myApp->getCursorPosition(x, y, b);
            myPopup->setX(x + myApp->getX());
            myPopup->setY(y + myApp->getY());
            myPopup->create();
            myPopup->show();
            myChanger->onRightBtnRelease(0);
            GUIGlObjectStorage::gIDStorage.unblockObject(id);
        }
        makeNonCurrent();
    }
}


long
GUISUMOAbstractView::onKeyPress(FXObject* o, FXSelector sel, void* data) {
    FXEvent* e = (FXEvent*) data;
    if ((e->state & ALTMASK) != 0) {
        setDefaultCursor(getApp()->getDefaultCursor(DEF_CROSSHAIR_CURSOR));
        grabKeyboard();
    }
    /*
    switch(e->code) {
    case KEY_Left:
        myChanger->move((SUMOReal) -p2m((SUMOReal) getWidth()/10), 0);
        break;
    case KEY_Right:
        myChanger->move((SUMOReal) p2m((SUMOReal) getWidth()/10), 0);
        break;
    case KEY_Up:
        myChanger->move(0, (SUMOReal) -p2m((SUMOReal) getHeight()/10));
        break;
    case KEY_Down:
        myChanger->move(0, (SUMOReal) p2m((SUMOReal) getHeight()/10));
        break;
    default:
        break;
    }
    */
    return FXGLCanvas::onKeyPress(o, sel, data);
}


long
GUISUMOAbstractView::onKeyRelease(FXObject* o, FXSelector sel, void* data) {
    FXEvent* e = (FXEvent*) data;
    if ((e->state & ALTMASK) == 0) {
        ungrabKeyboard();
        setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
    }
    return FXGLCanvas::onKeyRelease(o, sel, data);
}


// ------------ Dealing with snapshots
void
GUISUMOAbstractView::setSnapshots(std::map<SUMOTime, std::string> snaps) {
    mySnapshots.insert(snaps.begin(), snaps.end());
}


std::string
GUISUMOAbstractView::makeSnapshot(const std::string& destFile) {
    std::string errorMessage;
    FXString ext = FXPath::extension(destFile.c_str());
    bool useGL2PS = ext == "ps" || ext == "eps" || ext == "pdf" || ext == "svg" || ext == "tex" || ext == "pgf";

    for (int i = 0; i < 10 && !makeCurrent(); ++i) {
        FXSingleEventThread::sleep(100);
    }
    // draw
    glClearColor(
        myVisualizationSettings->backgroundColor.red(),
        myVisualizationSettings->backgroundColor.green(),
        myVisualizationSettings->backgroundColor.blue(),
        1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (myVisualizationSettings->dither) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
    if (myVisualizationSettings->antialiase) {
        glEnable(GL_BLEND);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
    } else {
        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
    }

    applyGLTransform();

    if (useGL2PS) {
        GLint format = GL2PS_PS;
        if (ext == "ps") {
            format = GL2PS_PS;
        } else if (ext == "eps") {
            format = GL2PS_EPS;
        } else if (ext == "pdf") {
            format = GL2PS_PDF;
        } else if (ext == "tex") {
            format = GL2PS_TEX;
        } else if (ext == "svg") {
            format = GL2PS_SVG;
        } else if (ext == "pgf") {
            format = GL2PS_PGF;
        } else {
            return "Could not save '" + destFile + "'.\n Unrecognized format '" + std::string(ext.text()) + "'.";
        }
        FILE* fp = fopen(destFile.c_str(), "wb");
        if (fp == 0) {
            return "Could not save '" + destFile + "'.\n Could not open file for writing";
        }
        GLint buffsize = 0, state = GL2PS_OVERFLOW;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        while (state == GL2PS_OVERFLOW) {
            buffsize += 1024 * 1024;
            gl2psBeginPage(destFile.c_str(), "sumo-gui; http://sumo.sf.net", viewport, format, GL2PS_SIMPLE_SORT,
                           GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT,
                           GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, "out.eps");
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            // compute lane width
            // draw decals (if not in grabbing mode)
            if (!myUseToolTips) {
                drawDecals();
                if (myVisualizationSettings->showGrid) {
                    paintGLGrid();
                }
            }
            glLineWidth(1);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            Boundary viewPort = myChanger->getViewport();
            float minB[2];
            float maxB[2];
            minB[0] = viewPort.xmin();
            minB[1] = viewPort.ymin();
            maxB[0] = viewPort.xmax();
            maxB[1] = viewPort.ymax();
            myVisualizationSettings->scale = m2p(SUMO_const_laneWidth);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_LINE);
            myGrid->Search(minB, maxB, *myVisualizationSettings);

            if (myVisualizationSettings->showSizeLegend) {
                displayLegend();
            }
            state = gl2psEndPage();
            glFinish();
        }
        fclose(fp);
    } else {
        doPaintGL(GL_RENDER, myChanger->getViewport());
        if (myVisualizationSettings->showSizeLegend) {
            displayLegend();
        }
        swapBuffers();
        glFinish();
        FXColor* buf;
        FXMALLOC(&buf, FXColor, getWidth()*getHeight());
        // read from the back buffer
        glReadBuffer(GL_BACK);
        // Read the pixels
        glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buf);
        makeNonCurrent();
        update();
        // mirror
        size_t mwidth = getWidth();
        size_t mheight = getHeight();
        FXColor* paa = buf;
        FXColor* pbb = buf + mwidth * (mheight - 1);
        do {
            FXColor* pa = paa;
            paa += mwidth;
            FXColor* pb = pbb;
            pbb -= mwidth;
            do {
                FXColor t = *pa;
                *pa++ = *pb;
                *pb++ = t;
            } while (pa < paa);
        } while (paa < pbb);
        try {
            if (!MFXImageHelper::saveImage(destFile, getWidth(), getHeight(), buf)) {
                errorMessage = "Could not save '" + destFile + "'.";
            }
        } catch (InvalidArgument& e) {
            errorMessage = "Could not save '" + destFile + "'.\n" + e.what();
        }
        FXFREE(&buf);
    }
    return errorMessage;
}


void
GUISUMOAbstractView::checkSnapshots() {
    std::map<SUMOTime, std::string>::iterator snapIt = mySnapshots.find(getCurrentTimeStep());
    if (snapIt != mySnapshots.end()) {
        std::string error = makeSnapshot(snapIt->second);
        if (error != "") {
            WRITE_WARNING(error);
        }
    }
}


void
GUISUMOAbstractView::showViewportEditor() {
    if (myViewportChooser == 0) {
        myViewportChooser =
            new GUIDialog_EditViewport(this, "Edit Viewport...",
                                       myChanger->getZoom(), myChanger->getXPos(), myChanger->getYPos(),
                                       0, 0);
        myViewportChooser->create();
    }
    myViewportChooser->setOldValues(
        myChanger->getZoom(), myChanger->getXPos(), myChanger->getYPos());
    myViewportChooser->show();
}


void
GUISUMOAbstractView::setViewport(SUMOReal zoom, SUMOReal xPos, SUMOReal yPos) {
    myChanger->setViewport(zoom, xPos, yPos);
    update();
}


void
GUISUMOAbstractView::showToolTips(bool val) {
    myUseToolTips = val;
}



SUMOReal
GUISUMOAbstractView::getGridWidth() const {
    return myGrid->getWidth();
}


SUMOReal
GUISUMOAbstractView::getGridHeight() const {
    return myGrid->getHeight();
}


FXComboBox&
GUISUMOAbstractView::getColoringSchemesCombo() {
    return myParent->getColoringSchemesCombo();
}


void
GUISUMOAbstractView::drawDecals() {
    glPushName(0);
    myDecalsLock.lock();
    for (std::vector<GUISUMOAbstractView::Decal>::iterator l = myDecals.begin(); l != myDecals.end();) {
        GUISUMOAbstractView::Decal& d = *l;
        if (!d.initialised) {
            try {
                FXImage* i = MFXImageHelper::loadImage(getApp(), d.filename);
                if (MFXImageHelper::scalePower2(i)) {
                    WRITE_WARNING("Scaling '" + d.filename + "'.");
                }
                d.glID = GUITexturesHelper::add(i);
                d.initialised = true;
                d.image = i;
            } catch (InvalidArgument& e) {
                WRITE_ERROR("Could not load '" + d.filename + "'.\n" + e.what());
                l = myDecals.erase(l);
                continue;
            }
        }
        glPushMatrix();
        glTranslated(d.centerX, d.centerY, d.layer);
        glRotated(d.rot, 0, 0, 1);
        glColor3d(1, 1, 1);
        SUMOReal halfWidth((d.width / 2.));
        SUMOReal halfHeight((d.height / 2.));
        GUITexturesHelper::drawTexturedBox(d.glID, -halfWidth, -halfHeight, halfWidth, halfHeight);
        glPopMatrix();
        ++l;
    }
    myDecalsLock.unlock();
    glPopName();
}


// ------------ Additional visualisations
bool
GUISUMOAbstractView::addAdditionalGLVisualisation(GUIGlObject* const which) {
    if (myAdditionallyDrawn.find(which) == myAdditionallyDrawn.end()) {
        myAdditionallyDrawn[which] = 1;
    } else {
        myAdditionallyDrawn[which] = myAdditionallyDrawn[which] + 1;
    }
    update();
    return true;
}


bool
GUISUMOAbstractView::removeAdditionalGLVisualisation(GUIGlObject* const which) {
    if (getTrackedID() == static_cast<int>(which->getGlID())) {
        stopTrack();
    }
    if (myAdditionallyDrawn.find(which) == myAdditionallyDrawn.end()) {
        return false;
    }
    int cnt = myAdditionallyDrawn[which];
    if (cnt == 1) {
        myAdditionallyDrawn.erase(which);
    } else {
        myAdditionallyDrawn[which] = myAdditionallyDrawn[which] - 1;
    }
    update();
    return true;
}


void
GUISUMOAbstractView::applyGLTransform(bool fixRatio) {
    Boundary bound = myChanger->getViewport(fixRatio);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // as a rough rule, each GLObject is drawn at z = -GUIGlObjectType
    // thus, objects with a higher value will be closer (drawn on top)
    // // @todo last param should be 0 after modifying all glDraw methods
    glOrtho(0, getWidth(), 0, getHeight(), -GLO_MAX - 1, GLO_MAX + 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    SUMOReal scaleX = (SUMOReal)getWidth() / bound.getWidth();
    SUMOReal scaleY = (SUMOReal)getHeight() / bound.getHeight();
    glScaled(scaleX, scaleY, 1);
    glTranslated(-bound.xmin(), -bound.ymin(), 0);
}

/****************************************************************************/

