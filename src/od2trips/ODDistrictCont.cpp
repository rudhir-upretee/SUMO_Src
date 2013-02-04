/****************************************************************************/
/// @file    ODDistrictCont.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id: ODDistrictCont.cpp 11671 2012-01-07 20:14:30Z behrisch $
///
// A container for districts
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
#include "ODDistrict.h"
#include "ODDistrictCont.h"
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/NamedObjectCont.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
ODDistrictCont::ODDistrictCont() {}


ODDistrictCont::~ODDistrictCont() {}


std::string
ODDistrictCont::getRandomSourceFromDistrict(const std::string& name) const throw(OutOfBoundsException, InvalidArgument) {
    ODDistrict* district = get(name);
    if (district == 0) {
        throw InvalidArgument("There is no district '" + name + "'.");
    }
    return district->getRandomSource();
}


std::string
ODDistrictCont::getRandomSinkFromDistrict(const std::string& name) const throw(OutOfBoundsException, InvalidArgument) {
    ODDistrict* district = get(name);
    if (district == 0) {
        throw InvalidArgument("There is no district '" + name + "'.");
    }
    return district->getRandomSink();
}



/****************************************************************************/

