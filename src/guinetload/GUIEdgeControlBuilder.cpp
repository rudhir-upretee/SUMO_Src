/****************************************************************************/
/// @file    GUIEdgeControlBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id: GUIEdgeControlBuilder.cpp 12980 2012-11-14 22:47:35Z behrisch $
///
// Derivation of NLEdgeControlBuilder which build gui-edges
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

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <guisim/GUIEdge.h>
#include <guisim/GUINet.h>
#include <guisim/GUILane.h>
#include <guisim/GUIInternalLane.h>
#include <microsim/MSJunction.h>
#include <netload/NLBuilder.h>
#include "GUIEdgeControlBuilder.h"
#include <gui/GUIGlobals.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
GUIEdgeControlBuilder::GUIEdgeControlBuilder()
    : NLEdgeControlBuilder() {}


GUIEdgeControlBuilder::~GUIEdgeControlBuilder() {}


MSEdge*
GUIEdgeControlBuilder::closeEdge() {
    MSEdge* ret = NLEdgeControlBuilder::closeEdge();
    static_cast<GUIEdge*>(ret)->initGeometry();
    return ret;
}


MSLane*
GUIEdgeControlBuilder::addLane(const std::string& id,
                               SUMOReal maxSpeed, SUMOReal length,
                               const PositionVector& shape,
                               SUMOReal width,
                               SVCPermissions permissions) {
    MSLane* lane = 0;
    switch (myActiveEdge->getPurpose()) {
        case MSEdge::EDGEFUNCTION_INTERNAL:
            lane = new GUIInternalLane(id, maxSpeed, length, myActiveEdge,
                                       myCurrentNumericalLaneID++, shape, width, permissions);
            break;
        case MSEdge::EDGEFUNCTION_NORMAL:
        case MSEdge::EDGEFUNCTION_CONNECTOR:
            lane = new GUILane(id, maxSpeed, length, myActiveEdge,
                               myCurrentNumericalLaneID++, shape, width, permissions);
            break;
        default:
            throw InvalidArgument("A lane with an unknown type occured (" + toString(myActiveEdge->getPurpose()) + ")");
    }
    myLaneStorage->push_back(lane);
    return lane;
}



MSEdge*
GUIEdgeControlBuilder::buildEdge(const std::string& id, const MSEdge::EdgeBasicFunction function, const std::string& streetName) {
    if (function == MSEdge::EDGEFUNCTION_INTERNAL) {
        return new GUIEdge(id, -1, function, streetName);
    }
    return new GUIEdge(id, myCurrentNumericalEdgeID++, function, streetName);
}

/****************************************************************************/

