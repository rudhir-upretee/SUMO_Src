/****************************************************************************/
/// @file    TraCIServerAPI_Lane.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    07.05.2009
/// @version $Id: TraCIServerAPI_Lane.h 13107 2012-12-02 13:57:34Z behrisch $
///
// APIs for getting/setting lane values via TraCI
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
#ifndef TraCIServerAPI_Lane_h
#define TraCIServerAPI_Lane_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#ifndef NO_TRACI

#include "TraCIException.h"
#include "TraCIServer.h"
#include <foreign/tcpip/storage.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class TraCIServerAPI_Lane
 * @brief APIs for getting/setting lane values via TraCI
 */
class TraCIServerAPI_Lane {
public:
    /** @brief Processes a get value command (Command 0xa3: Get Lane Variable)
     *
     * @param[in] server The TraCI-server-instance which schedules this request
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     */
    static bool processGet(traci::TraCIServer& server, tcpip::Storage& inputStorage,
                           tcpip::Storage& outputStorage);


    /** @brief Processes a set value command (Command 0xc3: Change Lane State)
     *
     * @param[in] server The TraCI-server-instance which schedules this request
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     */
    static bool processSet(traci::TraCIServer& server, tcpip::Storage& inputStorage,
                           tcpip::Storage& outputStorage);


    /** @brief Returns the named lane's shape
     *
     * @param[in] id The id of the searched lane
     * @param[out] shape The shape, if the lane is known
     * @return Whether the lane is known
     */
    static bool getShape(const std::string& id, PositionVector& shape);


    /** @brief Returns a tree filled with inductive loop instances
     * @return The rtree of inductive loop
     */
    static TraCIRTree* getTree();


private:
    /// @brief invalidated copy constructor
    TraCIServerAPI_Lane(const TraCIServerAPI_Lane& s);

    /// @brief invalidated assignment operator
    TraCIServerAPI_Lane& operator=(const TraCIServerAPI_Lane& s);


};


#endif

#endif

/****************************************************************************/

