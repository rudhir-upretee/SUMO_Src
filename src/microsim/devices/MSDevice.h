/****************************************************************************/
/// @file    MSDevice.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @date    Tue, 04 Dec 2007
/// @version $Id: MSDevice.h 11671 2012-01-07 20:14:30Z behrisch $
///
// Abstract in-vehicle device
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
#ifndef MSDevice_h
#define MSDevice_h


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
#include <microsim/MSMoveReminder.h>
#include <utils/common/Named.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class SUMOVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice
 * @brief Abstract in-vehicle device
 *
 * The MSDevice-interface brings the following interfaces to a vehicle that
 *  may be overwritten by real devices:
 * @arg Retrieval of the vehicle that holds the device
 * @arg Building and retrieval of a device id
 * @arg Methods called on vehicle movement / state change
 *
 * The "methods called on vehicle movement / state change" are called for each
 *  device within the corresponding vehicle methods. MSDevice brings already
 *  an empty (nothing doing) implementation of these.
 */
class MSDevice : public MSMoveReminder, public Named {
public:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice(SUMOVehicle& holder, const std::string& id)
        : Named(id), myHolder(holder) {
    }


    /// @brief Destructor
    virtual ~MSDevice() { }


    /** @brief Returns the vehicle that holds this device
     *
     * @return The vehicle that holds this device
     */
    SUMOVehicle& getHolder() const {
        return myHolder;
    }


    /** @brief Called on writing tripinfo output
     *
     * The device may write some statistics into the tripinfo output. It
     *  is assumed that the written information is a valid xml-snipplet, which
     *  will be embedded within the vehicle's information.
     *
     * The device should use the openTag / closeTag methods of the OutputDevice
     *  for correct indentation.
     *
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     */
    virtual void generateOutput() const {
    }


protected:
    /// @brief The vehicle that stores the device
    SUMOVehicle& myHolder;


private:
    /// @brief Invalidated copy constructor.
    MSDevice(const MSDevice&);

    /// @brief Invalidated assignment operator.
    MSDevice& operator=(const MSDevice&);

};


#endif

/****************************************************************************/

