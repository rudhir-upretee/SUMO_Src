/****************************************************************************/
/// @file    GUIDialog_Breakpoints.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Thu, 17 Jun 2004
/// @version $Id: GUIDialog_Breakpoints.h 12725 2012-09-17 10:37:29Z dkrajzew $
///
// Editor for simulation breakpoints
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
#ifndef GUIDialog_Breakpoints_h
#define GUIDialog_Breakpoints_h


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
#include <fx.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MFXAddEditTypedTable;


// ===========================================================================
// class definition
// ===========================================================================
/**
 * @class GUIDialog_Breakpoints
 * @brief Editor for simulation breakpoints
 *
 * This dialog shows and lets the user edit the list of breakpoints - simulation
 *  time steps where the simulation halts.
 * @todo Use a LineReader instead of >> while reading
 */
class GUIDialog_Breakpoints : public FXMainWindow {
    // FOX-declarations
    FXDECLARE(GUIDialog_Breakpoints)

public:
    /** @brief Constructor
     * @param[in] parent The parent window
     */
    GUIDialog_Breakpoints(GUIMainWindow* parent);


    /// @brief Destructor
    ~GUIDialog_Breakpoints();



    /// @name FOX-callbacks
    /// @{

    /// @brief Called when the user presses the Load-button
    long onCmdLoad(FXObject*, FXSelector, void*);

    /// @brief Called when the user presses the Save-button
    long onCmdSave(FXObject*, FXSelector, void*);

    /// @brief Called when the user presses the Clear-button
    long onCmdClear(FXObject*, FXSelector, void*);

    /// @brief Called when the user presses the Close-button
    long onCmdClose(FXObject*, FXSelector, void*);

    /// @brief Called when the table was changed
    long onCmdEditTable(FXObject*, FXSelector, void*);
    /// @}


private:
    /// @brief Rebuilds the entire list
    void rebuildList();

    /** @brief Builds a text representation of the items in the list
     * @return Breakpoints encoded as a string
     */
    std::string encode2TXT();


private:
    /// @brief The list that holds the ids
    MFXAddEditTypedTable* myTable;

    /// @brief The parent window
    GUIMainWindow* myParent;


protected:
    /// FOX needs this
    GUIDialog_Breakpoints() { }

};


#endif

/****************************************************************************/

