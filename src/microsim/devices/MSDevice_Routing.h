/****************************************************************************/
/// @file    MSDevice_Routing.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @date    Tue, 04 Dec 2007
/// @version $Id: MSDevice_Routing.h 13107 2012-12-02 13:57:34Z behrisch $
///
// A device that performs vehicle rerouting based on current edge speeds
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
#ifndef MSDevice_Routing_h
#define MSDevice_Routing_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <set>
#include <vector>
#include <map>
#include <utils/common/SUMOTime.h>
#include <utils/common/WrappingCommand.h>
#include <utils/common/SUMOAbstractRouter.h>
#include <microsim/MSVehicle.h>
#include "MSDevice.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_Routing
 * @brief A device that performs vehicle rerouting based on current edge speeds
 *
 * The routing-device system consists of in-vehicle devices that perform a routing
 *  and a simulation-wide (static) methods for colecting edge weights.
 *
 * The edge weights container "myEdgeEfforts" is pre-initialised as soon as one
 *  device is built and is kept updated via an event that adapts it to the current
 *  mean speed on the simulated network's edges.
 *
 * A device is assigned to a vehicle using the common explicit/probability - procedure.
 *
 * A device computes a new route for a vehicle as soon as the vehicle is inserted
 *  (within "enterLaneAtInsertion") - and, if the given period is larger than 0 - each
 *  x time steps where x is the period. This is triggered by an event that executes
 *  "wrappedRerouteCommandExecute".
 */
class MSDevice_Routing : public MSDevice {
public:
    /** @brief Inserts MSDevice_Routing-options
     */
    static void insertOptions();


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether rerouting-devices shall be built
     *  for the given vehicle.
     *
     * When the first device is built, the static container of edge weights
     *  used for routing is initialised with the mean speed the edges allow.
     *  In addition, an event is generated which updates these weights is
     *  built and added to the list of events to execute at a simulation end.
     *
     * For each seen vehicle, the global vehicle index is increased.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[in, filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSDevice*>& into);

    /// @brief deletes the router instance
    static void cleanup();

public:
    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Computes a new route on vehicle insertion
     *
     * A new route is computed by calling the vehicle's "reroute" method, supplying
     *  "getEffort" as the edge effort retrieval method.
     *
     * If the reroute period is larger than 0, an event is generated and added
     *  to the list of simulation step begin events which executes
     *  "wrappedRerouteCommandExecute".
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return Always false
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     * @see MSVehicle::reroute
     * @see MSEventHandler
     * @see WrappingCommand
     */
    bool notifyEnter(SUMOVehicle& veh, MSMoveReminder::Notification reason);
    /// @}


    /// @brief Destructor.
    ~MSDevice_Routing();


private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     * @param[in] period The period with which a new route shall be searched
     * @param[in] preInsertionPeriod The route search period before insertion
     */
    MSDevice_Routing(SUMOVehicle& holder, const std::string& id, SUMOTime period,
                     SUMOTime preInsertionPeriod);


    /** @brief Performs rerouting at insertion into the network
     *
     * A new route is computed by calling the vehicle's "reroute" method, supplying
     *  "getEffort" as the edge effort retrieval method.
     *
     * @param[in] currentTime The current simulation time
     * @return The offset to the next call (the rerouting period "myPeriod")
     * @see MSVehicle::reroute
     * @see MSEventHandler
     * @see WrappingCommand
     */
    SUMOTime preInsertionReroute(SUMOTime currentTime);


    /** @brief Performs rerouting after a period
     *
     * A new route is computed by calling the vehicle's "reroute" method, supplying
     *  "getEffort" as the edge effort retrieval method.
     *
     * This method is called from the event handler at the begin of a simulation
     *  step after the rerouting period is over. The reroute period is returned.
     *
     * @param[in] currentTime The current simulation time
     * @return The offset to the next call (the rerouting period "myPeriod")
     * @see MSVehicle::reroute
     * @see MSEventHandler
     * @see WrappingCommand
     */
    SUMOTime wrappedRerouteCommandExecute(SUMOTime currentTime);


    /** @brief Returns the effort to pass an edge
     *
     * This method is given to the used router in order to obtain the efforts
     *  to pass an edge from the internal edge weights container.
     *
     * The time is not used, here, as the current simulation state is
     *  used in an aggregated way.
     *
     * @param[in] e The edge for which the effort to be passed shall be returned
     * @param[in] v The vehicle that is rerouted
     * @param[in] t The time for which the effort shall be returned
     * @return The effort (time to pass in this case) for an edge
     * @see DijkstraRouterTT_ByProxi
     */
    static SUMOReal getEffort(const MSEdge* const e, const SUMOVehicle* const v, SUMOReal t);


    /// @name Network state adaptation
    /// @{

    /** @brief Adapt edge efforts by the current edge states
     *
     * This method is called by the event handler at the end of a simulation
     *  step. The current edge weights are combined with the previously stored.
     *
     * @param[in] currentTime The current simulation time
     * @return The offset to the next call (always 1 in this case - edge weights are updated each time step)
     * @todo Describe how the weights are adapted
     * @see MSEventHandler
     * @see StaticCommand
     */
    static SUMOTime adaptEdgeEfforts(SUMOTime currentTime);
    /// @}


    /// @brief get the router, initialize on first use
    static SUMOAbstractRouter<MSEdge, SUMOVehicle>& getRouter();

private:
    /// @brief The period with which a vehicle shall be rerouted
    SUMOTime myPeriod;

    /// @brief The period with which a vehicle shall be rerouted before insertion
    SUMOTime myPreInsertionPeriod;

    /// @brief The (optional) command responsible for rerouting
    WrappingCommand< MSDevice_Routing >* myRerouteCommand;

    /// @brief The weights adaptation/overwriting command
    static Command* myEdgeWeightSettingCommand;

    /// @brief The container of edge efforts
    static std::map<const MSEdge*, SUMOReal> myEdgeEfforts;

    /// @brief Information which weight prior edge efforts have
    static SUMOReal myAdaptationWeight;

    /// @brief Information which weight prior edge efforts have
    static SUMOTime myAdaptationInterval;

    /// @brief whether taz shall be used at initial rerouting
    static bool myWithTaz;

    /// @brief The container of pre-calculated routes
    static std::map<std::pair<const MSEdge*, const MSEdge*>, const MSRoute*> myCachedRoutes;

    /// @brief The router to use
    static SUMOAbstractRouter<MSEdge, SUMOVehicle>* myRouter;

    /// @brief the vehicles which explicitly carry a device
    static std::set<std::string> myExplicitIDs;


private:
    /// @brief Invalidated copy constructor.
    MSDevice_Routing(const MSDevice_Routing&);

    /// @brief Invalidated assignment operator.
    MSDevice_Routing& operator=(const MSDevice_Routing&);


};


#endif

/****************************************************************************/

