/****************************************************************************/
/// @file    NIVissimNodeParticipatingEdgeVector.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id: NIVissimNodeParticipatingEdgeVector.h 11671 2012-01-07 20:14:30Z behrisch $
///
// -------------------
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
#ifndef NIVissimNodeParticipatingEdgeVector_h
#define NIVissimNodeParticipatingEdgeVector_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif


#include <vector>
#include "NIVissimNodeParticipatingEdge.h"

typedef std::vector<NIVissimNodeParticipatingEdge*> NIVissimNodeParticipatingEdgeVector;


#endif

/****************************************************************************/

