
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

#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

namespace EUROPA {


namespace {
template <class ELEMENT>
void deleteFirstIfEqual(std::vector<ELEMENT>& elements, ELEMENT element){
  for(typename std::vector<ELEMENT>::iterator it = elements.begin(); it != elements.end(); ++it){
    if((*it) == element){
      elements.erase(it);
      break;
      // return;
    }
  }
  //elements.erase(std::remove(elements.begin(), elements.end(), element), elements.end());
}

template <typename ELEMENT>
void deleteIfEqual(std::vector<ELEMENT>& elements, ELEMENT element){
  elements.erase(std::remove(elements.begin(), elements.end(), element), elements.end());
}


}

// Global value overridden only for Rax-derived system test.
// Bool IsOkToRemoveConstraintTwice = false;

DistanceGraph::DistanceGraph() : edges(), dijkstraGeneration(0), nodes(),
                                 dqueue(new Dqueue()),
                                 bqueue(new BucketQueue(100)), edgeNogoodList()
{
}

DistanceGraph::~DistanceGraph()
{
  this->nodes.clear();
}

void DistanceGraph::addNode(DnodeId node) {
  node->potential = 0;
  this->nodes.push_back(node);
  
}

DnodeId DistanceGraph::makeNode()
{
  return boost::make_shared<Dnode>();
}


DnodeId DistanceGraph::createNode()
{
  DnodeId node = makeNode();
  check_error(node, "Memory allocation failed for TemporalNetwork node",
              TempNetErr::TempNetMemoryError());
  addNode(node);
  return node;
}

Void DistanceGraph::attachEdge(std::vector<Dedge*>& edgeArray, Int& size, Int& count,
                               Dedge& edge) {
  check_error(!(count > size), "Corrupted edge-array in TemporalNetwork",
              TempNetErr::TempNetInternalError());

  edgeArray.push_back(&edge);
  count = edgeArray.size();
  size = edgeArray.capacity();
}


Void DistanceGraph::detachEdge(std::vector<Dedge*>& edgeArray, Int& count,
                               const Dedge& edge)
{
  edgeArray.erase(std::find(edgeArray.begin(), edgeArray.end(), &edge));
  count = edgeArray.size();
}

Void DistanceGraph::deleteNode(Dnode& node)
{
  check_error(isValid(node), "node is not defined in this graph");

  for (Int i=0; i < node.outCount; i++) {
    Dedge* edge = node.outArray[i];
    detachEdge(edge->to.inArray, edge->to.inCount, *edge);
    eraseEdge(*edge);
  }
  for (Int j=0; j < node.inCount; j++) {
    Dedge* edge = node.inArray[j];
    detachEdge(edge->from.outArray, edge->from.outCount, *edge);
    eraseEdge(*edge);
  }
  node.inCount = node.outCount = 0;
  node.potential = 99;  // A clue for debugging purposes
  nodes.erase(std::remove_if(nodes.begin(), nodes.end(), ptr_compare<Dnode>(&node)),
              nodes.end());
}

Dedge* DistanceGraph::findEdge(Dnode& from, Dnode& to)
{
 check_error(isValid(from), "node is not defined in this graph");
 check_error(isValid(to),   "node is not defined in this graph");

  // Cache node vars -- Chucko 22 Apr 2002 
  Int fromOutCount = from.outCount; 
  if (fromOutCount > 0) {
    /*
    DedgeId* fromOutArray = from->outArray;
    for (Int i=0; i < fromOutCount; i++) {
      DedgeId edge = fromOutArray[i];
      if (to == edge->to)
	return edge;
    }
    */
    // PHM 06/20/2007 Speedup by using map instead.
    return from.edgemap[&to];
  }
  return NULL;//DedgeId();
}

Dedge* DistanceGraph::createEdge(Dnode& from, Dnode& to, Time length) {
  using boost::ref;
  check_error(isValid(from), "node is not defined in this graph");
  check_error(isValid(to), "node is not defined in this graph");


  DedgeId edge = boost::make_shared<Dedge>(ref(from), ref(to));
  check_error(edge, "Memory allocation failed for TemporalNetwork edge",
              TempNetErr::TempNetMemoryError());

  edge->length = length;
  this->edges.insert(edge);
  attachEdge (from.outArray, from.outArraySize, from.outCount, *edge.get());
  attachEdge (to.inArray, to.inArraySize, to.inCount, *edge.get());
  from.edgemap[&to] = edge.get();
  return edge.get();
}

void DistanceGraph::handleNodeUpdate(const Dnode&) {}

Void DistanceGraph::deleteEdge(Dedge& edge)
{
  detachEdge (edge.from.outArray, edge.from.outCount, edge);
  detachEdge (edge.to.inArray, edge.to.inCount, edge);
  edge.from.edgemap.erase(&edge.to);
  eraseEdge(edge);
}

Void DistanceGraph::eraseEdge(Dedge& edge)
{
  //deleteIfEqual(edges, edge);

  // edges.erase(edge);
  edge.length = 99;  // A clue for debugging purposes
  edges.erase(std::find_if(edges.begin(), edges.end(), ptr_compare<Dedge>(&edge)));
}

Void DistanceGraph::addEdgeSpec(Dnode& from, Dnode& to, Time length)
{

  check_error(!(length > MAX_LENGTH || length < MIN_LENGTH), 
              "addEdgeSpec with length too large or too small",
              TempNetErr::TempNetInternalError());

  Dedge* edge = findEdge(from,to);
  if (edge == NULL)
    edge = createEdge(from,to,length);
  edge->lengthSpecs.push_back(length);
  if (length < edge->length)
    edge->length = length;
}

Void DistanceGraph::removeEdgeSpec(Dnode& from, Dnode& to, Time length)
{

 check_error(isValid(from), "node is not defined in this graph");
 check_error(isValid(to), "node is not defined in this graph");

  check_error(!(length > MAX_LENGTH || length < MIN_LENGTH),
              "removeEdgeSpec with length too large or too small",
              TempNetErr::TempNetInternalError());

  Dedge* edge = findEdge (from,to);
  check_error(edge, "Removing spec from non-existent edge",
              TempNetErr::TempNetInternalError());

  std::vector<Time>& lengthSpecs = edge->lengthSpecs;

  deleteFirstIfEqual(lengthSpecs, length);

  if (lengthSpecs.empty())
    deleteEdge(*edge);
  else {
    edge->length = *std::min_element(lengthSpecs.begin(), lengthSpecs.end());
  }  
}

Bool DistanceGraph::bellmanFord()
{
  BucketQueue& queue = initializeBqueue();
  for (std::vector<DnodeId>::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
    DnodeId node = *it;
    Time oldPotential = node->potential;
    // Cache beginning potential in distance field.
    node->distance = oldPotential;
    node->potential = 0;
    node->depth = 0;
    // Use diff from oldPotential as priority ordering.  This
    // minimizes the amount of wasted superseded propagations.
    queue.insertInQueue (node.get(), -oldPotential);
  }
  Int BFbound = static_cast<int>(nodes.size());
  while (true) {
    Dnode* node = queue.popMinFromQueue();
    if (node == NULL)
      break;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    if (nodeOutCount > 0) {
      std::vector<Dedge*>& nodeOutArray = node->outArray;
      Time nodePotential = node->potential;
      for (Int i=0; i< nodeOutCount; i++) {
	Dedge* edge = nodeOutArray[i];
	check_error(edge); 
	Dnode& next = edge->to;
	Time potential = nodePotential + edge->length;
	if (potential < next.potential) {
	  next.potential = potential;
	  next.predecessor = edge;
	  handleNodeUpdate(next);
	  // In following cycleDetected() is a no-op hook to allow
	  // specialized cycle detectors to be defined in subclasses
	  // ** Try to keep results in registers for speed.
	  // Chucko 23 Apr 2002
	  if ((next.depth = node->depth + 1) > BFbound  // Exceeded BF limit.
	      || cycleDetected (next)) {
	    updateNogoodList(next);
	    return false;
	  }
          Time oldPotential = next.distance; // See earlier in function
          // Use diff from oldPotential as priority ordering.  This
          // minimizes the amount of wasted superseded propagations.
          queue.insertInQueue (&next, potential - oldPotential);
	}
      }
    }
  }
  return true;
}

Bool DistanceGraph::incBellmanFord()
{
  Int BFbound = static_cast<Int>(nodes.size());

  preventGenerationOverflow();
  ++dijkstraGeneration;

  while (true) {
    Dnode* node = bqueue->popMinFromQueue();
    if (node == NULL)
      break;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    if (nodeOutCount > 0) {
      std::vector<Dedge*>& nodeOutArray = node->outArray;
      Time nodePotential = node->potential;
      for (Int i=0; i< nodeOutCount; i++) {
	Dedge* edge = nodeOutArray[i];
	check_error(edge);
	Dnode& next = edge->to;
	Time potential = nodePotential + edge->length;

	if (potential < next.potential) {
  check_error(!(potential < MIN_DISTANCE),
              "Potential underflow during distance propagation",
              TempNetErr::TimeOutOfBoundsError());

          // Cache the beginning potential in next->distance
          if (next.generation < this->dijkstraGeneration) {
            next.generation = this->dijkstraGeneration;
            next.distance = next.potential;
          }
          Time oldPotential = next.distance;
   
	  next.potential = potential;
	  next.predecessor = edge;
	  handleNodeUpdate(next);

	  // In following cycleDetected() is a no-op hook to allow
	  // specialized cycle detectors to be defined in subclasses
	  // ** Try to keep results in registers for speed.
	  // Chucko 23 Apr 2002
	  if ((next.depth = node->depth + 1) > BFbound  // Exceeded BF limit.
	      || cycleDetected (next)) {
	    updateNogoodList(next);
	    return false;
	  }
          // Give priority to "stronger" propagations.  This minimizes
          // the amount of wasted superseded propagations.
          bqueue->insertInQueue(&next, potential - oldPotential);
	}
      }
    }
  }
  return true;
}

Void DistanceGraph::dijkstra(Dnode& source, Dnode* destination)
{
 check_error(isValid(source), "node is not defined in this graph");

 // PHM 05/16/2007 The previous isValid(destination) check was
 // mistaken; a null destination was intended to be allowed; in that
 // case dijkstra computes the distance to ALL nodes in the graph.
 // (See DistanceGraph.hh, which has destination = noId() as default!)

 check_error(destination == NULL || isValid(*destination),
             "node is not null or defined in this graph");

 //debugMsg("DistanceGraph:dijkstra", "from " << source << " to " << destination);
  source.distance = 0;
  source.depth=0;
  preventGenerationOverflow();
  Int generation = ++(this->dijkstraGeneration);
  source.generation = generation;
  BucketQueue& queue = initializeBqueue();
  queue.insertInQueue(&source);
#ifndef EUROPA_FAST
  Int BFbound = static_cast<Int>(this->nodes.size());
#endif
  while (true) {
    Dnode* node = queue.popMinFromQueue();
    //debugMsg("DistanceGraph:dijkstra", "Visiting " << node);
    if (node == NULL || node == destination)
      return;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    if (nodeOutCount > 0) {
      std::vector<Dedge*>& nodeOutArray = node->outArray;
      Time nodeDistance = node->distance;
      for (Int i=0; i< nodeOutCount; i++) {
	Dedge* edge = nodeOutArray[i];
	Dnode& next = edge->to;
	Time newDistance = nodeDistance + edge->length;
	/*
	condDebugMsg(next->generation >= generation, 
		     "DistanceGraph:dijkstra", next->generation << " <= " << generation << " for " << next);

      condDebugMsg(newDistance >= next->distance, 
		     "DistanceGraph:dijkstra", newDistance << " >= " <<  next->distance << " for " << next);
	*/
	if (newDistance > MAX_DISTANCE)
	  continue;
	if (next.generation < generation || newDistance < next.distance) {
	  next.generation = generation;
    check_error(!(newDistance < MIN_DISTANCE),
                "Potential underflow during distance propagation",
                TempNetErr::TimeOutOfBoundsError());
	  // Next check is a failsafe to prevent infinite propagation.
    check_error(!((next.depth = node->depth + 1) > BFbound),
                "Dijkstra propagation in inconsistent network",
                TempNetErr::TempNetInternalError());
	  next.distance = newDistance;
	  next.predecessor = edge;
	  queue.insertInQueue (&next);
	  //debugMsg("DistanceGraph:dijkstra", "New distance of " << newDistance << " through node " << next);
	  handleNodeUpdate(next);
	}
      }
    }
  }
}

Time DistanceGraph::getDistance(const Dnode& node)
{
  check_error(isValid(node), "node is not defined in this graph");
  if (node.generation == this->dijkstraGeneration)
    return node.distance;
  else
    return POS_INFINITY;
}

Bool DistanceGraph::isDistanceLessThan (Dnode& src, Dnode& targ, Time bound)
{
 check_error(isValid(src), "node is not defined in this graph");
 check_error(isValid(targ), "node is not defined in this graph");

  // Depth-first simulated propagation from src to targ as if
  // a -bound constraint had been added from targ to src.
  // This would cause a cycle iff isDistanceLessThan is true,
  // so we need only check if the propagation reaches targ.

  preventNodeMarkOverflow();
  Dnode::unmarkAll();
  Time newPotential = targ.potential - bound;

  if (bound == 1) {
    // In this case, the call is always from isSlotDurationZero().
    // For efficiency, we use the approximation of only checking
    // paths with all zero links.
    return (isAllZeroPropagationPath(src, targ, newPotential));
  }

  return isPropagationPath(src, targ, newPotential);
}

Bool DistanceGraph::isValid(const Dnode& node) {
  return hasNode(node);
}

Bool DistanceGraph::isAllZeroPropagationPath(Dnode& node, Dnode& targ,
                                             Time potential)
{

 check_error(isValid(node), "node is not defined in this graph");
 check_error(isValid(targ), "node is not defined in this graph");

  if (potential >= node.potential)  // propagation is ineffective
    return false;
  if (&node == &targ)
    return true;
  if (node.isMarked())
    return false;
  node.mark();
  // Cache node vars -- Chucko 22 Apr 2002
  Int nodeOutCount = node.outCount;
  if (nodeOutCount > 0) {
    std::vector<Dedge*>& nodeOutArray = node.outArray;
    for (int i=0; i< nodeOutCount; i++) {
      Dedge* edge = nodeOutArray[i];
      Time length = edge->length;
      if (length == 0)
	if (isAllZeroPropagationPath(edge->to, targ, potential))
	  return true;
    }
  }
  return false;
}

Bool DistanceGraph::isPropagationPath(Dnode& src, Dnode& targ, Time pot) {

  check_error(isValid(src), "node is not defined in this graph");
  check_error(isValid(targ), "node is not defined in this graph");


  // Even though this seems like a full search, it is actually an
  // approximation because the marking scheme could possibly prevent a
  // new propagation with a smaller potential across nodes that have
  // previously been propagated across.  Would need a Dijkstra-like
  // priority queue to do a full search, but this seems a good enough
  // approximation to satisfy the calls from the zigzag check.
  if (pot >= src.potential)  // propagation is ineffective
    return false;
  src.mark();
  src.distance = pot;
  Dnode* propQ = &src; 
  propQ->link = NULL;
  while (propQ != NULL) {
    Dnode* node = propQ; propQ = propQ->link;
    // Cache node vars -- Chucko 22 Apr 2002
    Int nodeOutCount = node->outCount;
    std::vector<Dedge*>& nodeOutArray = node->outArray;
    // We iterate downwards to simulate the behavior of the previous
    // recursive version of this function (to satisfy make tests).
    for (int i=nodeOutCount-1; i>=0 ; i--) {
      Dedge* edge = nodeOutArray[i];
      Dnode& next = edge->to;
      if (next.isMarked())
        continue;
      Time newPotential = node->distance + edge->length;
      if (newPotential >= next.potential)  // propagation is ineffective
        continue;  // Don't mark---may be later effective propagation
      if (&next == &targ)
        return true;
      next.mark();
      next.distance = newPotential;
      next.link = propQ; propQ = &next;
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

Void DistanceGraph::updateNogoodList(Dnode& start)
{

  preventNodeMarkOverflow();
  Dnode::unmarkAll();
  Dnode* node = &start;
  // Search for predecessor cycle
  while (! node->isMarked()) {
    node->mark ();
    Dedge* predEdge = node->predecessor;
    check_error(predEdge,
                "Broken predecessor chain",
                TempNetErr::TempNetInternalError());
    node = &predEdge->from;
  }
  // Now trace out the cycle into edgeNogoodList
  edgeNogoodList.clear();
  Dnode* node1 = node;
  Dedge* edge;
  do {
    edge = node1->predecessor;
    edgeNogoodList.push_back(edge);
    node1 = &edge->from;
  }
  while (node1 != node);
}

Void DistanceGraph::preventNodeMarkOverflow()
{
  // Unlikely to happen, but just in case...
  if (Dnode::markGlobal == INT_MAX) {
    // Roll all marks over to zero.
    unsigned long nodeCount = this->nodes.size();
    for (unsigned long i=0; i< static_cast<unsigned long>(nodeCount); i++)
      nodes[i]->markLocal = 0;
    Dnode::markGlobal = 0;
  }
}

Void DistanceGraph::preventGenerationOverflow()
{
  // Unlikely to happen, but just in case...
  if (this->dijkstraGeneration == INT_MAX) {
    // Roll all generations over to zero.
    unsigned long nodeCount = this->nodes.size();
    for (unsigned long i=0; i< nodeCount; i++)
      this->nodes[i]->generation = 0;
    this->dijkstraGeneration = 0;
  }
}

//TODO: Clean this up?  Seems like it's never called.
Dqueue& DistanceGraph::initializeDqueue()
{
  preventNodeMarkOverflow();
  dqueue->reset();
  return *dqueue;
}

BucketQueue& DistanceGraph::initializeBqueue()
{
  preventNodeMarkOverflow();
  bqueue->reset();
  return *bqueue;
}

bool DistanceGraph::hasNode(const Dnode& node) const {
  return std::find_if(nodes.begin(), nodes.end(), ptr_compare<Dnode>(&node)) !=
      nodes.end();
}

std::string DistanceGraph::toString() const {
 std::stringstream sstr;

 for (std::set<DedgeId>::const_iterator it = edges.begin(); it != edges.end(); ++it){
   DedgeId edge = *it;
   sstr << &edge->from << " " << &edge->to << " " << edge->length << std::endl;
 }

 return sstr.str();
}

Void DistanceGraph::boundedDijkstra (Dnode& source,
                                     Time bound,
                                     Time destPotential,
                                     int direction) {
  source.distance = 0;
  source.depth=0;
  preventGenerationOverflow();
  Int generation = ++(this->dijkstraGeneration);
  source.generation = generation;
  BucketQueue& queue = initializeBqueue();
  queue.insertInQueue (&source);

  check_error_variable(Int BFbound = static_cast<Int>(this->nodes.size()));
  while (true) {
    Dnode* node = queue.popMinFromQueue();
    if (node == NULL)
      return;
    Int nodeCount = (direction == -1) ? node->inCount : node->outCount;
    if (nodeCount > 0) {
      std::vector<Dedge*>& nodeArray = (direction == -1) ? node->inArray : node->outArray;
      Time nodeDistance = node->distance;
      for (Int i=0; i< nodeCount; i++) {
        Dedge* edge = nodeArray[i];
        Dnode& next = (direction == -1) ? edge->from : edge->to;
        Time newDistance = nodeDistance + edge->length;

        // Admissible estimate of remaining distance to go
        Time toGo = direction * (destPotential - next.potential);

        if (newDistance + toGo >= bound)
          continue;

        if (next.generation < generation || newDistance < next.distance) {
          next.generation = generation;
          check_error(!(newDistance > MAX_DISTANCE ||
                        newDistance < MIN_DISTANCE),
                      "Out of bounds during distance propagation",
                      TempNetErr::TimeOutOfBoundsError());
          // Next check is a failsafe to prevent infinite propagation.
          check_error(!((next.depth = node->depth + 1) > BFbound),
                      "Dijkstra propagation in inconsistent network",
                      TempNetErr::TempNetInternalError());
          next.distance = newDistance;
          queue.insertInQueue(&next, newDistance + toGo);
        }
      }
    }
  }
}


} /* namespace Europa */
