/****************************************************************************/
/// @file    MSLinkCont.cpp
/// @author  Daniel Krajzewicz
/// @date    15 Feb 2004
/// @version $Id: MSLinkCont.cpp 12830 2012-10-11 13:44:57Z namdre $
///
// Helpers for link vector
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

#include "MSLinkCont.h"
#include "MSLane.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
#ifdef HAVE_INTERNAL_LANES
const MSEdge*
MSLinkContHelper::getInternalFollowingEdge(MSLane* fromLane,
        const MSEdge* followerAfterInternal) {
    //@todo to be optimized
    const MSLinkCont& lc = fromLane->getLinkCont();
    for (MSLinkCont::const_iterator j = lc.begin(); j != lc.end(); j++) {
        MSLink* link = *j;
        if (&link->getLane()->getEdge() == followerAfterInternal) {
            if (link->getViaLane() != 0) {
                return &link->getViaLane()->getEdge();
            } else {
                return 0; // network without internal links
            }
        }
    }
    return 0;
}
#endif


MSLink*
MSLinkContHelper::getConnectingLink(const MSLane& from, const MSLane& to) {
    const MSLinkCont& lc = from.getLinkCont();
    for (MSLinkCont::const_iterator j = lc.begin(); j != lc.end(); j++) {
        MSLink* link = *j;
        if (link->getLane() == &to) {
            return link;
        }
    }
    return 0;
}



/****************************************************************************/

