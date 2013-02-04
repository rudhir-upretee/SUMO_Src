/****************************************************************************/
/// @file    MFXEventQue.h
/// @author  Daniel Krajzewicz
/// @date    2004-03-19
/// @version $Id: MFXEventQue.h 11671 2012-01-07 20:14:30Z behrisch $
///
// missing_desc
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
#ifndef MFXEventQue_h
#define MFXEventQue_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <fx.h>
#include <list>
#include <utils/foxtools/MFXMutex.h>

class MFXEventQue {
public:
    MFXEventQue() { }
    ~MFXEventQue() { }

    void* top();
    void pop();
    void add(void* what);
    size_t size();
    bool empty();
private:
    MFXMutex myMutex;
    std::list<void*> myEvents;
};


#endif

/****************************************************************************/

