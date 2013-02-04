/****************************************************************************/
/// @file    GUIPersonControl.cpp
/// @author  Daniel Krajzewicz
/// @date    Wed, 13.06.2012
/// @version $Id: GUIPersonControl.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// GUI-version of the person control for building gui persons
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

#include <vector>
#include <algorithm>
#include "GUINet.h"
#include "GUIPersonControl.h"
#include "GUIPerson.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
GUIPersonControl::GUIPersonControl() {}


GUIPersonControl::~GUIPersonControl() {
}


MSPerson*
GUIPersonControl::buildPerson(const SUMOVehicleParameter* pars, const MSVehicleType* vtype, MSPerson::MSPersonPlan* plan) const {
    return new GUIPerson(pars, vtype, plan);
}


/****************************************************************************/
