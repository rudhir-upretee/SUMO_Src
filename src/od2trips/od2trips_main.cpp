/****************************************************************************/
/// @file    od2trips_main.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Thu, 12 September 2002
/// @version $Id: od2trips_main.cpp 13114 2012-12-03 09:15:16Z behrisch $
///
// Main for OD2TRIPS
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

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <algorithm>
#include <math.h>
#include <cstdlib>
#include <string>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/RandHelper.h>
#include <utils/common/ToString.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/StringUtils.h>
#include <od2trips/ODDistrictCont.h>
#include <od2trips/ODDistrictHandler.h>
#include <od2trips/ODMatrix.h>
#include <utils/common/TplConvert.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/SUMOVehicleParameter.h>
#include <utils/importio/LineReader.h>
#include <utils/iodevices/OutputDevice.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// functions
// ===========================================================================
void
fillOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.addCallExample("-c <CONFIGURATION>", "run with configuration file");

    // insert options sub-topics
    SystemFrame::addConfigurationOptions(oc); // fill this subtopic, too
    oc.addOptionSubTopic("Input");
    oc.addOptionSubTopic("Output");
    oc.addOptionSubTopic("Time");
    oc.addOptionSubTopic("Processing");
    oc.addOptionSubTopic("Defaults");
    SystemFrame::addReportOptions(oc); // fill this subtopic, too


    // register the file input options
    oc.doRegister("net-file", 'n', new Option_FileName());
    oc.addSynonyme("net-file", "net");
    oc.addDescription("net-file", "Input", "Loads network (districts) from FILE");

    oc.doRegister("od-matrix-files", 'd', new Option_FileName());
    oc.addSynonyme("od-matrix-files", "od-files");
    oc.addSynonyme("od-matrix-files", "od");
    oc.addDescription("od-matrix-files", "Input", "Loads O/D-files from FILE(s)");


    // register the file output options
    oc.doRegister("output-file", 'o', new Option_FileName());
    oc.addSynonyme("output-file", "output", true);
    oc.addDescription("output-file", "Output", "Writes trip definitions into FILE");

    oc.doRegister("ignore-vehicle-type", new Option_Bool(false));
    oc.addSynonyme("ignore-vehicle-type", "no-vtype", true);
    oc.addDescription("ignore-vehicle-type", "Output", "Does not save vtype information");


    // register the time settings
    oc.doRegister("begin", 'b', new Option_String("0", "TIME"));
    oc.addDescription("begin", "Time", "Defines the begin time; Previous trips will be discarded");

    oc.doRegister("end", 'e', new Option_String(SUMOTIME_MAXSTRING, "TIME"));
    oc.addDescription("end", "Time", "Defines the end time; Later trips will be discarded; Defaults to the maximum time that SUMO can represent");


    // register the data processing options
    oc.doRegister("scale", 's', new Option_Float(1));
    oc.addDescription("scale", "Processing", "Scales the loaded flows by FLOAT");

    oc.doRegister("spread.uniform", new Option_Bool(false));
    oc.addDescription("spread.uniform", "Processing", "Spreads trips uniformly over each time period");

    oc.doRegister("vtype", new Option_String(""));
    oc.addDescription("vtype", "Processing", "Defines the name of the vehicle type to use");

    oc.doRegister("prefix", new Option_String(""));
    oc.addDescription("prefix", "Processing", "Defines the prefix for vehicle names");

    oc.doRegister("timeline", new Option_String());
    oc.addDescription("timeline", "Processing", "Uses STR as a timeline definition");

    oc.doRegister("timeline.day-in-hours", new Option_Bool(false));
    oc.addDescription("timeline.day-in-hours", "Processing", "Uses STR as a 24h-timeline definition");

    oc.doRegister("dismiss-loading-errors", new Option_Bool(false)); // !!! describe, document
    oc.addDescription("dismiss-loading-errors", "Processing", "Continue on broken input");

    oc.doRegister("no-step-log", new Option_Bool(false));
    oc.addDescription("no-step-log", "Processing", "Disable console output of current time step");


    // register defaults options
    oc.doRegister("departlane", new Option_String("free"));
    oc.addDescription("departlane", "Defaults", "Assigns a default depart lane");

    oc.doRegister("departpos", new Option_String());
    oc.addDescription("departpos", "Defaults", "Assigns a default depart position");

    oc.doRegister("departspeed", new Option_String("max"));
    oc.addDescription("departspeed", "Defaults", "Assigns a default depart speed");

    oc.doRegister("arrivallane", new Option_String());
    oc.addDescription("arrivallane", "Defaults", "Assigns a default arrival lane");

    oc.doRegister("arrivalpos", new Option_String());
    oc.addDescription("arrivalpos", "Defaults", "Assigns a default arrival position");

    oc.doRegister("arrivalspeed", new Option_String());
    oc.addDescription("arrivalspeed", "Defaults", "Assigns a default arrival speed");

    // add rand options
    RandHelper::insertRandOptions();
}


Distribution_Points
parseTimeLine(const std::vector<std::string>& def, bool timelineDayInHours) {
    bool interpolating = !timelineDayInHours;
    PositionVector points;
    SUMOReal prob = 0;
    if (timelineDayInHours) {
        if (def.size() != 24) {
            throw ProcessError("Assuming 24 entries for a day timeline, but got " + toString(def.size()) + ".");
        }
        for (int chour = 0; chour < 24; ++chour) {
            prob = TplConvert::_2SUMOReal(def[chour].c_str());
            points.push_back(Position((SUMOReal)(chour * 3600), prob));
        }
        points.push_back(Position((SUMOReal)(24 * 3600), prob));
    } else {
        size_t i = 0;
        while (i < def.size()) {
            StringTokenizer st2(def[i++], ":");
            if (st2.size() != 2) {
                throw ProcessError("Broken time line definition: missing a value in '" + def[i - 1] + "'.");
            }
            int time = TplConvert::_2int(st2.next().c_str());
            prob = TplConvert::_2SUMOReal(st2.next().c_str());
            points.push_back(Position((SUMOReal) time, prob));
        }
    }
    return Distribution_Points("N/A", points, interpolating);
}


bool
checkOptions() {
    OptionsCont& oc = OptionsCont::getOptions();
    bool ok = true;
    if (!oc.isSet("net-file")) {
        WRITE_ERROR("No net input file (-n) specified.");
        ok = false;
    }
    if (!oc.isSet("od-matrix-files")) {
        WRITE_ERROR("No input specified.");
        ok = false;
    }
    if (!oc.isSet("output-file")) {
        WRITE_ERROR("No trip table output file (-o) specified.");
        ok = false;
    }
    //
    SUMOVehicleParameter p;
    std::string error;
    if (oc.isSet("departlane") && !SUMOVehicleParameter::parseDepartLane(oc.getString("departlane"), "option", "departlane", p.departLane, p.departLaneProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("departpos") && !SUMOVehicleParameter::parseDepartPos(oc.getString("departpos"), "option", "departpos", p.departPos, p.departPosProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("departspeed") && !SUMOVehicleParameter::parseDepartSpeed(oc.getString("departspeed"), "option", "departspeed", p.departSpeed, p.departSpeedProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("arrivallane") && !SUMOVehicleParameter::parseArrivalLane(oc.getString("arrivallane"), "option", "arrivallane", p.arrivalLane, p.arrivalLaneProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("arrivalpos") && !SUMOVehicleParameter::parseArrivalPos(oc.getString("arrivalpos"), "option", "arrivalpos", p.arrivalPos, p.arrivalPosProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    if (oc.isSet("arrivalspeed") && !SUMOVehicleParameter::parseArrivalSpeed(oc.getString("arrivalspeed"), "option", "arrivalspeed", p.arrivalSpeed, p.arrivalSpeedProcedure, error)) {
        WRITE_ERROR(error);
        ok = false;
    }
    return ok;
}


void
loadDistricts(ODDistrictCont& districts, OptionsCont& oc) {
    // check whether the user gave a net filename
    if (!oc.isSet("net-file")) {
        WRITE_ERROR("You must supply a network ('-n').");
        return;
    }
    // get the file name and set it
    std::string file = oc.getString("net-file");
    if (!FileHelpers::exists(file)) {
        throw ProcessError("Could not find network '" + file + "' to load.");
    }
    PROGRESS_BEGIN_MESSAGE("Loading districts from '" + file + "'");
    // build the xml-parser and handler
    ODDistrictHandler handler(districts, file);
    if (!XMLSubSys::runParser(handler, file)) {
        PROGRESS_FAILED_MESSAGE();
    } else {
        PROGRESS_DONE_MESSAGE();
    }
}


std::string
getNextNonCommentLine(LineReader& lr) {
    std::string line;
    do {
        line = lr.readLine();
        if (line[0] != '*') {
            return StringUtils::prune(line);
        }
    } while (lr.good() && lr.hasMore());
    throw ProcessError();
}


SUMOTime
parseSingleTime(const std::string& time) {
    if (time.find('.') == std::string::npos) {
        throw OutOfBoundsException();
    }
    std::string hours = time.substr(0, time.find('.'));
    std::string minutes = time.substr(time.find('.') + 1);
    return TIME2STEPS(TplConvert::_2int(hours.c_str()) * 3600 + TplConvert::_2int(minutes.c_str()) * 60);
}


std::pair<SUMOTime, SUMOTime>
readTime(LineReader& lr) {
    std::string line = getNextNonCommentLine(lr);
    try {
        StringTokenizer st(line, StringTokenizer::WHITECHARS);
        SUMOTime begin = parseSingleTime(st.next());
        SUMOTime end = parseSingleTime(st.next());
        if (begin >= end) {
            throw ProcessError("Begin time is larger than end time.");
        }
        return std::make_pair(begin, end);
    } catch (OutOfBoundsException&) {
        throw ProcessError("Broken period definition '" + line + "'.");
    } catch (NumberFormatException&) {
        throw ProcessError("Broken period definition '" + line + "'.");
    }
}


SUMOReal
readFactor(LineReader& lr, SUMOReal scale) {
    std::string line = getNextNonCommentLine(lr);
    SUMOReal factor = -1;
    try {
        factor = TplConvert::_2SUMOReal(line.c_str()) * scale;
    } catch (NumberFormatException&) {
        throw ProcessError("Broken factor: '" + line + "'.");
    }
    return factor;
}



void
readV(LineReader& lr, ODMatrix& into, SUMOReal scale,
      std::string vehType, bool matrixHasVehType) {
    PROGRESS_BEGIN_MESSAGE("Reading matrix '" + lr.getFileName() + "' stored as VMR");
    // parse first defs
    std::string line;
    if (matrixHasVehType) {
        line = getNextNonCommentLine(lr);
        if (vehType == "") {
            vehType = StringUtils::prune(line);
        }
    }

    // parse time
    std::pair<SUMOTime, SUMOTime> times = readTime(lr);
    SUMOTime begin = times.first;
    SUMOTime end = times.second;

    // factor
    SUMOReal factor = readFactor(lr, scale);

    // districts
    line = getNextNonCommentLine(lr);
    int districtNo = TplConvert::_2int(StringUtils::prune(line).c_str());
    // parse district names (normally ints)
    std::vector<std::string> names;
    do {
        line = getNextNonCommentLine(lr);
        StringTokenizer st2(line, StringTokenizer::WHITECHARS);
        while (st2.hasNext()) {
            names.push_back(st2.next());
        }
    } while ((int) names.size() != districtNo);

    // parse the cells
    for (std::vector<std::string>::iterator si = names.begin(); si != names.end(); ++si) {
        std::vector<std::string>::iterator di = names.begin();
        //
        do {
            line = getNextNonCommentLine(lr);
            if (line.length() == 0) {
                continue;
            }
            try {
                StringTokenizer st2(line, StringTokenizer::WHITECHARS);
                while (st2.hasNext()) {
                    assert(di != names.end());
                    SUMOReal vehNumber = TplConvert::_2SUMOReal(st2.next().c_str()) * factor;
                    if (vehNumber != 0) {
                        into.add(vehNumber, begin, end, *si, *di, vehType);
                    }
                    if (di == names.end()) {
                        throw ProcessError("More entries than districts found.");
                    }
                    ++di;
                }
            } catch (NumberFormatException&) {
                throw ProcessError("Not numeric vehicle number in line '" + line + "'.");
            }
            if (!lr.hasMore()) {
                break;
            }
        } while (di != names.end());
    }
    PROGRESS_DONE_MESSAGE();
}


void
readO(LineReader& lr, ODMatrix& into, SUMOReal scale,
      std::string vehType, bool matrixHasVehType) {
    PROGRESS_BEGIN_MESSAGE("Reading matrix '" + lr.getFileName() + "' stored as OR");
    // parse first defs
    std::string line;
    if (matrixHasVehType) {
        line = getNextNonCommentLine(lr);
        int type = TplConvert::_2int(StringUtils::prune(line).c_str());
        if (vehType == "") {
            vehType = toString(type);
        }
    }

    // parse time
    std::pair<SUMOTime, SUMOTime> times = readTime(lr);
    SUMOTime begin = times.first;
    SUMOTime end = times.second;

    // factor
    SUMOReal factor = readFactor(lr, scale);

    // parse the cells
    while (lr.hasMore()) {
        line = getNextNonCommentLine(lr);
        if (line.length() == 0) {
            continue;
        }
        StringTokenizer st2(line, StringTokenizer::WHITECHARS);
        if (st2.size() == 0) {
            continue;
        }
        try {
            std::string sourceD = st2.next();
            std::string destD = st2.next();
            SUMOReal vehNumber = TplConvert::_2SUMOReal(st2.next().c_str()) * factor;
            if (vehNumber != 0) {
                into.add(vehNumber, begin, end, sourceD, destD, vehType);
            }
        } catch (OutOfBoundsException&) {
            throw ProcessError("Missing at least one information in line '" + line + "'.");
        } catch (NumberFormatException&) {
            throw ProcessError("Not numeric vehicle number in line '" + line + "'.");
        }
    }
    PROGRESS_DONE_MESSAGE();
}


void
loadMatrix(OptionsCont& oc, ODMatrix& into) {
    std::vector<std::string> files = oc.getStringVector("od-files");
    //  check
    if (files.size() == 0) {
        throw ProcessError("No files to parse are given.");
    }
    //  parse
    for (std::vector<std::string>::iterator i = files.begin(); i != files.end(); ++i) {
        LineReader lr(*i);
        if (!lr.good()) {
            throw ProcessError("Could not open '" + (*i) + "'.");
        }
        std::string type = lr.readLine();
        // get the type only
        if (type.find(';') != std::string::npos) {
            type = type.substr(0, type.find(';'));
        }
        // parse type-dependant
        if (type.length() > 1 && type[1] == 'V') {
            // process ptv's 'V'-matrices
            if (type.find('N') != std::string::npos) {
                throw ProcessError("'" + *i + "' does not contain the needed information about the time described.");
            }
            readV(lr, into, oc.getFloat("scale"), oc.getString("vtype"), type.find('M') != std::string::npos);
        } else if (type.length() > 1 && type[1] == 'O') {
            // process ptv's 'O'-matrices
            if (type.find('N') != std::string::npos) {
                throw ProcessError("'" + *i + "' does not contain the needed information about the time described.");
            }
            readO(lr, into, oc.getFloat("scale"), oc.getString("vtype"), type.find('M') != std::string::npos);
        } else {
            throw ProcessError("'" + *i + "' uses an unknown matrix type '" + type + "'.");
        }
    }
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char** argv) {
    OptionsCont& oc = OptionsCont::getOptions();
    // give some application descriptions
    oc.setApplicationDescription("Importer of O/D-matrices for the road traffic simulation SUMO.");
    oc.setApplicationName("od2trips", "SUMO od2trips Version " + (std::string)VERSION_STRING);
    int ret = 0;
    try {
        // initialise subsystems
        XMLSubSys::init();
        fillOptions();
        OptionsIO::getOptions(true, argc, argv);
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        XMLSubSys::setValidation(oc.getBool("xml-validation"));
        MsgHandler::initOutputOptions();
        if (!checkOptions()) {
            throw ProcessError();
        }
        RandHelper::initRandGlobal();
        // load the districts
        ODDistrictCont districts;
        loadDistricts(districts, oc);
        if (districts.size() == 0) {
            throw ProcessError("No districts loaded...");
        }
        // load the matrix
        ODMatrix matrix(districts);
        loadMatrix(oc, matrix);
        if (matrix.getNoLoaded() == 0) {
            throw ProcessError("No vehicles loaded...");
        }
        if (MsgHandler::getErrorInstance()->wasInformed() && !oc.getBool("dismiss-loading-errors")) {
            throw ProcessError("Loading failed...");
        }
        WRITE_MESSAGE(toString(matrix.getNoLoaded()) + " vehicles loaded.");
        // apply a curve if wished
        if (oc.isSet("timeline")) {
            matrix.applyCurve(parseTimeLine(oc.getStringVector("timeline"), oc.getBool("timeline.day-in-hours")));
        }
        // write
        if (!OutputDevice::createDeviceByOption("output-file", "trips")) {
            throw ProcessError("No output name is given.");
        }
        OutputDevice& dev = OutputDevice::getDeviceByOption("output-file");
        matrix.write(string2time(oc.getString("begin")), string2time(oc.getString("end")),
                     dev, oc.getBool("spread.uniform"), oc.getBool("ignore-vehicle-type"),
                     oc.getString("prefix"), !oc.getBool("no-step-log"));
        WRITE_MESSAGE(toString(matrix.getNoDiscarded()) + " vehicles discarded.");
        WRITE_MESSAGE(toString(matrix.getNoWritten()) + " vehicles written.");
    } catch (const ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
#ifndef _DEBUG
    } catch (const std::exception& e) {
        if (std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on unknown error).", false);
        ret = 1;
#endif
    }
    SystemFrame::close();
    if (ret == 0) {
        std::cout << "Success." << std::endl;
    }
    return ret;
}



/****************************************************************************/

