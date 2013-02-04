/****************************************************************************/
/// @file    TraCIServerAPI_Edge.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Jerome Haerri
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
/// @version $Id: TraCIServerAPI_Edge.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// APIs for getting/setting edge values via TraCI
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

#include <utils/common/StdDefs.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include "TraCIConstants.h"
#include "TraCIServerAPI_Edge.h"
#include <microsim/MSEdgeWeightsStorage.h>
#include <utils/common/HelpersHarmonoise.h>

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
TraCIServerAPI_Edge::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                tcpip::Storage& outputStorage) {
    // variable & id
    int variable = inputStorage.readUnsignedByte();
    std::string id = inputStorage.readString();
    // check variable
    if (variable != ID_LIST && variable != VAR_EDGE_TRAVELTIME && variable != VAR_EDGE_EFFORT && variable != VAR_CURRENT_TRAVELTIME
            && variable != VAR_CO2EMISSION && variable != VAR_COEMISSION && variable != VAR_HCEMISSION && variable != VAR_PMXEMISSION
            && variable != VAR_NOXEMISSION && variable != VAR_FUELCONSUMPTION && variable != VAR_NOISEEMISSION
            && variable != LAST_STEP_VEHICLE_NUMBER && variable != LAST_STEP_MEAN_SPEED && variable != LAST_STEP_OCCUPANCY
            && variable != LAST_STEP_VEHICLE_HALTING_NUMBER && variable != LAST_STEP_LENGTH
            && variable != LAST_STEP_VEHICLE_ID_LIST && variable != ID_COUNT) {
        server.writeStatusCmd(CMD_GET_EDGE_VARIABLE, RTYPE_ERR, "Get Edge Variable: unsupported variable specified", outputStorage);
        return false;
    }
    // begin response building
    tcpip::Storage tempMsg;
    //  response-code, variableID, objectID
    tempMsg.writeUnsignedByte(RESPONSE_GET_EDGE_VARIABLE);
    tempMsg.writeUnsignedByte(variable);
    tempMsg.writeString(id);
    // process request
    if (variable == ID_LIST) {
        std::vector<std::string> ids;
        MSEdge::insertIDs(ids);
        tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
        tempMsg.writeStringList(ids);
    } else if (variable == ID_COUNT) {
        std::vector<std::string> ids;
        MSEdge::insertIDs(ids);
        tempMsg.writeUnsignedByte(TYPE_INTEGER);
        tempMsg.writeInt((int) ids.size());
    } else {
        MSEdge* e = MSEdge::dictionary(id);
        if (e == 0) {
            server.writeStatusCmd(CMD_GET_EDGE_VARIABLE, RTYPE_ERR, "Edge '" + id + "' is not known", outputStorage);
            return false;
        }
        switch (variable) {
            case VAR_EDGE_TRAVELTIME: {
                // time
                if (inputStorage.readUnsignedByte() != TYPE_INTEGER) {
                    server.writeStatusCmd(CMD_GET_EDGE_VARIABLE, RTYPE_ERR, "The message must contain the time definition.", outputStorage);
                    return false;
                }
                SUMOTime time = inputStorage.readInt();
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                SUMOReal value;
                if (!MSNet::getInstance()->getWeightsStorage().retrieveExistingTravelTime(e, 0, time, value)) {
                    tempMsg.writeDouble(-1);
                } else {
                    tempMsg.writeDouble(value);
                }
            }
            break;
            case VAR_EDGE_EFFORT: {
                // time
                if (inputStorage.readUnsignedByte() != TYPE_INTEGER) {
                    server.writeStatusCmd(CMD_GET_EDGE_VARIABLE, RTYPE_ERR, "The message must contain the time definition.", outputStorage);
                    return false;
                }
                SUMOTime time = inputStorage.readInt();
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                SUMOReal value;
                if (!MSNet::getInstance()->getWeightsStorage().retrieveExistingEffort(e, 0, time, value)) {
                    tempMsg.writeDouble(-1);
                } else {
                    tempMsg.writeDouble(value);
                }
            }
            break;
            case VAR_CURRENT_TRAVELTIME:
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(e->getCurrentTravelTime());
                break;
            case LAST_STEP_VEHICLE_ID_LIST: {
                std::vector<std::string> vehIDs;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    const std::deque<MSVehicle*>& vehs = (*i)->getVehiclesSecure();
                    for (std::deque<MSVehicle*>::const_iterator j = vehs.begin(); j != vehs.end(); ++j) {
                        vehIDs.push_back((*j)->getID());
                    }
                    (*i)->releaseVehicles();
                }
                tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
                tempMsg.writeStringList(vehIDs);
            }
            break;
            case VAR_CO2EMISSION: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getHBEFA_CO2Emissions();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum);
            }
            break;
            case VAR_COEMISSION: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getHBEFA_COEmissions();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum);
            }
            break;
            case VAR_HCEMISSION: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getHBEFA_HCEmissions();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum);
            }
            break;
            case VAR_PMXEMISSION: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getHBEFA_PMxEmissions();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum);
            }
            break;
            case VAR_NOXEMISSION: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getHBEFA_NOxEmissions();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum);
            }
            break;
            case VAR_FUELCONSUMPTION: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getHBEFA_FuelConsumption();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum);
            }
            break;
            case VAR_NOISEEMISSION: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (SUMOReal) pow(10., ((*i)->getHarmonoise_NoiseEmissions() / 10.));
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                if (sum != 0) {
                    tempMsg.writeDouble(HelpersHarmonoise::sum(sum));
                } else {
                    tempMsg.writeDouble(0);
                }
            }
            break;
            case LAST_STEP_VEHICLE_NUMBER: {
                int sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getVehicleNumber();
                }
                tempMsg.writeUnsignedByte(TYPE_INTEGER);
                tempMsg.writeInt(sum);
            }
            break;
            case LAST_STEP_MEAN_SPEED: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getMeanSpeed();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum / (SUMOReal) lanes.size());
            }
            break;
            case LAST_STEP_OCCUPANCY: {
                SUMOReal sum = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    sum += (*i)->getOccupancy();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                tempMsg.writeDouble(sum / (SUMOReal) lanes.size());
            }
            break;
            case LAST_STEP_VEHICLE_HALTING_NUMBER: {
                int halting = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    const std::deque<MSVehicle*>& vehs = (*i)->getVehiclesSecure();
                    for (std::deque<MSVehicle*>::const_iterator j = vehs.begin(); j != vehs.end(); ++j) {
                        if ((*j)->getSpeed() < 0.1) {
                            ++halting;
                        }
                    }
                    (*i)->releaseVehicles();
                }
                tempMsg.writeUnsignedByte(TYPE_INTEGER);
                tempMsg.writeInt(halting);
            }
            break;
            case LAST_STEP_LENGTH: {
                SUMOReal lengthSum = 0;
                int noVehicles = 0;
                const std::vector<MSLane*>& lanes = e->getLanes();
                for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                    const std::deque<MSVehicle*>& vehs = (*i)->getVehiclesSecure();
                    for (std::deque<MSVehicle*>::const_iterator j = vehs.begin(); j != vehs.end(); ++j) {
                        lengthSum += (*j)->getVehicleType().getLength();
                    }
                    noVehicles += (int) vehs.size();
                    (*i)->releaseVehicles();
                }
                tempMsg.writeUnsignedByte(TYPE_DOUBLE);
                if (noVehicles == 0) {
                    tempMsg.writeDouble(0);
                } else {
                    tempMsg.writeDouble(lengthSum / (SUMOReal) noVehicles);
                }
            }
            break;
            default:
                break;
        }
    }
    server.writeStatusCmd(CMD_GET_EDGE_VARIABLE, RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, tempMsg);
    return true;
}


bool
TraCIServerAPI_Edge::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    int variable = inputStorage.readUnsignedByte();
    if (variable != VAR_EDGE_TRAVELTIME && variable != VAR_EDGE_EFFORT && variable != VAR_MAXSPEED) {
        server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "Change Edge State: unsupported variable specified", outputStorage);
        return false;
    }
    // id
    std::string id = inputStorage.readString();
    MSEdge* e = MSEdge::dictionary(id);
    if (e == 0) {
        server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "Edge '" + id + "' is not known", outputStorage);
        return false;
    }
    // process
    int valueDataType = inputStorage.readUnsignedByte();
    switch (variable) {
        case LANE_ALLOWED: {
            if (inputStorage.readUnsignedByte() != TYPE_STRINGLIST) {
                server.writeStatusCmd(CMD_GET_EDGE_VARIABLE, RTYPE_ERR, "Allowed vehicle classes must be given as a list of strings.", outputStorage);
                return false;
            }
            SVCPermissions permissions = parseVehicleClasses(inputStorage.readStringList());
            const std::vector<MSLane*>& lanes = e->getLanes();
            for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                (*i)->setPermissions(permissions);
            }
            e->rebuildAllowedLanes();
        }
        break;
        case LANE_DISALLOWED: {
            // time
            if (inputStorage.readUnsignedByte() != TYPE_STRINGLIST) {
                server.writeStatusCmd(CMD_GET_EDGE_VARIABLE, RTYPE_ERR, "Not allowed vehicle classes must be given as a list of strings.", outputStorage);
                return false;
            }
            SVCPermissions permissions = ~parseVehicleClasses(inputStorage.readStringList()); // negation yields allowed
            const std::vector<MSLane*>& lanes = e->getLanes();
            for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                (*i)->setPermissions(permissions);
            }
            e->rebuildAllowedLanes();
        }
        break;
        case VAR_EDGE_TRAVELTIME: {
            if (valueDataType != TYPE_COMPOUND) {
                server.writeStatusCmd(CMD_SET_VEHICLE_VARIABLE, RTYPE_ERR, "Setting travel time requires a compound object.", outputStorage);
                return false;
            }
            int parameterCount = inputStorage.readInt();
            if (parameterCount == 3) {
                // begin
                if (inputStorage.readUnsignedByte() != TYPE_INTEGER) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The first variable must be the begin time given as int.", outputStorage);
                    return false;
                }
                SUMOTime begTime = inputStorage.readInt();
                // end
                if (inputStorage.readUnsignedByte() != TYPE_INTEGER) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The second variable must be the end time given as int.", outputStorage);
                    return false;
                }
                SUMOTime endTime = inputStorage.readInt();
                // value
                if (inputStorage.readUnsignedByte() != TYPE_DOUBLE) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The second variable must be the value given as double", outputStorage);
                    return false;
                }
                SUMOReal value = inputStorage.readDouble();
                // set
                MSNet::getInstance()->getWeightsStorage().addTravelTime(e, begTime, endTime, value);
            } else if (parameterCount == 1) {
                // value
                if (inputStorage.readUnsignedByte() != TYPE_DOUBLE) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The variable must be the value given as double", outputStorage);
                    return false;
                }
                SUMOReal value = inputStorage.readDouble();
                // set
                MSNet::getInstance()->getWeightsStorage().addTravelTime(e, 0, SUMOTime_MAX, value);
            } else {
                server.writeStatusCmd(CMD_SET_VEHICLE_VARIABLE, RTYPE_ERR, "Setting travel time requires either begin time, end time, and value, or only value as parameter.", outputStorage);
                return false;
            }
        }
        break;
        case VAR_EDGE_EFFORT: {
            if (valueDataType != TYPE_COMPOUND) {
                server.writeStatusCmd(CMD_SET_VEHICLE_VARIABLE, RTYPE_ERR, "Setting effort requires a compound object.", outputStorage);
                return false;
            }
            int parameterCount = inputStorage.readInt();
            if (parameterCount == 3) {
                // begin
                if (inputStorage.readUnsignedByte() != TYPE_INTEGER) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The first variable must be the begin time given as int.", outputStorage);
                    return false;
                }
                SUMOTime begTime = inputStorage.readInt();
                // end
                if (inputStorage.readUnsignedByte() != TYPE_INTEGER) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The second variable must be the end time given as int.", outputStorage);
                    return false;
                }
                SUMOTime endTime = inputStorage.readInt();
                // value
                if (inputStorage.readUnsignedByte() != TYPE_DOUBLE) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The second variable must be the value given as double", outputStorage);
                    return false;
                }
                SUMOReal value = inputStorage.readDouble();
                // set
                MSNet::getInstance()->getWeightsStorage().addEffort(e, begTime, endTime, value);
            } else if (parameterCount == 1) {
                // value
                if (inputStorage.readUnsignedByte() != TYPE_DOUBLE) {
                    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The variable must be the value given as double", outputStorage);
                    return false;
                }
                SUMOReal value = inputStorage.readDouble();
                // set
                MSNet::getInstance()->getWeightsStorage().addEffort(e, 0, SUMOTime_MAX, value);
            } else {
                server.writeStatusCmd(CMD_SET_VEHICLE_VARIABLE, RTYPE_ERR, "Setting effort requires either begin time, end time, and value, or only value as parameter.", outputStorage);
                return false;
            }
        }
        break;
        case VAR_MAXSPEED: {
            // speed
            if (valueDataType != TYPE_DOUBLE) {
                server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_ERR, "The speed must be given as a double.", outputStorage);
                return false;
            }
            SUMOReal val = inputStorage.readDouble();
            const std::vector<MSLane*>& lanes = e->getLanes();
            for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
                (*i)->setMaxSpeed(val);
            }
        }
        break;
        default:
            break;
    }
    server.writeStatusCmd(CMD_SET_EDGE_VARIABLE, RTYPE_OK, warning, outputStorage);
    return true;
}


bool
TraCIServerAPI_Edge::getShape(const std::string& id, PositionVector& shape) {
    MSEdge* e = MSEdge::dictionary(id);
    if (e == 0) {
        return false;
    }
    const std::vector<MSLane*>& lanes = e->getLanes();
    shape.push_back(lanes.front()->getShape());
    if (lanes.size() > 1) {
        shape.push_back(lanes.back()->getShape().reverse());
    }
    return true;
}


TraCIRTree*
TraCIServerAPI_Edge::getTree() {
    TraCIRTree* t = new TraCIRTree();
    const std::vector<MSEdge*>& edges = MSNet::getInstance()->getEdgeControl().getEdges();
    for (std::vector<MSEdge*>::const_iterator i = edges.begin(); i != edges.end(); ++i) {
        const std::vector<MSLane*>& lanes = (*i)->getLanes();
        Boundary b;
        for (std::vector<MSLane*>::const_iterator j = lanes.begin(); j != lanes.end(); ++j) {
            b.add((*j)->getShape().getBoxBoundary());
        }
        t->addObject(*i, b);
    }
    return t;
}

#endif


/****************************************************************************/

