/****************************************************************************/
/// @file    AGActivityTripWriter.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
/// @version $Id: AGActivityTripWriter.h 11671 2012-01-07 20:14:30Z behrisch $
///
// Class for writing Trip objects in a SUMO-route file.
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
#ifndef AGACTIVITYTRIPWRITER_H
#define AGACTIVITYTRIPWRITER_H


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "activities/AGTrip.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>


// ===========================================================================
// class definitions
// ===========================================================================
class AGActivityTripWriter {
public:
    AGActivityTripWriter(std::string file) :
        fileName(file),
        routes(file.c_str()) {
        initialize();
    }

    void initialize();
    void addTrip(AGTrip trip);
    void writeOutputFile();

private:
    std::string fileName;
    std::ofstream routes;
    std::map<std::string, std::string> colors;

    void vtypes();
};


#endif

/****************************************************************************/
