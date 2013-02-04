/****************************************************************************/
/// @file    NWFrame.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
/// @version $Id: NWFrame.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// Sets and checks options for netwrite
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
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/SystemFrame.h>
#include <utils/iodevices/OutputDevice.h>
#include <netbuild/NBNetBuilder.h>
#include "NWFrame.h"
#include "NWWriter_SUMO.h"
#include "NWWriter_MATSim.h"
#include "NWWriter_XML.h"
#include "NWWriter_OpenDrive.h"
#include "NWWriter_DlrNavteq.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS

// ===========================================================================
// static members
// ===========================================================================
const std::string NWFrame::MAJOR_VERSION = "version=\"0.13\"";


// ===========================================================================
// method definitions
// ===========================================================================
void
NWFrame::fillOptions(bool forNetgen) {
    OptionsCont& oc = OptionsCont::getOptions();
    // register options
    oc.doRegister("output-file", 'o', new Option_FileName());
    oc.addSynonyme("output-file", "sumo-output");
    oc.addSynonyme("output-file", "output");
    oc.addDescription("output-file", "Output", "The generated net will be written to FILE");

    oc.doRegister("plain-output-prefix", new Option_FileName());
    oc.addSynonyme("plain-output-prefix", "plain-output");
    oc.addSynonyme("plain-output-prefix", "plain");
    oc.addDescription("plain-output-prefix", "Output", "Prefix of files to write plain xml nodes, edges and connections to");

    oc.doRegister("junctions.join-output", new Option_FileName());
    oc.addDescription("junctions.join-output", "Output",
                      "Writes information about joined junctions to FILE (can be loaded as additional node-file to reproduce joins");

#ifdef HAVE_PROJ
    if (!forNetgen) {
        oc.doRegister("proj.plain-geo", new Option_Bool(false));
        oc.addDescription("proj.plain-geo", "Projection", "Write geo coordinates in plain-xml");
    }
#endif // HAVE_PROJ

    oc.doRegister("map-output", 'M', new Option_FileName());
    oc.addDescription("map-output", "Output", "Writes joined edges information to FILE");

    oc.doRegister("matsim-output", new Option_FileName());
    oc.addDescription("matsim-output", "Output", "The generated net will be written to FILE using MATsim format.");

    oc.doRegister("opendrive-output", new Option_FileName());
    oc.addDescription("opendrive-output", "Output", "The generated net will be written to FILE using openDRIVE format.");

    oc.doRegister("dlr-navteq-output", new Option_FileName());
    oc.addDescription("dlr-navteq-output", "Output", "The generated net will be written to dlr-navteq files with the given PREFIX.");

    oc.doRegister("output.street-names", new Option_Bool(false));
    oc.addDescription("output.street-names", "Output", "Street names will be included in the output (if available).");

    oc.doRegister("output.original-names", new Option_Bool(false));
    oc.addDescription("output.original-names", "Output", "Writes original names, if given, as parameter.");

    oc.doRegister("street-sign-output", new Option_FileName());
    oc.addDescription("street-sign-output", "Output", "Writes street signs as POIs to FILE.");
}


bool
NWFrame::checkOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    bool ok = true;
    // check whether the output is valid and can be build
    if (!oc.isSet("output-file")
            && !oc.isSet("plain-output-prefix")
            && !oc.isSet("matsim-output")
            && !oc.isSet("opendrive-output")
            && !oc.isSet("dlr-navteq-output")) {
        oc.set("output-file", "net.net.xml");
    }
    // some outputs need internal lanes
    if (oc.isSet("opendrive-output") && oc.getBool("no-internal-links")) {
        WRITE_ERROR("openDRIVE export needs internal links computation.");
        ok = false;
    }
    return ok;
}


void
NWFrame::writeNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    NWWriter_SUMO::writeNetwork(oc, nb);
    NWWriter_MATSim::writeNetwork(oc, nb);
    NWWriter_OpenDrive::writeNetwork(oc, nb);
    NWWriter_DlrNavteq::writeNetwork(oc, nb);
    NWWriter_XML::writeNetwork(oc, nb);
    // save the mapping information when wished
    if (oc.isSet("map-output")) {
        OutputDevice& mdevice = OutputDevice::getDevice(oc.getString("map-output"));
        mdevice << nb.getJoinedEdgesMap();
        mdevice.close();
    }
}


void
NWFrame::writePositionLong(const Position& pos, OutputDevice& dev) {
    dev.writeAttr(SUMO_ATTR_X, pos.x());
    dev.writeAttr(SUMO_ATTR_Y, pos.y());
    if (pos.z() != 0) {
        dev.writeAttr(SUMO_ATTR_Z, pos.z());
    }
}

/****************************************************************************/

