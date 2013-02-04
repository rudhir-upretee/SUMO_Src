/****************************************************************************/
/// @file    MSTriggeredXMLReader.cpp
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id: MSTriggeredXMLReader.cpp 13109 2012-12-02 14:49:47Z behrisch $
///
// The basic class for classes that read XML-triggers
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

#include <string>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <utils/common/TplConvert.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/XMLSubSys.h>
#include <microsim/MSEventControl.h>
#include "MSTriggeredReader.h"
#include "MSTriggeredXMLReader.h"
#include <utils/common/WrappingCommand.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
MSTriggeredXMLReader::MSTriggeredXMLReader(MSNet& net,
        const std::string& filename)
    : MSTriggeredReader(net),
      SUMOSAXHandler(filename),
      myParser(0), myHaveMore(true) {
    Command* c = new WrappingCommand< MSTriggeredReader >(this, &MSTriggeredReader::wrappedExecute);
    MSNet::getInstance()->getInsertionEvents().addEvent(c, net.getCurrentTimeStep(), MSEventControl::NO_CHANGE);
}


MSTriggeredXMLReader::~MSTriggeredXMLReader() {}


bool
MSTriggeredXMLReader::readNextTriggered() {
    while (myHaveMore && myParser->parseNext()) {
        if (nextRead()) {
            return true;
        }
    }
    myHaveMore = false;
    return false;
}


void
MSTriggeredXMLReader::myInit() {
    try {
        myParser = XMLSubSys::getSAXReader(*this);
        if (!myParser->parseFirst(getFileName())) {
            throw ProcessError("Can not read XML-file '" + getFileName() + "'.");
        }
    } catch (XERCES_CPP_NAMESPACE::SAXException& e) {
        throw ProcessError(TplConvert::_2str(e.getMessage()));
    } catch (XERCES_CPP_NAMESPACE::XMLException& e) {
        throw ProcessError(TplConvert::_2str(e.getMessage()));
    }

    if (readNextTriggered()) {
        if (myOffset < MSNet::getInstance()->getCurrentTimeStep()) {
            myOffset = MSNet::getInstance()->getCurrentTimeStep() + 1;
            // !!! Warning?
        }
    } else {
        myHaveMore = false;
    }
}



/****************************************************************************/
