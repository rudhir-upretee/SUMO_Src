/****************************************************************************/
/// @file    MSBaseVehicle.h
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 8 Nov 2010
/// @version $Id: MSBaseVehicle.h 13107 2012-12-02 13:57:34Z behrisch $
///
// A base class for vehicle implementations
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
#ifndef MSBaseVehicle_h
#define MSBaseVehicle_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>
#include <vector>
#include <utils/common/SUMOVehicle.h>
#include <utils/common/StdDefs.h>
#include "MSRoute.h"
#include "MSMoveReminder.h"


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicleParameter;
class MSVehicleType;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSBaseVehicle
 * @brief The base class for microscopic and mesoscopic vehicles
 */
class MSBaseVehicle : public SUMOVehicle {
public:
    /** @brief Constructor
     * @param[in] pars The vehicle description
     * @param[in] route The vehicle's route
     * @param[in] type The vehicle's type
     * @param[in] speedFactor The factor for driven lane's speed limits
     * @exception ProcessError If a value is wrong
     */
    MSBaseVehicle(SUMOVehicleParameter* pars, const MSRoute* route, const MSVehicleType* type, SUMOReal speedFactor);


    /// @brief Destructor
    virtual ~MSBaseVehicle();


    /// Returns the name of the vehicle
    const std::string& getID() const;

    /** @brief Returns the vehicle's parameter (including departure definition)
     *
     * @return The vehicle's parameter
     */
    const SUMOVehicleParameter& getParameter() const;


    /** @brief Returns the current route
     * @return The route the vehicle uses
     */
    const MSRoute& getRoute() const;


    /** @brief Returns the vehicle's type definition
     * @return The vehicle's type definition
     */
    const MSVehicleType& getVehicleType() const;


    /** @brief Returns the maximum speed
     * @return The vehicle's maximum speed
     */
    SUMOReal getMaxSpeed() const;


    /** @brief Returns the nSuccs'th successor of edge the vehicle is currently at
     *
     * If the rest of the route (counted from the current edge) has less than nSuccs edges,
     *  0 is returned.
     * @param[in] nSuccs The number of edge to look forward
     * @return The nSuccs'th following edge in the vehicle's route
     */
    const MSEdge* succEdge(unsigned int nSuccs) const;

    /** @brief Returns the edge the vehicle is currently at
     *
     * @return The current edge in the vehicle's route
     */
    const MSEdge* getEdge() const;


    /** @brief Returns the information whether the vehicle is on a road (is simulated)
     * @return Whether the vehicle is simulated
     */
    virtual bool isOnRoad() const {
        return true;
    }


    /** @brief Performs a rerouting using the given router
     *
     * Tries to find a new route between the current edge and the destination edge, first.
     * Tries to replace the current route by the new one using replaceRoute.
     *
     * @param[in] t The time for which the route is computed
     * @param[in] router The router to use
     * @see replaceRoute
     */
    void reroute(SUMOTime t, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, bool withTaz = false);


    /** @brief Replaces the current route by the given edges
     *
     * It is possible that the new route is not accepted, if a) it does not
     *  contain the vehicle's current edge, or b) something fails on insertion
     *  into the routes container (see in-line comments).
     *
     * @param[in] edges The new list of edges to pass
     * @param[in] simTime The time at which the route was replaced
     * @return Whether the new route was accepted
     */
    bool replaceRouteEdges(const MSEdgeVector& edges, bool onInit = false);


    /** @brief Returns the vehicle's acceleration
     *
     * This default implementation returns always 0.
     * @return The acceleration
     */
    virtual SUMOReal getAcceleration() const;

    /** @brief Called when the vehicle is inserted into the network
     *
     * Sets optional information about departure time, informs the vehicle
     *  control about a further running vehicle.
     */
    void onDepart();

    /** @brief Returns this vehicle's real departure time
     * @return This vehicle's real departure time
     */
    inline SUMOTime getDeparture() const {
        return myDeparture;
    }

    /** @brief Returns whether this vehicle has already departed
     */
    bool hasDeparted() const;

    /** @brief Returns whether this vehicle has already arived
     * (by default this is true if the vehicle has reached its final edge)
     */
    virtual bool hasArrived() const;

    /** @brief Returns the number of new routes this vehicle got
     * @return the number of new routes this vehicle got
     */
    inline unsigned int getNumberReroutes() const {
        return myNumberReroutes;
    }

    /** @brief Returns this vehicle's devices
     * @return This vehicle's devices
     */
    inline const std::vector<MSDevice*>& getDevices() const {
        return myDevices;
    }

    /** @brief Adds a person to this vehicle
     *
     * The default implementation does nothing since persons are not supported by default
     *
     * @param[in] person The person to add
     */
    virtual void addPerson(MSPerson* person);

    /** @brief Validates the current route
     * @param[out] msg Description why the route is not valid (if it is the case)
     * @return Whether the vehicle's current route is valid
     */
    bool hasValidRoute(std::string& msg) const;

    /** @brief Adds a MoveReminder dynamically
     *
     * @param[in] rem the reminder to add
     * @see MSMoveReminder
     */
    void addReminder(MSMoveReminder* rem);

    /** @brief Removes a MoveReminder dynamically
     *
     * @param[in] rem the reminder to remove
     * @see MSMoveReminder
     */
    void removeReminder(MSMoveReminder* rem);

    /** @brief "Activates" all current move reminder
     *
     * For all move reminder stored in "myMoveReminders", their method
     *  "MSMoveReminder::notifyEnter" is called.
     *
     * @param[in] reason The reason for changing the reminders' states
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    virtual void activateReminders(const MSMoveReminder::Notification reason);

    /** @brief Returns the precomputed factor by which the driver wants to be faster than the speed limit
     * @return Speed limit factor
     */
    SUMOReal getChosenSpeedFactor() const {
        return myChosenSpeedFactor;
    }

protected:
    /** @brief (Re-)Calculates the arrival position from the vehicle parameters
     */
    void calculateArrivalPos();

protected:
    /// @brief This Vehicle's parameter.
    const SUMOVehicleParameter* myParameter;

    /// @brief This Vehicle's route.
    const MSRoute* myRoute;

    /// @brief This Vehicle's type.
    const MSVehicleType* myType;

    /// @brief Iterator to current route-edge
    MSRouteIterator myCurrEdge;

    /// @brief A precomputed factor by which the driver wants to be faster than the speed limit
    SUMOReal myChosenSpeedFactor;


    /// @name Move reminder structures
    /// @{

    /// @brief Definition of a move reminder container
    typedef std::vector< std::pair<MSMoveReminder*, SUMOReal> > MoveReminderCont;

    /// @brief Current lane's move reminder
    MoveReminderCont myMoveReminders;
    /// @}

    /// @brief The devices this vehicle has
    std::vector<MSDevice*> myDevices;

    /// @brief The real departure time
    SUMOTime myDeparture;

    /// the position on the destination lane where the vehicle stops
    SUMOReal myArrivalPos;

    /// @brief The number of reroutings
    unsigned int myNumberReroutes;

private:
    /* @brief magic value for undeparted vehicles
     * @note: in previous versions this was -1
     */
    static const SUMOTime NOT_YET_DEPARTED;

};

#endif

/****************************************************************************/
