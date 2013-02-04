/****************************************************************************/
/// @file    ROVehicle.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id: ROVehicle.h 13107 2012-12-02 13:57:34Z behrisch $
///
// A vehicle as used by router
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
#ifndef ROVehicle_h
#define ROVehicle_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <iostream>
#include <utils/common/SUMOTime.h>
#include <utils/common/RGBColor.h>
#include <utils/common/SUMOVehicleParameter.h>
#include <utils/common/SUMOVTypeParameter.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SUMOAbstractRouter.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVTypeParameter;
class RORouteDef;
class OutputDevice;
class ROEdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROVehicle
 * @brief A vehicle as used by router
 */
class ROVehicle {
public:
    /** @brief Constructor
     *
     * @param[in] pars Parameter of this vehicle
     * @param[in] route The definition of the route the vehicle shall use
     * @param[in] type The type of the vehicle
     *
     * @todo Why is the vehicle builder given?
     */
    ROVehicle(const SUMOVehicleParameter& pars,
              RORouteDef* route, const SUMOVTypeParameter* type);


    /// @brief Destructor
    virtual ~ROVehicle();


    /** @brief Returns the definition of the route the vehicle takes
     *
     * @return The vehicle's route definition
     *
     * @todo Why not return a reference?
     */
    RORouteDef* getRouteDefinition() const {
        return myRoute;
    }


    /** @brief Returns the type of the vehicle
     *
     * @return The vehicle's type
     *
     * @todo Why not return a reference?
     */
    const SUMOVTypeParameter* getType() const {
        return myType;
    }


    /** @brief Returns the id of the vehicle
     *
     * @return The id of the vehicle
     */
    const std::string& getID() const {
        return myParameter.id;
    }


    /** @brief Returns the time the vehicle starts at
     *
     * @return The vehicle's depart time
     */
    SUMOTime getDepartureTime() const {
        return myParameter.depart;
    }


    /// @brief Returns the vehicle's maximum speed
    SUMOReal getMaxSpeed() const;


    inline SUMOVehicleClass getVClass() const {
        return getType() != 0 ? getType()->vehicleClass : DEFAULT_VEH_CLASS;
    }


    /** @brief Saves the complete vehicle description.
     *
     * Saves the vehicle type if it was not saved before.
     * Saves the vehicle route if it was not saved before.
     * Saves the vehicle itself.
     *
     * @param[in] os The routes - output device to store the vehicle's description into
     * @param[in] altos The route alternatives - output device to store the vehicle's description into
     * @param[in] typeos The types - output device to store the vehicle types into
     * @param[in] withExitTimes whether exit times for the edges shall be written
     * @exception IOError If something fails (not yet implemented)
     */
    void saveAllAsXML(OutputDevice& os, OutputDevice* const altos,
                      OutputDevice* const typeos, bool withExitTimes) const;


    /** @brief Returns a copy of the vehicle using a new id, departure time and route
     *
     * @param[in] id the new id to use
     * @param[in] depTime The new vehicle's departure time
     * @param[in] newRoute The new vehicle's route
     * @return The new vehicle
     *
     * @todo Is this used? What for if everything is replaced?
     */
    virtual ROVehicle* copy(const std::string& id, unsigned int depTime, RORouteDef* newRoute) const;


protected:
    /// @brief The vehicle's parameter
    SUMOVehicleParameter myParameter;

    /// @brief The type of the vehicle
    const SUMOVTypeParameter* const myType;

    /// @brief The route the vehicle takes
    RORouteDef* const myRoute;


private:
    /// @brief Invalidated copy constructor
    ROVehicle(const ROVehicle& src);

    /// @brief Invalidated assignment operator
    ROVehicle& operator=(const ROVehicle& src);

};


#endif

/****************************************************************************/

