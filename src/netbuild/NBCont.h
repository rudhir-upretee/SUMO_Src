/****************************************************************************/
/// @file    NBCont.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @date    Mon, 17 Dec 2001
/// @version $Id: NBCont.h 11671 2012-01-07 20:14:30Z behrisch $
///
// Some list definitions
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
#ifndef NBCont_h
#define NBCont_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;


// ===========================================================================
// container definitions
// ===========================================================================
/** container for (sorted) edges */
typedef std::vector<NBEdge*> EdgeVector;


/** container for (sorted) lanes.
    The lanes are sorted from rightmost (id=0) to leftmost (id=nolanes-1) */
typedef std::vector<size_t> LaneVector;


#endif

/****************************************************************************/

