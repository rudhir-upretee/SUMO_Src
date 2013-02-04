/****************************************************************************/
/// @file    TraCIServerAPI_Polygon.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Sept 2002
/// @version $Id: TraCIServerAPI_Polygon.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// APIs for getting/setting polygon values via TraCI
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
#include <utils/shapes/Polygon.h>
#include <utils/shapes/ShapeContainer.h>
#include "TraCIConstants.h"
#include "TraCIServerAPI_Polygon.h"

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
TraCIServerAPI_Polygon::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                   tcpip::Storage& outputStorage) {
    // variable & id
    int variable = inputStorage.readUnsignedByte();
    std::string id = inputStorage.readString();
    // check variable
    if (variable != ID_LIST && variable != VAR_TYPE && variable != VAR_COLOR && variable != VAR_SHAPE && variable != VAR_FILL
            && variable != ID_COUNT) {
        server.writeStatusCmd(CMD_GET_POLYGON_VARIABLE, RTYPE_ERR, "Get Polygon Variable: unsupported variable specified", outputStorage);
        return false;
    }
    // begin response building
    tcpip::Storage tempMsg;
    //  response-code, variableID, objectID
    tempMsg.writeUnsignedByte(RESPONSE_GET_POLYGON_VARIABLE);
    tempMsg.writeUnsignedByte(variable);
    tempMsg.writeString(id);
    // process request
    if (variable == ID_LIST || variable == ID_COUNT) {
        std::vector<std::string> ids;
        ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
        shapeCont.getPolygons().insertIDs(ids);
        if (variable == ID_LIST) {
            tempMsg.writeUnsignedByte(TYPE_STRINGLIST);
            tempMsg.writeStringList(ids);
        } else {
            tempMsg.writeUnsignedByte(TYPE_INTEGER);
            tempMsg.writeInt((int) ids.size());
        }
    } else {
        int layer;
        Polygon* p = getPolygon(id, layer);
        if (p == 0) {
            server.writeStatusCmd(CMD_GET_POLYGON_VARIABLE, RTYPE_ERR, "Polygon '" + id + "' is not known", outputStorage);
            return false;
        }
        switch (variable) {
            case VAR_TYPE:
                tempMsg.writeUnsignedByte(TYPE_STRING);
                tempMsg.writeString(p->getType());
                break;
            case VAR_COLOR:
                tempMsg.writeUnsignedByte(TYPE_COLOR);
                tempMsg.writeUnsignedByte(static_cast<int>(p->getColor().red() * 255. + .5));
                tempMsg.writeUnsignedByte(static_cast<int>(p->getColor().green() * 255. + .5));
                tempMsg.writeUnsignedByte(static_cast<int>(p->getColor().blue() * 255. + .5));
                tempMsg.writeUnsignedByte(255);
                break;
            case VAR_SHAPE:
                tempMsg.writeUnsignedByte(TYPE_POLYGON);
                tempMsg.writeUnsignedByte(MIN2(static_cast<int>(255), static_cast<int>(p->getShape().size())));
                for (unsigned int iPoint = 0; iPoint < MIN2(static_cast<size_t>(255), p->getShape().size()); ++iPoint) {
                    tempMsg.writeDouble(p->getShape()[iPoint].x());
                    tempMsg.writeDouble(p->getShape()[iPoint].y());
                }
                break;
            case VAR_FILL:
                tempMsg.writeUnsignedByte(TYPE_UBYTE);
                tempMsg.writeUnsignedByte(p->getFill() ? 1 : 0);
                break;
            default:
                break;
        }
    }
    server.writeStatusCmd(CMD_GET_POLYGON_VARIABLE, RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, tempMsg);
    return true;
}


bool
TraCIServerAPI_Polygon::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                   tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    int variable = inputStorage.readUnsignedByte();
    if (variable != VAR_TYPE && variable != VAR_COLOR && variable != VAR_SHAPE && variable != VAR_FILL
            && variable != ADD && variable != REMOVE) {
        server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "Change Polygon State: unsupported variable specified", outputStorage);
        return false;
    }
    // id
    std::string id = inputStorage.readString();
    Polygon* p = 0;
    int layer = 0;
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    if (variable != ADD && variable != REMOVE) {
        p = getPolygon(id, layer);
        if (p == 0) {
            server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "Polygon '" + id + "' is not known", outputStorage);
            return false;
        }
    }
    // process
    int valueDataType = inputStorage.readUnsignedByte();
    switch (variable) {
        case VAR_TYPE: {
            if (valueDataType != TYPE_STRING) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The type must be given as a string.", outputStorage);
                return false;
            }
            std::string type = inputStorage.readString();
            p->setType(type);
        }
        break;
        case VAR_COLOR: {
            if (valueDataType != TYPE_COLOR) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The color must be given using an accoring type.", outputStorage);
                return false;
            }
            SUMOReal r = (SUMOReal) inputStorage.readUnsignedByte() / 255.;
            SUMOReal g = (SUMOReal) inputStorage.readUnsignedByte() / 255.;
            SUMOReal b = (SUMOReal) inputStorage.readUnsignedByte() / 255.;
            //read SUMOReal a
            inputStorage.readUnsignedByte();
            p->setColor(RGBColor(r, g, b));
        }
        break;
        case VAR_SHAPE: {
            if (valueDataType != TYPE_POLYGON) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The shape must be given using an accoring type.", outputStorage);
                return false;
            }
            unsigned int noEntries = inputStorage.readUnsignedByte();
            PositionVector shape;
            for (unsigned int i = 0; i < noEntries; ++i) {
                SUMOReal x = inputStorage.readDouble();
                SUMOReal y = inputStorage.readDouble();
                shape.push_back(Position(x, y));
            }
            shapeCont.reshapePolygon(id, shape);
        }
        break;
        case VAR_FILL: {
            if (valueDataType != TYPE_UBYTE) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "'fill' must be defined using an unsigned byte.", outputStorage);
                return false;
            }
            bool fill = inputStorage.readUnsignedByte() != 0;
            p->setFill(fill);
        }
        break;
        case ADD: {
            if (valueDataType != TYPE_COMPOUND) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "A compound object is needed for setting a new polygon.", outputStorage);
                return false;
            }
            //readt itemNo
            inputStorage.readInt();
            // type
            if (inputStorage.readUnsignedByte() != TYPE_STRING) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The first polygon parameter must be the type encoded as a string.", outputStorage);
                return false;
            }
            std::string type = inputStorage.readString();
            // color
            if (inputStorage.readUnsignedByte() != TYPE_COLOR) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The second polygon parameter must be the color.", outputStorage);
                return false;
            }
            SUMOReal r = (SUMOReal) inputStorage.readUnsignedByte() / 255.;
            SUMOReal g = (SUMOReal) inputStorage.readUnsignedByte() / 255.;
            SUMOReal b = (SUMOReal) inputStorage.readUnsignedByte() / 255.;
            //read SUMOReal a
            inputStorage.readUnsignedByte();
            // fill
            if (inputStorage.readUnsignedByte() != TYPE_UBYTE) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The third polygon parameter must be 'fill' encoded as ubyte.", outputStorage);
                return false;
            }
            bool fill = inputStorage.readUnsignedByte() != 0;
            // layer
            if (inputStorage.readUnsignedByte() != TYPE_INTEGER) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The fourth polygon parameter must be the layer encoded as int.", outputStorage);
                return false;
            }
            layer = inputStorage.readInt();
            // shape
            if (inputStorage.readUnsignedByte() != TYPE_POLYGON) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The fifth polygon parameter must be the shape.", outputStorage);
                return false;
            }
            unsigned int noEntries = inputStorage.readUnsignedByte();
            PositionVector shape;
            for (unsigned int i = 0; i < noEntries; ++i) {
                SUMOReal x = inputStorage.readDouble();
                SUMOReal y = inputStorage.readDouble();
                shape.push_back(Position(x, y));
            }
            //
            if (!shapeCont.addPolygon(id, type, RGBColor(r, g, b), (SUMOReal)layer,
                                      Shape::DEFAULT_ANGLE, Shape::DEFAULT_IMG_FILE, shape, fill)) {
                delete p;
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "Could not add polygon.", outputStorage);
                return false;
            }
        }
        break;
        case REMOVE: {
            if (valueDataType != TYPE_INTEGER) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "The layer must be given using an int.", outputStorage);
                return false;
            }
            layer = inputStorage.readInt();
            if (!shapeCont.removePolygon(id)) {
                server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_ERR, "Could not remove polygon '" + id + "'", outputStorage);
                return false;
            }
        }
        break;
        default:
            break;
    }
    server.writeStatusCmd(CMD_SET_POLYGON_VARIABLE, RTYPE_OK, warning, outputStorage);
    return true;
}


bool
TraCIServerAPI_Polygon::getShape(const std::string& id, PositionVector& shape) {
    int layer;
    Polygon* poly = getPolygon(id, layer);
    if (poly == 0) {
        return false;
    }
    shape.push_back(poly->getShape());
    return true;
}


Polygon*
TraCIServerAPI_Polygon::getPolygon(const std::string& id, int& layer) {
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    return shapeCont.getPolygons().get(id);
}


TraCIRTree*
TraCIServerAPI_Polygon::getTree() {
    TraCIRTree* t = new TraCIRTree();
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    const std::map<std::string, Polygon*>& polygons = shapeCont.getPolygons().getMyMap();
    for (std::map<std::string, Polygon*>::const_iterator i = polygons.begin(); i != polygons.end(); ++i) {
        Boundary b = (*i).second->getShape().getBoxBoundary();
        t->addObject((*i).second, b);
    }
    return t;
}


#endif


/****************************************************************************/

