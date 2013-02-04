/****************************************************************************/
/// @file    MSNoLogicJunction.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 12 Dez 2001
/// @version $Id: MSNoLogicJunction.h 12128 2012-03-19 11:54:26Z dkrajzew $
///
// logic, e.g. for exits.
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
#ifndef MSNoLogicJunction_h
#define MSNoLogicJunction_h


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
#include <bitset>
#include "MSJunction.h"

// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSNoLogicJunction
 * This junctions let all vehicles past through so they only should be used on
 * junctions where incoming vehicles are no foes to each other (may drive
 * simultaneously).
 */
class MSNoLogicJunction  : public MSJunction {
public:
    /// Destructor.
    virtual ~MSNoLogicJunction();

    /** @brief Constructor
     * @param[in] id The id of the junction
     * @param[in] position The position of the junction
     * @param[in] shape The shape of the junction
     * @param[in] incoming The incoming lanes
     * @param[in] internal The internal lanes
     */
    MSNoLogicJunction(const std::string& id, const Position& position,
                      const PositionVector& shape,
                      std::vector<MSLane*> incoming
#ifdef HAVE_INTERNAL_LANES
                      , std::vector<MSLane*> internal
#endif
                     );

    /** Initialises the junction after the net was completely loaded */
    void postloadInit();

private:
    /** Lanes incoming to the junction */
    std::vector<MSLane*> myIncomingLanes;

#ifdef HAVE_INTERNAL_LANES
    /** The junctions internal lanes */
    std::vector<MSLane*> myInternalLanes;
#endif

private:
    /// @brief Invalidated copy constructor.
    MSNoLogicJunction(const MSNoLogicJunction&);

    /// @brief Invalidated assignment operator.
    MSNoLogicJunction& operator=(const MSNoLogicJunction&);

    /** @brief a dump container
        Request-setting vehicles may write into this container and responds
        are read from it. As responds are always true, this container is
        set to true for all links.
        This dump is also used as the mask for incoming non-first vehicles */
    static std::bitset<64> myDump;

};


#endif

/****************************************************************************/

