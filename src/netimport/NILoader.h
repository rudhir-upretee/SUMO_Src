/****************************************************************************/
/// @file    NILoader.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
/// @version $Id: NILoader.h 13107 2012-12-02 13:57:34Z behrisch $
///
// Perfoms network import
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
#ifndef NILoader_h
#define NILoader_h


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
#include <xercesc/sax2/SAX2XMLReader.hpp>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class SUMOSAXHandler;
class NBNetBuilder;
class Position;
class PositionVector;
class GeoConvHelper;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NILoader
 * @brief Perfoms network import
 *
 * A plain loader which encapsulates calls to the import modules.
 */
class NILoader {
public:
    /** @brief Constructor
     * @param[in] nb The network builder to fill with loaded data
     */
    NILoader(NBNetBuilder& nb);


    /// @brief Destructor
    ~NILoader();


    /** loads data from the files specified in the given option container */
    void load(OptionsCont& oc);

    /**
     * @brief transforms loaded coordinates
     * handles projections, offsets (using GeoConvHelper) and import of height data (using Heightmapper if available)
     * @param[in,out] from The coordinate to be transformed
     * @param[in] includeInBoundary Whether to patch the convex boundary of the GeoConvHelper default instance
     * @param[in] from_srs The spatial reference system of the input coordinate
     */
    static bool transformCoordinates(Position& from, bool includeInBoundary = true, GeoConvHelper* from_srs = 0);
    static bool transformCoordinates(PositionVector& from, bool includeInBoundary = true, GeoConvHelper* from_srs = 0);

private:
    /** loads data from sumo-files */
    //void loadSUMO(OptionsCont &oc);

    /** loads data from XML-files */
    void loadXML(OptionsCont& oc);

    /** loads data from the list of xml-files of certain type */
    void loadXMLType(SUMOSAXHandler* handler,
                     const std::vector<std::string>& files, const std::string& type);

private:
    /// @brief The network builder to fill with loaded data
    NBNetBuilder& myNetBuilder;


private:
    /// @brief Invalidated copy constructor.
    NILoader(const NILoader&);

    /// @brief Invalidated assignment operator.
    NILoader& operator=(const NILoader&);


};


#endif

/****************************************************************************/

