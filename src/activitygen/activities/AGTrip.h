/****************************************************************************/
/// @file    AGTrip.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
/// @version $Id: AGTrip.h 12468 2012-07-03 19:15:42Z behrisch $
///
// Class containing all information of a given trip (car, bus)
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
#ifndef AGTRIP_H
#define AGTRIP_H


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <list>
#include "../city/AGPosition.h"
#include "../city/AGCar.h"
#include "../city/AGBus.h"


// ===========================================================================
// class definitions
// ===========================================================================
class AGTrip {
public:
    AGTrip(AGPosition from, AGPosition to, int at) : //vehicle not specified
        myFrom(from),
        myTo(to),
        myDepTime(at),
        myType("default"),
        myDay(0) {};
    AGTrip(AGPosition from, AGPosition to, AGCar c, int at) :
        myFrom(from),
        myTo(to),
        myDepTime(at),
        myVehicle(c.getName()),
        myType("default"),
        myDay(0) {};
    AGTrip(AGPosition from, AGPosition to, AGBus b, int at) :
        myFrom(from),
        myTo(to),
        myDepTime(at),
        myVehicle(b.getName()),
        myType("bus"),
        myDay(0) {};
    AGTrip(AGPosition from, AGPosition to, std::string v, int at) :
        myFrom(from),
        myTo(to),
        myDepTime(at),
        myVehicle(v),
        myType("default"),
        myDay(0) {};
    AGTrip(AGPosition from, AGPosition to, std::string v, int at, int day) :
        myFrom(from),
        myTo(to),
        myDepTime(at),
        myVehicle(v),
        myType("default"),
        myDay(day) {};
    void print();
    bool operator<(AGTrip& trip);

    void addLayOver(AGPosition by);
    void addLayOver(AGTrip& trip);
    void addLayOverWithoutDestination(AGTrip& trip);

    AGPosition getDep();
    AGPosition getArr();
    int getTime();
    void setDepTime(int time);
    std::string getVehicleName();
    void setVehicleName(std::string name);
    void setArr(AGPosition arrival);
    void setDep(AGPosition departure);
    int getDay();
    void setDay(int day);
    std::string getType();
    void setType(std::string type);
    std::list<AGPosition>* getPassed();

    /**
     * returns the time regarding the departure time
     * going through the different points and coming back to the initial position
     * given the time to make one kilometer
     */
    int getRideBackArrTime(SUMOReal secPerKm);
    /**
     * returns the estimated arrival time
     * given the time to make one kilometer
     */
    int getArrTime(SUMOReal secPerKm);
    /**
     * gives the time in seconds for the trip
     * given a speed in seconds per kilometer (in city, not car speed
     * but time needed to make a distance in the city)
     */
    int getTimeTrip(SUMOReal secPerKm);
    /**
     * estimate the departure time needed for a given arrival time
     * and a speed in seconds per kilometer
     */
    int estimateDepTime(int arrTime, SUMOReal secPerKm);
    /**
     * returns whether this is a daily trip or a one day trip
     */
    bool isDaily();

private:
    AGPosition myFrom;
    AGPosition myTo;
    int myDepTime;
    std::string myVehicle;
    /**
     * indicates if it is a bus or a car (or any type)
     * "bus", "default" or "random" (which is a kind of default)
     */
    std::string myType;
    /**
     * if everyday : 0
     * else        : number of the day ( != 0 )
     */
    int myDay;
    std::list<AGPosition> myPassBy;
};

#endif

/****************************************************************************/
