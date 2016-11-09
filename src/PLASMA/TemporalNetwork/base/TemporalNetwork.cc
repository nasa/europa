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

#include "TemporalNetworkDefs.hh"
#include "Domains.hh"
#include "TemporalNetwork.hh"
#include "Debug.hh"

#include <boost/cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

namespace EUROPA {

Bool TemporalNetwork::isValidId(const Timepoint* const id){
  return (id &&
          id->owner == this && hasNode(*id));
}
Bool TemporalNetwork::isValidId(const Timepoint& id){
  return (id.owner == this && hasNode(id));
}


Bool TemporalNetwork::isValidId(const TemporalConstraint* const id){
  return (id && id->owner == this);
}
Bool TemporalNetwork::isValidId(const TemporalConstraint& id){
  return (id.owner == this);
}



bool TemporalNetwork::hasEdgeToOrigin(Timepoint& timepoint) {
  // Order of operands is important for speed. Should be faster to look towards the origin
  Dedge* edgeToTheOrigin = findEdge(timepoint, getOrigin());
  checkError(edgeToTheOrigin == NULL || edgeToTheOrigin, edgeToTheOrigin);
  return edgeToTheOrigin != NULL;
}

TemporalNetwork::TemporalNetwork() : consistent(true), 
                                     hasDeletions(false), nodeCounter(0),
                                     incrementalSource(), m_constraints(), m_id(this),
                                     m_refpoint(), m_updatedTimepoints() {

  addTimepoint();
  fullPropagate();
}

  TemporalNetwork::~TemporalNetwork()
  {
  }

  DnodeId TemporalNetwork::makeNode()
  {
    // Overrides the definition in DistanceGraph class.
    TimepointId node = boost::make_shared<Tnode>(this);
    // PHM Support for reftime calculations
    node->prev_reftime = TIME_MAX; // will never == reftime
    if (m_refpoint != NULL) {
      if (m_refpoint->inCount == 0)
	node->reftime = POS_INFINITY;
      else
	node->reftime = NEG_INFINITY;
    }
    return node;
  }

  Bool TemporalNetwork::cycleDetected (const Dnode& next)
  {
    // Overrides the definition in DistanceGraph class.
    return (&next == this->incrementalSource);
  }

  Void TemporalNetwork::getTimepointBounds(const Timepoint& id, Time& lb, Time& ub)
  {
    // We need to be up-to-date to get the bounds.  Because of eager
    // prop on consistent additions, we only need to prop if there are
    // deletions.
    propagate();

    check_error(( this->isValid(id) ),
                "TemporalNetwork: Invalid timepoint identifier",
                TempNetErr::TempNetInvalidTimepointError());

    if(this->consistent){
      lb = id.lowerBound;
      ub = id.upperBound;
    }
    else{
      lb = 2;
      ub = -2;
    }
  }

Void TemporalNetwork::getLastTimepointBounds(const Timepoint& node, Time& lb, Time& ub) {
  check_error(( this->isValid(node) ),
              "TemporalNetwork: Invalid timepoint identifier",
              TempNetErr::TempNetInvalidTimepointError());

  lb = node.lowerBound;
  ub = node.upperBound;
}

Time TemporalNetwork::getLowerTimepointBound(const Timepoint& id) {
  Time result, junk;
  getTimepointBounds(id, result, junk);
  return(result);
}

Time TemporalNetwork::getUpperTimepointBound(const Timepoint& id) {
  Time result, junk;
  getTimepointBounds(id, junk, result);
  return(result);
}

  Bool TemporalNetwork::isDistanceLessThan(Timepoint& from, Timepoint& to,
                                           Time bound)
  {
    propagate();

    check_error(this->consistent,
                "TemporalNetwork: Checking distance in inconsistent network",
                TempNetErr::TempNetInconsistentError());
    return DistanceGraph::isDistanceLessThan(from, to, bound);
    // DistanceGraph* graph = boost::polymorphic_cast<DistanceGraph*>(this);
    // return graph->isDistanceLessThan(from, to, bound);
  }


Bool TemporalNetwork::isDistanceLessThanOrEqual(Timepoint& from,
                                                Timepoint& to,
                                                Time bound) {
  return isDistanceLessThan(from, to, bound + TIME_TICK);
}

Bool TemporalNetwork::isDistancePossiblyLessThan (const Timepoint& src,
                                                  const Timepoint& dest,
                                                  Time bound) {
  // An efficient approximate version of isDistanceLessThan.
  // (Performs the unrolled recursion only to depth 1 with
  //  some extra checks involving upper/lower bounds of src/dest.)
  propagate();
  check_error(this->consistent,
              "TemporalNetwork: Checking distance in inconsistent network",
              TempNetErr::TempNetInconsistentError());
  // We do not expect bound to be -infinity for normal
  // use of this function.
  check_error(!(bound < MIN_DISTANCE),
              "isDistancePossiblyLessThan:  bound is too small",
              TempNetErr::TempNetEmptyConstraintError());

  // The potential is always finite, so if bound is infinite,
  // following test will always safely fail.
  if (dest.potential >= src.potential + bound)
    return false;

  // Extra filtering from an analogous test using lower bounds, but we
  // must deal appropriately with infinite cases.
  if (dest.lowerBound >= MIN_DISTANCE) {  // There is path from dest to origin
    if (src.lowerBound == NEG_INFINITY)   // Can't be path from src to dest
      return false;
    // Now we know the src/dest lower bounds are both finite
    if (dest.lowerBound >= src.lowerBound + bound)
      return false;
  }

  // The increment in filtering power from the analogous test on upper
  // bounds seems to be not worth it.
  /*
    if (src->upperBound <= MAX_DISTANCE) {  // There is path from origin to src
    if (dest->upperBound == POS_INFINITY) // Can't be path from src to dest
    return false;
    // Now we know the src/dest upper bounds are both finite
    if (dest->upperBound >= src->upperBound + bound)
    return false;
    }
  */

  return true;
}

  //Bool TemporalNetwork::isConsistent() const {
  //return this->consistent;
  //}

  Bool TemporalNetwork::updateRequired()
  {
      // We propagate additions eagerly so only deletions need a new
      // propagation here, and then only if network was inconsistent.
      // (Deletions from a consistent network cannot cause inconsistency.)
      // (Yes, but this overlooks deletions followed by additions, so..)

      // More efficient test: (hasDeletions && (!consistent || hasAdditions))
      // but need to set up hasAdditions cache.  For now, just propagate...
      bool fullyPropagated = !this->hasDeletions;

      return !fullyPropagated;
  }

  Bool TemporalNetwork::propagate()
  {
    if (updateRequired())
      fullPropagate(); // Otherwise changes have been incrementally propagated

    return this->consistent;
  }

Void TemporalNetwork::calcDistanceBounds(Timepoint& src, Timepoint& targ,
                                         Time& lb, Time& ub, Bool exact) {
  propagate();

  // If inconsistent, return inconsistent bounds
  if(!this->consistent){
    lb = 2;
    ub = -2;
    return;
  }

  check_error(this->consistent,
              "TemporalNetwork: Getting bounds from inconsistent network",
              TempNetErr::TempNetInconsistentError());
  check_error(( this->isValid(src) ),
              "TemporalNetwork: Invalid source timepoint identifier",
              TempNetErr::TempNetInvalidTimepointError());
  check_error(( this->isValid(targ) ),
              "TemporalNetwork: Invalid target timepoint identifier",
              TempNetErr::TempNetInvalidTimepointError());    // Trying to simulate as best as possible the AKJ approximation
  if(exact == false) {
    Dedge* forwardEdge = findEdge(src,targ);
    Dedge* reverseEdge = findEdge(targ,src);
    //      if (forwardEdge != nullptr || reverseEdge != nullptr) {
    if (forwardEdge != NULL)
      ub = forwardEdge->length;
    else
      ub = POS_INFINITY;
    if (reverseEdge != NULL)
      lb = - (reverseEdge->length);
    else
      lb = NEG_INFINITY;
    return;
    //      }
  }

  // Otherwise calculate from two single-source propagations
  dijkstra(src,&targ);
  ub = getDistance(targ);
  dijkstra(targ,&src);
  lb = - getDistance(src);
}

Void TemporalNetwork::propagateBoundsFrom (Timepoint& src) {
  for(std::vector<DnodeId>::const_iterator it = nodes.begin(); it != nodes.end(); ++it){
    const TimepointId node = boost::dynamic_pointer_cast<Timepoint>(*it);
    node->upperBound = POS_INFINITY;
    node->lowerBound = NEG_INFINITY;
  }
  src.upperBound = 0;
  src.lowerBound = 0;
  src.depth = 0;
  BucketQueue& queue = initializeBqueue();
  queue.insertInQueue(&src);
  incDijkstraForward();
  queue.insertInQueue(&src);
  incDijkstraBackward();
}

Void TemporalNetwork::calcDistanceBounds(Timepoint& src,
                                         const std::vector<Timepoint*>& targs,
                                         std::vector<Time>& lbs,
                                         std::vector<Time>& ubs) {
  // Method: Calculate lower/upper bounds as if src was the origin.
  // Afterwards restore the proper bounds.  Requires only four
  // dijkstras instead of 2*n dijkstras.

  propagate();

  checkError(this->consistent, "TemporalNetwork: calcDistanceBounds from inconsistent network");

  propagateBoundsFrom(src);

  lbs.clear();
  ubs.clear();

  for (unsigned i=0; i<targs.size(); i++) {
    lbs.push_back(targs[i]->lowerBound);
    ubs.push_back(targs[i]->upperBound);
  }

  propagateBoundsFrom(*getOriginNode());

  return;
}

Void TemporalNetwork::calcDistanceSigns(Timepoint& src,
                                        const std::vector<Timepoint*>&
                                        targs,
                                        std::vector<Time>& lbs,
                                        std::vector<Time>& ubs) {
  // Method: Calculate lower/upper bounds as if src was the origin.
  // Don't disturb the existing bounds.  Requires only two dijkstras
  // instead of 2*n dijkstras.  Use special bounded-distance
  // dijkstra computations that only determine precedences.

  propagate();

  checkError(this->consistent,
             "TemporalNetwork: calcDistanceSigns from inconsistent network");

  lbs.clear();
  ubs.clear();

  if (targs.empty())
    return;

  Time minPotential = POS_INFINITY;
  Time maxPotential = NEG_INFINITY;

  for (unsigned i=0; i<targs.size(); i++) {
    if (targs[i]->potential < minPotential)
      minPotential = targs[i]->potential;
    if (targs[i]->potential > maxPotential)
      maxPotential = targs[i]->potential;
    ubs.push_back (1);
    lbs.push_back (-1);
  }

  boundedDijkstraForward(src, 1, minPotential);
  for (unsigned i=0; i<targs.size(); i++)
    ubs[i] = getDistance(*targs[i]);//(targs[i]->distance);

  boundedDijkstraBackward(src, 1, maxPotential);
  for (unsigned i=0; i<targs.size(); i++)
    lbs[i] = (- getDistance(*targs[i]));//(- targs[i]->distance);
    
  //sanity check
  for(unsigned int i = 0; i < targs.size(); i++) {
    checkError((lbs[i] < 0 || ubs[i] >= 0),
               "TemporalNetwork: calcDistanceSigns ub anomaly.");
    checkError((ubs[i] > 0 || lbs[i] <= 0),
               "TemporalNetwork: calcDistanceSigns lb anomaly.");
  }
  return;
}


std::vector<Timepoint*>
TemporalNetwork::getConstraintScope(TemporalConstraint& id) {
  std::vector<Timepoint*> result(2);
  result.push_back(&id.head);
  result.push_back(&id.foot);
  return(result);
}

void TemporalNetwork::getConstraintScope(const TemporalConstraint& constraint,
                                         Timepoint*& source,
                                         Timepoint*& target) const {
  source = &constraint.head;
  target = &constraint.foot;
}

Time
TemporalNetwork::getConstraintUpperBound(const TemporalConstraint& id) {
  return(id.upperBound);
}


Time
TemporalNetwork::getConstraintLowerBound(const TemporalConstraint& id) {
  return(id.lowerBound);
}

#ifdef EUROPA_NO_ERROR_CHECKS_
# define checkBoundsValidity(lo, hi) (true)
#else
  static Bool checkBoundsValidity(const Time lb, const Time ub) {
    checkError(!(lb > ub),
                "addTemporalConstraint: (LB, UB) interval was empty",
                TempNetErr::TempNetEmptyConstraintError());
    checkError( !((ub > MAX_LENGTH && ub < POS_INFINITY) || ub > POS_INFINITY),
                "addTemporalConstraint:  UB is in forbidden range: " << ub,
                 TempNetErr::TempNetEmptyConstraintError());
    checkError( !((ub < MIN_LENGTH)),
                "addTemporalConstraint:  UB is too small: " << ub,
                 TempNetErr::TempNetEmptyConstraintError());
    checkError( !((-lb > MAX_LENGTH && -lb < POS_INFINITY) || -lb > POS_INFINITY),
                "addTemporalConstraint:  LB is in forbidden range: " << lb,
                 TempNetErr::TempNetEmptyConstraintError());
    checkError( !((-lb < MIN_LENGTH)),
                "addTemporalConstraint:  LB is too large: " << lb,
                 TempNetErr::TempNetEmptyConstraintError());
    return(true);
  }
#endif

TemporalConstraint* TemporalNetwork::addTemporalConstraint(Timepoint& src,
                                                           Timepoint& targ,
                                                           const Time _lb,
                                                           const Time _ub,
                                                           bool _propagate) {
  using boost::ref;
  const Time lb = mapToInternalInfinity(_lb);
  const Time ub = mapToInternalInfinity(_ub);
  if (!checkBoundsValidity(lb, ub))
    return NULL;

  check_error(isValid(src),
              "addTemporalConstraint:  Invalid source timepoint",
              TempNetErr::TempNetInvalidTimepointError());
  check_error(isValid(targ),
              "addTemporalConstraint:  Invalid target timepoint",
              TempNetErr::TempNetInvalidTimepointError());
  check_error( (&src != &targ),
               "addTemporalConstraint:  source and target are the same",
               TempNetErr::TempNetEmptyConstraintError());
  maintainTEQ (lb,ub,src,targ);

  unsigned short edgeCount = 0;

  if (ub <= MAX_LENGTH){
    addEdgeSpec(src, targ, ub);
    edgeCount++;
  }

  if(lb >= MIN_LENGTH){
    edgeCount++;
    addEdgeSpec(targ, src, -lb);
  }

  TemporalConstraintId spec =
      boost::make_shared<Tspec>(this, ref(src), ref(targ), lb, ub, edgeCount);

  m_constraints.insert(spec);

  // As long as propagation is not turned off, we can process this constraint
  if (_propagate){
    incPropagate(src, targ);
  }

  return(spec.get());
}

Void TemporalNetwork::narrowTemporalConstraint(TemporalConstraint& spec,
                                               const Time newLb, const Time newUb) {
  if (!checkBoundsValidity(newLb, newUb))
    return;

  check_error(isValidId(spec),
              "narrowTemporalConstraint:  Invalid TemporalConstraint",
              TempNetErr::TempNetInvalidConstraintError());

  Time oldLb = spec.lowerBound;
  Time oldUb = spec.upperBound;

  check_error( !(newLb < oldLb || newUb > oldUb),
               "narrowTemporalConstraint: new bounds must be tighter",
               TempNetErr::TempNetInvalidConstraintError());

  Timepoint& src = spec.head;
  Timepoint& targ = spec.foot;
  maintainTEQ (newLb,newUb,src,targ);

  if (newUb <= MAX_LENGTH){
    addEdgeSpec(src, targ, newUb);
    spec.m_edgeCount++;
  }

  if (newLb >= MIN_LENGTH){
    addEdgeSpec(targ, src, -newLb);
    spec.m_edgeCount++;
  }
  if (oldUb <= MAX_LENGTH){
    removeEdgeSpec(src, targ, oldUb);
    spec.m_edgeCount--;
  }
  if (oldLb >= MIN_LENGTH){
    removeEdgeSpec(targ, src, -oldLb);
    spec.m_edgeCount--;
  }

  spec.lowerBound = newLb;
  spec.upperBound = newUb;

  checkError(spec.m_edgeCount <= 2, "Invalied edge count" <<  spec.m_edgeCount);

  if(!this->hasDeletions)
    incPropagate(src, targ);
}

Void TemporalNetwork::removeTemporalConstraint(TemporalConstraint& spec,
                                               bool markDeleted) {
  // Make sure it is valid, including belonging to this id manager
  check_error(isValidId(spec),
              "removeTemporalConstraint: invalid Id",
              TempNetErr::TempNetInvalidConstraintError());
  Time lb = spec.lowerBound;
  Time ub = spec.upperBound;
  Timepoint& src = spec.head;
  Timepoint& targ = spec.foot;
  check_error(isValidId(src));
  check_error(isValidId(targ));

  if (ub <= MAX_LENGTH)
    removeEdgeSpec(src, targ, ub);
  if (lb >= MIN_LENGTH)
    removeEdgeSpec(targ, src, -lb);
  this->hasDeletions = this->hasDeletions || markDeleted;
  m_constraints.erase(std::find_if(m_constraints.begin(), m_constraints.end(),
                                   ptr_compare<TemporalConstraint>(&spec)));
}

Timepoint& TemporalNetwork::getOrigin() {
  return *getOriginNode();
}

Timepoint& TemporalNetwork::addTimepoint() {
  TimepointId node = boost::make_shared<Tnode>(this);
  addNode(node);
  node->ordinal=++(this->nodeCounter);
  return *node.get();
}

Void TemporalNetwork::deleteTimepoint(Timepoint& node) {
  check_error(isValidId(node),
              "TemporalNetwork:: deleting invalid timepoint.",
              TempNetErr::TempNetInvalidTimepointError());
  check_error((&node != getOriginNode()),
              "TemporalNetwork:: deleting origin timepoint.",
              TempNetErr::TempNetDeletingOriginError());
  this->hasDeletions =  this->hasDeletions || node.getDeletionMarker();

  cleanupTEQ(node);

  m_updatedTimepoints.erase(&node);

  // Note: following causes all constraints involving
  // the node to be removed before removing the node.
  deleteNode(node);
}

  std::list<Timepoint*> TemporalNetwork::getInconsistencyReason() {
    check_error(!this->consistent,
                "Network is not inconsistent",
                TempNetErr::TempNetNoInconsistencyError());
    std::list<Timepoint*> ans;
    for (std::list<Dedge*>::const_iterator it=edgeNogoodList.begin();
	 it != edgeNogoodList.end(); ++it) {
      Dedge* edge = *it;
      Timepoint& node = dynamic_cast<Timepoint&>(edge->to);
      ans.push_back(&node);
    }
    return ans;
  }

  std::list<Dedge*> TemporalNetwork::getEdgeNogoodList()
  {
    if (propagate())
      return std::list<Dedge*>();
    return edgeNogoodList;
  }

  // PHM Support for reftime calculations
  void TemporalNetwork::setReferenceTimepoint (Timepoint* refpoint)
  {
    m_refpoint = refpoint;
    fullPropagate();
  }

Timepoint* TemporalNetwork::getOriginNode() const {
  return dynamic_cast<Timepoint*>(this->nodes.front().get()); //boost::dynamic_pointer_cast<Timepoint>(this->nodes.front());
  }

Void TemporalNetwork::fullPropagate() {
  debugMsg("TemporalNetwork:fullPropagate", "fullPropagate started");
  m_updatedTimepoints.clear();
  this->incrementalSource = NULL;   // Not applicable to a full prop.
  setConsistency(bellmanFord());
  this->hasDeletions = false;
  if (this->consistent == false)
    return;

  // We also need to do specialized Dijkstras in the forward
  // and backward directions to update the lower/upper bounds.
  // Note: these could be done lazily on request for bounds.
  for(std::vector<DnodeId>::const_iterator it = nodes.begin(); it != nodes.end(); ++it){
    TimepointId node = boost::dynamic_pointer_cast<Timepoint>(*it);
    node->upperBound = POS_INFINITY;
    node->lowerBound = NEG_INFINITY;
  }

  Timepoint* origin = getOriginNode();
  origin->upperBound = 0;
  origin->lowerBound = 0;
  origin->depth = 0;

  BucketQueue& queue = initializeBqueue();
  queue.insertInQueue(origin);
  incDijkstraForward();
  queue.insertInQueue(origin);
  incDijkstraBackward();

  // PHM 6/29/2010 Changes to support reftime calculations
  if (m_refpoint) {

    // We may use either all lower bounds or all upper bounds for
    // preferred time constraints.  Code adjusts to either case.

    Time initref =
	(m_refpoint->inCount == 0) ? POS_INFINITY : NEG_INFINITY;

    for (unsigned i=0; i < nodes.size(); i++) {
      TimepointId node = boost::dynamic_pointer_cast<Timepoint>(nodes[i]);
      node->reftime = initref;
    }
    m_refpoint->reftime = 0;
    m_refpoint->depth = 0;
    queue.insertInQueue(m_refpoint);

    if (m_refpoint->inCount == 0)
      incDijkstraReftime();
    else
      incDijkstraRefBack(); // Backwards propagation
  }

  debugMsg("TemporalNetwork:fullPropagate", "fullPropagate done");
}

Void TemporalNetwork::incPropagate(Timepoint& src, Timepoint& targ)
{

  // Do nothing if network inconsistent or there are deletions.
  // The next consistency check will cause full propagation.
  if (this->hasDeletions || this->consistent == false)
    return;

  check_error(isValidId(src));
  check_error(isValidId(targ));

  BucketQueue& queue = initializeBqueue();
  Timepoint* next;

  next = dynamic_cast<Timepoint*>(startNode(src, src.potential,
                                            targ, targ.potential));
  if (next != NULL) {
    Timepoint& start = (&src == next) ? targ : src;
    incrementalSource = &start;  // Used in specialized cycle detection
    next->predecessor = findEdge(start, *next);  // Used to trace nogood
    handleNodeUpdate(*next);
    queue.insertInQueue(next);
    setConsistency(incBellmanFord());
  }

  // Can't do Dijkstra if network is now inconsistent.
  if (this->consistent == false)
    return;

  // Now we need to do specialized Dijkstras in the forward
  // and backward directions to update the lower/upper bounds.

  BucketQueue& queue1 = initializeBqueue();

  next = dynamic_cast<Timepoint*>(startNode(src, src.upperBound,
                                            targ, targ.upperBound));
  if (next != NULL) {
    queue1.insertInQueue(next);
    handleNodeUpdate(*next);
    incDijkstraForward();
  }

  // For lower-bound propagation we need to do some finagling (Irish
  // word) to get the right effect from startNode().

  // Can't pass a negative as a reference value, so use locals
  Time headDistance = -(src.lowerBound);
  Time footDistance = -(targ.lowerBound);

  // Backwards propagation, so call with "forward" flag false.
  next = dynamic_cast<Timepoint*>(startNode(src, headDistance,
                                            targ, footDistance, false));
  if (next != NULL) {

    // Store propagated locals back to proper locations
    src.lowerBound = -(headDistance);
    targ.lowerBound = -(footDistance);

    queue1.insertInQueue(next);
    handleNodeUpdate(*next);
    incDijkstraBackward();
  }

  // PHM Support for reftime calculations
  // Adjust to either case of all lb or all ub constraints.
  if (m_refpoint) {
    if (m_refpoint->inCount == 0) { // all ub constraints
      next = dynamic_cast<Timepoint*>(startNode(src, src.reftime,
                                                targ, targ.reftime));
      if (next != NULL) {
        queue1.insertInQueue(next);
        handleNodeUpdate(*next);
        incDijkstraReftime();
      }
    }
    else { // all lb constraints
      headDistance = -(src.reftime);
      footDistance = -(targ.reftime);
      next = dynamic_cast<Timepoint*>(startNode(src, headDistance,
                                                targ, footDistance, false));
      if (next != NULL) {
        src.reftime = -(headDistance);
        targ.reftime = -(footDistance);
        queue1.insertInQueue(next);
        handleNodeUpdate(*next);
        incDijkstraRefBack(); // Backwards propagation
      }
    }
  }
}

Dnode* TemporalNetwork::startNode(Timepoint& head, Time& headDistance,
                                  Timepoint& foot, Time& footDistance,
                                  bool forwards) {
  // PHM 06/21/2007 Modified for efficiency to do first propagation
  // as side-effect.  (Avoids waste of unnecessary fan-out at first
  // node, which can be huge, for example O(n) at the origin.)

  Dedge* edge = findEdge((forwards ? head : foot),
                         (forwards ? foot : head));

  if (edge != NULL && headDistance < POS_INFINITY
      && headDistance + edge->length < footDistance) {
    // Propagate across edge
    footDistance = headDistance + edge->length;
    head.depth = 0;
    foot.depth = 1;
    return &foot;  // Continue propagation from foot
  }

  // Else Propagation, if any, is in the other direction.
  Dedge* revEdge = findEdge((forwards ? foot : head),
                            (forwards ? head : foot));

  if (revEdge != NULL && footDistance < POS_INFINITY
      && footDistance + revEdge->length < headDistance) {
    // Propagate across reverse edge
    headDistance = footDistance + revEdge->length;
    foot.depth = 0;
    head.depth = 1;
    return &head;  // Continue propagation from head
  }

  return NULL;
}

Void TemporalNetwork::incDijkstraForward() {

  BucketQueue& queue = *this->bqueue;
  check_error_variable(unsigned long BFbound = this->nodes.size());

  while (true) {
    Timepoint* node = dynamic_cast<Timepoint*>(queue.popMinFromQueue());
    if (node == NULL)
      return;

    for (int i=0; i< node->outCount; i++) {
      Dedge* edge = node->outArray[i];
      Timepoint& next = dynamic_cast<Timepoint&>(edge->to);
      Time newDistance = node->upperBound + edge->length;
      if (newDistance < next.upperBound) {
        check_error(!(newDistance > MAX_DISTANCE || newDistance < MIN_DISTANCE),
                    "Potential over(under)flow during upper bound propagation",
                    TempNetErr::TimeOutOfBoundsError());
        // Next check is a failsafe to prevent infinite propagation.
        check_error(!(static_cast<unsigned>(next.depth = node->depth + 1) > BFbound),
                    "Dijkstra propagation in inconsistent network",
                    TempNetErr::TempNetInternalError());
        next.upperBound = newDistance;
        // Appropriate priority key as derived from Johnson's algorithm
        queue.insertInQueue(&next, newDistance - next.potential);

        // Store in set of updated timepoints
        handleNodeUpdate(next);
      }
    }
  }
}

Void TemporalNetwork::incDijkstraBackward() {

  BucketQueue& queue = *(this->bqueue);
  check_error_variable(unsigned long BFbound = this->nodes.size());

  while (true) {

    Timepoint* node = dynamic_cast<Timepoint*>(queue.popMinFromQueue());
    if(node == NULL)
      return;

    for (int i=0; i< node->inCount; i++) {
      Dedge* edge = node->inArray[i];
      Timepoint& next = dynamic_cast<Timepoint&>(edge->from);
      Time newDistance = -(node->lowerBound) + edge->length;
      if (newDistance < -(next.lowerBound)) {
        check_error(!(newDistance > MAX_DISTANCE || newDistance < MIN_DISTANCE),
                    "Potential over(under)flow during lower bound propagation",
                    TempNetErr::TimeOutOfBoundsError());
        // Next check is a failsafe to prevent infinite propagation.
        //but a risky one, since it's got a side effect! ~MJI
        check_error(!(static_cast<unsigned>(next.depth = node->depth + 1) > BFbound),
                    "Dijkstra propagation in inconsistent network",
                    TempNetErr::TempNetInternalError());
        next.lowerBound = -newDistance;
        // 12/13/2002 Fix queue key computation.  Correct formula for
        // backward prop is key = (distance + potential).
        queue.insertInQueue(&next, newDistance + next.potential);

        // Store in set of updated timepoints
        handleNodeUpdate(next);
      }
    }
  }
}

  Void TemporalNetwork::incDijkstraReftime()
  {
    // PHM New function to support reftime calculations
    BucketQueue& queue = *(this->bqueue);
    check_error_variable(unsigned long BFbound = this->nodes.size());

    while (true) {
      Timepoint* node = dynamic_cast<Timepoint*>(queue.popMinFromQueue());
      if (node == NULL)
	return;

      for (int i=0; i< node->outCount; i++) {
	Dedge* edge = node->outArray[i];
	Timepoint& next = dynamic_cast<Timepoint&>(edge->to);
	Time newDistance = node->reftime + edge->length;
	if (newDistance < next.reftime) {
	  check_error(!(newDistance > MAX_DISTANCE || newDistance < MIN_DISTANCE),
		      "Potential over(under)flow during upper bound propagation",
		      TempNetErr::TimeOutOfBoundsError());
	  // Next check is a failsafe to prevent infinite propagation.
	  check_error(!(static_cast<unsigned>(next.depth = node->depth + 1) > BFbound),
		      "Dijkstra propagation in inconsistent network",
		      TempNetErr::TempNetInternalError());
	  next.reftime = newDistance;
	  // Appropriate priority key as derived from Johnson's algorithm
	  queue.insertInQueue(&next, newDistance - next.potential);

	  // Store in set of updated timepoints
	  handleNodeUpdate(next);
	}
      }
    }
  }

  Void TemporalNetwork::incDijkstraRefBack()
  {
    // PHM New function to support reftime calculations
    BucketQueue& queue = *(this->bqueue);

    check_error_variable(unsigned long BFbound = this->nodes.size());

    while (true) {
      Timepoint* node = dynamic_cast<Timepoint*>(queue.popMinFromQueue());
      if(node == NULL)
	return;
      for (int i=0; i< node->inCount; i++) {
	Dedge* edge = node->inArray[i];
	Timepoint& next = dynamic_cast<Timepoint&>(edge->from);
	Time newDistance = -(node->reftime) + edge->length;
	if (newDistance < -(next.reftime)) {
    check_error(!(newDistance > MAX_DISTANCE || newDistance < MIN_DISTANCE),
                "Potential over(under)flow during lower bound propagation",
                TempNetErr::TimeOutOfBoundsError());
	  // Next check is a failsafe to prevent infinite propagation.
    check_error(!(static_cast<unsigned>(next.depth = node->depth + 1) > BFbound),
                "Dijkstra propagation in inconsistent network",
                TempNetErr::TempNetInternalError());
	  next.reftime = -newDistance;
	  // For backward prop correct key = (distance + potential).
	  queue.insertInQueue(&next, newDistance + next.potential);

	  // Store in set of updated timepoints
	  handleNodeUpdate(next);
	}
      }
    }
  }


Timepoint* TemporalNetwork::getRingLeader(const Timepoint& tpId) {
  Timepoint* ringLeader = tpId.ringLeader;
  if (ringLeader == NULL)
    return &const_cast<Timepoint&>(tpId);   // Trivial TEQ, timepoint is own leader.
  else
    return ringLeader;
}

std::vector<Timepoint*> TemporalNetwork::getRingFollowers(const Timepoint& tpId) {
  Timepoint* ringLeader = tpId.ringLeader;
  if (ringLeader == NULL)
    return std::vector<Timepoint*>();   // Trivial TEQ, no followers.
  else
    return ringLeader->ringFollowers;
}

std::vector<Timepoint*> TemporalNetwork::getRingPredecessors(const Timepoint& tpId) {
  Timepoint* tpt = getRingLeader(tpId);
  // Timepoint* tpt = dynamic_cast<Timepoint*>(tpId.ringLeader);
  // if (!tpt)
  //   tpt = &const_cast<Timepoint&>(tpId);   // Trivial TEQ, timepoint is own leader.

  // Predecessors are computed dynamically.
  // Might be possible to cache these too.

  std::vector<Timepoint*> ans;
  int numedges = tpt->outCount;
  for (int i=0; i<numedges; i++) {
    Dedge* e = tpt->outArray[i];
    Time length = e->length;
    Timepoint& next = dynamic_cast<Tnode&>(e->to);
    if (length < 0)   // Negative predecessors are enabling.
      ans.push_back(&next);

    else if (length == 0) {
      // [0,<pos>] predecessors were requested to be also enabling.
      //
      // If network is consistent, it is sufficient to consider [0,x]
      // predecessors that are not in the same ring.  There can be
      // be no [0,<neg>] predecessors if the network is consistent.
      //
      // Although the Dispatchability processing does not make all
      // [0,<pos>] predecessors local, the ones it doesn't catch will
      // be enforced by propagation.  Example: Consider A->B [-INF,3]
      // and A->C [3,+INF].  There is an implied [0,INF] link from B
      // to C that is dominated by A->B->C.  Although B is not made an
      // enabling predecessor of C, it does not matter because A *is*,
      // and after A has been executed, propagation ensures C does not
      // precede B.

      if (next.ringLeader != tpt)
        ans.push_back(&next);
    }
  }
  return ans;
}

Void TemporalNetwork::maintainTEQ (Time lb, Time ub, Timepoint& src, Timepoint& targ) {
  // PHM 1/31/2001 Isolated in a separate function for new TNET.
  // PHM 9/13/2000 Perform maintenance step for TEQ
  // when adding or (1/31/2001) narrowing constraint.
  if (lb == 0 && ub == 0) {
    // First make sure at least one tp has a ringLeader,
    if (targ.ringLeader == NULL && src.ringLeader == NULL)
      src.ringLeader = &src;
    // Now place any leaderless tp under the other's leader.
    if (targ.ringLeader == NULL) {
      // In this case src->ringLeader must be non-null
      targ.ringLeader = src.ringLeader;
      src.ringLeader->ringFollowers.push_back(&targ);
    }
    if (src.ringLeader) {
      // In this case targ->ringLeader must be non-null
      src.ringLeader = targ.ringLeader;
      targ.ringLeader->ringFollowers.push_back(&src);
    }
    // Do nothing in case where both RingLeaders are non-null;
    // merging two TEQs is beyond the scope of this mechanism.
    // Note: any deletion may destroy the integrity of TEQs, but
    // making the network dispatchable will restore the TEQs.
  }
}

Void TemporalNetwork::cleanupTEQ(Timepoint& tpt) {
  // PHM 1/31/2001 Isolated in a separate function for new TNET.
  // Called by deleteTimepoint.
  // PHM 9/18/2000  Even though we are not maintaining TEQs under
  // deletion, we need to at least eliminate dangling pointers
  // when deleting a timepoint.
  if (tpt.ringLeader == &tpt) {
    // It's a leader
    for(std::vector<Timepoint*>::const_iterator it = tpt.ringFollowers.begin();
        it != tpt.ringFollowers.end(); ++it)
      (*it)->ringLeader = NULL;
    tpt.ringLeader = NULL;
  }
  else if (tpt.ringLeader != NULL) {// It's a follower
    std::vector<Timepoint*>::iterator it =  //TODO: make this const sometime
        std::find(tpt.ringLeader->ringFollowers.begin(),
                  tpt.ringLeader->ringFollowers.end(),
                  &tpt);
    if(it != tpt.ringLeader->ringFollowers.end()) 
      tpt.ringLeader->ringFollowers.erase(it);
  }
}

  TemporalNetworkId TemporalNetwork::getId() const {
    return m_id;
  }

  const std::set<Timepoint*>& TemporalNetwork::getUpdatedTimepoints() const {
    return m_updatedTimepoints;
  }

void TemporalNetwork::handleNodeUpdate(const Dnode& node) {
  const Timepoint& tnode = dynamic_cast<const Timepoint&>(node);
  //checkError(tnode, node);
  if(&node != &getOrigin())
    m_updatedTimepoints.insert(const_cast<Timepoint*>(&tnode));
}

  void TemporalNetwork::resetUpdatedTimepoints() {
    m_updatedTimepoints.clear();
  }

  void TemporalNetwork::setConsistency(Bool c){
    condDebugMsg(!c && this->consistent, "TemporalNetwork:setConsistency", "Network is inconsistent");
    this->consistent = c;
  }

Tnode::Tnode(TemporalNetwork* t) :
    Dnode(), lowerBound(NEG_INFINITY), upperBound(POS_INFINITY), reftime(0),
    prev_reftime(0), ordinal(0), m_baseDomainConstraint(), m_deletionMarker(true),
    index(0), ringLeader(), ringFollowers(), owner(t) {}

  Tnode::~Tnode(){
    handleDiscard();
  }

  void Tnode::handleDiscard(){
    // Should always be cleared by now if we have synchronized correctly
    //check_error(Entity::isPurging() || getExternalEntity().isNoId());
  }

  TemporalConstraint* Tnode::getBaseDomainConstraint() const { return m_baseDomainConstraint;}

void Tnode::setBaseDomainConstraint(TemporalConstraint* const constraint) {m_baseDomainConstraint = constraint;}

  Tspec::~Tspec() {
    handleDiscard();
  }

  void Tspec::handleDiscard(){
    // Should always be cleared by now if we have synchronized correctly
    //check_error(Entity::isPurging() || getExternalEntity().isNoId());
  }

  Time mapToInternalInfinity(const Time t) {
    if(t >= POS_INFINITY)
      return POS_INFINITY;
    if(t <= NEG_INFINITY)
      return NEG_INFINITY;
    return t;
  }

  Time mapToExternalInfinity(const Time t) {
    if(t >= POS_INFINITY)
      return cast_basis(PLUS_INFINITY);
    if(t <= NEG_INFINITY)
      return cast_basis(MINUS_INFINITY);
    return t;
    
  }

} /* namespace Europa */


