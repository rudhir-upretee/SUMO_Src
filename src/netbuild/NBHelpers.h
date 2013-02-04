/****************************************************************************/
/// @file    NBHelpers.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
/// @version $Id: NBHelpers.h 11999 2012-03-02 12:09:44Z dkrajzew $
///
// Some mathematical helper methods
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
#ifndef NBHelpers_h
#define NBHelpers_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class Position;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBHelpers
 * Some mathmatical methods for the computation of angles
 */
class NBHelpers {
public:
    /** computes the angle of the straight which is described by the two
        coordinates */
    static SUMOReal angle(SUMOReal x1, SUMOReal y1, SUMOReal x2, SUMOReal y2);

    /** computes the relative angle between the two angles */
    static SUMOReal relAngle(SUMOReal angle1, SUMOReal angle2);

    /** normalises angle <-170 and >170 to 180 after the computation with
        "relAngle" */
    static SUMOReal normRelAngle(SUMOReal angle1, SUMOReal angle2);

    /** converts the numerical id to its "normal" string representation */
    static std::string normalIDRepresentation(const std::string& id);

    /** returns the distance between both nodes */
    static SUMOReal distance(NBNode* node1, NBNode* node2);

};


#endif

/****************************************************************************/

