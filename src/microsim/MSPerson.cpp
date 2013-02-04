/****************************************************************************/
/// @file    MSPerson.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Mon, 9 Jul 2001
/// @version $Id: MSPerson.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// The class for modelling person-movements
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
#include <vector>
#include <utils/iodevices/OutputDevice.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/ToString.h>
#include "MSNet.h"
#include "MSEdge.h"
#include "MSLane.h"
#include "MSPerson.h"
#include "MSPersonControl.h"
#include "MSInsertionControl.h"
#include "MSVehicle.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS

/* -------------------------------------------------------------------------
 * static member definitions
 * ----------------------------------------------------------------------- */
const SUMOReal MSPerson::SIDEWALK_OFFSET(3);

// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage::MSPersonStage(const MSEdge& destination, StageType type)
    : myDestination(destination), myDeparted(-1), myArrived(-1), myType(type) {}


MSPerson::MSPersonStage::~MSPersonStage() {}


const MSEdge&
MSPerson::MSPersonStage::getDestination() const {
    return myDestination;
}


void
MSPerson::MSPersonStage::setDeparted(SUMOTime now) {
    if (myDeparted < 0) {
        myDeparted = now;
    }
}


void
MSPerson::MSPersonStage::setArrived(SUMOTime now) {
    myArrived = now;
}


bool
MSPerson::MSPersonStage::isWaitingFor(const std::string& /*line*/) const {
    return false;
}


Position
MSPerson::MSPersonStage::getEdgePosition(const MSEdge* e, SUMOReal at, SUMOReal offset) const {
    // @todo: well, definitely not the nicest way... Should be precomputed
    const MSLane* lane = e->getLanes()[0];
    PositionVector shp = lane->getShape();
    shp.move2side(offset);
    return shp.positionAtLengthPosition(lane->interpolateLanePosToGeometryPos(at));
}


SUMOReal
MSPerson::MSPersonStage::getEdgeAngle(const MSEdge* e, SUMOReal at) const {
    // @todo: well, definitely not the nicest way... Should be precomputed
    PositionVector shp = e->getLanes()[0]->getShape();
    return shp.rotationDegreeAtLengthPosition(at);
}


/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage_Walking - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Walking::MSPersonStage_Walking(const std::vector<const MSEdge*>& route,
        MSBusStop* toBS,
        SUMOTime walkingTime, SUMOReal speed,
        SUMOReal departPos, SUMOReal arrivalPos) :
    MSPersonStage(*route.back(), WALKING), myWalkingTime(walkingTime), myRoute(route),
    myDepartPos(departPos), myArrivalPos(arrivalPos), myDestinationBusStop(toBS),
    mySpeed(speed) {
    myDepartPos = SUMOVehicleParameter::interpretEdgePos(
                      myDepartPos, myRoute.front()->getLength(), SUMO_ATTR_DEPARTPOS, "person walking from " + myRoute.front()->getID());
    myArrivalPos = SUMOVehicleParameter::interpretEdgePos(
                       myArrivalPos, myRoute.back()->getLength(), SUMO_ATTR_ARRIVALPOS, "person walking to " + myRoute.back()->getID());
    if (walkingTime > 0) {
        SUMOReal length = 0;
        for (std::vector<const MSEdge*>::const_iterator i = route.begin(); i != route.end(); ++i) {
            length += (*i)->getLength();
        }
        length -= myDepartPos;
        length -= route.back()->getLength() - myArrivalPos;
        mySpeed = length / STEPS2TIME(walkingTime);
    }
}


MSPerson::MSPersonStage_Walking::~MSPersonStage_Walking() {}


const MSEdge*
MSPerson::MSPersonStage_Walking::getEdge(SUMOTime /* now */) const {
    return *myRouteStep;
}


const MSEdge*
MSPerson::MSPersonStage_Walking::getFromEdge() const {
    return myRoute.front();
}


SUMOReal
MSPerson::MSPersonStage_Walking::getEdgePos(SUMOTime now) const {
    SUMOReal off = STEPS2TIME(now - myLastEntryTime);
    return myCurrentBeginPos + myCurrentLength / myCurrentDuration * off;
}


Position
MSPerson::MSPersonStage_Walking::getPosition(SUMOTime now) const {
    const MSEdge* e = getEdge(now);
    SUMOReal off = STEPS2TIME(now - myLastEntryTime);
    return getEdgePosition(e, myCurrentBeginPos + myCurrentLength / myCurrentDuration * off, SIDEWALK_OFFSET);
}


SUMOReal
MSPerson::MSPersonStage_Walking::getAngle(SUMOTime now) const {
    const MSEdge* e = getEdge(now);
    SUMOReal off = STEPS2TIME(now - myLastEntryTime);
    return getEdgeAngle(e, myCurrentBeginPos + myCurrentLength / myCurrentDuration * off) + 90;
}


bool
MSPerson::MSPersonStage_Walking::checkNoDuration(MSNet* /* net */, MSPerson* /* person */, SUMOTime duration, SUMOTime /* now */) {
    if (duration == 0) {

        return true;
    }
    return false;
}


void
MSPerson::MSPersonStage_Walking::proceed(MSNet* net, MSPerson* person, SUMOTime now,
        MSEdge* previousEdge, const SUMOReal at) {
    previousEdge->removePerson(person);
    myRouteStep = myRoute.begin();
    myLastEntryTime = now;
    if (myWalkingTime == 0) {
        if (!person->proceed(net, now)) {
            MSNet::getInstance()->getPersonControl().erase(person);
        };
        return;
    }
    MSNet::getInstance()->getPersonControl().setWalking(person);
    if (at >= 0) {
        myDepartPos = at;
    }
    ((MSEdge*) *myRouteStep)->addPerson(person);
    myRoute.size() == 1
    ? computeWalkingTime(*myRouteStep, myDepartPos, myArrivalPos, myDestinationBusStop)
    : computeWalkingTime(*myRouteStep, myDepartPos, -1, 0);
    net->getBeginOfTimestepEvents().addEvent(new MoveToNextEdge(person, *this), now + TIME2STEPS(myCurrentDuration), MSEventControl::ADAPT_AFTER_EXECUTION);
}


void
MSPerson::MSPersonStage_Walking::computeWalkingTime(const MSEdge* const e, SUMOReal fromPos, SUMOReal toPos, MSBusStop* bs) {
    if (bs != 0) {
        toPos = bs->getEndLanePosition();
    } else if (toPos < 0) {
        toPos = e->getLanes()[0]->getLength();
    }
    if (fromPos < 0) {
        fromPos = 0;
    }
    myCurrentBeginPos = fromPos;
    myCurrentLength = toPos - fromPos;
    assert(myCurrentLength >= 0);
    myCurrentDuration = MAX2(myCurrentLength, (SUMOReal)1.0) / mySpeed;
}


void
MSPerson::MSPersonStage_Walking::tripInfoOutput(OutputDevice& os) const {
    (os.openTag("walk") <<
     " arrival=\"" << time2string(myArrived) <<
     "\"").closeTag(true);
}


void
MSPerson::MSPersonStage_Walking::beginEventOutput(const MSPerson& p, SUMOTime t, OutputDevice& os) const {
    (os.openTag("event") <<
     " time=\"" << time2string(t) <<
     "\" type=\"departure" <<
     "\" agent=\"" << p.getID() <<
     "\" link=\"" << myRoute.front()->getID() <<
     "\"").closeTag(true);
}


void
MSPerson::MSPersonStage_Walking::endEventOutput(const MSPerson& p, SUMOTime t, OutputDevice& os) const {
    (os.openTag("event") <<
     " time=\"" << time2string(t) <<
     "\" type=\"arrival" <<
     "\" agent=\"" << p.getID() <<
     "\" link=\"" << myRoute.back()->getID() <<
     "\"").closeTag(true);
}


SUMOTime
MSPerson::MSPersonStage_Walking::moveToNextEdge(MSPerson* person, SUMOTime currentTime) {
    ((MSEdge*) *myRouteStep)->removePerson(person);
    if (myRouteStep == myRoute.end() - 1) {
        MSNet::getInstance()->getPersonControl().unsetWalking(person);
        if (myDestinationBusStop != 0) {
            myDestinationBusStop->addPerson(person);
        }
        if (!person->proceed(MSNet::getInstance(), currentTime)) {
            MSNet::getInstance()->getPersonControl().erase(person);
        }
        return 0;
    } else {
        ++myRouteStep;
        myRouteStep == myRoute.end() - 1
        ? computeWalkingTime(*myRouteStep, 0, myArrivalPos, myDestinationBusStop)
        : computeWalkingTime(*myRouteStep, 0, -1, 0);
        ((MSEdge*) *myRouteStep)->addPerson(person);
        myLastEntryTime = currentTime;
        return TIME2STEPS(myCurrentDuration);
    }
}



/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage_Driving - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Driving::MSPersonStage_Driving(const MSEdge& destination,
        MSBusStop* toBS, const std::vector<std::string>& lines)
    : MSPersonStage(destination, DRIVING), myLines(lines.begin(), lines.end()),
      myVehicle(0), myDestinationBusStop(toBS) {}


MSPerson::MSPersonStage_Driving::~MSPersonStage_Driving() {}


const MSEdge*
MSPerson::MSPersonStage_Driving::getEdge(SUMOTime /* now */) const {
    if (myVehicle != 0) {
        return myVehicle->getEdge();
    }
    return myWaitingEdge;
}


const MSEdge*
MSPerson::MSPersonStage_Driving::getFromEdge() const {
    return myWaitingEdge;
}


SUMOReal
MSPerson::MSPersonStage_Driving::getEdgePos(SUMOTime now) const {
    if (myVehicle != 0) {
        // vehicle may already have passed the lane (check whether this is correct)
        return MIN2(myVehicle->getPositionOnLane(), getEdge(now)->getLength());
    }
    return myWaitingPos;
}


Position
MSPerson::MSPersonStage_Driving::getPosition(SUMOTime /* now */) const {
    if (myVehicle != 0) {
        /// @bug this fails while vehicle is driving across a junction
        return myVehicle->getEdge()->getLanes()[0]->getShape().positionAtLengthPosition(myVehicle->getPositionOnLane());
    }
    return getEdgePosition(myWaitingEdge, myWaitingPos, SIDEWALK_OFFSET);
}


SUMOReal
MSPerson::MSPersonStage_Driving::getAngle(SUMOTime /* now */) const {
    if (myVehicle != 0) {
        MSVehicle* veh = dynamic_cast<MSVehicle*>(myVehicle);
        if (veh != 0) {
            return veh->getAngle() + 90;
        } else {
            return 0;
        }
    }
    return getEdgeAngle(myWaitingEdge, myWaitingPos);
}



void
MSPerson::MSPersonStage_Driving::proceed(MSNet* net, MSPerson* person, SUMOTime /* now */,
        MSEdge* previousEdge, const SUMOReal at) {
    myWaitingEdge = previousEdge;
    myWaitingPos = at;
    myVehicle = net->getVehicleControl().getWaitingVehicle(previousEdge, myLines);
    if (myVehicle != 0 && myVehicle->getParameter().departProcedure == DEPART_TRIGGERED) {
        previousEdge->removePerson(person);
        myVehicle->addPerson(person);
        net->getInsertionControl().add(myVehicle);
        net->getVehicleControl().removeWaiting(previousEdge, myVehicle);
        net->getVehicleControl().unregisterOneWaitingForPerson();
    } else {
        net->getPersonControl().addWaiting(previousEdge, person);
        previousEdge->addPerson(person);
    }
}


bool
MSPerson::MSPersonStage_Driving::isWaitingFor(const std::string& line) const {
    return myLines.count(line) > 0;
}


bool
MSPerson::MSPersonStage_Driving::isWaiting4Vehicle() const {
    return myVehicle == 0;
}


std::string
MSPerson::MSPersonStage_Driving::getStageTypeName() const {
    return isWaiting4Vehicle() ? "waiting for " + joinToString(myLines, ",") : "driving";
}


void
MSPerson::MSPersonStage_Driving::tripInfoOutput(OutputDevice& os) const {
    (os.openTag("ride") <<
     " depart=\"" << time2string(myDeparted) <<
     "\" arrival=\"" << time2string(myArrived) <<
     "\"").closeTag(true);
}


void
MSPerson::MSPersonStage_Driving::beginEventOutput(const MSPerson& p, SUMOTime t, OutputDevice& os) const {
    (os.openTag("event") <<
     " time=\"" << time2string(t) <<
     "\" type=\"arrival" <<
     "\" agent=\"" << p.getID() <<
     "\" link=\"" << getEdge(t)->getID() <<
     "\"").closeTag(true);
}


void
MSPerson::MSPersonStage_Driving::endEventOutput(const MSPerson& p, SUMOTime t, OutputDevice& os) const {
    (os.openTag("event") <<
     " time=\"" << time2string(t) <<
     "\" type=\"arrival" <<
     "\" agent=\"" << p.getID() <<
     "\" link=\"" << getEdge(t)->getID() <<
     "\"").closeTag(true);
}


/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage_Waiting - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Waiting::MSPersonStage_Waiting(const MSEdge& destination,
        SUMOTime duration, SUMOTime until, SUMOReal pos, const std::string& actType) :
    MSPersonStage(destination, WAITING),
    myWaitingDuration(duration),
    myWaitingUntil(until),
    myActType(actType),
    myStartPos(pos) {
    myStartPos = SUMOVehicleParameter::interpretEdgePos(
                     myStartPos, myDestination.getLength(), SUMO_ATTR_DEPARTPOS, "person stopping at " + myDestination.getID());
}


MSPerson::MSPersonStage_Waiting::~MSPersonStage_Waiting() {}


const MSEdge*
MSPerson::MSPersonStage_Waiting::getEdge(SUMOTime /* now */) const {
    return &myDestination;
}


const MSEdge*
MSPerson::MSPersonStage_Waiting::getFromEdge() const {
    return &myDestination;
}


SUMOReal
MSPerson::MSPersonStage_Waiting::getEdgePos(SUMOTime /* now */) const {
    return myStartPos;
}

Position
MSPerson::MSPersonStage_Waiting::getPosition(SUMOTime /* now */) const {
    return getEdgePosition(&myDestination, myStartPos, SIDEWALK_OFFSET);
}


SUMOReal
MSPerson::MSPersonStage_Waiting::getAngle(SUMOTime /* now */) const {
    return getEdgeAngle(&myDestination, myStartPos) + 45;
}


void
MSPerson::MSPersonStage_Waiting::proceed(MSNet* net, MSPerson* person, SUMOTime now,
        MSEdge* previousEdge, const SUMOReal /* at */) {
    previousEdge->addPerson(person);
    const SUMOTime until = MAX3(now, now + myWaitingDuration, myWaitingUntil);
    net->getPersonControl().setWaitEnd(until, person);
}


void
MSPerson::MSPersonStage_Waiting::tripInfoOutput(OutputDevice& os) const {
    (os.openTag("stop") <<
     " arrival=\"" << time2string(myArrived) <<
     "\"").closeTag(true);
}


void
MSPerson::MSPersonStage_Waiting::beginEventOutput(const MSPerson& p, SUMOTime t, OutputDevice& os) const {
    (os.openTag("event") <<
     " time=\"" << time2string(t) <<
     "\" type=\"actstart " << myActType <<
     "\" agent=\"" << p.getID() <<
     "\" link=\"" << getEdge(t)->getID() <<
     "\"").closeTag(true);
}


void
MSPerson::MSPersonStage_Waiting::endEventOutput(const MSPerson& p, SUMOTime t, OutputDevice& os) const {
    (os.openTag("event") <<
     " time=\"" << time2string(t) <<
     "\" type=\"actend " << myActType <<
     "\" agent=\"" << p.getID() <<
     "\" link=\"" << getEdge(t)->getID() <<
     "\"").closeTag(true);
}

/* -------------------------------------------------------------------------
 * MSPerson - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPerson(const SUMOVehicleParameter* pars, const MSVehicleType* vtype, MSPersonPlan* plan)
    : myParameter(pars), myVType(vtype), myPlan(plan) {
    myStep = myPlan->begin();
}


MSPerson::~MSPerson() {
    for (MSPersonPlan::const_iterator i = myPlan->begin(); i != myPlan->end(); ++i) {
        delete *i;
    }
    delete myPlan;
    delete myParameter;
}


const std::string&
MSPerson::getID() const {
    return myParameter->id;
}


bool
MSPerson::proceed(MSNet* net, SUMOTime time) {
    MSEdge* arrivedAt = (MSEdge*)(*myStep)->getEdge(time);
    SUMOReal atPos = (*myStep)->getEdgePos(time);
    //MSPersonPlan::iterator prior = myStep;
    (*myStep)->setArrived(time);
    /*
    if(myWriteEvents) {
        (*myStep)->endEventOutput(*this, time, OutputDevice::getDeviceByOption("person-event-output"));
    }
    */
    Position pos = (*myStep)->getPosition(time);
    myStep++;
    if (myStep != myPlan->end()) {
        (*myStep)->proceed(net, this, time, arrivedAt, atPos);
        /*
        if(myWriteEvents) {
            (*myStep)->beginEventOutput(*this, time, OutputDevice::getDeviceByOption("person-event-output"));
        }
        */
        return true;
    } else {
        arrivedAt->removePerson(this);
        return false;
    }
}


SUMOTime
MSPerson::getDesiredDepart() const {
    return myParameter->depart;
}


void
MSPerson::setDeparted(SUMOTime now) {
    (*myStep)->setDeparted(now);
}


void
MSPerson::tripInfoOutput(OutputDevice& os) const {
    for (MSPersonPlan::const_iterator i = myPlan->begin(); i != myPlan->end(); ++i) {
        (*i)->tripInfoOutput(os);
    }
}


/****************************************************************************/

