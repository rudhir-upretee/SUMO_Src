/****************************************************************************/
/// @file    MSVTypeProbe.cpp
/// @author  Tino Morenz
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 24.10.2007
/// @version $Id: MSVTypeProbe.cpp 11913 2012-02-16 15:05:59Z namdre $
///
// Writes positions of vehicles that have a certain (named) type
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
#include <utils/common/WrappingCommand.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/geom/GeoConvHelper.h>

#include "MSVTypeProbe.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
MSVTypeProbe::MSVTypeProbe(const std::string& id,
                           const std::string& vType,
                           OutputDevice& od, SUMOTime frequency)
    : Named(id), myVType(vType), myOutputDevice(od), myFrequency(frequency) {
    MSNet::getInstance()->getEndOfTimestepEvents().addEvent(this, 0, MSEventControl::ADAPT_AFTER_EXECUTION);
    myOutputDevice.writeXMLHeader("vehicle-type-probes");
}


MSVTypeProbe::~MSVTypeProbe() {
}


SUMOTime
MSVTypeProbe::execute(SUMOTime currentTime) {
    const std::string indent("    ");
    myOutputDevice << indent << "<timestep time=\"" << time2string(currentTime) << "\" id=\"" << getID() << "\" vType=\"" << myVType << "\">" << "\n";
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt it = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();
    for (; it != end; ++it) {
        const MSVehicle* veh = static_cast<const MSVehicle*>((*it).second);
        if (myVType == "" || myVType == veh->getVehicleType().getID()) {
            if (!veh->isOnRoad()) {
                continue;
            }
            Position pos = veh->getLane()->getShape().positionAtLengthPosition(veh->getPositionOnLane());
            myOutputDevice << indent << indent
                           << "<vehicle id=\"" << veh->getID()
                           << "\" lane=\"" << veh->getLane()->getID()
                           << "\" pos=\"" << veh->getPositionOnLane()
                           << "\" x=\"" << pos.x()
                           << "\" y=\"" << pos.y();
            if (GeoConvHelper::getFinal().usingGeoProjection()) {
                GeoConvHelper::getFinal().cartesian2geo(pos);
                myOutputDevice.setPrecision(GEO_OUTPUT_ACCURACY);
                myOutputDevice << "\" lat=\"" << pos.y() << "\" lon=\"" << pos.x();
                myOutputDevice.setPrecision();
            }
            myOutputDevice << "\" speed=\"" << veh->getSpeed() << "\"/>" << "\n";
        }

    }
    myOutputDevice << indent << "</timestep>" << "\n";
    return myFrequency;
}


/****************************************************************************/
