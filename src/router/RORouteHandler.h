/****************************************************************************/
/// @file    RORouteHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
/// @version $Id: RORouteHandler.h 13107 2012-12-02 13:57:34Z behrisch $
///
// Parser and container for routes during their loading
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
#ifndef RORouteHandler_h
#define RORouteHandler_h


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
#include <utils/common/RandomDistributor.h>
#include <utils/common/SUMOTime.h>
#include <utils/xml/SUMORouteHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice_String;
class ROEdge;
class RONet;
class RORoute;
class RORouteDef;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RORouteHandler
 * @brief Parser and container for routes during their loading
 *
 * RORouteHandler is the container for routes while they are build until
 * their transfering to the MSNet::RouteDict
 * The result of the operations are single MSNet::Route-instances
 */
class RORouteHandler : public SUMORouteHandler {
public:
    /// standard constructor
    RORouteHandler(RONet& net, const std::string& file,
                   const bool tryRepair);

    /// standard destructor
    virtual ~RORouteHandler();

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    virtual void myStartElement(int element,
                                const SUMOSAXAttributes& attrs);


    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    virtual void myEndElement(int element);
    //@}


    /** opens a type distribution for reading */
    void openVehicleTypeDistribution(const SUMOSAXAttributes& attrs);

    /** closes (ends) the building of a distribution */
    void closeVehicleTypeDistribution();

    /** opens a route for reading */
    void openRoute(const SUMOSAXAttributes& attrs);

    /** closes (ends) the building of a route.
        Afterwards no edges may be added to it;
        this method may throw exceptions when
        a) the route is empty or
        b) another route with the same id already exists */
    void closeRoute();

    /** opens a route distribution for reading */
    void openRouteDistribution(const SUMOSAXAttributes& attrs);

    /** closes (ends) the building of a distribution */
    void closeRouteDistribution();

    /// Ends the processing of a vehicle
    void closeVehicle();

    /// Ends the processing of a person
    void closePerson();

    /// Ends the processing of a flow
    void closeFlow();

    /// Processing of a stop
    void addStop(const SUMOSAXAttributes& attrs);

    /// Parse edges from strings
    void parseEdges(const std::string& desc, std::vector<const ROEdge*>& into,
                    const std::string& rid);

protected:
    /// @brief The current route
    RONet& myNet;

    /// @brief The current route
    std::vector<const ROEdge*> myActiveRoute;

    /// @brief The plan of the current person
    OutputDevice_String* myActivePlan;

    /// @brief Buffered person descriptions (with plans)
    std::map<const SUMOTime, std::string> myPersonBuffer;

    /// @brief Information whether routes shall be repaired
    const bool myTryRepair;

    /// @brief The currently parsed distribution of vehicle types (probability->vehicle type)
    RandomDistributor<SUMOVTypeParameter*>* myCurrentVTypeDistribution;

    /// @brief The id of the currently parsed vehicle type distribution
    std::string myCurrentVTypeDistributionID;

    /// @brief The currently parsed route alternatives
    RORouteDef* myCurrentAlternatives;

    /// @brief The currently parsed route costs
    SUMOReal myCurrentCosts;

private:
    /// @brief Invalidated copy constructor
    RORouteHandler(const RORouteHandler& s);

    /// @brief Invalidated assignment operator
    RORouteHandler& operator=(const RORouteHandler& s);

};


#endif

/****************************************************************************/

