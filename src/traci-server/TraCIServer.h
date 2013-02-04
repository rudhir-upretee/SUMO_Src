/****************************************************************************/
/// @file    TraCIServer.h
/// @author  Axel Wegener
/// @author  Friedemann Wesner
/// @author  Christoph Sommer
/// @author  Tino Morenz
/// @author  Daniel Krajzewicz
/// @author  Thimor Bohn
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    2007/10/24
/// @version $Id: TraCIServer.h 13107 2012-12-02 13:57:34Z behrisch $
///
/// TraCI server used to control sumo by a remote TraCI client
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
#ifndef TRACISERVER_H
#define TRACISERVER_H


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#ifndef NO_TRACI

#include "TraCIConstants.h"

#define BUILD_TCPIP
#include <foreign/tcpip/socket.h>
#include <foreign/tcpip/storage.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/ToString.h>

#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <utils/shapes/Polygon.h>
#include <utils/shapes/PointOfInterest.h>
#include <utils/options/OptionsCont.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSNet.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include "TraCIException.h"
#include "TraCIRTree.h"

#include <map>
#include <string>
#include <set>



// ===========================================================================
// class definitions
// ===========================================================================
namespace traci {

/** @class TraCIServer
 * @brief TraCI server used to control sumo by a remote TraCI client
 */
class TraCIServer : public MSNet::VehicleStateListener {
public:
    /// @brief Definition of a method to be called for serving an associated commandID
    typedef bool(*CmdExecutor)(traci::TraCIServer& server, tcpip::Storage& inputStorage, tcpip::Storage& outputStorage);


    /// @name Initialisation and Shutdown
    /// @{

    /** @brief Initialises the server
     * @param[in] execs The (additional) command executors to use
     */
    static void openSocket(const std::map<int, CmdExecutor>& execs);


    /// @brief request termination of connection
    static void close();


    /** @brief check whether close was requested
     * @return Whether the connection was closed
     */
    static bool wasClosed();
    /// @}




    /// @brief process all commands until a simulation step is wanted
    static void processCommandsUntilSimStep(SUMOTime step);




#ifdef HAVE_PYTHON
    /// @brief process the command
    static std::string execute(std::string cmd);

    /// @brief run the given script
    static void runEmbedded(std::string pyFile);
#endif

    void vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to);

    void writeStatusCmd(int commandId, int status, const std::string& description);
    void writeStatusCmd(int commandId, int status, const std::string& description, tcpip::Storage& outputStorage);

    const std::map<MSNet::VehicleState, std::vector<std::string> >& getVehicleStateChanges() const {
        return myVehicleStateChanges;
    }

    void writeResponseWithLength(tcpip::Storage& outputStorage, tcpip::Storage& tempMsg);

    void collectObjectsInRange(int domain, const PositionVector& shape, SUMOReal range, std::set<std::string>& into);


private:
    /** @brief Constructor
     * @param[in] port The port to listen to (to open)
     */
    TraCIServer(int port = 0);


    /// @brief Destructor
    virtual ~TraCIServer();



    /// @name Server-internal command handling
    /// @{

    /** @brief Returns the TraCI-version
     * @return Always true
     */
    bool commandGetVersion();


    /** @brief Indicates the connection as being closed
     * @return Always true
     */
    bool commandCloseConnection();


    /** @brief Handles subscriptions to send after a simstep2 command
     */
    void postProcessSimulationStep2();


    /** @brief Adds a vehicle
     * @deprecated Vehicles shall be inserted using the vehicle-API
    */
    bool commandAddVehicle();
    /// @}





    int dispatchCommand();


private:
    /// @brief Singleton instance of the server
    static TraCIServer* myInstance;

    /// @brief Whether the connection was set to be to close
    static bool myDoCloseConnection;

    /// @brief The socket on which server is listening on
    tcpip::Socket* mySocket;

    /// @brief The time step to reach until processing the next commands
    SUMOTime myTargetTime;

    /// @brief The storage to read from
    tcpip::Storage myInputStorage;

    /// @brief The storage to writeto
    tcpip::Storage myOutputStorage;

    /// @brief Whether a step is currently done
    /// @todo: What is this for?
    bool myDoingSimStep;

    /// @brief Whether the usage of deprecated methods was already reported
    bool myHaveWarnedDeprecation;

    /// @brief Whether the server runs in embedded mode
    const bool myAmEmbedded;

    /// @brief Map of commandIds -> their executors; applicable if the executor applies to the method footprint
    std::map<int, CmdExecutor> myExecutors;


    /** @class Subscription
     * @brief Representation of a subscription
     */
    class Subscription {
    public:
        /** @brief Constructor
         * @param[in] commandIdArg The command id of the subscription
         * @param[in] idArg The id of the object that is subscribed
         * @param[in] variablesArg The subscribed variables
         * @param[in] beginTimeArg The begin time of the subscription
         * @param[in] endTimeArg The end time of the subscription
         * @param[in] contextVarsArg Whether the subscription is a context subscription (variable subscription otherwise)
         * @param[in] contextDomainArg The domain ID of the context
         * @param[in] rangeArg The range of the context
         */
        Subscription(int commandIdArg, const std::string& idArg, const std::vector<int>& variablesArg,
                     SUMOTime beginTimeArg, SUMOTime endTimeArg, bool contextVarsArg, int contextDomainArg, SUMOReal rangeArg)
            : commandId(commandIdArg), id(idArg), variables(variablesArg), beginTime(beginTimeArg), endTime(endTimeArg),
              contextVars(contextVarsArg), contextDomain(contextDomainArg), range(rangeArg) {}

        /// @brief commandIdArg The command id of the subscription
        int commandId;
        /// @brief The id of the object that is subscribed
        std::string id;
        /// @brief The subscribed variables
        std::vector<int> variables;
        /// @brief The begin time of the subscription
        SUMOTime beginTime;
        /// @brief The end time of the subscription
        SUMOTime endTime;
        /// @brief Whether the subscription is a context subscription (variable subscription otherwise)
        bool contextVars;
        /// @brief The domain ID of the context
        int contextDomain;
        /// @brief The range of the context
        SUMOReal range;

    };

    /// @brief The list of known, still valid subscriptions
    std::vector<Subscription> mySubscriptions;

    /// @brief Changes in the states of simulated vehicles
    std::map<MSNet::VehicleState, std::vector<std::string> > myVehicleStateChanges;

    /// @brief A storage of objects
    std::map<int, TraCIRTree*> myObjects;


private:
    bool addObjectVariableSubscription(int commandId);
    bool addObjectContextSubscription(int commandId);
    void initialiseSubscription(const Subscription& s);
    void removeSubscription(int commandId, const std::string& identity, int domain);
    bool processSingleSubscription(const TraCIServer::Subscription& s, tcpip::Storage& writeInto,
                                   std::string& errors);


    bool findObjectShape(int domain, const std::string& id, PositionVector& shape);


private:
    /// @brief Invalidated assignment operator
    TraCIServer& operator=(const TraCIServer& s);

};

}


#endif

#endif
