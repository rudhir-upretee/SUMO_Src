/****************************************************************************/
/// @file    NBHelpers.cpp
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
/// @version $Id: NBHelpers.cpp 11999 2012-03-02 12:09:44Z dkrajzew $
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <cmath>
#include <string>
#include <sstream>
#include "NBNode.h"
#include "NBHelpers.h"
#include <utils/common/StringTokenizer.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <iostream>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
SUMOReal
NBHelpers::angle(SUMOReal x1, SUMOReal y1, SUMOReal x2, SUMOReal y2) {
    SUMOReal angle = (SUMOReal) atan2(x1 - x2, y1 - y2) * (SUMOReal) 180.0 / (SUMOReal) PI;
    if (angle < 0) {
        angle = 360 + angle;
    }
    return angle;
}


SUMOReal
NBHelpers::relAngle(SUMOReal angle1, SUMOReal angle2) {
    angle2 -= angle1;
    if (angle2 > 180) {
        angle2 = (360 - angle2) * -1;
    }
    while (angle2 < -180) {
        angle2 = 360 + angle2;
    }
    return angle2;
}


SUMOReal
NBHelpers::normRelAngle(SUMOReal angle1, SUMOReal angle2) {
    SUMOReal rel = relAngle(angle1, angle2);
    if (rel < -170 || rel > 170) {
        rel = -180;
    }
    return rel;
}


std::string
NBHelpers::normalIDRepresentation(const std::string& id) {
    std::stringstream strm1(id);
    long numid;
    strm1 >> numid;
    std::stringstream strm2;
    strm2 << numid;
    return strm2.str();
}


SUMOReal
NBHelpers::distance(NBNode* node1, NBNode* node2) {
    return node1->getPosition().distanceTo(node2->getPosition());
}



/****************************************************************************/
