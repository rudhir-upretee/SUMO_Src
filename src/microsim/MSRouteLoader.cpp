/****************************************************************************/
/// @file    MSRouteLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 6 Nov 2002
/// @version $Id: MSRouteLoader.cpp 12927 2012-11-04 06:47:13Z behrisch $
///
// A class that performs the loading of routes
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
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/XMLSubSys.h>
#include "MSNet.h"
#include "MSRouteHandler.h"
#include "MSRouteLoader.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
MSRouteLoader::MSRouteLoader(MSNet&,
                             MSRouteHandler* handler)
    : myParser(0), myMoreAvailable(true), myHandler(handler) {
    myParser = XMLSubSys::getSAXReader(*myHandler);
}


MSRouteLoader::~MSRouteLoader() {
    delete myParser;
    delete myHandler;
}


void
MSRouteLoader::init() {
    myMoreAvailable = true;
    if (!myParser->parseFirst(myHandler->getFileName())) {
        throw ProcessError("Can not read XML-file '" + myHandler->getFileName() + "'.");
    }
}


void
MSRouteLoader::loadUntil(SUMOTime time) {
    // read only when further data is available, no error occured
    //  and vehicles may be found in the between the departure time of
    //  the last read vehicle and the time to read until
    if (!myMoreAvailable || time <= myHandler->getLastDepart()) {
        return;
    }

    // read vehicles until specified time or the period to read vehicles
    //  until is reached
    while (myParser->parseNext()) {
        // return when the last read vehicle is beyond the period
        if (time <= myHandler->getLastDepart()) {
            return;
        }
    }

    // no data are available anymore
    myMoreAvailable = false;
    return;
}


bool
MSRouteLoader::moreAvailable() const {
    return myMoreAvailable;
}



/****************************************************************************/

