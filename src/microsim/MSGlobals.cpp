/****************************************************************************/
/// @file    MSGlobals.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    late summer 2003
/// @version $Id: MSGlobals.cpp 13049 2012-11-26 09:10:34Z namdre $
///
// Some static variables for faster access
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSGlobals.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// static member variable definitions
// ===========================================================================
bool MSGlobals::gOmitEmptyEdgesOnDump;

bool MSGlobals::gUsingInternalLanes;

SUMOTime MSGlobals::gTimeToGridlock;

bool MSGlobals::gCheck4Accidents;

bool MSGlobals::gCheckRoutes;

#ifdef HAVE_INTERNAL
bool MSGlobals::gStateLoaded;
bool MSGlobals::gUseMesoSim;
bool MSGlobals::gMesoLimitedJunctionControl;
MELoop* MSGlobals::gMesoNet;
#else
const bool MSGlobals::gUseMesoSim = false;
#endif

#ifdef _DEBUG
bool MSGlobals::gDebugFlag1 = false;
bool MSGlobals::gDebugFlag2 = false;
#endif

/****************************************************************************/

