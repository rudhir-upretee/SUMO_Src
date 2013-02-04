/****************************************************************************/
/// @file    NBNodeCont.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Yun-Pang Wang
/// @author  Walter Bamberger
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Sascha Krieg
/// @date    Tue, 20 Nov 2001
/// @version $Id: NBNodeCont.cpp 13107 2012-12-02 13:57:34Z behrisch $
///
// Container for nodes during the netbuilding process
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
#include <map>
#include <algorithm>
#include <cmath>
#include <utils/options/OptionsCont.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StdDefs.h>
#include <utils/common/ToString.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/iodevices/OutputDevice.h>
#include "NBDistrict.h"
#include "NBEdgeCont.h"
#include "NBTrafficLightLogicCont.h"
#include "NBJoinedEdgesMap.h"
#include "NBOwnTLDef.h"
#include "NBNodeCont.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
NBNodeCont::NBNodeCont()
    : myInternalID(1) {}


NBNodeCont::~NBNodeCont() {
    clear();
}


// ----------- Insertion/removal/retrieval of nodes
bool
NBNodeCont::insert(const std::string& id, const Position& position,
                   NBDistrict* district) {
    NodeCont::iterator i = myNodes.find(id);
    if (i != myNodes.end()) {
        return false;
    }
    NBNode* node = new NBNode(id, position, district);
    myNodes[id] = node;
    return true;
}


bool
NBNodeCont::insert(const std::string& id, const Position& position) {
    NodeCont::iterator i = myNodes.find(id);
    if (i != myNodes.end()) {
        return false;
    }
    NBNode* node = new NBNode(id, position);
    myNodes[id] = node;
    return true;
}


Position
NBNodeCont::insert(const std::string& id) {
    std::pair<SUMOReal, SUMOReal> ret(-1.0, -1.0);
    NodeCont::iterator i = myNodes.find(id);
    if (i != myNodes.end()) {
        return (*i).second->getPosition();
    } else {
        NBNode* node = new NBNode(id, Position(-1.0, -1.0));
        myNodes[id] = node;
    }
    return Position(-1, -1);
}


bool
NBNodeCont::insert(NBNode* node) {
    std::string id = node->getID();
    NodeCont::iterator i = myNodes.find(id);
    if (i != myNodes.end()) {
        return false;
    }
    myNodes[id] = node;
    return true;
}


NBNode*
NBNodeCont::retrieve(const std::string& id) const {
    NodeCont::const_iterator i = myNodes.find(id);
    if (i == myNodes.end()) {
        return 0;
    }
    return (*i).second;
}


NBNode*
NBNodeCont::retrieve(const Position& position, SUMOReal offset) const {
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        NBNode* node = (*i).second;
        if (fabs(node->getPosition().x() - position.x()) < offset
                &&
                fabs(node->getPosition().y() - position.y()) < offset) {
            return node;
        }
    }
    return 0;
}


bool
NBNodeCont::erase(NBNode* node) {
    if (extract(node)) {
        delete node;
        return true;
    } else {
        return false;
    }
}


bool
NBNodeCont::extract(NBNode* node, bool remember) {
    NodeCont::iterator i = myNodes.find(node->getID());
    if (i == myNodes.end()) {
        return false;
    }
    myNodes.erase(i);
    node->removeTrafficLights();
    if (remember) {
        myExtractedNodes.insert(node);
    }
    return true;
}


// ----------- Adapting the input
void
NBNodeCont::removeSelfLoops(NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tc) {
    unsigned int no = 0;
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        no += (*i).second->removeSelfLoops(dc, ec, tc);
    }
    if (no != 0) {
        WRITE_WARNING(toString(no) + " self-looping edge(s) removed.");
    }
}


void
NBNodeCont::joinSimilarEdges(NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    // magic values
    SUMOReal distanceThreshold = 7; // don't merge edges further apart
    SUMOReal lengthThreshold = 0.05; // don't merge edges with higher relative length-difference

    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        // count the edges to other nodes outgoing from the current node
        std::map<NBNode*, EdgeVector> connectionCount;
        const EdgeVector& outgoing = (*i).second->getOutgoingEdges();
        for (EdgeVector::const_iterator j = outgoing.begin(); j != outgoing.end(); j++) {
            NBEdge* e = (*j);
            NBNode* connected = e->getToNode();
            if (connectionCount.find(connected) == connectionCount.end()) {
                connectionCount[connected] = EdgeVector();
            }
            connectionCount[connected].push_back(e);
        }
        // check whether more than a single edge connect another node and join them
        std::map<NBNode*, EdgeVector>::iterator k;
        for (k = connectionCount.begin(); k != connectionCount.end(); k++) {
            // possibly we do not have anything to join...
            if ((*k).second.size() < 2) {
                continue;
            }
            // for the edges that seem to be a single street,
            //  check whether the geometry is similar
            const EdgeVector& ev = (*k).second;
            const NBEdge* const first = ev.front();
            EdgeVector::const_iterator jci; // join candidate iterator
            for (jci = ev.begin() + 1; jci != ev.end(); ++jci) {
                const SUMOReal relativeLengthDifference = fabs(first->getLoadedLength() - (*jci)->getLoadedLength()) / first->getLoadedLength();
                if ((!first->isNearEnough2BeJoined2(*jci, distanceThreshold)) ||
                        (relativeLengthDifference > lengthThreshold) ||
                        (first->getSpeed() != (*jci)->getSpeed())
                        // @todo check vclass
                   ) {
                    break;
                }
            }
            // @bug If there are 3 edges of which 2 can be joined, no joining will
            //   take place with the current implementation
            if (jci == ev.end()) {
                ec.joinSameNodeConnectingEdges(dc, tlc, ev);
            }
        }
    }
}


void
NBNodeCont::removeIsolatedRoads(NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tc) {
    UNUSED_PARAMETER(tc);
    // Warn of isolated edges, i.e. a single edge with no connection to another edge
    int edgeCounter = 0;
    const std::vector<std::string>& edgeNames = ec.getAllNames();
    for (std::vector<std::string>::const_iterator it = edgeNames.begin(); it != edgeNames.end(); ++it) {
        // Test whether this node starts at a dead end, i.e. it has only one adjacent node
        // to which an edge exists and from which an edge may come.
        NBEdge* e = ec.retrieve(*it);
        if (e == 0) {
            continue;
        }
        NBNode* from = e->getFromNode();
        const EdgeVector& outgoingEdges = from->getOutgoingEdges();
        if (outgoingEdges.size() != 1) {
            // At this node, several edges or no edge start; so, this node is no dead end.
            continue;
        }
        const EdgeVector& incomingEdges = from->getIncomingEdges();
        if (incomingEdges.size() > 1) {
            // At this node, several edges end; so, this node is no dead end.
            continue;
        } else if (incomingEdges.size() == 1) {
            NBNode* fromNodeOfIncomingEdge = incomingEdges[0]->getFromNode();
            NBNode* toNodeOfOutgoingEdge = outgoingEdges[0]->getToNode();
            if (fromNodeOfIncomingEdge != toNodeOfOutgoingEdge) {
                // At this node, an edge ends which is not the inverse direction of
                // the starting node.
                continue;
            }
        }
        // Now we know that the edge e starts a dead end.
        // Next we test if the dead end is isolated, i.e. does not lead to a junction
        bool hasJunction = false;
        EdgeVector road;
        NBEdge* eOld = 0;
        NBNode* to;
        std::set<NBNode*> adjacentNodes;
        do {
            road.push_back(e);
            eOld = e;
            from = e->getFromNode();
            to = e->getToNode();
            const EdgeVector& outgoingEdgesOfToNode = to->getOutgoingEdges();
            const EdgeVector& incomingEdgesOfToNode = to->getIncomingEdges();
            adjacentNodes.clear();
            for (EdgeVector::const_iterator itOfOutgoings = outgoingEdgesOfToNode.begin(); itOfOutgoings != outgoingEdgesOfToNode.end(); ++itOfOutgoings) {
                if ((*itOfOutgoings)->getToNode() != from        // The back path
                        && (*itOfOutgoings)->getToNode() != to   // A loop / dummy edge
                   ) {
                    e = *itOfOutgoings; // Probably the next edge
                }
                adjacentNodes.insert((*itOfOutgoings)->getToNode());
            }
            for (EdgeVector::const_iterator itOfIncomings = incomingEdgesOfToNode.begin(); itOfIncomings != incomingEdgesOfToNode.end(); ++itOfIncomings) {
                adjacentNodes.insert((*itOfIncomings)->getFromNode());
            }
            adjacentNodes.erase(to);  // Omit loops
            if (adjacentNodes.size() > 2) {
                hasJunction = true;
            }
        } while (!hasJunction && eOld != e);
        if (!hasJunction) {
            edgeCounter +=  int(road.size());
            std::string warningString = "Removed a road without junctions: ";
            for (EdgeVector::iterator roadIt = road.begin(); roadIt != road.end(); ++roadIt) {
                if (roadIt == road.begin()) {
                    warningString += (*roadIt)->getID();
                } else {
                    warningString += ", " + (*roadIt)->getID();
                }

                NBNode* fromNode = (*roadIt)->getFromNode();
                NBNode* toNode = (*roadIt)->getToNode();
                ec.erase(dc, *roadIt);
                if (fromNode->getIncomingEdges().size() == 0 && fromNode->getOutgoingEdges().size() == 0) {
                    // Node is empty; can be removed
                    erase(fromNode);
                }
                if (toNode->getIncomingEdges().size() == 0 && toNode->getOutgoingEdges().size() == 0) {
                    // Node is empty; can be removed
                    erase(toNode);
                }
            }
            WRITE_WARNING(warningString);
        }
    }
    if (edgeCounter > 0 && !OptionsCont::getOptions().getBool("remove-edges.isolated")) {
        WRITE_WARNING("Detected isolated roads. Use the option --remove-edges.isolated to get a list of all affected edges.");
    }
}


unsigned int
NBNodeCont::removeUnwishedNodes(NBDistrictCont& dc, NBEdgeCont& ec,
                                NBJoinedEdgesMap& je, NBTrafficLightLogicCont& tlc,
                                bool removeGeometryNodes) {
    unsigned int no = 0;
    std::vector<NBNode*> toRemove;
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        NBNode* current = (*i).second;
        bool remove = false;
        std::vector<std::pair<NBEdge*, NBEdge*> > toJoin;
        // check for completely empty nodes
        if (current->getOutgoingEdges().size() == 0 && current->getIncomingEdges().size() == 0) {
            // remove if empty
            remove = true;
        }
        // check for nodes which are only geometry nodes
        if (removeGeometryNodes) {
            if ((current->getOutgoingEdges().size() == 1 && current->getIncomingEdges().size() == 1)
                    ||
                    (current->getOutgoingEdges().size() == 2 && current->getIncomingEdges().size() == 2)) {
                // ok, one in, one out or two in, two out
                //  -> ask the node whether to join
                remove = current->checkIsRemovable();
                if (remove) {
                    toJoin = current->getEdgesToJoin();
                }
            }
        }
        // remove the node and join the geometries when wished
        if (!remove) {
            continue;
        }
        for (std::vector<std::pair<NBEdge*, NBEdge*> >::iterator j = toJoin.begin(); j != toJoin.end(); j++) {
            NBEdge* begin = (*j).first;
            NBEdge* continuation = (*j).second;
            begin->append(continuation);
            continuation->getToNode()->replaceIncoming(continuation, begin, 0);
            tlc.replaceRemoved(continuation, -1, begin, -1);
            je.appended(begin->getID(), continuation->getID());
            ec.erase(dc, continuation);
        }
        toRemove.push_back(current);
        no++;
    }
    // erase all
    for (std::vector<NBNode*>::iterator j = toRemove.begin(); j != toRemove.end(); ++j) {
        erase(*j);
    }
    return no;
}


// ----------- (Helper) methods for joining nodes
void
NBNodeCont::generateNodeClusters(SUMOReal maxDist, NodeClusters& into) const {
    std::set<NBNode*> visited;
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        std::vector<NBNode*> toProc;
        if (visited.find((*i).second) != visited.end()) {
            continue;
        }
        toProc.push_back((*i).second);
        std::set<NBNode*> c;
        while (!toProc.empty()) {
            NBNode* n = toProc.back();
            toProc.pop_back();
            if (visited.find(n) != visited.end()) {
                continue;
            }
            c.insert(n);
            visited.insert(n);
            const EdgeVector& edges = n->getEdges();
            for (EdgeVector::const_iterator j = edges.begin(); j != edges.end(); ++j) {
                NBEdge* e = *j;
                NBNode* s = 0;
                if (n->hasIncoming(e)) {
                    s = e->getFromNode();
                } else {
                    s = e->getToNode();
                }
                if (visited.find(s) != visited.end()) {
                    continue;
                }
                if (e->getLoadedLength() < maxDist) {
                    toProc.push_back(s);
                }
            }
        }
        if (c.size() < 2) {
            continue;
        }
        into.push_back(c);
    }
}


void
NBNodeCont::addJoinExclusion(const std::vector<std::string>& ids, bool check) {
    for (std::vector<std::string>::const_iterator it = ids.begin(); it != ids.end(); it++) {
        // error handling has to take place here since joinExclusions could be
        // loaded from multiple files / command line
        if (myJoined.count(*it) > 0) {
            WRITE_WARNING("Ignoring join exclusion for node '" + *it +  "' since it already occured in a list of nodes to be joined");
        } else if (check && retrieve(*it) == 0) {
            WRITE_WARNING("Ignoring join exclusion for unknown node '" + *it + "'");
        } else {
            myJoinExclusions.insert(*it);
        }
    }
}


void
NBNodeCont::addCluster2Join(std::set<std::string> cluster) {
    // error handling has to take place here since joins could be loaded from multiple files
    for (std::set<std::string>::const_iterator it = cluster.begin(); it != cluster.end(); it++) {
        if (myJoinExclusions.count(*it) > 0) {
            WRITE_WARNING("Ignoring join-cluster because node '" + *it + "' was already excluded from joining");
            return;
        } else if (myJoined.count(*it) > 0) {
            WRITE_WARNING("Ignoring join-cluster because node '" + *it + "' already occured in another join-cluster");
            return;
        } else {
            myJoined.insert(*it);
        }
    }
    myClusters2Join.push_back(cluster);
}


unsigned int
NBNodeCont::joinLoadedClusters(NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    NodeClusters clusters;
    for (std::vector<std::set<std::string> >::iterator it = myClusters2Join.begin(); it != myClusters2Join.end(); it++) {
        // verify loaded cluster
        std::set<NBNode*> cluster;
        for (std::set<std::string>::iterator it_id = it->begin(); it_id != it->end(); it_id++) {
            NBNode* node = retrieve(*it_id);
            if (node == 0) {
                WRITE_WARNING("Ignoring unknown node '" + *it_id + "' while joining");
            } else {
                cluster.insert(node);
            }
        }
        if (cluster.size() > 1) {
            clusters.push_back(cluster);
        }
    }
    joinNodeClusters(clusters, dc, ec, tlc);
    myClusters2Join.clear(); // make save for recompute
    return (int)clusters.size();
}


unsigned int
NBNodeCont::joinJunctions(SUMOReal maxdist, NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    NodeClusters cands;
    NodeClusters clusters;
    generateNodeClusters(maxdist, cands);
    for (NodeClusters::iterator i = cands.begin(); i != cands.end(); ++i) {
        std::set<NBNode*> cluster = (*i);
        // remove join exclusions
        for (std::set<NBNode*>::iterator j = cluster.begin(); j != cluster.end();) {
            std::set<NBNode*>::iterator check = j;
            ++j;
            if (myJoinExclusions.count((*check)->getID()) > 0) {
                cluster.erase(check);
            }
        }
        // iteratively remove the fringe
        bool pruneFringe = true;
        while (pruneFringe) {
            pruneFringe = false;
            for (std::set<NBNode*>::iterator j = cluster.begin(); j != cluster.end();) {
                std::set<NBNode*>::iterator check = j;
                NBNode* n = *check;
                ++j;
                // remove nodes with degree <= 2 at fringe of the cluster (at least one edge leads to a non-cluster node)
                if (
                    (n->getIncomingEdges().size() <= 1 && n->getOutgoingEdges().size() <= 1) &&
                    ((n->getIncomingEdges().size() == 0 ||
                      (n->getIncomingEdges().size() == 1 && cluster.count(n->getIncomingEdges()[0]->getFromNode()) == 0)) ||
                     (n->getOutgoingEdges().size() == 0 ||
                      (n->getOutgoingEdges().size() == 1 && cluster.count(n->getOutgoingEdges()[0]->getToNode()) == 0)))
                ) {
                    cluster.erase(check);
                    pruneFringe = true; // other nodes could belong to the fringe now
                }
            }
        }
        if (cluster.size() > 1) {
            clusters.push_back(cluster);
        }
    }
    joinNodeClusters(clusters, dc, ec, tlc);
    return (int)clusters.size();
}


void
NBNodeCont::joinNodeClusters(NodeClusters clusters,
                             NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    for (NodeClusters::iterator i = clusters.begin(); i != clusters.end(); ++i) {
        std::set<NBNode*> cluster = *i;
        assert(cluster.size() > 1);
        Position pos;
        bool setTL;
        std::string id;
        analyzeCluster(cluster, id, pos, setTL);
        if (!insert(id, pos)) {
            // should not fail
            WRITE_WARNING("Could not join junctions " + id);
            continue;
        }
        NBNode* newNode = retrieve(id);
        if (setTL) {
            NBTrafficLightDefinition* tlDef = new NBOwnTLDef(id, newNode, 0);
            if (!tlc.insert(tlDef)) {
                // actually, nothing should fail here
                delete tlDef;
                throw ProcessError("Could not allocate tls '" + id + "'.");
            }
        }
        // collect edges
        std::set<NBEdge*> allEdges;
        for (std::set<NBNode*>::const_iterator j = cluster.begin(); j != cluster.end(); ++j) {
            const std::vector<NBEdge*>& edges = (*j)->getEdges();
            allEdges.insert(edges.begin(), edges.end());
        }

        // remap and remove edges which are completely within the new intersection
        for (std::set<NBEdge*>::iterator j = allEdges.begin(); j != allEdges.end();) {
            NBEdge* e = (*j);
            NBNode* from = e->getFromNode();
            NBNode* to = e->getToNode();
            if (cluster.count(from) > 0 && cluster.count(to) > 0) {
                for (std::set<NBEdge*>::iterator l = allEdges.begin(); l != allEdges.end(); ++l) {
                    if (e != *l) {
                        (*l)->replaceInConnections(e, e->getConnections());
                    }
                }
                ec.erase(dc, e);
                allEdges.erase(j++); // erase does not invalidate the other iterators
            } else {
                ++j;
            }
        }

        // remap edges which are incoming / outgoing
        for (std::set<NBEdge*>::iterator j = allEdges.begin(); j != allEdges.end(); ++j) {
            NBEdge* e = (*j);
            std::vector<NBEdge::Connection> conns = e->getConnections();
            const bool outgoing = cluster.count(e->getFromNode()) > 0;
            NBNode* from = outgoing ? newNode : e->getFromNode();
            NBNode* to   = outgoing ? e->getToNode() : newNode;
            e->reinitNodes(from, to);
            // re-add connections which previously existed and may still valid.
            // connections to removed edges will be ignored
            for (std::vector<NBEdge::Connection>::iterator k = conns.begin(); k != conns.end(); ++k) {
                e->addLane2LaneConnection((*k).fromLane, (*k).toEdge, (*k).toLane, NBEdge::L2L_USER, false, (*k).mayDefinitelyPass);
            }
        }
        registerJoinedCluster(cluster);
    }
}


void
NBNodeCont::registerJoinedCluster(const std::set<NBNode*>& cluster) {
    std::set<std::string> ids;
    for (std::set<NBNode*>::const_iterator j = cluster.begin(); j != cluster.end(); j++) {
        ids.insert((*j)->getID());
    }
    myJoinedClusters.push_back(ids);
}


void
NBNodeCont::analyzeCluster(std::set<NBNode*> cluster, std::string& id, Position& pos, bool& hasTLS) {
    id = "cluster";
    hasTLS = false;
    std::vector<std::string> member_ids;
    for (std::set<NBNode*>::const_iterator j = cluster.begin(); j != cluster.end(); j++) {
        member_ids.push_back((*j)->getID());
        pos.add((*j)->getPosition());
        // add a traffic light if any of the cluster members was controlled
        if ((*j)->isTLControlled()) {
            hasTLS = true;
        }
    }
    pos.mul(1.0 / cluster.size());
    // need to sort the member names to make the output deterministic
    sort(member_ids.begin(), member_ids.end());
    for (std::vector<std::string>::iterator j = member_ids.begin(); j != member_ids.end(); j++) {
        id = id + "_" + (*j);
    }
}


// ----------- (Helper) methods for guessing/computing traffic lights
bool
NBNodeCont::shouldBeTLSControlled(const std::set<NBNode*>& c) const {
    unsigned int noIncoming = 0;
    unsigned int noOutgoing = 0;
    bool tooFast = false;
    SUMOReal f = 0;
    std::set<NBEdge*> seen;
    for (std::set<NBNode*>::const_iterator j = c.begin(); j != c.end(); ++j) {
        const EdgeVector& edges = (*j)->getEdges();
        for (EdgeVector::const_iterator k = edges.begin(); k != edges.end(); ++k) {
            if (c.find((*k)->getFromNode()) != c.end() && c.find((*k)->getToNode()) != c.end()) {
                continue;
            }
            if ((*j)->hasIncoming(*k)) {
                ++noIncoming;
                f += (SUMOReal)(*k)->getNumLanes() * (*k)->getLaneSpeed(0);
            } else {
                ++noOutgoing;
            }
            if ((*k)->getLaneSpeed(0) * 3.6 > 79) {
                tooFast = true;
            }
        }
    }
    return !tooFast && f >= 150. / 3.6 && c.size() != 0;
}


void
NBNodeCont::guessTLs(OptionsCont& oc, NBTrafficLightLogicCont& tlc) {
    // build list of definitely not tls-controlled junctions
    std::vector<NBNode*> ncontrolled;
    if (oc.isSet("tls.unset")) {
        std::vector<std::string> notTLControlledNodes = oc.getStringVector("tls.unset");
        for (std::vector<std::string>::const_iterator i = notTLControlledNodes.begin(); i != notTLControlledNodes.end(); ++i) {
            NBNode* n = NBNodeCont::retrieve(*i);
            if (n == 0) {
                throw ProcessError(" The node '" + *i + "' to set as not-controlled is not known.");
            }
            std::set<NBTrafficLightDefinition*> tls = n->getControllingTLS();
            for (std::set<NBTrafficLightDefinition*>::const_iterator j = tls.begin(); j != tls.end(); ++j) {
                (*j)->removeNode(n);
            }
            n->removeTrafficLights();
            ncontrolled.push_back(n);
        }
    }

    // loop#1 checking whether the node shall be tls controlled,
    //  because it is assigned to a district
    if (oc.exists("tls.taz-nodes") && oc.getBool("tls.taz-nodes")) {
        for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
            NBNode* cur = (*i).second;
            if (cur->isNearDistrict() && find(ncontrolled.begin(), ncontrolled.end(), cur) == ncontrolled.end()) {
                setAsTLControlled(cur, tlc);
            }
        }
    }

    // maybe no tls shall be guessed
    if (!oc.getBool("tls.guess")) {
        return;
    }

    // guess joined tls first, if wished
    if (oc.getBool("tls.join")) {
        // get node clusters
        std::vector<std::set<NBNode*> > cands;
        generateNodeClusters(oc.getFloat("tls.join-dist"), cands);
        // check these candidates (clusters) whether they should be controlled by a tls
        for (std::vector<std::set<NBNode*> >::iterator i = cands.begin(); i != cands.end();) {
            std::set<NBNode*>& c = (*i);
            // regard only junctions which are not yet controlled and are not
            //  forbidden to be controlled
            for (std::set<NBNode*>::iterator j = c.begin(); j != c.end();) {
                if ((*j)->isTLControlled() || find(ncontrolled.begin(), ncontrolled.end(), *j) != ncontrolled.end()) {
                    c.erase(j++);
                } else {
                    ++j;
                }
            }
            // check whether the cluster should be controlled
            if (!shouldBeTLSControlled(c)) {
                i = cands.erase(i);
            } else {
                ++i;
            }
        }
        // cands now only contain sets of junctions that shall be joined into being tls-controlled
        unsigned int index = 0;
        for (std::vector<std::set<NBNode*> >::iterator i = cands.begin(); i != cands.end(); ++i) {
            std::vector<NBNode*> nodes;
            for (std::set<NBNode*>::iterator j = (*i).begin(); j != (*i).end(); j++) {
                nodes.push_back(*j);
            }
            std::string id = "joinedG_" + toString(index++);
            NBTrafficLightDefinition* tlDef = new NBOwnTLDef(id, nodes, 0);
            if (!tlc.insert(tlDef)) {
                // actually, nothing should fail here
                WRITE_WARNING("Could not build guessed, joined tls");
                delete tlDef;
                return;
            }
        }
    }

    // guess tls
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        NBNode* cur = (*i).second;
        //  do nothing if already is tl-controlled
        if (cur->isTLControlled()) {
            continue;
        }
        // do nothing if in the list of explicit non-controlled junctions
        if (find(ncontrolled.begin(), ncontrolled.end(), cur) != ncontrolled.end()) {
            continue;
        }
        std::set<NBNode*> c;
        c.insert(cur);
        if (!shouldBeTLSControlled(c) || cur->getIncomingEdges().size() < 3) {
            continue;
        }
        setAsTLControlled((*i).second, tlc);
    }
}


void
NBNodeCont::joinTLS(NBTrafficLightLogicCont& tlc, SUMOReal maxdist) {
    std::vector<std::set<NBNode*> > cands;
    generateNodeClusters(maxdist, cands);
    unsigned int index = 0;
    for (std::vector<std::set<NBNode*> >::iterator i = cands.begin(); i != cands.end(); ++i) {
        std::set<NBNode*>& c = (*i);
        for (std::set<NBNode*>::iterator j = c.begin(); j != c.end();) {
            if (!(*j)->isTLControlled()) {
                c.erase(j++);
            } else {
                ++j;
            }
        }
        if (c.size() < 2) {
            continue;
        }
        for (std::set<NBNode*>::iterator j = c.begin(); j != c.end(); ++j) {
            std::set<NBTrafficLightDefinition*> tls = (*j)->getControllingTLS();
            (*j)->removeTrafficLights();
            for (std::set<NBTrafficLightDefinition*>::iterator k = tls.begin(); k != tls.end(); ++k) {
                tlc.removeFully((*j)->getID());
            }
        }
        std::string id = "joinedS_" + toString(index++);
        std::vector<NBNode*> nodes;
        for (std::set<NBNode*>::iterator j = c.begin(); j != c.end(); j++) {
            nodes.push_back(*j);
        }
        NBTrafficLightDefinition* tlDef = new NBOwnTLDef(id, nodes, 0);
        if (!tlc.insert(tlDef)) {
            // actually, nothing should fail here
            WRITE_WARNING("Could not build a joined tls.");
            delete tlDef;
            return;
        }
    }
}


void
NBNodeCont::setAsTLControlled(NBNode* node, NBTrafficLightLogicCont& tlc, std::string id) {
    if (id == "") {
        id = node->getID();
    }
    NBTrafficLightDefinition* tlDef = new NBOwnTLDef(id, node, 0);
    if (!tlc.insert(tlDef)) {
        // actually, nothing should fail here
        WRITE_WARNING("Building a tl-logic for node '" + id + "' twice is not possible.");
        delete tlDef;
        return;
    }
}


// -----------
void
NBNodeCont::computeLanes2Lanes() {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        (*i).second->computeLanes2Lanes();
    }
}


// computes the "wheel" of incoming and outgoing edges for every node
void
NBNodeCont::computeLogics(const NBEdgeCont& ec, OptionsCont& oc) {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        (*i).second->computeLogic(ec, oc);
    }
}


void
NBNodeCont::clear() {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        delete((*i).second);
    }
    myNodes.clear();
    for (std::set<NBNode*>::iterator i = myExtractedNodes.begin(); i != myExtractedNodes.end(); i++) {
        delete(*i);
    }
    myExtractedNodes.clear();
}


std::string
NBNodeCont::getFreeID() {
    // !!! not guaranteed to be free
    std::string ret = "SUMOGenerated" + toString<int>(size());
    assert(retrieve(ret) == 0);
    return ret;
}


void
NBNodeCont::computeNodeShapes(bool leftHand) {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        (*i).second->computeNodeShape(leftHand);
    }
}


void
NBNodeCont::printBuiltNodesStatistics() const {
    int numUnregulatedJunctions = 0;
    int numDeadEndJunctions = 0;
    int numPriorityJunctions = 0;
    int numRightBeforeLeftJunctions = 0;
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        switch ((*i).second->getType()) {
            case NODETYPE_NOJUNCTION:
                ++numUnregulatedJunctions;
                break;
            case NODETYPE_DEAD_END:
                ++numDeadEndJunctions;
                break;
            case NODETYPE_PRIORITY_JUNCTION:
            case NODETYPE_TRAFFIC_LIGHT:
                ++numPriorityJunctions;
                break;
            case NODETYPE_RIGHT_BEFORE_LEFT:
                ++numRightBeforeLeftJunctions;
                break;
            case NODETYPE_DISTRICT:
                ++numRightBeforeLeftJunctions;
                break;
            case NODETYPE_UNKNOWN:
                break;
            default:
                break;
        }
    }
    WRITE_MESSAGE(" Node type statistics:");
    WRITE_MESSAGE("  Unregulated junctions       : " + toString(numUnregulatedJunctions));
    if (numDeadEndJunctions > 0) {
        WRITE_MESSAGE("  Dead-end junctions          : " + toString(numDeadEndJunctions));
    }
    WRITE_MESSAGE("  Priority junctions          : " + toString(numPriorityJunctions));
    WRITE_MESSAGE("  Right-before-left junctions : " + toString(numRightBeforeLeftJunctions));
}


std::vector<std::string>
NBNodeCont::getAllNames() const {
    std::vector<std::string> ret;
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); ++i) {
        ret.push_back((*i).first);
    }
    return ret;
}


void
NBNodeCont::rename(NBNode* node, const std::string& newID) {
    if (myNodes.count(newID) != 0) {
        throw ProcessError("Attempt to rename node using existing id '" + newID + "'");
    }
    myNodes.erase(node->getID());
    node->setID(newID);
    myNodes[newID] = node;
}


/****************************************************************************/

