/****************************************************************************/
/// @file    PCLoaderOSM.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 19.11.2008
/// @version $Id: PCLoaderOSM.h 13107 2012-12-02 13:57:34Z behrisch $
///
// A reader of pois and polygons stored in OSM-format
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
#ifndef PCLoaderOSM_h
#define PCLoaderOSM_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include "PCPolyContainer.h"
#include "PCTypeMap.h"
#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class definitions
// ===========================================================================
class OptionsCont;


// ===========================================================================
// class declarations
// ===========================================================================
/**
 * @class PCLoaderOSM
 * @brief A reader of pois and polygons stored in OSM-format
 *
 * Reads pois stored as XML definition as given by the OpenStreetMap-API.
 */
class PCLoaderOSM : public SUMOSAXHandler {
public:
    /** @brief Loads pois/polygons assumed to be stored as OSM-XML
     *
     * If the option "osm-files" is set within the given options container,
     *  an instance of PCLoaderOSM is built and used as a handler for the
     *  files given in this option.
     *
     * @param[in] oc The options container to get further options from
     * @param[in] toFill The poly/pois container to add loaded polys/pois to
     * @param[in] tm The type map to use for setting values of loaded polys/pois
     * @exception ProcessError if something fails
     */
    static void loadIfSet(OptionsCont& oc, PCPolyContainer& toFill,
                          PCTypeMap& tm);


protected:
    /** @brief An internal representation of an OSM-node
     */
    struct PCOSMNode {
        /// @brief The node's id
        SUMOLong id;
        /// @brief The longitude the node is located at
        SUMOReal lon;
        /// @brief The latitude the node is located at
        SUMOReal lat;
        /// @brief The type (<KEY>"."<VALUE>)
        std::string myType;
        /// @brief Information whether this shall be parsed
        bool myIsAdditional;
    };


    /** @brief An internal definition of a loaded edge
     */
    struct PCOSMEdge {
        /// @brief The edge's id
        std::string id;
        /// @brief The edge's name (if any)
        std::string name;
        /// @brief The type (<KEY>"."<VALUE>)
        std::string myType;
        /// @brief Information whether this area is closed
        bool myIsClosed;
        /// @brief The list of nodes this edge is made of
        std::vector<SUMOLong> myCurrentNodes;
        /// @brief Information whether this shall be parsed
        bool myIsAdditional;
    };


protected:
    /**
     * @class NodesHandler
     * @brief A class which extracts OSM-nodes from a parsed OSM-file
     */
    class NodesHandler : public SUMOSAXHandler {
    public:
        /** @brief Contructor
         * @param[in] toFill The nodes container to fill
         */
        NodesHandler(std::map<SUMOLong, PCOSMNode*>& toFill);


        /// @brief Destructor
        ~NodesHandler();


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
        void myStartElement(int element, const SUMOSAXAttributes& attrs);


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element);
        //@}


    private:
        /// @brief The nodes container to fill
        std::map<SUMOLong, PCOSMNode*>& myToFill;

        /// @brief Current path in order to know to what occuring values belong
        std::vector<int> myParentElements;

        /// @brief The id of the last parsed node
        SUMOLong myLastNodeID;


    private:
        /// @brief Invalidated copy constructor
        NodesHandler(const NodesHandler& s);

        /// @brief Invalidated assignment operator
        NodesHandler& operator=(const NodesHandler& s);

    };



    /**
     * @class EdgesHandler
     * @brief A class which extracts OSM-edges from a parsed OSM-file
     */
    class EdgesHandler : public SUMOSAXHandler {
    public:
        /** @brief Constructor
         *
         * @param[in] osmNodes The previously parsed (osm-)nodes
         * @param[in] toFill The edges container to fill with read edges
         */
        EdgesHandler(const std::map<SUMOLong, PCOSMNode*>& osmNodes,
                     std::map<std::string, PCOSMEdge*>& toFill);


        /// @brief Destructor
        ~EdgesHandler();


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
        void myStartElement(int element, const SUMOSAXAttributes& attrs);


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element);
        //@}


    private:
        /// @brief The previously parsed nodes
        const std::map<SUMOLong, PCOSMNode*>& myOSMNodes;

        /// @brief A map of built edges
        std::map<std::string, PCOSMEdge*>& myEdgeMap;

        /// @brief The currently built edge
        PCOSMEdge* myCurrentEdge;

        /// @brief Current path in order to know to what occuring values belong
        std::vector<int> myParentElements;


    private:
        /// @brief Invalidated copy constructor
        EdgesHandler(const EdgesHandler& s);

        /// @brief Invalidated assignment operator
        EdgesHandler& operator=(const EdgesHandler& s);

    };


};


#endif

/****************************************************************************/

