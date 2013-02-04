/****************************************************************************/
/// @file    GUIParameterTableWindow.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id: GUIParameterTableWindow.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// The window that holds the table of an object's parameter
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
#include <fx.h>
#include "GUIParameterTableWindow.h"
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/common/ToString.h>
#include <utils/gui/div/GUIParam_PopupMenu.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUIMainWindow.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/div/GUIParameterTableItem.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GUIParameterTableWindow) GUIParameterTableWindowMap[] = {
    FXMAPFUNC(SEL_COMMAND,          MID_SIMSTEP,    GUIParameterTableWindow::onSimStep),
    FXMAPFUNC(SEL_SELECTED,         MID_TABLE,      GUIParameterTableWindow::onTableSelected),
    FXMAPFUNC(SEL_DESELECTED,       MID_TABLE,      GUIParameterTableWindow::onTableDeselected),
    FXMAPFUNC(SEL_RIGHTBUTTONPRESS, MID_TABLE,      GUIParameterTableWindow::onRightButtonPress),
};

FXIMPLEMENT(GUIParameterTableWindow, FXMainWindow, GUIParameterTableWindowMap, ARRAYNUMBER(GUIParameterTableWindowMap))


// ===========================================================================
// method definitions
// ===========================================================================
GUIParameterTableWindow::GUIParameterTableWindow(GUIMainWindow& app,
        GUIGlObject& o, size_t noRows)
    : FXMainWindow(app.getApp(), (o.getFullName() + " Parameter").c_str(),
                   NULL, NULL, DECOR_ALL, 20, 20, 400, (FXint)(noRows * 20 + 60)),
    myObject(&o),
    myApplication(&app), myCurrentPos(0) {
    myTable = new FXTable(this, this, MID_TABLE, TABLE_COL_SIZABLE | TABLE_ROW_SIZABLE | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    myTable->setVisibleRows((FXint)(noRows + 1));
    myTable->setVisibleColumns(3);
    myTable->setTableSize((FXint)(noRows + 1), 3);
    myTable->setBackColor(FXRGB(255, 255, 255));
    myTable->setColumnText(0, "Name");
    myTable->setColumnText(1, "Value");
    myTable->setColumnText(2, "Dynamic");
    myTable->getRowHeader()->setWidth(0);
    FXHeader* header = myTable->getColumnHeader();
    header->setItemJustify(0, JUSTIFY_CENTER_X);
    header->setItemSize(0, 250);
    header->setItemJustify(1, JUSTIFY_CENTER_X);
    header->setItemSize(1, 80);
    header->setItemJustify(2, JUSTIFY_CENTER_X);
    header->setItemSize(2, 60);
    setIcon(GUIIconSubSys::getIcon(ICON_APP_TABLE));
    myLock.lock();
    myObject->addParameterTable(this);
    myLock.unlock();
}


GUIParameterTableWindow::~GUIParameterTableWindow() {
    myApplication->removeChild(this);
    myLock.lock();
    for (std::vector<GUIParameterTableItemInterface*>::iterator i = myItems.begin(); i != myItems.end(); ++i) {
        delete(*i);
    }
    if (myObject != 0) {
        myObject->removeParameterTable(this);
    }
    myLock.unlock();
}


void
GUIParameterTableWindow::removeObject(GUIGlObject* /*i*/) {
    AbstractMutex::ScopedLocker locker(myLock);
    myObject = 0;
}


long
GUIParameterTableWindow::onSimStep(FXObject*, FXSelector, void*) {
    updateTable();
    update();
    return 1;
}


long
GUIParameterTableWindow::onTableSelected(FXObject*, FXSelector, void*) {
    return 1;
}


long
GUIParameterTableWindow::onTableDeselected(FXObject*, FXSelector, void*) {
    return 1;
}


long
GUIParameterTableWindow::onRightButtonPress(FXObject* sender,
        FXSelector sel,
        void* data) {
    // check which value entry was pressed
    myTable->onLeftBtnPress(sender, sel, data);
    int row = myTable->getCurrentRow();
    if (row == -1 || row >= (int)(myItems.size())) {
        return 1;
    }
    GUIParameterTableItemInterface* i = myItems[row];
    if (!i->dynamic()) {
        return 1;
    }

    GUIParam_PopupMenuInterface* p = new GUIParam_PopupMenuInterface(*myApplication, *this, *myObject, i->getName(), i->getSUMORealSourceCopy());
    new FXMenuCommand(p, "Open in new Tracker", 0, p, MID_OPENTRACKER);
    // set geometry
    p->setX(static_cast<FXEvent*>(data)->root_x);
    p->setY(static_cast<FXEvent*>(data)->root_y);
    p->create();
    // show
    p->show();
    return 1;
}



void
GUIParameterTableWindow::mkItem(const char* name, bool dynamic,
                                ValueSource<unsigned>* src) {
    GUIParameterTableItemInterface* i = new GUIParameterTableItem<unsigned>(myTable, myCurrentPos++, name, dynamic, src);
    myItems.push_back(i);
}


void
GUIParameterTableWindow::mkItem(const char* name, bool dynamic,
                                ValueSource<SUMOReal>* src) {
    GUIParameterTableItemInterface* i = new GUIParameterTableItem<SUMOReal>(myTable, myCurrentPos++, name, dynamic, src);
    myItems.push_back(i);
}


#ifndef HAVE_SUBSECOND_TIMESTEPS
void
GUIParameterTableWindow::mkItem(const char* name, bool dynamic,
                                ValueSource<SUMOTime>* src) {
    GUIParameterTableItemInterface* i = new GUIParameterTableItem<SUMOTime>(myTable, myCurrentPos++, name, dynamic, src);
    myItems.push_back(i);
}
#endif


void
GUIParameterTableWindow::mkItem(const char* name, bool dynamic,
                                std::string value) {
    // T = SUMOReal is only a dummy type here
    GUIParameterTableItemInterface* i = new GUIParameterTableItem<SUMOReal>(myTable, myCurrentPos++, name, dynamic, value);
    myItems.push_back(i);
}


void
GUIParameterTableWindow::mkItem(const char* name, bool dynamic,
                                SUMOReal value) {
    GUIParameterTableItemInterface* i = new GUIParameterTableItem<SUMOReal>(myTable, myCurrentPos++, name, dynamic, value);
    myItems.push_back(i);
}


void
GUIParameterTableWindow::mkItem(const char* name, bool dynamic,
                                unsigned value) {
    GUIParameterTableItemInterface* i = new GUIParameterTableItem<unsigned>(myTable, myCurrentPos++, name, dynamic, value);
    myItems.push_back(i);
}


#ifndef HAVE_SUBSECOND_TIMESTEPS
void
GUIParameterTableWindow::mkItem(const char* name, bool dynamic,
                                SUMOTime value) {
    GUIParameterTableItemInterface* i = new GUIParameterTableItem<SUMOTime>(myTable, myCurrentPos++, name, dynamic, value);
    myItems.push_back(i);
}
#endif


void
GUIParameterTableWindow::updateTable() {
    AbstractMutex::ScopedLocker locker(myLock);
    if (myObject == 0) {
        return;
    }
    for (std::vector<GUIParameterTableItemInterface*>::iterator i = myItems.begin(); i != myItems.end(); i++) {
        (*i)->update();
    }
}


void
GUIParameterTableWindow::closeBuilding() {
    myApplication->addChild(this, true);
    create();
    show();
}



/****************************************************************************/

