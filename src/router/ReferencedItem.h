/****************************************************************************/
/// @file    ReferencedItem.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
/// @version $Id: ReferencedItem.h 11671 2012-01-07 20:14:30Z behrisch $
///
// Helper base for things that are referenced and have to be saved only once
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
#ifndef ReferencedItem_h
#define ReferencedItem_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ReferencedItem
 * @brief Helper base for things that are referenced and have to be saved only once
 *
 * Basic class for structures which are referenced by other classes. The wish
 *  is to have an information whether an instance of a derived class was already
 *  saved (together with a referencing class) or shall be saved this time.
 */
class ReferencedItem {
public:
    /// @brief Constructor
    ReferencedItem() : myWasSaved(false) { }


    /// @brief Destructor
    virtual ~ReferencedItem()  { }


    /** @brief Returns the information whether this item was already saved
     *
     * @return Whether this item was saved
     */
    bool isSaved() const  {
        return myWasSaved;
    }


    /** @brief Marks the item as saved
     */
    void markSaved()  {
        myWasSaved = true;
    }


private:
    /// @brief Information whether the item was already saved
    bool myWasSaved;


private:
    /// @brief Invalidated copy constructor
    ReferencedItem(const ReferencedItem& src);

    /// @brief Invalidated assignment operator
    ReferencedItem& operator=(const ReferencedItem& src);

};


#endif

/****************************************************************************/

