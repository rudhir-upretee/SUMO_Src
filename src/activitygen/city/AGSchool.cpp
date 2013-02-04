/****************************************************************************/
/// @file    AGSchool.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
/// @version $Id: AGSchool.cpp 11671 2012-01-07 20:14:30Z behrisch $
///
// Correspond to given ages and referenced by children. Has a precise location.
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
#include <string>
#include "AGSchool.h"
#include "AGPosition.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
AGSchool::print() {
    std::cout << "- school: " << " placeNbr=" << capacity << " hours=[" << opening << ";" << closing << "] ages=[" << beginAge << ";" << endAge << "]" << std::endl;
}

int
AGSchool::getPlaces() {
    return capacity;
}

bool
AGSchool::addNewChild() {
    if (capacity > 0) {
        --capacity;
        return true;
    }
    return false;
}

bool
AGSchool::removeChild() {
    if (capacity < initCapacity) {
        ++capacity;
        return true;
    }
    return false;
}

bool
AGSchool::acceptThisAge(int age) {
    if (age <= endAge && age >= beginAge) {
        return true;
    }
    return false;
}

int
AGSchool::getBeginAge() {
    return beginAge;
}

int
AGSchool::getEndAge() {
    return endAge;
}

AGPosition
AGSchool::getPosition() {
    return location;
}

int
AGSchool::getClosingHour() {
    return closing;
}

int
AGSchool::getOpeningHour() {
    return opening;
}

/****************************************************************************/
