/****************************************************************************/
/// @file    MSXMLRawOut.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Bjoern Hendriks
/// @author  Michael Behrisch
/// @date    Mon, 10.05.2004
/// @version $Id: MSXMLRawOut.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// Realises dumping the complete network state
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

#include <microsim/MSEdgeControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSGlobals.h>
#include <utils/iodevices/OutputDevice.h>
#include "MSXMLRawOut.h"

#ifdef HAVE_INTERNAL
#include <mesosim/MELoop.h>
#include <mesosim/MESegment.h>
#endif

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
void
MSXMLRawOut::write(OutputDevice& of, const MSEdgeControl& ec,
                   SUMOTime timestep) {
    of.openTag("timestep") << " time=\"" << time2string(timestep) << "\">\n";
    const std::vector<MSEdge*>& edges = ec.getEdges();
    for (std::vector<MSEdge*>::const_iterator e = edges.begin(); e != edges.end(); ++e) {
        writeEdge(of, **e);
    }
    of.closeTag();
}


void
MSXMLRawOut::writeEdge(OutputDevice& of, const MSEdge& edge) {
    //en
    bool dump = !MSGlobals::gOmitEmptyEdgesOnDump;
    if (!dump) {
#ifdef HAVE_INTERNAL
        if (MSGlobals::gUseMesoSim) {
            MESegment* seg = MSGlobals::gMesoNet->getSegmentForEdge(edge);
            while (seg != 0) {
                if (seg->getCarNumber() != 0) {
                    dump = true;
                    break;
                }
                seg = seg->getNextSegment();
            }
        } else {
#endif
            const std::vector<MSLane*>& lanes = edge.getLanes();
            for (std::vector<MSLane*>::const_iterator lane = lanes.begin(); lane != lanes.end(); ++lane) {
                if (((**lane).getVehicleNumber() != 0)) {
                    dump = true;
                    break;
                }
            }
#ifdef HAVE_INTERNAL
        }
#endif
    }
    //en
    if (dump) {
        of.openTag("edge") << " id=\"" << edge.getID() << "\">\n";
#ifdef HAVE_INTERNAL
        if (MSGlobals::gUseMesoSim) {
            MESegment* seg = MSGlobals::gMesoNet->getSegmentForEdge(edge);
            while (seg != 0) {
                seg->writeVehicles(of);
                seg = seg->getNextSegment();
            }
        } else {
#endif
            const std::vector<MSLane*>& lanes = edge.getLanes();
            for (std::vector<MSLane*>::const_iterator lane = lanes.begin(); lane != lanes.end(); ++lane) {
                writeLane(of, **lane);
            }
#ifdef HAVE_INTERNAL
        }
#endif
        of.closeTag();
    }
}


void
MSXMLRawOut::writeLane(OutputDevice& of, const MSLane& lane) {
    of.openTag("lane") << " id=\"" << lane.myID << "\"";
    if (lane.getVehicleNumber() != 0) {
        of << ">\n";
        for (std::vector<MSVehicle*>::const_iterator veh = lane.myVehBuffer.begin();
                veh != lane.myVehBuffer.end(); ++veh) {
            writeVehicle(of, **veh);
        }
        for (MSLane::VehCont::const_iterator veh = lane.myVehicles.begin();
                veh != lane.myVehicles.end(); ++veh) {
            writeVehicle(of, **veh);
        }
    }
    of.closeTag(lane.getVehicleNumber() == 0);
}


void
MSXMLRawOut::writeVehicle(OutputDevice& of, const MSBaseVehicle& veh) {
    if (veh.isOnRoad()) {
        of.openTag("vehicle") << " id=\"" << veh.getID() << "\" pos=\""
                              << veh.getPositionOnLane() << "\" speed=\"" << veh.getSpeed() << "\"";
        of.closeTag(true);
    }
}



/****************************************************************************/

