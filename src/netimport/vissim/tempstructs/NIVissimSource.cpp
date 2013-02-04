/****************************************************************************/
/// @file    NIVissimSource.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id: NIVissimSource.cpp 11671 2012-01-07 20:14:30Z behrisch $
///
// -------------------
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
#include <map>
#include "NIVissimSource.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS

NIVissimSource::DictType NIVissimSource::myDict;

NIVissimSource::NIVissimSource(const std::string& id, const std::string& name,
                               const std::string& edgeid, SUMOReal q,
                               bool exact, int vehicle_combination,
                               SUMOReal beg, SUMOReal end)
    : myID(id), myName(name), myEdgeID(edgeid), myQ(q), myExact(exact),
      myVehicleCombination(vehicle_combination),
      myTimeBeg(beg), myTimeEnd(end) {}


NIVissimSource::~NIVissimSource() {}


bool
NIVissimSource::dictionary(const std::string& id, const std::string& name,
                           const std::string& edgeid, SUMOReal q, bool exact,
                           int vehicle_combination, SUMOReal beg, SUMOReal end) {
    NIVissimSource* o = new NIVissimSource(id, name, edgeid, q, exact,
                                           vehicle_combination, beg, end);
    if (!dictionary(id, o)) {
        delete o;
        return false;
    }
    return true;
}


bool
NIVissimSource::dictionary(const std::string& id, NIVissimSource* o) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        myDict[id] = o;
        return true;
    }
    return false;
}


NIVissimSource*
NIVissimSource::dictionary(const std::string& id) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        return 0;
    }
    return (*i).second;
}


void
NIVissimSource::clearDict() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        delete(*i).second;
    }
    myDict.clear();
}



/****************************************************************************/

