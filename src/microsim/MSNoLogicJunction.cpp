/****************************************************************************/
/// @file    MSNoLogicJunction.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Thu, 06 Jun 2002
/// @version $Id: MSNoLogicJunction.cpp 11671 2012-01-07 20:14:30Z behrisch $
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

#include "MSNoLogicJunction.h"
#include "MSLane.h"
#include "MSInternalLane.h"
#include <algorithm>
#include <cassert>
#include <cmath>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// static member definitions
// ===========================================================================
std::bitset<64> MSNoLogicJunction::myDump((unsigned long long) 0xffffffff);



// ===========================================================================
// method definitions
// ===========================================================================
MSNoLogicJunction::MSNoLogicJunction(const std::string& id,
                                     const Position& position,
                                     const PositionVector& shape,
                                     std::vector<MSLane*> incoming
#ifdef HAVE_INTERNAL_LANES
                                     , std::vector<MSLane*> internal
#endif
                                    )
    : MSJunction(id, position, shape),
      myIncomingLanes(incoming)
#ifdef HAVE_INTERNAL_LANES
      , myInternalLanes(internal)
#endif
{
}


MSNoLogicJunction::~MSNoLogicJunction() {}


void
MSNoLogicJunction::postloadInit() {
    std::vector<MSLane*>::iterator i;
    // inform links where they have to report approaching vehicles to
    for (i = myIncomingLanes.begin(); i != myIncomingLanes.end(); ++i) {
        const MSLinkCont& links = (*i)->getLinkCont();
        for (MSLinkCont::const_iterator j = links.begin(); j != links.end(); j++) {
            (*j)->setRequestInformation(0, 0, false, false, std::vector<MSLink*>(), std::vector<MSLane*>());
        }
    }
#ifdef HAVE_INTERNAL_LANES
    // set information for the internal lanes
    for (i = myInternalLanes.begin(); i != myInternalLanes.end(); ++i) {
        // ... set information about participation
        static_cast<MSInternalLane*>(*i)->setParentJunctionInformation(&myDump, 0);
    }
#endif
}



/****************************************************************************/

