/****************************************************************************/
/// @file    NIVissimSource.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id: NIVissimSource.h 11671 2012-01-07 20:14:30Z behrisch $
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
#ifndef NIVissimSource_h
#define NIVissimSource_h


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

// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class NIVissimSource {
public:
    NIVissimSource(const std::string& id, const std::string& name,
                   const std::string& edgeid, SUMOReal q, bool exact,
                   int vehicle_combination, SUMOReal beg, SUMOReal end);
    ~NIVissimSource();
    static bool dictionary(const std::string& id, const std::string& name,
                           const std::string& edgeid, SUMOReal q, bool exact,
                           int vehicle_combination, SUMOReal beg, SUMOReal end);
    static bool dictionary(const std::string& id, NIVissimSource* o);
    static NIVissimSource* dictionary(const std::string& id);
    static void clearDict();
private:
    std::string myID;
    std::string myName;
    std::string myEdgeID;
    SUMOReal myQ;
    bool myExact;
    int myVehicleCombination;
    SUMOReal myTimeBeg;
    SUMOReal myTimeEnd;

private:
    typedef std::map<std::string, NIVissimSource*> DictType;
    static DictType myDict;
};


#endif

/****************************************************************************/

