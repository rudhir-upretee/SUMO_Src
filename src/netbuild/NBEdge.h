/****************************************************************************/
/// @file    NBEdge.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
/// @version $Id: NBEdge.h 13107 2012-12-02 13:57:34Z behrisch $
///
// The representation of a single edge during network building
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
#ifndef NBEdge_h
#define NBEdge_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <map>
#include <vector>
#include <string>
#include <set>
#include "NBCont.h"
#include <utils/common/Named.h>
#include <utils/common/Parameterised.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/VectorHelper.h>
#include <utils/geom/Bresenham.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Line.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "NBHelpers.h"
#include "NBSign.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class NBConnection;
class NBNodeCont;
class NBEdgeCont;
class OutputDevice;
class GNELane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBEdge
 * @brief The representation of a single edge during network building
 */
class NBEdge : public Named, public Parameterised {
    /** used for the computation of connections to following edges */
    friend class NBEdgeSuccessorBuilder;
    friend class NBEdgeCont;

    /** used for visualization (NETEDIT) */
    friend class GNELane;
    friend class GNEEdge;

public:

    /** @enum EdgeBuildingStep
     * @brief Current state of the edge within the building process
     *
     * As the network is build in a cascaded way, considering loaded
     *  information, a counter holding the current step is needed. This is done
     *  by using this enumeration.
     */
    enum EdgeBuildingStep {
        /// @brief The edge has been loaded and connections shall not be added
        INIT_REJECT_CONNECTIONS,
        /// @brief The edge has been loaded, nothing is computed yet
        INIT,
        /// @brief The relationships between edges are computed/loaded
        EDGE2EDGES,
        /// @brief Lanes to edges - relationships are computed/loaded
        LANES2EDGES,
        /// @brief Lanes to lanes - relationships are computed; should be recheked
        LANES2LANES_RECHECK,
        /// @brief Lanes to lanes - relationships are computed; no recheck is necessary/wished
        LANES2LANES_DONE,
        /// @brief Lanes to lanes - relationships are loaded; no recheck is necessary/wished
        LANES2LANES_USER
    };


    /** @enum Lane2LaneInfoType
    * @brief Modes of setting connections between lanes
    */
    enum Lane2LaneInfoType {
        /// @brief The connection was computed
        L2L_COMPUTED,
        /// @brief The connection was given by the user
        L2L_USER,
        /// @brief The connection was computed and validated
        L2L_VALIDATED
    };


    /** @struct Lane
     * @brief An (internal) definition of a single lane of an edge
     */
    struct Lane {
        Lane(NBEdge* e) :
            speed(e->getSpeed()), permissions(SVCFreeForAll), preferred(0),
            offset(e->getOffset()), width(e->getWidth()) {}
        /// @brief The lane's shape
        PositionVector shape;
        /// @brief The speed allowed on this lane
        SUMOReal speed;
        /// @brief List of vehicle types that are allowed on this lane
        SVCPermissions permissions;
        /// @brief List of vehicle types that are preferred on this lane
        SVCPermissions preferred;
        /// @brief This lane's offset to the intersection begin
        SUMOReal offset;
        /// @brief This lane's width
        SUMOReal width;
        /// @brief An original ID, if given (@todo: is only seldom used, should be stored somewhere else, probably)
        std::string origID;
    };


    /** @struct Connection
     * @brief A structure which describes a connection between edges or lanes
     */
    struct Connection {
        /** @brief Constructor
         * @param[in] fromLane_ The lane the connections starts at
         * @param[in] toEdge_ The edge the connections yields in
         * @param[in] toLane_ The lane the connections yields in
         */
        Connection(int fromLane_, NBEdge* toEdge_, int toLane_)
            : fromLane(fromLane_), toEdge(toEdge_), toLane(toLane_),
              mayDefinitelyPass(false), haveVia(false) { }

        ~Connection() { }


        /// @brief The lane the connections starts at
        int fromLane;
        /// @brief The edge the connections yields in
        NBEdge* toEdge;
        /// @brief The lane the connections yields in
        int toLane;
        /// @brief The id of the traffic light that controls this connection
        std::string tlID;
        /// @brief The index of this connection within the controlling traffic light
        unsigned int tlLinkNo;
        /// @brief Information about being definitely free to drive (on-ramps)
        bool mayDefinitelyPass;


        std::string origID;

        std::string id;
        PositionVector shape;
        SUMOReal vmax;

        bool haveVia;
        std::string viaID;
        SUMOReal viaVmax;
        PositionVector viaShape;

        std::string foeInternalLanes;
        std::string foeIncomingLanes;

    };

    /// Computes the offset from the edge shape on the current segment
    static std::pair<SUMOReal, SUMOReal> laneOffset(const Position& from,
            const Position& to, SUMOReal lanewidth, unsigned int lane,
            size_t noLanes, LaneSpreadFunction lsf, bool leftHand);

    /// @brief unspecified lane width
    static const SUMOReal UNSPECIFIED_WIDTH;
    /// @brief unspecified lane offset
    static const SUMOReal UNSPECIFIED_OFFSET;
    /// @brief no length override given
    static const SUMOReal UNSPECIFIED_LOADED_LENGTH;


public:
    /** @brief Constructor
     *
     * Use this if no edge geometry is given.
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (my be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] width This edge's lane width
     * @param[in] offset Additional offset to the destination node
     * @param[in] streetName The street name (need not be unique)
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @see init
     * @see LaneSpreadFunction
     */
    NBEdge(const std::string& id,
           NBNode* from, NBNode* to, std::string type,
           SUMOReal speed, unsigned int nolanes, int priority,
           SUMOReal width, SUMOReal offset,
           const std::string& streetName = "",
           LaneSpreadFunction spread = LANESPREAD_RIGHT);


    /** @brief Constructor
     *
     * Use this if the edge's geometry is given.
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (may be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] width This edge's lane width
     * @param[in] offset Additional offset to the destination node
     * @param[in] geom The edge's geomatry
     * @param[in] streetName The street name (need not be unique)
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     * @see init
     * @see LaneSpreadFunction
     */
    NBEdge(const std::string& id,
           NBNode* from, NBNode* to, std::string type,
           SUMOReal speed, unsigned int nolanes, int priority,
           SUMOReal width, SUMOReal offset,
           PositionVector geom,
           const std::string& streetName = "",
           LaneSpreadFunction spread = LANESPREAD_RIGHT,
           bool tryIgnoreNodePositions = false);

    /** @brief Constructor
     *
     * Use this to copy attribuets from another edge
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] tpl The template edge to copy attributes from
     */
    NBEdge(const std::string& id,
           NBNode* from, NBNode* to,
           NBEdge* tpl);


    /** @brief Destructor
     */
    ~NBEdge();


    /** @brief Resets initial values
     *
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (may be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] geom The edge's geomatry
     * @param[in] width This edge's lane width
     * @param[in] offset Additional offset to the destination node
     * @param[in] streetName The street name (need not be unique)
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     */
    void reinit(NBNode* from, NBNode* to, const std::string& type,
                SUMOReal speed, unsigned int nolanes, int priority,
                PositionVector geom, SUMOReal width, SUMOReal offset,
                const std::string& streetName,
                LaneSpreadFunction spread = LANESPREAD_RIGHT,
                bool tryIgnoreNodePositions = false);

    /** @brief Resets nodes but keeps all other values the same (used when joining)
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     */
    void reinitNodes(NBNode* from, NBNode* to);


    /// @name Applying offset
    /// @{

    /** @brief Applies an offset to the edge
     * @param[in] xoff The x-offset to apply
     * @param[in] yoff The y-offset to apply
     */
    void reshiftPosition(SUMOReal xoff, SUMOReal yoff);
    /// @}



    /** @brief Marks this edge to be left-handed
     */
    void setLeftHanded() {
        myAmLeftHand = true;
    }


    /// @name Atomar getter methods
    //@{

    /** @brief Returns the number of lanes
     * @returns This edge's number of lanes
     */
    unsigned int getNumLanes() const {
        return (unsigned int) myLanes.size();
    }


    /** @brief Returns the priority of the edge
     * @return This edge's priority
     */
    int getPriority() const {
        return myPriority;
    }


    /** @brief Returns the origin node of the edge
     * @return The node this edge starts at
     */
    NBNode* getFromNode() const {
        return myFrom;
    }


    /** @brief Returns the destination node of the edge
     * @return The node this edge ends at
     */
    NBNode* getToNode() const {
        return myTo;
    }


    /** @brief Returns the angle of the edge
     *
     * The angle is computed within the constructor using NBHelpers::angle
     *
     * @return This edge's angle
     * @see NBHelpers::angle
     */
    SUMOReal getAngle() const {
        return myAngle;
    }


    /** @brief Returns the computed length of the edge
     * @return The edge's computed length
     */
    SUMOReal getLength() const {
        return myLength;
    }


    /** @brief Returns the length was set explicitly or the computed length if it wasn't set
     * @todo consolidate use of myLength and myLoaded length
     * @return The edge's specified length
     */
    SUMOReal getLoadedLength() const {
        return myLoadedLength > 0 ? myLoadedLength : myLength;
    }


    /** @brief Returns whether a length was set explicitly
     * @return Wether the edge's length was specified
     */
    bool hasLoadedLength() const {
        return myLoadedLength > 0;
    }


    /** @brief Returns the speed allowed on this edge
     * @return The maximum speed allowed on this edge
     */
    SUMOReal getSpeed() const {
        return mySpeed;
    }


    /** @brief The building step of this edge
     * @return The current building step for this edge
     * @todo Recheck usage!
     * @see EdgeBuildingStep
     */
    EdgeBuildingStep getStep() const {
        return myStep;
    }


    /** @brief Returns the width of lanes of this edge
     * @return The width of lanes of this edge
     */
    SUMOReal getWidth() const {
        return myWidth;
    }


    /** @brief Returns the street name of this edge
     */
    const std::string& getStreetName() const {
        return myStreetName;
    }

    /** @brief sets the street name of this edge
     */
    void setStreetName(const std::string& name) {
        myStreetName = name;
    }

    /** @brief Returns the offset to the destination node
     * @return The offset to the destination node
     */
    SUMOReal getOffset() const {
        return myOffset;
    }


    /** @brief Returns the type name
     * @return The name of this edge's type
     */
    const std::string& getTypeName() const {
        return myType;
    }


    /** @brief Returns the lane definitions
     * @return The stored lane definitions
     */
    const std::vector<NBEdge::Lane>& getLanes() const {
        return myLanes;
    }
    //@}



    /// @name Edge geometry access and computation
    //@{

    /** @brief Returns the geometry of the edge
     * @return The edge's geometry
     */
    const PositionVector& getGeometry() const {
        return myGeom;
    }


    /** @brief Returns the geometry of the edge without the endpoints */
    const PositionVector getInnerGeometry() const;


    /** @brief Returns whether the geometry consists only of the node positions
     */
    bool hasDefaultGeometry() const;


    /** @brief Returns whether the geometry is terminated by the node positions
     * This default may be violated by initializing with
     * tryIgnoreNodePositions=true' or with setGeometry()
     * non-default endpoints are useful to control the generated node shape
     */
    bool hasDefaultGeometryEndpoints() const;


    /** @brief (Re)sets the edge's geometry
     *
     * Replaces the edge's prior geometry by the given. Then, computes
     *  the geometries of all lanes using computeLaneShapes.
     * Definitely not the best way to have it accessable from outside...
     * @param[in] g The edge's new geometry
     * @param[in] inner whether g should be interpreted as inner points
     * @todo Recheck usage, disallow access
     * @see computeLaneShapes
     */
    void setGeometry(const PositionVector& g, bool inner = false);


    /** @brief Adds a further geometry point
     *
     * Some importer do not know an edge's geometry when it is initialised.
     *  This method allows to insert further geometry points after the edge
     *  has been built.
     *
     * @param[in] index The position at which the point shall be added
     * @param[in] p The point to add
     */
    void addGeometryPoint(int index, const Position& p);


    /** @brief Recomputeds the lane shapes to terminate at the node shape
     * For every lane the intersection with the fromNode and toNode is
     * calculated and the lane shorted accordingly. The edge length is then set
     * to the average of all lane lenghts (which may differ). This average length is used as the lane
     * length when writing the network.
     * @note All lanes of an edge in a sumo net must have the same nominal length
     *  but may differ in actual geomtric length.
     * @note Depends on previous call to NBNodeCont::computeNodeShapes
     */
    void computeEdgeShape();


    /** @brief Returns the shape of the nth lane
     * @return The shape of the lane given by its index (counter from right)
     */
    const PositionVector& getLaneShape(unsigned int i) const;


    /** @brief (Re)sets how the lanes lateral offset shall be computed
     * @param[in] spread The type of lateral offset to apply
     * @see LaneSpreadFunction
     */
    void setLaneSpreadFunction(LaneSpreadFunction spread);


    /** @brief Returns how this edge's lanes' lateral offset is computed
     * @return The type of lateral offset that is applied on this edge
     * @see LaneSpreadFunction
     */
    LaneSpreadFunction getLaneSpreadFunction() const {
        return myLaneSpreadFunction;
    }


    /** @brief Splits this edge at geometry points
     * @param[in] ec The edge cont to add new edges to
     * @param[in] nc The node cont to add new nodes to
     * @return Whether the geometry was changed
     */
    bool splitGeometry(NBEdgeCont& ec, NBNodeCont& nc);
    //@}



    /// @name Setting and getting connections
    /// @{

    /** @brief Adds a connection to another edge
     *
     * If the given edge does not start at the node this edge ends on, false is returned.
     *
     * All other cases return true. Though, a connection may not been added if this edge
     *  is in step "INIT_REJECT_CONNECTIONS". Also, this method assures that a connection
     *  to an edge is set only once, no multiple connections to next edge are stored.
     *
     * After a first connection to an edge was set, the process step is set to "EDGE2EDGES".
     * @note Passing 0 implicitly removes all existing connections
     *
     * @param[in] dest The connection's destination edge
     * @return Whether the connection was valid
     */
    bool addEdge2EdgeConnection(NBEdge* dest);


    /** @brief Adds a connection between the specified this edge's lane and an approached one
     *
     * If the given edge does not start at the node this edge ends on, false is returned.
     *
     * All other cases return true. Though, a connection may not been added if this edge
     *  is in step "INIT_REJECT_CONNECTIONS". Before the lane-to-lane connection is set,
     *  a connection between edges is established using "addEdge2EdgeConnection". Then,
     *  "setConnection" is called for inserting the lane-to-lane connection.
     *
     * @param[in] fromLane The connection's starting lane (of this edge)
     * @param[in] dest The connection's destination edge
     * @param[in] toLane The connection's destination lane
     * @param[in] type The connections's type
     * @param[in] mayUseSameDestination Whether this connection may be set though connecting an already connected lane
     * @param[in] mayDefinitelyPass Whether this connection is definitely undistrubed (special case for on-ramps)
     * @return Whether the connection was added / exists
     * @see addEdge2EdgeConnection
     * @see setConnection
     * @todo Check difference between "setConnection" and "addLane2LaneConnection"
     */
    bool addLane2LaneConnection(unsigned int fromLane, NBEdge* dest,
                                unsigned int toLane, Lane2LaneInfoType type,
                                bool mayUseSameDestination = false,
                                bool mayDefinitelyPass = false);


    /** @brief Builds no connections starting at the given lanes
     *
     * If "invalidatePrevious" is true, a call to "invalidateConnections(true)" is done.
     * This method loops through the given connections to set, calling "addLane2LaneConnection"
     *  for each.
     *
     * @param[in] fromLane The first of the connections' starting lanes (of this edge)
     * @param[in] dest The connections' destination edge
     * @param[in] toLane The first of the connections' destination lanes
     * @param[in] no The number of connections to set
     * @param[in] type The connections' type
     * @param[in] invalidatePrevious Whether previously set connection shall be deleted
     * @param[in] mayDefinitelyPass Whether these connections are definitely undistrubed (special case for on-ramps)
     * @return Whether the connections were added / existed
     * @see addLane2LaneConnection
     * @see invalidateConnections
     */
    bool addLane2LaneConnections(unsigned int fromLane,
                                 NBEdge* dest, unsigned int toLane, unsigned int no,
                                 Lane2LaneInfoType type, bool invalidatePrevious = false,
                                 bool mayDefinitelyPass = false);


    /** @brief Adds a connection to a certain lane of a certain edge
     *
     * @param[in] lane The connection's starting lane (of this edge)
     * @param[in] destEdge The connection's destination edge
     * @param[in] destLane The connection's destination lane
     * @param[in] type The connections's type
     * @param[in] mayUseSameDestination Whether this connection may be set though connecting an already connected lane
     * @param[in] mayDefinitelyPass Whether this connection is definitely undistrubed (special case for on-ramps)
     * @todo Check difference between "setConnection" and "addLane2LaneConnection"
     */
    void setConnection(unsigned int lane, NBEdge* destEdge,
                       unsigned int destLane,
                       Lane2LaneInfoType type,
                       bool mayUseSameDestination = false,
                       bool mayDefinitelyPass = false);



    /** @brief Returns connections from a given lane
     *
     * This method goes through "myConnections" and copies those which are
     *  starting at the given lane.
     * @param[in] lane The lane which connections shall be returned
     * @return The connections from the given lane
     * @see NBEdge::Connection
     */
    std::vector<Connection> getConnectionsFromLane(unsigned int lane) const;


    /** @brief Retrieves info about a connection to a certain lane of a certain edge
     *
     * Turnaround edge is ignored!
     * @param[in] destEdge The connection's destination edge
     * @param[in] destLane The connection's destination lane
     * @return whether a connection to the specified lane exists
     */
    bool hasConnectionTo(NBEdge* destEdge, unsigned int destLane) const;


    /** @brief Returns the information whethe a connection to the given edge has been added (or computed)
     *
     * Turnaround edge is not ignored!
     * @param[in] e The destination edge
     * @return Whether a connection to the specified edge exists
     */
    bool isConnectedTo(NBEdge* e);


    /** @brief Returns the connections
     * @return This edge's connections to following edges
     */
    const std::vector<Connection>& getConnections() const {
        return myConnections;
    }


    /** @brief Returns the connections
     * @return This edge's connections to following edges
     */
    std::vector<Connection>& getConnections() {
        return myConnections;
    }


    /** @brief Returns the list of outgoing edges without the turnaround sorted in clockwise direction
     * @return Connected edges, sorted clockwise
     */
    const EdgeVector* getConnectedSorted();


    /** @brief Returns the list of outgoing edges unsorted
     * @return Connected edges
     */
    EdgeVector getConnectedEdges() const;


    /** @brief Returns the list of lanes that may be used to reach the given edge
     * @return Lanes approaching the given edge
     */
    std::vector<int> getConnectionLanes(NBEdge* currentOutgoing) const;


    /** @brief sorts the outgoing connections by their angle relative to their junction
     */
    void sortOutgoingConnectionsByAngle();


    /** @brief sorts the outgoing connections by their from-lane-index and their to-lane-index
     */
    void sortOutgoingConnectionsByIndex();


    /** @brief Remaps the connection in a way that allows the removal of it
     *
     * This edge (which is a self loop edge, in fact) connections are spread over the valid incoming edges
     * @todo recheck!
     */
    void remapConnections(const EdgeVector& incoming);


    /** @brief Removes the specified connection(s)
     * @param[in] toEdge The destination edge
     * @param[in] fromLane The lane from which connections shall be removed; -1 means remove all
     * @param[in] toLane   The lane to which connections shall be removed; -1 means remove all
     */
    void removeFromConnections(NBEdge* toEdge, int fromLane = -1, int toLane = -1);

    void invalidateConnections(bool reallowSetting = false);

    void replaceInConnections(NBEdge* which, NBEdge* by, unsigned int laneOff);
    void replaceInConnections(NBEdge* which, const std::vector<NBEdge::Connection>& origConns);
    void copyConnectionsFrom(NBEdge* src);
    /// @}



    /** @brief Returns whether the given edge is the opposite direction to this edge
     * @param[in] n The node at which this may be turnaround direction
     * @param[in] edge The edge which may be the turnaround direction
     * @return Whether the given edge is this edge's turnaround direction
     */
    bool isTurningDirectionAt(const NBNode* n, const NBEdge* const edge) const;
    void setTurningDestination(NBEdge* e);



    /// @name Setting/getting special types
    /// @{

    /** @brief Marks this edge as a macroscopic connector
     */
    void setAsMacroscopicConnector() {
        myAmMacroscopicConnector = true;
    }


    /** @brief Returns whether this edge was marked as a macroscopic connector
     * @return Whether this edge was marked as a macroscopic connector
     */
    bool isMacroscopicConnector() const {
        return myAmMacroscopicConnector;
    }


    /** @brief Marks this edge being within an intersection
     */
    void setIsInnerEdge() {
        myAmInnerEdge = true;
    }


    /** @brief Returns whether this edge was marked as being within an intersection
     * @return Whether this edge was marked as being within an intersection
     */
    bool isInnerEdge() const {
        return myAmInnerEdge;
    }
    /// @}




    /** @brief Sets the junction priority of the edge
     * @param[in] node The node for which the edge's priority is given
     * @param[in] prio The edge's new priority at this node
     * @todo Maybe the edge priority whould be stored in the node
     */
    void setJunctionPriority(const NBNode* const node, int prio);


    /** @brief Returns the junction priority (normalised for the node currently build)
     *
     * If the given node is neither the edge's start nor the edge's ending node, the behaviour
     *  is undefined.
     *
     * @param[in] node The node for which the edge's priority shall be returned
     * @return The edge's priority at the given node
     * @todo Maybe the edge priority whould be stored in the node
     */
    int getJunctionPriority(const NBNode* const node) const;

    void setLoadedLength(SUMOReal val);
    void dismissVehicleClassInformation();


    const std::string& getTypeID() const {
        return myType;
    }

    /// @brief whether at least one lane has values differing from the edges values
    bool needsLaneSpecificOutput() const;

    /// @brief whether at least one lane has restrictions
    bool hasRestrictions() const;

    /// @brief whether lanes differ in allowed vehicle classes
    bool hasLaneSpecificPermissions() const;

    /// @brief whether lanes differ in speed
    bool hasLaneSpecificSpeed() const;

    /// @brief whether lanes differ in offset
    bool hasLaneSpecificOffset() const;

    /// @brief whether lanes differ in width
    bool hasLaneSpecificWidth() const;

    /// computes the edge (step1: computation of approached edges)
    bool computeEdge2Edges(bool noLeftMovers);

    /// computes the edge, step2: computation of which lanes approach the edges)
    bool computeLanes2Edges();

    /** recheck whether all lanes within the edge are all right and
        optimises the connections once again */
    bool recheckLanes();

    /** @brief Add a connection to the previously computed turnaround, if wished
     *
     * If a turning direction exists (myTurnDestination!=0) and either the
     *  edge is not controlled by a tls or noTLSControlled is false, a connection
     *  to the edge stored in myTurnDestination is added (from the leftmost lane
     *  of this edge to the leftmost lane of myTurnDestination).
     * @param[in] noTLSControlled Whether the turnaround shall not be connected if this edge is controlled by a tls
     */
    void appendTurnaround(bool noTLSControlled);



    /** @brief Returns the node at the given edges length (using an epsilon)
        When no node is existing at the given position, 0 is returned
        The epsilon is a static member of NBEdge, should be setable via program options */
    NBNode* tryGetNodeAtPosition(SUMOReal pos, SUMOReal tolerance = 5.0) const;


    SUMOReal getMaxLaneOffset();

    Position getMinLaneOffsetPositionAt(NBNode* node, SUMOReal width) const;
    Position getMaxLaneOffsetPositionAt(NBNode* node, SUMOReal width) const;


    bool lanesWereAssigned() const;

    bool mayBeTLSControlled(int fromLane, NBEdge* toEdge, int toLane) const;

    /// Returns if the link could be set as to be controlled
    bool setControllingTLInformation(const NBConnection& c, const std::string& tlID);

    /// @brief clears tlID for all connections
    void clearControllingTLInformation();

    void addCrossingPointsAsIncomingWithGivenOutgoing(NBEdge* o,
            PositionVector& into);

    SUMOReal width() const;

    PositionVector getCWBoundaryLine(const NBNode& n, SUMOReal offset) const;
    PositionVector getCCWBoundaryLine(const NBNode& n, SUMOReal offset) const;

    bool expandableBy(NBEdge* possContinuation) const;
    void append(NBEdge* continuation);

    bool hasSignalisedConnectionTo(const NBEdge* const e) const;


    void moveOutgoingConnectionsFrom(NBEdge* e, unsigned int laneOff);

    NBEdge* getTurnDestination() const;

    std::string getLaneID(unsigned int lane) const;

    SUMOReal getLaneSpeed(unsigned int lane) const;

    bool isNearEnough2BeJoined2(NBEdge* e, SUMOReal threshold) const;


    /** @brief Returns the angle of the edge's geometry at the given node
     *
     * The angle is signed, regards direction, and starts at 12 o'clock
     *  (north->south), proceeds positive clockwise.
     * @param[in] node The node for which the edge's angle shall be returned
     * @return This edge's angle at the given node
     */
    SUMOReal getAngleAtNode(const NBNode* const node) const;


    void incLaneNo(unsigned int by);

    void decLaneNo(unsigned int by, int dir = 0);

    void markAsInLane2LaneState();

    /// @brief set allowed/disallowed classes for the given lane or for all lanes if -1 is given
    void setPermissions(SVCPermissions permissions, int lane = -1);

    void setPreferredVehicleClass(SVCPermissions permissions, int lane = -1);

    /// @brief set allowed class for the given lane or for all lanes if -1 is given
    void allowVehicleClass(int lane, SUMOVehicleClass vclass);
    /// @brief set disallowed class for the given lane or for all lanes if -1 is given
    void disallowVehicleClass(int lane, SUMOVehicleClass vclass);

    void preferVehicleClass(int lane, SUMOVehicleClass vclass);



    /// @brief set lane specific width (negative lane implies set for all lanes)
    void setWidth(int lane, SUMOReal width);

    /// @brief set lane specific end-offset (negative lane implies set for all lanes)
    void setOffset(int lane, SUMOReal offset);

    /// @brief set lane specific speed (negative lane implies set for all lanes)
    void setSpeed(int lane, SUMOReal offset);

    /// @brief get the union of allowed classes over all lanes or for a specific lane
    SVCPermissions getPermissions(int lane = -1) const;

    void disableConnection4TLS(int fromLane, NBEdge* toEdge, int toLane);


    // returns a reference to the internal structure for the convenience of NETEDIT
    Lane& getLaneStruct(unsigned int lane) {
        return myLanes[lane];
    }

    // returns a reference to the internal structure for the convenience of NETEDIT
    const Lane& getLaneStruct(unsigned int lane) const {
        return myLanes[lane];
    }

    /* declares connections as fully loaded. This is needed to avoid recomputing connections
    * if an edge has no connections intentionally. */
    void declareConnectionsAsLoaded() {
        myStep = LANES2LANES_USER;
    }

    void buildInnerEdges(const NBNode& n, unsigned int noInternalNoSplits, unsigned int& lno, unsigned int& splitNo);

    inline const std::vector<NBSign>& getSigns() const {
        return mySigns;
    }

    inline void addSign(NBSign sign) {
        mySigns.push_back(sign);
    }


private:
    /**
     * ToEdgeConnectionsAdder
     * A class that being a bresenham-callback assigns the incoming
     * lanes to the edges
     */
    class ToEdgeConnectionsAdder : public Bresenham::BresenhamCallBack {
    private:
        /// map of edges to this edge's lanes that reach them
        std::map<NBEdge*, std::vector<unsigned int> > myConnections;

        /// the transition from the virtual lane to the edge it belongs to
        const EdgeVector& myTransitions;

    public:
        /// constructor
        ToEdgeConnectionsAdder(const EdgeVector& transitions)
            : myTransitions(transitions) { }

        /// destructor
        ~ToEdgeConnectionsAdder() { }

        /// executes a bresenham - step
        void execute(const unsigned int lane, const unsigned int virtEdge);

        const std::map<NBEdge*, std::vector<unsigned int> >& getBuiltConnections() const {
            return myConnections;
        }

    private:
        /// @brief Invalidated copy constructor.
        ToEdgeConnectionsAdder(const ToEdgeConnectionsAdder&);

        /// @brief Invalidated assignment operator.
        ToEdgeConnectionsAdder& operator=(const ToEdgeConnectionsAdder&);

    };


    /**
     * MainDirections
     * Holds (- relative to the edge it is build from -!!!) the list of
     * main directions a vehicle that drives on this street may take on
     * the junction the edge ends in
     * The back direction is not regarded
     */
    class MainDirections {
    public:
        /// enum of possible directions
        enum Direction { DIR_RIGHTMOST, DIR_LEFTMOST, DIR_FORWARD };

        /** list of the main direction within the following junction relative
            to the edge */
        std::vector<Direction> myDirs;

    public:
        /// constructor
        MainDirections(const EdgeVector& outgoing,
                       NBEdge* parent, NBNode* to);

        /// destructor
        ~MainDirections();

        /** returns the information whether no following street has a higher
            priority */
        bool empty() const;

        /** returns the information whether the street in the given direction
            has a higher priority */
        bool includes(Direction d) const;

    private:
        /// @brief Invalidated copy constructor.
        MainDirections(const MainDirections&);

        /// @brief Invalidated assignment operator.
        MainDirections& operator=(const MainDirections&);

    };

    /// Computes the shape for the given lane
    PositionVector computeLaneShape(unsigned int lane) throw(InvalidArgument);

    /// Computes the offset from the edge shape on the current segment
    std::pair<SUMOReal, SUMOReal> laneOffset(const Position& from,
            const Position& to, SUMOReal lanewidth, unsigned int lane) throw(InvalidArgument);

    void computeLaneShapes();



private:
    /** @brief Initialization routines common to all constructors
     *
     * Checks whether the number of lanes>0, whether the junction's from-
     *  and to-nodes are given (!=0) and whether they are distict. Throws
     *  a ProcessError if any of these checks fails.
     *
     * Adds the nodes positions to geometry if it shall not be ignored or
     *  if the geometry is empty.
     *
     * Computes the angle and length, and adds this edge to its node as
     *  outgoing/incoming. Builds lane informations.
     *
     * @param[in] noLanes The number of lanes this edge has
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     */
    void init(unsigned int noLanes, bool tryIgnoreNodePositions);


    /** divides the lanes on the outgoing edges */
    void divideOnEdges(const EdgeVector* outgoing);

    /** recomputes the priorities and manipulates them for a distribution
        of lanes on edges which is more like in real-life */
    std::vector<unsigned int>* preparePriorities(
        const EdgeVector* outgoing);

    /** computes the sum of the given list's entries (sic!) */
    unsigned int computePrioritySum(std::vector<unsigned int>* priorities);


    /// @name Setting and getting connections
    /// @{

    /** moves a connection one place to the left;
        Attention! no checking for field validity */
    void moveConnectionToLeft(unsigned int lane);

    /** moves a connection one place to the right;
        Attention! no checking for field validity */
    void moveConnectionToRight(unsigned int lane);
    /// @}


    /** returns a modified version of laneShape which starts at the outside of
     * startNode. laneShape may be shorted or extended
     * @note see [wiki:Developer/Network_Building_Process]
     */
    PositionVector startShapeAt(const PositionVector& laneShape, const NBNode* startNode, unsigned int laneIndex) const;

private:
    /** @brief The building step
     * @see EdgeBuildingStep
     */
    EdgeBuildingStep myStep;

    /// @brief The type of the edge
    std::string myType;

    /// @brief The source and the destination node
    NBNode* myFrom, *myTo;

    /// @brief The length of the edge
    SUMOReal myLength;

    /// @brief The angle of the edge
    SUMOReal myAngle;

    /// @brief The priority of the edge
    int myPriority;

    /// @brief The maximal speed
    SUMOReal mySpeed;

    /** @brief List of connections to following edges
     * @see Connection
     */
    std::vector<Connection> myConnections;

    /// @brief The turn destination edge
    NBEdge* myTurnDestination;

    /// @brief The priority normalised for the node the edge is outgoing of
    int myFromJunctionPriority;

    /// @brief The priority normalised for the node the edge is incoming in
    int myToJunctionPriority;

    /// @brief The geometry for the edge
    PositionVector myGeom;

    /// @brief The information about how to spread the lanes
    LaneSpreadFunction myLaneSpreadFunction;

    /// @brief This edges's offset to the intersection begin (will be applied to all lanes)
    SUMOReal myOffset;

    /// @brief This width of this edge's lanes
    SUMOReal myWidth;

    /** @brief Lane information
     * @see Lane
     */
    std::vector<Lane> myLanes;

    /// @brief An optional length to use (-1 if not valid)
    SUMOReal myLoadedLength;

    /// @brief Whether this edge is a left-hand edge
    bool myAmLeftHand;


    /// @brief Information whether this is a junction-inner edge
    bool myAmInnerEdge;

    /// @brief Information whether this edge is a (macroscopic) connector
    bool myAmMacroscopicConnector;

    struct TLSDisabledConnection {
        int fromLane;
        NBEdge* to;
        int toLane;
    };

    std::vector<TLSDisabledConnection> myTLSDisabledConnections;


    /// @brief The street name (or whatever arbitrary string you wish to attach)
    std::string myStreetName;


    /// @brief the street signs along this edge
    std::vector<NBSign> mySigns;


public:
    /**
     * @class tls_disable_finder
     */
    class tls_disable_finder {
    public:
        /// constructor
        tls_disable_finder(const TLSDisabledConnection& tpl) : myDefinition(tpl) { }

        bool operator()(const TLSDisabledConnection& e) const {
            if (e.to != myDefinition.to) {
                return false;
            }
            if (e.fromLane != myDefinition.fromLane) {
                return false;
            }
            if (e.toLane != myDefinition.toLane) {
                return false;
            }
            return true;
        }

    private:
        TLSDisabledConnection myDefinition;

    private:
        /// @brief invalidated assignment operator
        tls_disable_finder& operator=(const tls_disable_finder& s);

    };



    /**
     * @class connections_toedge_finder
     */
    class connections_toedge_finder {
    public:
        /// constructor
        connections_toedge_finder(NBEdge* const edge2find) : myEdge2Find(edge2find) { }

        bool operator()(const Connection& c) const {
            return c.toEdge == myEdge2Find;
        }

    private:
        NBEdge* const myEdge2Find;

    private:
        /// @brief invalidated assignment operator
        connections_toedge_finder& operator=(const connections_toedge_finder& s);

    };

    /**
     * @class connections_toedgelane_finder
     */
    class connections_toedgelane_finder {
    public:
        /// constructor
        connections_toedgelane_finder(NBEdge* const edge2find, int lane2find) : myEdge2Find(edge2find), myLane2Find(lane2find) { }

        bool operator()(const Connection& c) const {
            return c.toEdge == myEdge2Find && c.toLane == myLane2Find;
        }

    private:
        NBEdge* const myEdge2Find;
        int myLane2Find;

    private:
        /// @brief invalidated assignment operator
        connections_toedgelane_finder& operator=(const connections_toedgelane_finder& s);

    };


    /**
     * @class connections_finder
     */
    class connections_finder {
    public:
        /// constructor
        connections_finder(int fromLane, NBEdge* const edge2find, int lane2find) : myFromLane(fromLane), myEdge2Find(edge2find), myLane2Find(lane2find) { }

        bool operator()(const Connection& c) const {
            return c.fromLane == myFromLane && c.toEdge == myEdge2Find && c.toLane == myLane2Find;
        }

    private:
        int myFromLane;
        NBEdge* const myEdge2Find;
        int myLane2Find;

    private:
        /// @brief invalidated assignment operator
        connections_finder& operator=(const connections_finder& s);

    };

    /**
     * @class connections_fromlane_finder
     */
    class connections_fromlane_finder {
    public:
        /// constructor
        connections_fromlane_finder(int lane2find) : myLane2Find(lane2find) { }

        bool operator()(const Connection& c) const {
            return c.fromLane == myLane2Find;
        }

    private:
        int myLane2Find;

    private:
        /// @brief invalidated assignment operator
        connections_fromlane_finder& operator=(const connections_fromlane_finder& s);

    };

    /**
     * connections_sorter
     */
    static bool connections_sorter(const Connection& c1, const Connection& c2) {
        if (c1.fromLane != c2.fromLane) {
            return c1.fromLane < c2.fromLane;
        }
        if (c1.toEdge != c2.toEdge) {
            return c1.toEdge->getID().compare(c1.toEdge->getID()) < 0;
        }
        return c1.toLane < c2.toLane;
    }

    /**
     * connections_relative_edgelane_sorter
     * Class to sort edges by their angle
     */
    class connections_relative_edgelane_sorter {
    public:
        /// constructor
        explicit connections_relative_edgelane_sorter(NBEdge* e, NBNode* n)
            : myEdge(e), myNode(n) {}

    public:
        /// comparing operation
        int operator()(const Connection& c1, const Connection& c2) const {
            if (c1.toEdge != c2.toEdge) {
                SUMOReal relAngle1 = NBHelpers::normRelAngle(
                                         myEdge->getAngle(), c1.toEdge->getAngle());
                SUMOReal relAngle2 = NBHelpers::normRelAngle(
                                         myEdge->getAngle(), c2.toEdge->getAngle());
                return relAngle1 > relAngle2;
            }
            return c1.toLane < c2.toLane;
        }

    private:
        /// the edge to compute the relative angle of
        NBEdge* myEdge;

        /// the node to use
        NBNode* myNode;

    };

private:
    /// @brief invalidated copy constructor
    NBEdge(const NBEdge& s);

    /// @brief invalidated assignment operator
    NBEdge& operator=(const NBEdge& s);


};


#endif

/****************************************************************************/

