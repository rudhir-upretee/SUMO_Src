/****************************************************************************/
/// @file    TraCIServerAPI_InductionLoop.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    07.05.2009
/// @version $Id: TraCIServerAPI_InductionLoop.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// APIs for getting/setting induction loop values via TraCI
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

#ifndef NO_TRACI

#include "TraCIConstants.h"
#include <microsim/MSNet.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/output/MSInductLoop.h>
#include "TraCIServerAPI_InductionLoop.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace traci;


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_InductionLoop::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
        tcpip::Storage& outputStorage) {
    // variable & id
    int variable = inputStorage.readUnsignedByte();
    std::string id = inputStorage.readString();
    // check variable
    if (variable != ID_LIST && variable != LAST_STEP_VEHICLE_NUMBER && variable != LAST_STEP_MEAN_SPEED
            && variable != LAST_STEP_VEHICLE_ID_LIST && variable != LAST_STEP_OCCUPANCY
            && variable != LAST_STEP_LENGTH && variable != LAST_STEP_TIME_SINCE_DETECTION
            && variable != LAST_STEP_VEHICLE_DATA && variable != ID_COUNT
            && variable != VAR_POSITION && variable != VAR_LANE_ID) {
        server.writeStatusCmd(CMD_GET_INDUCTIONLOOP_VARIABLE, RTYPE_ERR, "Get Induction Loop Variable: unsupported variable specified", outputStorage);
        return false;
    }
    // begin response building
    tcpip::Storage tempMsg;
    //  response-code, variableID, objectID
    tempMsg.writeUnsignedByte(RESPONSE_GET_INDUCTIONLOOP_VARIABLE);
    tempMsg.writeUnsignedByte(variable);
    tempMsg.writeString(id);
    // process request
    if (variable == ID_LIST) {
        std::vector<std::string> ids;
        MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).insertIDs(ids);
        tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
        tempMsg.writeStringList(ids);
    } else if (variable == ID_COUNT) {
        std::vector<std::string> ids;
        MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).insertIDs(ids);
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt((int) ids.size());
    } else {
        MSInductLoop* il = static_cast<MSInductLoop*>(MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).get(id));
        if (il == 0) {
            server.writeStatusCmd(CMD_GET_INDUCTIONLOOP_VARIABLE, RTYPE_ERR, "Induction loop '" + id + "' is not known", outputStorage);
            return false;
        }
        switch (variable) {
            case ID_LIST:
                break;
            case LAST_STEP_VEHICLE_NUMBER:
                tempMsg.writeUnsignedByte(TYPE_INTEGER);
                tempMsg.writeInt((int)(il->getCurrentPassedNumber()));
                break;
            case LAST_STEP_MEAN_SPEED:
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(il->getCurrentSpeed());
                break;
            case LAST_STEP_VEHICLE_ID_LIST: {
                tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
                std::vector<std::string> ids = il->getCurrentVehicleIDs();
                tempMsg.writeStringList(ids);
            }
            break;
            case LAST_STEP_OCCUPANCY:
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(il->getCurrentOccupancy());
                break;
            case LAST_STEP_LENGTH:
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(il->getCurrentLength());
                break;
            case LAST_STEP_TIME_SINCE_DETECTION:
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(il->getTimestepsSinceLastDetection());
                break;
            case LAST_STEP_VEHICLE_DATA: {
                std::vector<MSInductLoop::VehicleData> vd = il->collectVehiclesOnDet(MSNet::getInstance()->getCurrentTimeStep() - DELTA_T);
                tempMsg.writeUnsignedByte(TYPE_COMPOUND);
                tcpip::Storage tempContent;
                unsigned int cnt = 0;
                tempContent.writeUnsignedByte(TYPE_INTEGER);
                tempContent.writeInt((int) vd.size());
                ++cnt;
                for (unsigned int i = 0; i < vd.size(); ++i) {
                    MSInductLoop::VehicleData& svd = vd[i];
                    tempContent.writeUnsignedByte(TYPE_STRING);
                    tempContent.writeString(svd.idM);
                    ++cnt;
                    tempContent.writeUnsignedByte(TYPE_DOUBLE);
                    tempContent.writeDouble(svd.lengthM);
                    ++cnt;
                    tempContent.writeUnsignedByte(TYPE_DOUBLE);
                    tempContent.writeDouble(svd.entryTimeM);
                    ++cnt;
                    tempContent.writeUnsignedByte(TYPE_DOUBLE);
                    tempContent.writeDouble(svd.leaveTimeM);
                    ++cnt;
                    tempContent.writeUnsignedByte(TYPE_STRING);
                    tempContent.writeString(svd.typeIDM);
                    ++cnt;
                }

                tempMsg.writeInt((int) cnt);
                tempMsg.writeStorage(tempContent);
                break;
            }
            case VAR_POSITION:
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(il->getPosition());
                break;
            case VAR_LANE_ID:
                tempMsg.writeUnsignedByte(TYPE_STRING);
                tempMsg.writeString(il->getLane()->getID());
                break;
            default:
                break;
        }
    }
    server.writeStatusCmd(CMD_GET_INDUCTIONLOOP_VARIABLE, RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, tempMsg);
    return true;
}


bool
TraCIServerAPI_InductionLoop::getPosition(const std::string& id, Position& p) {
    MSInductLoop* il = static_cast<MSInductLoop*>(MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).get(id));
    if (il == 0) {
        return false;
    }
    p = il->getLane()->getShape().positionAtLengthPosition(il->getPosition());
    return true;
}


TraCIRTree*
TraCIServerAPI_InductionLoop::getTree() {
    TraCIRTree* t = new TraCIRTree();
    const std::map<std::string, MSDetectorFileOutput*>& dets = MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).getMyMap();
    for (std::map<std::string, MSDetectorFileOutput*>::const_iterator i = dets.begin(); i != dets.end(); ++i) {
        MSInductLoop* il = static_cast<MSInductLoop*>((*i).second);
        Position p = il->getLane()->getShape().positionAtLengthPosition(il->getPosition());
        t->addObject(il, p);
    }
    return t;
}

#endif


/****************************************************************************/

