/****************************************************************************/
/// @file    ODDistrict.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id: ODDistrict.cpp 11671 2012-01-07 20:14:30Z behrisch $
///
// A district (origin/destination)
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
#include <string>
#include <utility>
#include <utils/common/UtilExceptions.h>
#include <utils/common/Named.h>
#include <utils/common/MsgHandler.h>
#include "ODDistrict.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
ODDistrict::ODDistrict(const std::string& id)
    : Named(id) {}


ODDistrict::~ODDistrict() {}


void
ODDistrict::addSource(const std::string& id, SUMOReal weight) {
    mySources.add(weight, id);
}


void
ODDistrict::addSink(const std::string& id, SUMOReal weight) {
    mySinks.add(weight, id);
}


std::string
ODDistrict::getRandomSource() const throw(OutOfBoundsException) {
    return mySources.get();
}


std::string
ODDistrict::getRandomSink() const throw(OutOfBoundsException) {
    return mySinks.get();
}


unsigned int
ODDistrict::sinkNumber() const {
    return (unsigned int) mySinks.getVals().size();
}


unsigned int
ODDistrict::sourceNumber() const {
    return (unsigned int) mySources.getVals().size();
}


/****************************************************************************/

