
//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

/**************************************************************************
     File: DispatchGraph.cc
   Author: Paul H. Morris
Purpose:
    Define DispatchGraph module for dispatchability processing.
Contents:
    The implementation of the DispatchGraph class and associated classes.
Updates:
    20000406 - PHM - Initial version
Notes:
    Efficiency is paramount since we have to do a full propagation
    from each node. (At least O(n^2) complexity.)  We use the
    algorithm of Tsamardinos, Muscettola, Morris in AAAI-98.
    This is a fairly straight port of the Lisp version (file filter.lisp)
    used in the RAX experiment.
**************************************************************************/

#include <stdlib.h>
#include <limits.h>
#include "DispatchGraph.hh"
#include "Error.hh"

namespace EUROPA {

DnodeId DispatchGraph::makeNode()
{
  // Overrides the definition in DistanceGraph class.
  DispatchNode* node = new DispatchNode();
  return node;
}

DispatchNode* DispatchGraph::createNode(Referent name)
{
  DispatchNode* node = (DispatchNode*)DistanceGraph::createNode();
  node->name = name;
  node->isSccMember = false;
  return node;
}

void DispatchGraph::createEdge(DispatchNode* from, DispatchNode* to,
                               Time length)
{
  (void)DistanceGraph::createEdge(from,to,length);
}

void DispatchGraph::filter( void (*keepEdge)(DispatchNode*, DispatchNode*,
                                             Time) )
{
  if (bellmanFord() == false)
    handle_error(bellmanFord() == false,
                "Dispatchability analysis called on inconsistent graph",
                TempNetErr::DistanceGraphInconsistentError());
  this->reversePostorder = new DispatchNode*[this->nodes.size()];

  if (!this->reversePostorder)
    handle_error(!this->reversePostorder, 
                 "Could not allocate memory to process dispatchability",
                 TempNetErr::TempNetMemoryError());
  this->sccLeaders = std::vector<DispatchNode*>();
  // Set initial distances to same as potential
  for (std::vector<DnodeId>::const_iterator it=nodes.begin(); it != nodes.end(); ++it) {
    DispatchNode* node = (DispatchNode*) *it;
    node->distance = node->potential;
  }

  this->findSccs(keepEdge);  // Processes SCCs and sets sccLeaders.
  for (std::vector<DispatchNode*>::const_iterator it=sccLeaders.begin(); it != sccLeaders.end(); ++it) {
    DispatchNode* node = *it;
    this->dijkstra(node);
    this->findKeptEdges(node, keepEdge);
  }
  delete[] this->reversePostorder;
}

Int compareNodes (const DispatchNode** node1, const DispatchNode** node2)
{
  Time d1 = (*node1)->distance;
  Time d2 = (*node2)->distance;
  if (d1 < d2)
    return -1;
  else if (d1 == d2)
    return 0;
  else
    return 1;
}

void DispatchGraph::findSccs( void (*keepEdge)(DispatchNode*, DispatchNode*,
                                               Time) )
{
  // SCC = Strongly Connected Component.
  // See P. 488 in "Introduction To Algorithms"
  //                by Cormen, Leiverson, & Rivest.
  // The SCCs contain all the rigid (no slack) edges in the network.
  Int nodeCount = this->nodes.size();
  DispatchNode** scc = new DispatchNode*[nodeCount];  // Scratch list for SCCs.
  if (!scc)
    handle_error(!scc, 
                "Could not allocate memory to process dispatchability",
                TempNetErr::TempNetMemoryError());
  buildReversePostorder (this->nodes);
  Dnode::unmarkAll();
  for (Int i=0; i < nodeCount; i++) {
    DispatchNode* node = this->reversePostorder[i];
    if (!node->isMarked()) {
      size_t sccSize = 0;
      predGraphTraceScc (node, scc, sccSize, nodeCount);  // Builds scc.
      qsort ((void*)scc, sccSize, sizeof(DispatchNode*),
             (Int (*)(const void*,const void*)) compareNodes);
      processScc (scc, sccSize, keepEdge);
    }
  }
  delete[] scc;
}

void DispatchGraph::buildReversePostorder (std::vector<DnodeId>& argNodes)
{
  // Do depth-first searches, collecting nodes into reverse-postorder.
  Dnode::unmarkAll();
  Int position = argNodes.size();
  for (std::vector<DnodeId>::const_iterator it=argNodes.begin(); it != argNodes.end(); ++it) {
    Dnode* node = *it;
    if (!node->isMarked())
      predGraphDfs ((DispatchNode*)node, position);
  }

  check_error(!(position > 0), "Lost some nodes from reversePostorder list",
              TempNetErr::TempNetInternalError());
}

void DispatchGraph::predGraphDfs (DispatchNode* node, Int& position)
{
  // Depth-first-search through predecessor graph (PG).  An edge
  // is in PG if start_distance + length(edge) == end_distance.
  node->mark();
  for (Int i=0; i < node->outCount; i++) {
    Dedge* edge = node->outArray[i];
    DispatchNode* next = (DispatchNode*) edge->to;
    if (!next->isMarked() && node->distance + edge->length == next->distance)
      this->predGraphDfs (next, position);
  }

  check_error(!(position <= 0), "Inserting node before start of node-array",
              TempNetErr::TempNetInternalError());

  this->reversePostorder[--position] = node;
}

void DispatchGraph::predGraphTraceScc (DispatchNode* node, DispatchNode* scc[],
                                       size_t& sccSize, Int nodeCount)
{
  node->mark();
  node->isSccMember = true;
  check_error(!((Int)sccSize >= nodeCount), "Inserting node beyond end of node-array",
              TempNetErr::TempNetInternalError());

  scc[sccSize++] = node;

  for (Int i=0; i< node->inCount; i++) {
    Dedge* edge = node->inArray[i];
    DispatchNode* parent = (DispatchNode*) edge->from;
    if (!parent->isMarked()
        && parent->distance + edge->length == node->distance)
      predGraphTraceScc (parent, scc, sccSize, nodeCount);
  }
}

void DispatchGraph::processScc (DispatchNode* scc[], size_t sccSize,
                                void (*keepEdge)(DispatchNode*, DispatchNode*,
                                                 Time))
{
  // This figures out what edges in the SCC to keep,
  // and detaches the interior of the SCC from the graph.
  check_error(!(sccSize < 1), "Strongly Connected Component with no nodes",
              TempNetErr::TempNetInternalError());

  DispatchNode* leader = scc[0];
  this->sccLeaders.push_back(leader);
  // Manually compute the filtered edges internal to the SCC.
  // Arrange in a doubly-linked chain.
  DispatchNode* previous = leader;
  Time prevdistance = leader->distance;
  for (Int i=1; i < (Int)sccSize; i++) {
    DispatchNode* node = scc[i];
    Time distance = node->distance;
    Time increment = distance - prevdistance;
    (*keepEdge) (previous, node, increment);
    (*keepEdge) (node, previous, -increment);
    // Move the dangling edges to the leader.
    sccMoveFluids (node, leader);
    previous = node;
    prevdistance = distance;
  }
  // Finished with this scc, reset isSccMember value.
  // Only need to do it for leader because interior
  // of scc is now detached from the rest of the graph.
  leader->isSccMember = false;
}

void DispatchGraph::sccMoveFluids (DispatchNode* node, DispatchNode* leader)
{
  // The purpose of this function is to replace all constraints
  // between the interior of the SCC and the outside with equivalent
  // constraints between the leader and the outside.  This is
  // visualized as "moving" those constraints (which are called
  // "fluids" here).

  Time relativeDistance = node->distance - leader->distance;

  // First move edges from the node to outside
  // TODO: gcc 4.1.1 is comlaining about not finding a match for the 2 calls to sccMoveDirectional below
  sccMoveDirectional (node, 
		              leader, 
		              relativeDistance,
                      &DispatchNode::inArray, &DispatchNode::inCount,
                      &DispatchNode::outArray, &DispatchNode::outCount,
                      &DispatchNode::outArraySize,
                      &Dedge::to, &Dedge::from);

  // Now move edges TO the node FROM outside, by reversing directions.
  sccMoveDirectional (node, 
		              leader, 
		              -relativeDistance,
                      &DispatchNode::outArray, &DispatchNode::outCount,
                      &DispatchNode::inArray, &DispatchNode::inCount,
                      &DispatchNode::inArraySize,
                      &Dedge::from, &Dedge::to);
  
}

// Following are defined in DistanceGraph.cc
/*
void attachEdge (Dedge**& edgeArray, int& size, int& count, Dedge* edge);
void detachEdge (Dedge**& edgeArray, int& count, Dedge* edge);
*/
Void attachEdge (DedgeId*& edgeArray, Int& size, Int& count, DedgeId edge);
Void detachEdge (DedgeId*& edgeArray, Int& count, DedgeId edge);


/* TBW: Replaced by newer version that does more error checking
void DispatchGraph::sccMoveDirectional (DispatchNode* node,
                                        DispatchNode* leader,
                                        Time offset,
                                        Dedge** Dnode::*ins,
                                        int Dnode::*inCount,
                                        Dedge** Dnode::*outs,
                                        int Dnode::*outCount,
                                        int Dnode::*outSize,
                                        Dnode* Dedge::*to,
                                        Dnode* Dedge::*from)
{
  // We use args that are pointers to the graph traversal member functions
  // so we can easily reverse the direction of graph operations.
  // For example, Dnode::*ins = Dnode::ins in the forward call,
  //              Dnode::*ins = Dnode::outs in the reverse call.

  for (Int i=0; i< node->*outCount; i++) {
    Dedge* edge = (node->*outs)[i];
    DispatchNode* next = (DispatchNode*) (edge->*to);
    if (next == leader)  // Delink from leader
      detachEdge (leader->*ins, leader->*inCount, edge);
    if (next->isSccMember == false) {   // next is not in the SCC.
      Time movedDistance = edge->length + offset;
      // Look for an *out edge of the leader that also points *to next
      Dedge* leaderEdge = 0;
      for (Int j=0; j < leader->*outCount; j++) {
        Dedge* e = (leader->*outs)[j];
        if (e->*to == next)
          leaderEdge = e;
      }
      if (leaderEdge) {
        // Move the constraint to the leaderEdge and delink edge.
        if (movedDistance < leaderEdge->length)
          leaderEdge->length = movedDistance;
        detachEdge (next->*ins, next->*inCount, edge);
      }
      else {
        // Modify and redirect the edge.
        edge->length = movedDistance;
        edge->*from = leader;
        attachEdge (leader->*outs, leader->*outSize, leader->*outCount, edge);
      }
      // No need to remove edge from node->*outs because
      // node will be unreachable from sccLeaders.
    }
  }
}
*/

void DispatchGraph::sccMoveDirectional (DispatchNode* node,
                                        DispatchNode* leader,
                                        Time offset,
                                        DedgeId* Dnode::*ins,
                                        int Dnode::*inCount,
                                        DedgeId* Dnode::*outs,
                                        int Dnode::*outCount,
                                        int Dnode::*outSize,
                                        DnodeId Dedge::*to,
                                        DnodeId Dedge::*from)
{
  // We use args that are pointers to the graph traversal member functions
  // so we can easily reverse the direction of graph operations.
  // For example, Dnode::*ins = Dnode::ins in the forward call,
  //              Dnode::*ins = Dnode::outs in the reverse call.

  for (Int i=0; i< node->*outCount; i++) {
    Dedge* edge = (node->*outs)[i];
    DispatchNode* next = (DispatchNode*) (edge->*to);
    if (next == leader)  // Delink from leader
      detachEdge (leader->*ins, leader->*inCount, edge);
    if (next->isSccMember == false) {   // next is not in the SCC.
      Time movedDistance = edge->length + offset;
      check_error(!(movedDistance > MAX_LENGTH || movedDistance < MIN_LENGTH),
                  "Dispatchability edge with length too large or too small",
                  TempNetErr::TempNetInternalError());
      // Look for an *out edge of the leader that also points *to next
      Dedge* leaderEdge = 0;
      for (Int j=0; j < leader->*outCount; j++) {
        Dedge* e = (leader->*outs)[j];
        Dnode* eTo = (Dnode*)(e->*to);
        if (eTo == next)
          leaderEdge = e;
      }
      if (leaderEdge) {
        // Move the constraint to the leaderEdge and delink edge.
        if (movedDistance < leaderEdge->length)
          leaderEdge->length = movedDistance;
        detachEdge (next->*ins, next->*inCount, edge);
      }
      else {
        // Modify and redirect the edge.
        edge->length = movedDistance;
        edge->*from = leader;
        attachEdge (leader->*outs, leader->*outSize, leader->*outCount, edge);
      }
      // No need to remove edge from node->*outs because
      // node will be unreachable from sccLeaders.
    }
  }
}

void DispatchGraph::findKeptEdges (DispatchNode* source,
                                   void (*keepEdge)(DispatchNode*,
                                                    DispatchNode*,
                                                    Time))
{
  // This computes what edges to keep among those that are outside
  // the SCCs.  These are the fluid (non-rigid) edges.
  Int leaderCount = this->sccLeaders.size();
  Dnode::unmarkAll();
  Int position = leaderCount;
  // Following collects nodes downward into reverse-postorder
  // while decrementing position.
  predGraphDfs (source, position);
  //
  // First find lower dominators = minimal negative-distance nodes.
  // Minimality determined by marking descendants.
  Dnode::unmarkAll();
  for (Int i=position; i < leaderCount; i++) {
    DispatchNode* node = this->reversePostorder[i];
    if ( !node->isMarked() && node->distance < 0 ) {
      // Found minimal (= unmarked) neg-distance node
      (*keepEdge) (source, node, node->distance);
      node->mark();
    }
    // Propagate mark to pred-graph children.
    if ( node->isMarked() ) {
      for (Int j=0; j < node->outCount; j++) {
        Dedge* edge = node->outArray[j];
        DispatchNode* child = (DispatchNode*) edge->to;
        if ( node->distance + edge->length == child->distance )
          child->mark();
      }
    }
  }
  // Next we find the upper dominators.
  // Propagate mins of ancestors (except source node).
  source->minDistance = POS_INFINITY;  // source distance not included in min.
  for (Int j=position+1; j < leaderCount; j++) {
    DispatchNode* node = this->reversePostorder[j];
    Time minDistance = POS_INFINITY;
    for (Int k=0; k < node->inCount; k++) {
      Dedge* edge = node->inArray[k];
      DispatchNode* parent = (DispatchNode*) edge->from;
      if ( parent->distance + edge->length == node->distance  // Pred graph
           && parent->minDistance < minDistance)
        minDistance = parent->minDistance;
    }
    if (minDistance > node->distance) {  // Not upper-dominated.
      if (node->distance >= 0)
        (*keepEdge) (source, node, node->distance);
      node->minDistance = node->distance;
    }
    else
      node->minDistance = minDistance;
  }
}

} /* namespace Europa */
