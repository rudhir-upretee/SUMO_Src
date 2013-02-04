/****************************************************************************/
/// @file    GUISettingsHandler.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Fri, 24. Apr 2009
/// @version $Id: GUISettingsHandler.h 12502 2012-08-06 05:22:55Z behrisch $
///
// The handler for parsing gui settings from xml.
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
#ifndef GUISettingsHandler_h
#define GUISettingsHandler_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/XMLSubSys.h>


// ===========================================================================
// class definitions
// ===========================================================================
/** @class GUISettingsHandler
 * @brief An XML-handler for visualisation schemes
 */
class GUISettingsHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     * @param[in] file the file to parse
     */
    GUISettingsHandler(const std::string& content, bool isFile = true);


    /// @brief Destructor
    ~GUISettingsHandler();



    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element, const SUMOSAXAttributes& attrs);
    //@}



    /** @brief Adds the parsed settings to the global list of settings
     * @return the name of the parsed settings
     */
    std::string addSettings(GUISUMOAbstractView* view = 0) const;


    /** @brief Sets the viewport which has been parsed
     * @param[in] parent the view for which the viewport has to be set
     */
    void setViewport(GUISUMOAbstractView* view) const;


    /** @brief Sets the viewport which has been parsed
     * @param[out] zoom Variable to store the loaded zoom into
     * @param[out] xoff Variable to store the loaded x-offset into
     * @param[out] yoff Variable to store the loaded y-offset into
     */
    void setViewport(SUMOReal& zoom, SUMOReal& xoff, SUMOReal& yoff) const;


    /** @brief Makes a snapshot if it has been parsed
     * @param[in] parent the view which needs to be shot
     * @todo Please describe why the snapshots are only set if no other existed before (see code)
     */
    void setSnapshots(GUISUMOAbstractView* view) const;


    /** @brief Returns whether any decals have been parsed
     * @return whether decals have been parsed
     */
    bool hasDecals() const;


    /** @brief Returns the parsed decals
     * @return the parsed decals
     */
    const std::vector<GUISUMOAbstractView::Decal>& getDecals() const;


    /** @brief Returns the parsed delay
     * @return the parsed delay
     */
    SUMOReal getDelay() const;


private:
    /// @brief The settings to fill
    GUIVisualizationSettings mySettings;

    /// @brief The delay loaded
    SUMOReal myDelay;

    /// @brief The viewport loaded
    SUMOReal myZoom, myXPos, myYPos;

    /// @brief mappig of time steps to filenames for potential snapshots
    std::map<SUMOTime, std::string> mySnapshots;

    /// @brief The decals list to fill
    std::vector<GUISUMOAbstractView::Decal> myDecals;

    /// @brief The last color scheme category (edges or vehicles)
    int myCurrentColorer;

    /// @brief The current color scheme
    GUIColorScheme* myCurrentScheme;

private:

    /// @brief parse combined settings of bool, size and color
    GUIVisualizationTextSettings parseTextSettings(
        const std::string& prefix, const SUMOSAXAttributes& attrs,
        GUIVisualizationTextSettings defaults);

};

#endif

/****************************************************************************/
