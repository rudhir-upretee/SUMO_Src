/****************************************************************************/
/// @file    NIVissimSingleTypeParser_Routenentscheidungsdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
/// @version $Id: NIVissimSingleTypeParser_Routenentscheidungsdefinition.cpp 11671 2012-01-07 20:14:30Z behrisch $
///
//
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
#include <utils/common/TplConvert.h>
#include "../NIImporter_Vissim.h"
#include "NIVissimSingleTypeParser_Routenentscheidungsdefinition.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Routenentscheidungsdefinition::NIVissimSingleTypeParser_Routenentscheidungsdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Routenentscheidungsdefinition::~NIVissimSingleTypeParser_Routenentscheidungsdefinition() {}


bool
NIVissimSingleTypeParser_Routenentscheidungsdefinition::parse(std::istream& from) {
    std::string tag;
    while (tag != "fahrzeugklassen") {
        tag = myRead(from);
    }
    do {
        while (tag != "DATAEND" || tag == "route") {
            if (tag == "route") {
                while (tag != "strecke") {
                    tag = myRead(from);
                }
                tag = readEndSecure(from);
            } else {
                tag = readEndSecure(from);
            }
        }
        if (tag != "DATAEND") {
            tag = readEndSecure(from);
        }
    } while (tag != "DATAEND");
    return true;
}



/****************************************************************************/

