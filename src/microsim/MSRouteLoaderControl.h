/****************************************************************************/
/// @file    MSRouteLoaderControl.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Wed, 06 Nov 2002
/// @version $Id: MSRouteLoaderControl.h 12392 2012-06-15 08:54:23Z dkrajzew $
///
// Class responsible for loading of routes from some files
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
#ifndef MSRouteLoaderControl_h
#define MSRouteLoaderControl_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <vector>
#include "MSNet.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSRouteLoader;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSRouteLoaderControl
 *
 * MSRouteLoaderControl
 * This controls is initialised with the list of route loaders and uses them
 * to load routes step wise.
 * The parameter myInAdvanceStepNo holds the number of time steps to read the
 * routes in forward. If it is 0 (default), all routes will be read at once.
 */
class MSRouteLoaderControl {
public:
    /// definition of the loader vector
    typedef std::vector<MSRouteLoader*> LoaderVector;

public:
    /// constructor
    MSRouteLoaderControl(MSNet& net, SUMOTime inAdvanceStepNo,
                         LoaderVector loader);

    /// destructor
    ~MSRouteLoaderControl();

    /// loads the next routes
    void loadNext(SUMOTime step);

private:
    /// the last time step new routes were loaded
    SUMOTime myLastLoadTime;

    /// the number of routes to read in forward
    SUMOTime myInAdvanceStepNo;

    /// the list of route loaders
    LoaderVector myRouteLoaders;

    /** information whether all routes shall be loaded and whether
        they were loaded */
    bool myLoadAll, myAllLoaded;
};


#endif

/****************************************************************************/

