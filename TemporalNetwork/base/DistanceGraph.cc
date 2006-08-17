
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

#include <stdlib.h>
#include <limits.h>
#include <sstream>

#include "DistanceGraph.hh"
#include "Error.hh"
#include "Utils.hh"
//#include "Debug.hh"

namespace EUROPA {


template <class ELEMENT>
void deleteIfEqual(std::vector<ELEMENT>& elements, ELEMENT element){
  for(typename std::vector<ELEMENT>::iterator it = elements.begin(); it != elements.end(); ++it){
    if((*it) == element){
      elements.erase(it);
      return;
    }
  }
}

// Global value overridden only for Rax-derived system test.
Bool IsOkToRemoveConstraintTwice = false;

DistanceGraph::DistanceGraph ()
{
  dijkstraGeneration=0;
  dqueue= new Dqueue;
  bqueue= new BucketQueue(100);
}

DistanceGraph::~DistanceGraph()
{
  cleanup(edges);
  cleanup(nodes);
  delete dqueue;
  delete bqueue;
}

DnodeId DistanceGraph::makeNode()
{
  return (new Dnode())->getId();
}


DnodeId DistanceGraph::createNode()
{
  DnodeId node = makeNode();
  check_error(node, "Memory allocation failed for TemporalNetwork node",
              TempNetErr::TempNetMemoryError());

  node->potential = 0;
  this->nodes.push_back(node);
  return node;
}

Void detachEdge (DedgeId*& edgeArray, Int& count, DedgeId edge);

Void DistanceGraph::deleteNode(DnodeId node)
{
  check_error(isValid(node), "node is not defined in this graph");

  for (Int i=0; i < node->outCount; i++) {
    DedgeId edge = node->outArray[i];
    detachEdge (edge->to->inArray, edge->to->inCount, edge);
    eraseEdge(edge);
  }
  for (Int j=0; j < node->inCount; j++) {
    DedgeId edge = node->inArray[j];
    detachEdge (edge->from->outArray, edge->from->outCount, edge);
    eraseEdge(edge);
  }
  node->inCount = node->outCount = 0;
  node->potential = 99;  // A clue for debugging purposes
  deleteIfEqual(nodes, node);
  delete (Dnode*) node;
}

DedgeId DistanceGraph::findEdge(DnodeId from, DnodeId to)
{
 check_error(isValid(from), "node is not defined in this graph");
 check_error(isValid(to),   "node is not defined in this graph");

  // Cache node vars -- Chucko 22 Apr 2002 
  Int fromOutCount = from->outCount; 
  if (fromOutCount > 0) {
    DedgeId* fromOutArray = from->outArray;
    for (Int i=0; i < fromOutCount; i++) {
      DedgeId edge = fromOutArray[i];
      if (to == edge->to)
	return edge;
    }
  }
  return DedgeId::noId();
}

Void attachEdge (DedgeId*& edgeArray, Int& size, Int& count, DedgeId edge)
{
  check_error(!(count > size), "Corrupted edge-array in TemporalNetwork",
              TempNetErr::TempNetInternalError());

  if (count == size) {
    // Grow edge-array
    if (size < 1)
      size = 1;
    else
      size = 2*size;
    DedgeId* newEdgeArray = new DedgeId[size];
    if(!newEdgeArray)
      handle_error(!newEdgeArray,
                   "Memory allocation failed for TemporalNetwork edge-array",
                   TempNetErr::TempNetMemoryError());
    for (Int i=0; i<count; i++)
      newEdgeArray[i] = edgeArray[i];
    if (edgeArray != nullptr)  // edgeArray starts out as null.
      delete[] edgeArray;
    edgeArray = newEdgeArray;
  }
  edgeArray[count++] = edge;
}

Void detachEdge (DedgeId*& edgeArray, Int& count, DedgeId edge)
{
  Int i = 0;
  while (i < count && edgeArray[i] != edge)
      i++;
  check_error(!(i == count && IsOkToRemoveConstraintTwice),
              "Trying to delete edge not in edge-array",
              TempNetErr::TempNetInternalError());

  for (--count; i < count; i++)
    edgeArray[i] = edgeArray[i + 1];
}

DedgeId DistanceGraph::createEdge(DnodeId from, DnodeId to, Time length)
{

 check_error(isValid(from), "node is not defined in this graph");
 check_error(isValid(to), "node is not defined in this graph");


  DedgeId edge = (new Dedge())->getId();
  check_error(edge, "Memory allocation failed for TemporalNetwork edge",
              TempNetErr::TempNetMemoryError());

  edge->from = from;
  edge->to = to;
  edge->length = length;
  this->edges.push_back(edge);
  attachEdge (from->outArray, from->outArraySize, from->outCount, edge);
  attachEdge (to->inArray, to->inArraySize, to->inCount, edge);
  return edge;
}

Void DistanceGraph::deleteEdge(DedgeId edge)
{
  detachEdge (edge->from->outArray, edge->from->outCount, edge);
  detachEdge (edge->to->inArray, edge->to->inCount, edge);
  eraseEdge(edge);
}

Void DistanceGraph::eraseEdge(DedgeId edge)
{
  deleteIfEqual(edges, edge);
  edge->from = DnodeId::noId();
  edge->to = DnodeId::noId();
  edge->length = 99;  // A clue for debugging purposes
  delete (Dedge*) edge;
}

Void DistanceGraph::addEdgeSpec(DnodeId from, DnodeId to, Time length)
{

  check_error(!(length > MAX_LENGTH || length < MIN_LENGTH), 
              "addEdgeSpec with length too large or too small",
              TempNetErr::TempNetInternalError());

  DedgeId edge = findEdge (from,to);
  if (edge.isNoId())
    edge = createEdge(from,to,length);
  edge->lengthSpecs.push_back(length);
  if (length < edge->length)
    edge->length = length;
}

Void DistanceGraph::removeEdgeSpec(DnodeId from, DnodeId to, Time length)
{

 check_error(isValid(from), "node is not defined in this graph");
 check_error(isValid(to), "node is not defined in this graph");

  check_error(!(length > MAX_LENGTH || length < MIN_LENGTH),
              "removeEdgeSpec with length too large or too small",
              TempNetErr::TempNetInternalError());

  DedgeId edge = findEdge (from,to);
  check_error(edge.isValid(), "Removing spec from non-existent edge",
              TempNetErr::TempNetInternalError());

  std::vector<Time>& lengthSpecs = edge->lengthSpecs;

  deleteIfEqual(lengthSpecs, length);

  if (lengthSpecs.empty())
    deleteEdge(edge);
  else {
    Time min = lengthSpecs.front();
    for(std::vector<Time>::const_iterator it = lengthSpecs.begin(); it != lengthSpecs.end(); ++it){
      Time current = *it;
      if (current < min)
        min = current;
    }
    edge->length = min;
  }  
}

Bool DistanceGraph::bellmanFord()
{
  BucketQueue* queue = initializeBqueue();
  for (std::vector<DnodeId>::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
    DnodeId node = *it;
    Time oldPotential = node->potential;
    // Cache beginning potential in distance field.
    node->distance = oldPotential;
    node->potential = 0;
    node->depth = 0;
    // Use diff from oldPotential as priority ordering.  This
    // minimizes the amount of wasted superseded propagations.
    queue->insertInQueue (node, -oldPotential);
  }
  Int BFbound = nodes.size();
  while (true) {
    DnodeId node = queue->popMinFromQueue();
    if (node.isNoId())
      break;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    if (nodeOutCount > 0) {
      DedgeId* nodeOutArray = node->outArray;
      Time nodePotential = node->potential;
      for (Int i=0; i< nodeOutCount; i++) {
	DedgeId edge = nodeOutArray[i];
	check_error(edge.isValid()); 
	DnodeId next = edge->to;
	Time potential = nodePotential + edge->length;
	if (potential < next->potential) {
	  next->potential = potential;
	  next->predecessor = edge;
	  handleNodeUpdate(next);
	  // In following cycleDetected() is a no-op hook to allow
	  // specialized cycle detectors to be defined in subclasses
	  // ** Try to keep results in registers for speed.
	  // Chucko 23 Apr 2002
	  if ((next->depth = node->depth + 1) > BFbound  // Exceeded BF limit.
	      || cycleDetected (next)) {
	    updateNogoodList(next);
	    return false;
	  }
          Time oldPotential = next->distance; // See earlier in function
          // Use diff from oldPotential as priority ordering.  This
          // minimizes the amount of wasted superseded propagations.
          queue->insertInQueue (next, potential - oldPotential);
	}
      }
    }
  }
  return true;
}

Bool DistanceGraph::incBellmanFord()
{
  Int BFbound = nodes.size();
  //Dqueue* queue = dqueue;
  BucketQueue* queue = bqueue;

  preventGenerationOverflow();
  ++dijkstraGeneration;

  while (true) {
    DnodeId node = queue->popMinFromQueue();
    if (node.isNoId())
      break;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    if (nodeOutCount > 0) {
      DedgeId* nodeOutArray = node->outArray;
      Time nodePotential = node->potential;
      for (Int i=0; i< nodeOutCount; i++) {
	DedgeId edge = nodeOutArray[i];
	check_error(edge.isValid());
	DnodeId next = edge->to;
	Time potential = nodePotential + edge->length;

  check_error(!(potential > MAX_DISTANCE || potential < MIN_DISTANCE),
              "Potential over(under)flow during distance propagation",
              TempNetErr::TimeOutOfBoundsError());
	if (potential < next->potential) {

          // Cache the beginning potential in next->distance
          if (next->generation < this->dijkstraGeneration) {
            next->generation = this->dijkstraGeneration;
            next->distance = next->potential;
          }
          Time oldPotential = next->distance;
   
	  next->potential = potential;
	  next->predecessor = edge;
	  handleNodeUpdate(next);

	  // In following cycleDetected() is a no-op hook to allow
	  // specialized cycle detectors to be defined in subclasses
	  // ** Try to keep results in registers for speed.
	  // Chucko 23 Apr 2002
	  if ((next->depth = node->depth + 1) > BFbound  // Exceeded BF limit.
	      || cycleDetected (next)) {
	    updateNogoodList(next);
	    return false;
	  }
          // Give priority to "stronger" propagations.  This minimizes
          // the amount of wasted superseded propagations.
          queue->insertInQueue (next, potential - oldPotential);
	}
      }
    }
  }
  return true;
}

Void DistanceGraph::dijkstra (DnodeId source, DnodeId destination)
{
 check_error(isValid(source), "node is not defined in this graph");
 check_error(isValid(destination), "node is not defined in this graph");
 //debugMsg("DistanceGraph:dijkstra", "from " << source << " to " << destination);
  source->distance = 0;
  source->depth=0;
  preventGenerationOverflow();
  Int generation = ++(this->dijkstraGeneration);
  source->generation = generation;
  BucketQueue* queue = initializeBqueue();
  queue->insertInQueue (source);
#ifndef EUROPA_FAST
  Int BFbound = this->nodes.size();
#endif
  while (true) {
    DnodeId node = queue->popMinFromQueue();
    //debugMsg("DistanceGraph:dijkstra", "Visiting " << node);
    if (node.isNoId() || node == destination)
      return;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    if (nodeOutCount > 0) {
      DedgeId* nodeOutArray = node->outArray;
      Time nodeDistance = node->distance;
      for (Int i=0; i< nodeOutCount; i++) {
	DedgeId edge = nodeOutArray[i];
	DnodeId next = edge->to;
	Time newDistance = nodeDistance + edge->length;
	/*
	condDebugMsg(next->generation >= generation, 
		     "DistanceGraph:dijkstra", next->generation << " <= " << generation << " for " << next);

      condDebugMsg(newDistance >= next->distance, 
		     "DistanceGraph:dijkstra", newDistance << " >= " <<  next->distance << " for " << next);
	*/
	if (next->generation < generation || newDistance < next->distance) {
	  next->generation = generation;
    check_error(!(newDistance > MAX_DISTANCE || newDistance < MIN_DISTANCE),
                "Potential over(under)flow during distance propagation",
                TempNetErr::TimeOutOfBoundsError());
	  // Next check is a failsafe to prevent infinite propagation.
    check_error(!((next->depth = node->depth + 1) > BFbound),
                "Dijkstra propagation in inconsistent network",
                TempNetErr::TempNetInternalError());
	  next->distance = newDistance;
	  next->predecessor = edge;
	  queue->insertInQueue (next);
	  //debugMsg("DistanceGraph:dijkstra", "New distance of " << newDistance << " through node " << next);
	  handleNodeUpdate(next);
	}
      }
    }
  }
}

Time DistanceGraph::getDistance(DnodeId node)
{
  check_error(isValid(node), "node is not defined in this graph");
  if (node->generation == this->dijkstraGeneration)
    return node->distance;
  else
    return POS_INFINITY;
}

Bool DistanceGraph::isDistanceLessThan (DnodeId src, DnodeId targ, Time bound)
{
 check_error(isValid(src), "node is not defined in this graph");
 check_error(isValid(targ), "node is not defined in this graph");

  // Depth-first simulated propagation from src to targ as if
  // a -bound constraint had been added from targ to src.
  // This would cause a cycle iff isDistanceLessThan is true,
  // so we need only check if the propagation reaches targ.

  preventNodeMarkOverflow();
  Dnode::unmarkAll();
  Time newPotential = targ->potential - bound;

  if (bound == 1) {
    // In this case, the call is always from isSlotDurationZero().
    // For efficiency, we use the approximation of only checking
    // paths with all zero links.
    return (isAllZeroPropagationPath(src, targ, newPotential));
  }

  return isPropagationPath(src, targ, newPotential);
}

Bool DistanceGraph::isValid(DnodeId node) {
  return hasNode(node);
}

Bool DistanceGraph::isAllZeroPropagationPath(DnodeId node, DnodeId targ,
						 Time potential)
{

 check_error(isValid(node), "node is not defined in this graph");
 check_error(isValid(targ), "node is not defined in this graph");

  if (potential >= node->potential)  // propagation is ineffective
    return false;
  if (node == targ)
    return true;
  if (node->isMarked())
    return false;
  node->mark();
  // Cache node vars -- Chucko 22 Apr 2002
  Int nodeOutCount = node->outCount;
  if (nodeOutCount > 0) {
    DedgeId* nodeOutArray = node->outArray;
    for (int i=0; i< nodeOutCount; i++) {
      DedgeId edge = nodeOutArray[i];
      Time length = edge->length;
      if (length == 0)
	if (isAllZeroPropagationPath(edge->to, targ, potential))
	  return true;
    }
  }
  return false;
}

Bool DistanceGraph::isPropagationPath(DnodeId src, DnodeId targ, Time pot)
{

 check_error(isValid(src), "node is not defined in this graph");
 check_error(isValid(targ), "node is not defined in this graph");


  // Even though this seems like a full search, it is actually an
  // approximation because the marking scheme could possibly prevent a
  // new propagation with a smaller potential across nodes that have
  // previously been propagated across.  Would need a Dijkstra-like
  // priority queue to do a full search, but this seems a good enough
  // approximation to satisfy the calls from the zigzag check.
  if (pot >= src->potential)  // propagation is ineffective
    return false;
  src->mark();
  src->distance = pot;
  DnodeId propQ = src; 
  propQ->link = DnodeId::noId();
  while (!propQ.isNoId()) {
    DnodeId node = propQ; propQ = propQ->link;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    DedgeId* nodeOutArray = node->outArray;
    // We iterate downwards to simulate the behavior of the previous
    // recursive version of this function (to satisfy make tests).
    for (int i=nodeOutCount-1; i>=0 ; i--) {
      DedgeId edge = nodeOutArray[i];
      DnodeId next = edge->to;
      if (next->isMarked())
        continue;
      Time newPotential = node->distance + edge->length;
      if (newPotential >= next->potential)  // propagation is ineffective
        continue;  // Don't mark---may be later effective propagation
      if (next == targ)
        return true;
      next->mark();
      next->distance = newPotential;
      next->link = propQ; propQ = next;
    }
  }
  return false;
}

// Following marking method allows easy global unmarking.
// Mark is used to tell whether node is in queue.
// Note that an aborted propagation (due to detection of
// inconsistency) may leave some nodes still marked, so
// simple flipping of a Boolean is not enough.

// WARNING: If there are multiple distance graphs, care must be
// taken not to allow the global mark to cause interactions.
// Since the mark only increases, this won't happen provided
// procedures that rely on the marks, such as propagation, are
// not interleaved between different distance graphs.  If such
// interleaving is desired, then this marking mechanism must be
// replaced by one that is graph-specific.

Int Dnode::markGlobal = 0;
Void Dnode::unmarkAll() { (Dnode::markGlobal)++; }
Void Dnode::mark () { markLocal = Dnode::markGlobal; }
Bool Dnode::isMarked() { return (markLocal == Dnode::markGlobal); }
Void Dnode::unmark () { markLocal = Dnode::markGlobal - 1; }

Void DistanceGraph::updateNogoodList(DnodeId start)
{

  preventNodeMarkOverflow();
  Dnode::unmarkAll();
  DnodeId node = start;
  // Search for predecessor cycle
  while (! node->isMarked()) {
    node->mark ();
    DedgeId predEdge = node->predecessor;
    check_error(predEdge.isValid(),
                "Broken predecessor chain",
                TempNetErr::TempNetInternalError());
    node = predEdge->from;
  }
  // Now trace out the cycle into edgeNogoodList
  edgeNogoodList.clear();
  DnodeId node1 = node;
  DedgeId edge;
  do {
    edge = node1->predecessor;
    edgeNogoodList.push_back(edge);
    node1 = edge->from;
  }
  while (node1 != node);
}

Void DistanceGraph::preventNodeMarkOverflow()
{
  // Unlikely to happen, but just in case...
  if (Dnode::markGlobal == INT_MAX) {
    // Roll all marks over to zero.
    Int nodeCount = this->nodes.size();
    for (Int i=0; i< nodeCount; i++)
      nodes[i]->markLocal = 0;
    Dnode::markGlobal = 0;
  }
}

Void DistanceGraph::preventGenerationOverflow()
{
  // Unlikely to happen, but just in case...
  if (this->dijkstraGeneration == INT_MAX) {
    // Roll all generations over to zero.
    Int nodeCount = this->nodes.size();
    for (Int i=0; i< nodeCount; i++)
      this->nodes[i]->generation = 0;
    this->dijkstraGeneration = 0;
  }
}

Dqueue* DistanceGraph::initializeDqueue()
{
  preventNodeMarkOverflow();
  Dqueue* queue = this->dqueue;
  queue->reset();
  return queue;
}

BucketQueue* DistanceGraph::initializeBqueue()
{
  preventNodeMarkOverflow();
  BucketQueue* queue = this->bqueue;
  queue->reset();
  return queue;
}

bool DistanceGraph::hasNode(const DnodeId node) const {
  for(std::vector<DnodeId>::const_iterator it = nodes.begin(); it != nodes.end(); ++it){
    DnodeId dn = *it;
    if(node == dn)
      return true;
  }
  return false;
}

std::string DistanceGraph::toString() const {
 std::stringstream sstr;

 for (std::vector<DedgeId>::const_iterator it = edges.begin(); it != edges.end(); ++it){
   DedgeId edge = *it;
   sstr << edge->from << " " << edge->to << " " << edge->length << std::endl;
 }

 return sstr.str();
}

} /* namespace Europa */
