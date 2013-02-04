/****************************************************************************/
/// @file    MSVehicleType.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Thimor Bohn
/// @author  Michael Behrisch
/// @date    Tue, 06 Mar 2001
/// @version $Id: MSVehicleType.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// The car-following model and parameter
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

#include <cassert>
#include <utils/iodevices/BinaryInputDevice.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOVTypeParameter.h>
#include "MSNet.h"
#include "cfmodels/MSCFModel_IDM.h"
#include "cfmodels/MSCFModel_Kerner.h"
#include "cfmodels/MSCFModel_Krauss.h"
#include "cfmodels/MSCFModel_KraussOrig1.h"
#include "cfmodels/MSCFModel_SmartSK.h"
#include "cfmodels/MSCFModel_Daniel1.h"
#include "cfmodels/MSCFModel_PWag2009.h"
#include "cfmodels/MSCFModel_Wiedemann.h"
#include "MSVehicleType.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
MSVehicleType::MSVehicleType(const SUMOVTypeParameter& parameter)
    : myParameter(parameter), myOriginalType(0) {
    assert(getLength() > 0);
    assert(getMaxSpeed() > 0);
}


MSVehicleType::~MSVehicleType() {
    delete myCarFollowModel;
}


SUMOReal
MSVehicleType::computeChosenSpeedDeviation(MTRand& rng) const {
    // for speedDev = 0.1, most 95% of the vehicles will drive between 80% and 120% of speedLimit * speedFactor
    const SUMOReal devA = MIN2(SUMOReal(2.), RandHelper::randNorm(0, 1., rng));
    // avoid voluntary speeds below 20% of the requested speedFactor
    return MAX2(0.2 * myParameter.speedFactor,
                (devA * myParameter.speedDev + 1.) * myParameter.speedFactor);
}


void
MSVehicleType::saveState(std::ostream& os) {
    FileHelpers::writeString(os, myParameter.id);
    FileHelpers::writeFloat(os, myParameter.length);
    FileHelpers::writeFloat(os, myParameter.minGap);
    FileHelpers::writeFloat(os, getMaxSpeed());
    FileHelpers::writeInt(os, (int) myParameter.vehicleClass);
    FileHelpers::writeInt(os, (int) myParameter.emissionClass);
    FileHelpers::writeInt(os, (int) myParameter.shape);
    FileHelpers::writeFloat(os, myParameter.width);
    FileHelpers::writeFloat(os, myParameter.defaultProbability);
    FileHelpers::writeFloat(os, myParameter.speedFactor);
    FileHelpers::writeFloat(os, myParameter.speedDev);
    FileHelpers::writeFloat(os, myParameter.color.red());
    FileHelpers::writeFloat(os, myParameter.color.green());
    FileHelpers::writeFloat(os, myParameter.color.blue());
    FileHelpers::writeInt(os, myCarFollowModel->getModelID());
    FileHelpers::writeString(os, myParameter.lcModel);
    //myCarFollowModel->saveState(os);
}


// ------------ Setter methods
void
MSVehicleType::setLength(const SUMOReal& length) {
    assert(myOriginalType != 0);
    if (length < 0) {
        myParameter.length = myOriginalType->getLength();
    } else {
        myParameter.length = length;
    }
}


void
MSVehicleType::setMinGap(const SUMOReal& minGap) {
    assert(myOriginalType != 0);
    if (minGap < 0) {
        myParameter.minGap = myOriginalType->getMinGap();
    } else {
        myParameter.minGap = minGap;
    }
}


void
MSVehicleType::setMaxSpeed(const SUMOReal& maxSpeed) {
    assert(myOriginalType != 0);
    if (maxSpeed < 0) {
        myParameter.maxSpeed = myOriginalType->getMaxSpeed();
    } else {
        myParameter.maxSpeed = maxSpeed;
    }
}


void
MSVehicleType::setVClass(SUMOVehicleClass vclass) {
    myParameter.vehicleClass = vclass;
}


void
MSVehicleType::setDefaultProbability(const SUMOReal& prob) {
    assert(myOriginalType != 0);
    if (prob < 0) {
        myParameter.defaultProbability = myOriginalType->getDefaultProbability();
    } else {
        myParameter.defaultProbability = prob;
    }
}


void
MSVehicleType::setSpeedFactor(const SUMOReal& factor) {
    assert(myOriginalType != 0);
    if (factor < 0) {
        myParameter.speedFactor = myOriginalType->getSpeedFactor();
    } else {
        myParameter.speedFactor = factor;
    }
}


void
MSVehicleType::setSpeedDeviation(const SUMOReal& dev) {
    assert(myOriginalType != 0);
    if (dev < 0) {
        myParameter.speedDev = myOriginalType->getSpeedDeviation();
    } else {
        myParameter.speedDev = dev;
    }
}


void
MSVehicleType::setEmissionClass(SUMOEmissionClass eclass) {
    myParameter.emissionClass = eclass;
}


void
MSVehicleType::setColor(const RGBColor& color) {
    myParameter.color = color;
}


void
MSVehicleType::setWidth(const SUMOReal& width) {
    assert(myOriginalType != 0);
    if (width < 0) {
        myParameter.width = myOriginalType->getWidth();
    } else {
        myParameter.width = width;
    }
}


void
MSVehicleType::setShape(SUMOVehicleShape shape) {
    myParameter.shape = shape;
}



// ------------ Static methods for building vehicle types
MSVehicleType*
MSVehicleType::build(SUMOVTypeParameter& from) {
    MSVehicleType* vtype = new MSVehicleType(from);
    MSCFModel* model = 0;
    switch (from.cfModel) {
        case SUMO_TAG_CF_IDM:
            model = new MSCFModel_IDM(vtype,
                                      from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                      from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                      from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                      from.get(SUMO_ATTR_CF_IDM_DELTA, 4.),
                                      from.get(SUMO_ATTR_CF_IDM_STEPPING, .25));
            break;
        case SUMO_TAG_CF_IDMM:
            model = new MSCFModel_IDM(vtype,
                                      from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                      from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                      from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                      from.get(SUMO_ATTR_CF_IDMM_ADAPT_FACTOR, 1.8),
                                      from.get(SUMO_ATTR_CF_IDMM_ADAPT_TIME, 600.),
                                      from.get(SUMO_ATTR_CF_IDM_STEPPING, .25));
            break;
        case SUMO_TAG_CF_BKERNER:
            model = new MSCFModel_Kerner(vtype,
                                         from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                         from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                         from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                         from.get(SUMO_ATTR_K, .5),
                                         from.get(SUMO_ATTR_CF_KERNER_PHI, 5.));
            break;
        case SUMO_TAG_CF_KRAUSS_ORIG1:
            model = new MSCFModel_KraussOrig1(vtype,
                                              from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                              from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                              from.get(SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA),
                                              from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU));
            break;
        case SUMO_TAG_CF_SMART_SK:
            model = new MSCFModel_SmartSK(vtype,
                                          from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                          from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                          from.get(SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA),
                                          from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                          from.get(SUMO_ATTR_TMP1, DEFAULT_VEH_TMP1),
                                          from.get(SUMO_ATTR_TMP2, DEFAULT_VEH_TMP2),
                                          from.get(SUMO_ATTR_TMP3, DEFAULT_VEH_TMP3),
                                          from.get(SUMO_ATTR_TMP4, DEFAULT_VEH_TMP4),
                                          from.get(SUMO_ATTR_TMP5, DEFAULT_VEH_TMP5));
            break;
        case SUMO_TAG_CF_DANIEL1:
            model = new MSCFModel_Daniel1(vtype,
                                          from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                          from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                          from.get(SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA),
                                          from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                          from.get(SUMO_ATTR_TMP1, DEFAULT_VEH_TMP1),
                                          from.get(SUMO_ATTR_TMP2, DEFAULT_VEH_TMP2),
                                          from.get(SUMO_ATTR_TMP3, DEFAULT_VEH_TMP3),
                                          from.get(SUMO_ATTR_TMP4, DEFAULT_VEH_TMP4),
                                          from.get(SUMO_ATTR_TMP5, DEFAULT_VEH_TMP5));
            break;
        case SUMO_TAG_CF_PWAGNER2009:
            model = new MSCFModel_PWag2009(vtype,
                                           from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                           from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                           from.get(SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA),
                                           from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU),
                                           from.get(SUMO_ATTR_CF_PWAGNER2009_TAULAST, 0.3),
                                           from.get(SUMO_ATTR_CF_PWAGNER2009_APPROB, 0.5));
            break;
        case SUMO_TAG_CF_WIEDEMANN:
            model = new MSCFModel_Wiedemann(vtype,
                                            from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                            from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                            from.get(SUMO_ATTR_CF_WIEDEMANN_SECURITY, 0.5),
                                            from.get(SUMO_ATTR_CF_WIEDEMANN_ESTIMATION, 0.5));
            break;
        case SUMO_TAG_CF_KRAUSS:
        default:
            model = new MSCFModel_Krauss(vtype,
                                         from.get(SUMO_ATTR_ACCEL, DEFAULT_VEH_ACCEL),
                                         from.get(SUMO_ATTR_DECEL, DEFAULT_VEH_DECEL),
                                         from.get(SUMO_ATTR_SIGMA, DEFAULT_VEH_SIGMA),
                                         from.get(SUMO_ATTR_TAU, DEFAULT_VEH_TAU));
            break;
    }
    vtype->myCarFollowModel = model;
    return vtype;
}


MSVehicleType*
MSVehicleType::build(const std::string& id, const MSVehicleType* from) {
    MSVehicleType* vtype = new MSVehicleType(from->myParameter);
    vtype->myParameter.id = id;
    vtype->myCarFollowModel = from->myCarFollowModel->duplicate(vtype);
    vtype->myOriginalType = from->myOriginalType != 0 ? from->myOriginalType : from;
    return vtype;
}


/****************************************************************************/

