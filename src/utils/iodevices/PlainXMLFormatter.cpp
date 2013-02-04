/****************************************************************************/
/// @file    PlainXMLFormatter.cpp
/// @author  Michael Behrisch
/// @date    2012
/// @version $Id: PlainXMLFormatter.cpp 11832 2012-02-06 08:22:14Z bieker $
///
// Static storage of an output device and its base (abstract) implementation
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

#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include "PlainXMLFormatter.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// member method definitions
// ===========================================================================
PlainXMLFormatter::PlainXMLFormatter(const unsigned int defaultIndentation)
    : myDefaultIndentation(defaultIndentation) {
}


bool
PlainXMLFormatter::writeXMLHeader(std::ostream& into, const std::string& rootElement, const std::string xmlParams,
                                  const std::string& attrs, const std::string& comment) {
    if (myXMLStack.empty()) {
        OptionsCont::getOptions().writeXMLHeader(into, xmlParams);
        if (comment != "") {
            into << comment << "\n";
        }
        openTag(into, rootElement);
        if (attrs != "") {
            into << " " << attrs;
        }
        into << ">\n";
        return true;
    }
    return false;
}


void
PlainXMLFormatter::openTag(std::ostream& into, const std::string& xmlElement) {
    into << std::string(4 * (myXMLStack.size() + myDefaultIndentation), ' ') << "<" << xmlElement;
    myXMLStack.push_back(xmlElement);
}


void
PlainXMLFormatter::openTag(std::ostream& into, const SumoXMLTag& xmlElement) {
    openTag(into, toString(xmlElement));
}


void
PlainXMLFormatter::closeOpener(std::ostream& into) {
    into << ">\n";
}


bool
PlainXMLFormatter::closeTag(std::ostream& into, bool abbreviated) {
    if (!myXMLStack.empty()) {
        if (abbreviated) {
            into << "/>\n";
        } else {
            const std::string indent(4 * (myXMLStack.size() + myDefaultIndentation - 1), ' ');
            into << indent << "</" << myXMLStack.back() << ">\n";
        }
        myXMLStack.pop_back();
        return true;
    }
    return false;
}


void
PlainXMLFormatter::writeAttr(std::ostream& into, const std::string& attr, const std::string& val) {
    into << " " << attr << "=\"" << val << "\"";
}

/****************************************************************************/

