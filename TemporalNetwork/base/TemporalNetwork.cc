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
#include "IntervalIntDomain.hh"
#include "TemporalNetwork.hh"
#include "Debug.hh"

namespace EUROPA {

  Bool TemporalNetwork::isValidId(const TimepointId& id){
    return (id.isValid() &&
	    id->owner == this && hasNode(id) &&
	    hasNode(id));
  }

  Bool TemporalNetwork::isValidId(const TemporalConstraintId& id){
    return (id.isValid() && id->owner == this);
  }

  bool TemporalNetwork::hasEdgeToOrigin(const TimepointId& timepoint) {
    // Order of operands is important for speed. Should be faster to look towards the origin
    DedgeId edgeToTheOrigin = findEdge(timepoint, getOrigin());
    checkError(edgeToTheOrigin.isNoId() || edgeToTheOrigin.isValid(), edgeToTheOrigin);
    return edgeToTheOrigin.isId();
  }

  TemporalNetwork::TemporalNetwork() :   m_id(this)
  {
    consistent=true; hasDeletions=false; nodeCounter=0;
    addTimepoint();
    fullPropagate();
  }

  TemporalNetwork::~TemporalNetwork()
  {
    for(std::set<TemporalConstraintId>::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
      TemporalConstraintId constraint = *it;
      check_error(constraint.isValid());
      constraint->discard();
    }

    m_id.remove();
  }

  DnodeId TemporalNetwork::makeNode()
  {
    // Overrides the definition in DistanceGraph class.
    TimepointId node = (new Tnode(this))->getId();
    return node;
  }

  Bool TemporalNetwork::cycleDetected (DnodeId next)
  {
    // Overrides the definition in DistanceGraph class.
    return (next == this->incrementalSource);
  }

  Void TemporalNetwork::getTimepointBounds(const TimepointId& id, Time& lb, Time& ub)
  {
    check_error(this->consistent,
                "TemporalNetwork: Getting bounds from inconsistent network",
                TempNetErr::TempNetInconsistentError());
    check_error(( this->isValidId(id) ),
                "TemporalNetwork: Invalid timepoint identifier",
                TempNetErr::TempNetInvalidTimepointError());

    // We need to be up-to-date to get the bounds.  Because of eager
    // prop on consistent additions, we only need to prop if there are
    // deletions.

    isConsistent(); // Forces appropariate propagation.

    lb = id->lowerBound;
    ub = id->upperBound;
  }

  Void TemporalNetwork::getLastTimepointBounds(const TimepointId& node, Time& lb, Time& ub)
  {
    check_error(( this->isValidId(node) ),
                "TemporalNetwork: Invalid timepoint identifier",
                TempNetErr::TempNetInvalidTimepointError());

    lb = node->lowerBound;
    ub = node->upperBound;
  }

  Time TemporalNetwork::getLowerTimepointBound(const TimepointId& id) {
    Time result, junk;
    getTimepointBounds(id, result, junk);
    return(result);
  }

  Time TemporalNetwork::getUpperTimepointBound(const TimepointId& id) {
    Time result, junk;
    getTimepointBounds(id, junk, result);
    return(result);
  }

  Bool TemporalNetwork::isDistanceLessThan (const TimepointId& from, const TimepointId& to,
					    Time bound)
  {
    check_error(this->consistent,
                "TemporalNetwork: Checking distance in inconsistent network",
                TempNetErr::TempNetInconsistentError());
    DistanceGraph* graph = (DistanceGraph*) this;
    return graph->isDistanceLessThan(from, to, bound);
  }


  Bool TemporalNetwork::isDistanceLessThanOrEqual (const TimepointId& from,
						   const TimepointId& to,
						   Time bound)
  {
    return isDistanceLessThan(from, to, bound + TIME_TICK);
  }

  Bool TemporalNetwork::isDistancePossiblyLessThan (const TimepointId& src,
						    const TimepointId& dest,
						    Time bound)
  {
    // An efficient approximate version of isDistanceLessThan.
    // (Performs the unrolled recursion only to depth 1 with
    //  some extra checks involving upper/lower bounds of src/dest.)
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
    if (dest->potential >= src->potential + bound)
      return false;

    // Extra filtering from an analogous test using lower bounds, but we
    // must deal appropriately with infinite cases.
    if (dest->lowerBound >= MIN_DISTANCE) {  // There is path from dest to origin
      if (src->lowerBound == NEG_INFINITY)   // Can't be path from src to dest
	return false;
      // Now we know the src/dest lower bounds are both finite
      if (dest->lowerBound >= src->lowerBound + bound)
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

  Bool TemporalNetwork::isConsistent()
  {
    // We propagate additions eagerly so only deletions need a new
    // propagation here, and then only if network was inconsistent.
    // (Deletions from a consistent network cannot cause inconsistency.)
    // (Yes, but this overlooks deletions followed by additions, so..)

    // More efficient test: (hasDeletions && (!consistent || hasAdditions))
    // but need to set up hasAdditions cache.  For now, just propagate...

    if (this->hasDeletions)
      fullPropagate(); // Otherwise changes have been incrementally propagated

    return this->consistent;
  }

  Void TemporalNetwork::calcDistanceBounds(const TimepointId& src, const TimepointId& targ,
					   Time& lb, Time& ub, Bool exact)
  {
    check_error(this->consistent,
                "TemporalNetwork: Getting bounds from inconsistent network",
                TempNetErr::TempNetInconsistentError());
    check_error(( this->isValidId(src) ),
                "TemporalNetwork: Invalid source timepoint identifier",
                TempNetErr::TempNetInvalidTimepointError());
    check_error(( this->isValidId(targ) ),
                "TemporalNetwork: Invalid target timepoint identifier",
                TempNetErr::TempNetInvalidTimepointError());    // Trying to simulate as best as possible the AKJ approximation
    if(exact == false) {
      DedgeId forwardEdge = findEdge (src,targ);
      DedgeId reverseEdge = findEdge (targ,src);
      //      if (forwardEdge != nullptr || reverseEdge != nullptr) {
      if (!forwardEdge.isNoId())
	ub = forwardEdge->length;
      else
	ub = POS_INFINITY;
      if (!reverseEdge.isNoId())
	lb = - (reverseEdge->length);
      else
	lb = NEG_INFINITY;
      return;
      //      }
    }

    // Otherwise calculate from two single-source propagations
    dijkstra(src,targ);
    ub = getDistance(targ);
    dijkstra(targ,src);
    lb = - getDistance(src);
  }

  Void TemporalNetwork::propagateBoundsFrom (const TimepointId& src)
  {
    for(std::vector<DnodeId>::const_iterator it = nodes.begin(); it != nodes.end(); ++it){
      TimepointId node = (TimepointId) *it;
      node->upperBound = POS_INFINITY;
      node->lowerBound = NEG_INFINITY;
    }
    src->upperBound = 0;
    src->lowerBound = 0;
    src->depth = 0;
    BucketQueue* queue = initializeBqueue();
    queue->insertInQueue(src);
    incDijkstraForward();
    queue->insertInQueue(src);
    incDijkstraBackward();
  }

  Void TemporalNetwork::calcDistanceBounds(const TimepointId& src,
                                           const std::vector<TimepointId>&
                                           targs,
					   std::vector<Time>& lbs,
                                           std::vector<Time>& ubs)
  {
    // Method: Calculate lower/upper bounds as if src was the origin.
    // Afterwards restore the proper bounds.  Requires only four
    // dijkstras instead of 2*n dijkstras.
   
    checkError(this->consistent, "TemporalNetwork: calcDistanceBounds from inconsistent network");
    
    propagateBoundsFrom(src);

    lbs.clear();
    ubs.clear();

    for (unsigned i=0; i<targs.size(); i++) {
      lbs.push_back(targs[i]->lowerBound);
      ubs.push_back(targs[i]->upperBound);
    }

    propagateBoundsFrom(getOriginNode());

    return;
  }

  std::list<TimepointId>
  TemporalNetwork::getConstraintScope(const TemporalConstraintId& id) {
    std::list<TimepointId> result;
    
    if(id.isInvalid())
      handle_error(is.isInvalid(), 
                   "Cannot get scope of invalid constraint.",
                   TempNetErr::TempNetInvalidConstraintError());
    
    Tspec* spec = id.operator->();
    result.push_back(spec->head->getId());
    result.push_back(spec->foot->getId());
    return(result);
  }

  void TemporalNetwork::getConstraintScope(const TemporalConstraintId& constraint, TimepointId& source, TimepointId& target) const{
    check_error(constraint.isValid());
    Tspec* spec = (Tspec*) constraint;
    source = spec->head->getId();
    target = spec->foot->getId();
  }

  Time 
  TemporalNetwork::getConstraintUpperBound(const TemporalConstraintId& id) {
    if(id.isInvalid())
      handle_error(id.isInvalid(), 
                   "Cannot get scope of invalid constraint.",
                   TempNetErr::TempNetInvalidConstraintError());

    Tspec* spec = id.operator->();
    return(spec->upperBound);
  }


  Time 
  TemporalNetwork::getConstraintLowerBound(const TemporalConstraintId& id) {
    if(id.isInvalid())
      handle_error(id.isInvalid(),
                   "Cannot get scope of invalid constraint.",
                   TempNetErr::TempNetInvalidConstraintError());
    Tspec* spec = id.operator->();
    return(spec->lowerBound);
  }

#ifdef _EUROPA_NO_ERROR_CHECKS_
# define checkBoundsValidity(lo, hi) (true)
#else
  static Bool checkBoundsValidity(const Time lb, const Time ub) {
    check_error(!(lb > ub), 
                "addTemporalConstraint: (LB, UB) interval was empty",
                TempNetErr::TempNetEmptyConstraintError());
    check_error( !((ub > MAX_LENGTH && ub < POS_INFINITY) || ub > POS_INFINITY),
                 "addTemporalConstraint:  UB is in forbidden range",
                 TempNetErr::TempNetEmptyConstraintError());
    check_error( !((ub < MIN_LENGTH)),
                 "addTemporalConstraint:  UB is too small",
                 TempNetErr::TempNetEmptyConstraintError());
    check_error( !((-lb > MAX_LENGTH && -lb < POS_INFINITY) || -lb > POS_INFINITY),
                 "addTemporalConstraint:  LB is in forbidden range",
                 TempNetErr::TempNetEmptyConstraintError());
    check_error( !((-lb < MIN_LENGTH)),
                 "addTemporalConstraint:  LB is too large",
                 TempNetErr::TempNetEmptyConstraintError());
    return(true);
  }
#endif

  TemporalConstraintId TemporalNetwork::addTemporalConstraint(const TimepointId& src,
							      const TimepointId& targ,
							      const Time lb,
							      const Time ub,
							      bool propagate) {
    if (!checkBoundsValidity(lb, ub))
      return(TemporalConstraintId::noId());

    check_error(isValidId(src),
                "addTemporalConstraint:  Invalid source timepoint",
                TempNetErr::TempNetInvalidTimepointError());
    check_error(isValidId(targ),
                "addTemporalConstraint:  Invalid target timepoint",
                TempNetErr::TempNetInvalidTimepointError());
    check_error( (src != targ),
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

    Tspec* spec = new Tspec (this, src, targ, lb, ub, edgeCount);

    m_constraints.insert(spec->getId());

    // As long as propagation is not turned off, we can process this constraint
    if (propagate){
      incPropagate(src, targ);
    }

    return(spec->getId());
  }

  Void TemporalNetwork::narrowTemporalConstraint(const TemporalConstraintId& tcId,
						 const Time newLb, const Time newUb)
  {
    check_error(tcId.isValid());
    if (!checkBoundsValidity(newLb, newUb))
      return;

    check_error(isValidId(tcId),
                "narrowTemporalConstraint:  Invalid TemporalConstraint",
                TempNetErr::TempNetInvalidConstraintError());

    Tspec* spec = tcId.operator->();
    Time oldLb = spec->lowerBound;
    Time oldUb = spec->upperBound;

    check_error( !(newLb < oldLb || newUb > oldUb),
                 "narrowTemporalConstraint: new bounds must be tighter",
                 TempNetErr::TempNetInvalidConstraintError());

    TimepointId src = spec->head;
    TimepointId targ = spec->foot;
    maintainTEQ (newLb,newUb,src,targ);

    if (newUb <= MAX_LENGTH){
      addEdgeSpec(src, targ, newUb);
      spec->m_edgeCount++;
    }

    if (newLb >= MIN_LENGTH){
      addEdgeSpec(targ, src, -newLb);
      spec->m_edgeCount++;
    }
    if (oldUb <= MAX_LENGTH){
      removeEdgeSpec(src, targ, oldUb);
      spec->m_edgeCount--;
    }
    if (oldLb >= MIN_LENGTH){
      removeEdgeSpec(targ, src, -oldLb);
      spec->m_edgeCount--;
    }

    spec->lowerBound = newLb;
    spec->upperBound = newUb;

    checkError(spec->m_edgeCount >= 0 && spec->m_edgeCount <= 2, "Invalied edge count" <<  spec->m_edgeCount);

    if(!this->hasDeletions)
      incPropagate(src, targ);
  }

  Void TemporalNetwork::removeTemporalConstraint(const TemporalConstraintId& tcId, bool markDeleted) {
    // Make sure it is valid, including belonging to this id manager
    check_error(isValidId(tcId),
                "removeTemporalConstraint: invalid Id",
                TempNetErr::TempNetInvalidConstraintError());
    Tspec* spec = tcId.operator->();
    Time lb = spec->lowerBound;
    Time ub = spec->upperBound;
    TimepointId src = spec->head;
    TimepointId targ = spec->foot;
    check_error(isValidId(src));
    check_error(isValidId(targ));

    if (ub <= MAX_LENGTH)
      removeEdgeSpec(src, targ, ub);
    if (lb >= MIN_LENGTH)
      removeEdgeSpec(targ, src, -lb);
    this->hasDeletions = this->hasDeletions || markDeleted;
    m_constraints.erase(spec->getId());
    spec->discard();
  }

  TimepointId TemporalNetwork::getOrigin()
  {
    TimepointId origin = getOriginNode();
    return origin->getId();
  }

  TimepointId TemporalNetwork::addTimepoint()
  {
    TimepointId node = (TimepointId) createNode();
    node->ordinal=++(this->nodeCounter);
    return node->getId();
  }

  Void TemporalNetwork::deleteTimepoint(const TimepointId& node)
  {
    check_error(isValidId(node),
                "TemporalNetwork:: deleting invalid timepoint.",
                TempNetErr::TempNetInvalidTimepointError());
    check_error((node != getOriginNode()),
                "TemporalNetwork:: deleting origin timepoint.",
                TempNetErr::TempNetDeletingOriginError());
    this->hasDeletions =  this->hasDeletions || node->getDeletionMarker();

    cleanupTEQ(node);

    m_updatedTimepoints.erase(node);

    // Note: following causes all constraints involving
    // the node to be removed before removing the node.
    deleteNode(node);
  }

  std::list<TimepointId> TemporalNetwork::getInconsistencyReason() {
    check_error(!this->consistent,
                "Network is not inconsistent",
                TempNetErr::TempNetNoInconsistencyError());
    std::list<TimepointId> ans;
    for (std::list<DedgeId>::const_iterator it=edgeNogoodList.begin(); 
	 it != edgeNogoodList.end(); ++it) {
      DedgeId edge = *it;
      TimepointId node = (TimepointId) edge->to;
      ans.push_back(node->getId());
    }
    return ans;
  }

  std::list<DedgeId> TemporalNetwork::getEdgeNogoodList()
  {
    if (isConsistent())
      return std::list<DedgeId>();
    return edgeNogoodList;
  }

  TimepointId TemporalNetwork::getOriginNode() const {
    return this->nodes.front();
  }

  Void TemporalNetwork::fullPropagate()
  {
    m_updatedTimepoints.clear();
    this->incrementalSource = TimepointId::noId();   // Not applicable to a full prop.
    setConsistency(bellmanFord());
    this->hasDeletions = false;
    if (this->consistent == false)
      return;

    // We also need to do specialized Dijkstras in the forward
    // and backward directions to update the lower/upper bounds.
    // Note: these could be done lazily on request for bounds.
    for(std::vector<DnodeId>::const_iterator it = nodes.begin(); it != nodes.end(); ++it){
      TimepointId node = (TimepointId) *it;
      node->upperBound = POS_INFINITY;
      node->lowerBound = NEG_INFINITY;
    }

    TimepointId origin = getOriginNode();
    origin->upperBound = 0;
    origin->lowerBound = 0;
    origin->depth = 0;

    BucketQueue* queue = initializeBqueue();
    queue->insertInQueue(origin);
    incDijkstraForward();
    queue->insertInQueue(origin);
    incDijkstraBackward();
  }

  Void TemporalNetwork::incPropagate(TimepointId src, TimepointId targ)
  {

    // Do nothing if network inconsistent or there are deletions.
    // The next consistency check will cause full propagation.
    if (this->hasDeletions || this->consistent == false)
      return;

    check_error(isValidId(src));
    check_error(isValidId(targ));

    BucketQueue* queue = initializeBqueue();
    TimepointId next;

    next = startNode(src, src->potential, targ, targ->potential);
    if (!next.isNoId()) {
      TimepointId start = (next == src) ? targ : src;
      incrementalSource = start;  // Used in specialized cycle detection
      next->predecessor = findEdge(start,next);  // Used to trace nogood
      handleNodeUpdate(next);
      queue->insertInQueue(next);
      setConsistency(incBellmanFord());
    }

    // Can't do Dijkstra if network is now inconsistent.
    if (this->consistent == false)
      return;

    // Now we need to do specialized Dijkstras in the forward
    // and backward directions to update the lower/upper bounds.

    BucketQueue* queue1 = initializeBqueue();

    next = startNode(src, src->upperBound, targ, targ->upperBound);
    if (!next.isNoId()) {
      queue1->insertInQueue(next);
      handleNodeUpdate(next);
      incDijkstraForward();
    }

    // For lower-bound propagation we need to do some finagling (Irish
    // word) to get the right effect from startNode().

    // Can't pass a negative as a reference value, so use locals
    Time headDistance = -(src->lowerBound);
    Time footDistance = -(targ->lowerBound);

    // Backwards propagation, so call with "forward" flag false.
    next = startNode(src, headDistance, targ, footDistance, false);
    if (!next.isNoId()) {

      // Store propagated locals back to proper locations
      src->lowerBound = -(headDistance);
      targ->lowerBound = -(footDistance);

      queue1->insertInQueue(next);
      handleNodeUpdate(next);
      incDijkstraBackward();
    }
  }

  DnodeId TemporalNetwork::startNode (TimepointId head, Time& headDistance,
                                      TimepointId foot, Time& footDistance,
                                      bool forwards)
  {
    // PHM 06/21/2007 Modified for efficiency to do first propagation
    // as side-effect.  (Avoids waste of unnecessary fan-out at first
    // node, which can be huge, for example O(n) at the origin.)

    DedgeId edge = findEdge(forwards ? head : foot,
                            forwards ? foot : head);

    if (!edge.isNoId() && headDistance < g_infiniteTime()
        && headDistance + edge->length < footDistance) {
      // Propagate across edge
      footDistance = headDistance + edge->length;
      head->depth = 0;
      foot->depth = 1;
      return foot;  // Continue propagation from foot
    }
    
    // Else Propagation, if any, is in the other direction.
    DedgeId revEdge = findEdge(forwards ? foot : head,
                               forwards ? head : foot);

    if (!revEdge.isNoId() && footDistance < g_infiniteTime()
        && footDistance + revEdge->length < headDistance) {
      // Propagate across reverse edge
      headDistance = footDistance + revEdge->length;
      foot->depth = 0;
      head->depth = 1;
      return head;  // Continue propagation from head
    }

    return DnodeId::noId();
  }

  Void TemporalNetwork::incDijkstraForward()
  {


    BucketQueue* queue = this->bqueue;
#ifndef EUROPA_FAST
    int BFbound = this->nodes.size();
#endif
    while (true) {
      DnodeId dnode = queue->popMinFromQueue();
      if (dnode.isNoId())
	return;

      TimepointId node(dnode);

      for (int i=0; i< node->outCount; i++) {
	DedgeId edge = node->outArray[i];
	TimepointId next = (TimepointId) edge->to;
	Time newDistance = node->upperBound + edge->length;
	if (newDistance < next->upperBound) {
    check_error(!(newDistance > MAX_DISTANCE || newDistance < MIN_DISTANCE),
                "Potential over(under)flow during upper bound propagation",
                TempNetErr::TimeOutOfBoundsError());
	  // Next check is a failsafe to prevent infinite propagation.
    check_error(!((next->depth = node->depth + 1) > BFbound),
                "Dijkstra propagation in inconsistent network",
                TempNetErr::TempNetInternalError());
	  next->upperBound = newDistance;
	  // Appropriate priority key as derived from Johnson's algorithm
	  queue->insertInQueue (next, newDistance - next->potential);

	  // Store in set of updated timepoints
	  handleNodeUpdate(next);
	}
      }
    }
  }

  Void TemporalNetwork::incDijkstraBackward()
  {
 
    BucketQueue* queue = this->bqueue;
#ifndef EUROPA_FAST
    int BFbound = this->nodes.size();
#endif
    while (true) {
      DnodeId dnode =  queue->popMinFromQueue();
      if(dnode.isNoId())
	return;

      TimepointId node(dnode);

      for (int i=0; i< node->inCount; i++) {
	DedgeId edge = node->inArray[i];
	TimepointId next = (TimepointId) edge->from;
	Time newDistance = -(node->lowerBound) + edge->length;
	if (newDistance < -(next->lowerBound)) {
    check_error(!(newDistance > MAX_DISTANCE || newDistance < MIN_DISTANCE),
                "Potential over(under)flow during lower bound propagation",
                TempNetErr::TimeOutOfBoundsError());
	  // Next check is a failsafe to prevent infinite propagation.
    check_error(!((next->depth = node->depth + 1) > BFbound),
                "Dijkstra propagation in inconsistent network",
                TempNetErr::TempNetInternalError());
	  next->lowerBound = -newDistance;
	  // 12/13/2002 Fix queue key computation.  Correct formula for
	  // backward prop is key = (distance + potential).
	  queue->insertInQueue (next, newDistance + next->potential);

	  // Store in set of updated timepoints
	  handleNodeUpdate(next);
	}
      }
    }
  }
  

  TimepointId TemporalNetwork::getRingLeader(TimepointId tpId)
  {
    check_error(tpId.isValid(),
                "TemporalNetwork:: accessing invalid timepoint.",
                TempNetErr::TempNetInvalidTimepointError());
    Tnode* tpt = tpId.operator->();
    TimepointId ringLeader = tpt->ringLeader;
    if (ringLeader.isNoId())
      return tpId;   // Trivial TEQ, timepoint is own leader.
    else
      return ringLeader;
  }

  std::list<TimepointId> TemporalNetwork::getRingFollowers (TimepointId tpId)
  {
    check_error(tpId.isValid(),
                "TemporalNetwork:: accessing invalid timepoint.",
                TempNetErr::TempNetInvalidTimepointError());
    Tnode* tpt = tpId.operator->();
    TimepointId ringLeader = tpt->ringLeader;
    if (ringLeader.isNoId())
      return std::list<TimepointId>();   // Trivial TEQ, no followers.
    else
      return ringLeader->ringFollowers;
  }

  std::list<TimepointId> TemporalNetwork::getRingPredecessors (TimepointId tpId)
  {
    check_error(tpId.isValid(),
                "TemporalNetwork:: accessing invalid timepoint.",
                TempNetErr::TempNetInvalidTimepointError());
    Tnode* tpoint = tpId.operator->();
    Tnode* tpt = (Tnode*) tpoint->ringLeader;
    if (tpt == 0)
      tpt = tpoint;   // Trivial TEQ, timepoint is own leader.

    // Predecessors are computed dynamically.
    // Might be possible to cache these too.

    std::list<TimepointId> ans;
    int numedges = tpt->outCount;
    for (int i=0; i<numedges; i++) {
      Dedge* e = (Dedge*) tpt->outArray[i];
      Time length = e->length;
      Tnode* next = (Tnode*) e->to;
      if (length < 0)   // Negative predecessors are enabling.
	ans.push_back (next->getId());

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

	if (next->ringLeader != tpt->getId())
	  ans.push_back (next->getId());
      }
    }
    return ans;
  }

  Void TemporalNetwork::maintainTEQ (Time lb, Time ub, TimepointId src, TimepointId targ)
  {
    // PHM 1/31/2001 Isolated in a separate function for new TNET.
    // PHM 9/13/2000 Perform maintenance step for TEQ
    // when adding or (1/31/2001) narrowing constraint.
    if (lb == 0 && ub == 0) {
      // First make sure at least one tp has a ringLeader,
      if (targ->ringLeader.isNoId() && src->ringLeader.isNoId())
	src->ringLeader = src;
      // Now place any leaderless tp under the other's leader.
      if (targ->ringLeader.isNoId()) {
	// In this case src->ringLeader must be non-null
	targ->ringLeader = src->ringLeader;
	src->ringLeader->ringFollowers.push_back(targ->getId());
      }
      if (src->ringLeader.isNoId()) {
	// In this case targ->ringLeader must be non-null
	src->ringLeader = targ->ringLeader;
	targ->ringLeader->ringFollowers.push_back(src->getId());
      }
      // Do nothing in case where both RingLeaders are non-null;
      // merging two TEQs is beyond the scope of this mechanism.
      // Note: any deletion may destroy the integrity of TEQs, but
      // making the network dispatchable will restore the TEQs.
    }
  }

  Void TemporalNetwork::cleanupTEQ(TimepointId tpt)
  {
    // PHM 1/31/2001 Isolated in a separate function for new TNET.
    // Called by deleteTimepoint.
    // PHM 9/18/2000  Even though we are not maintaining TEQs under
    // deletion, we need to at least eliminate dangling pointers
    // when deleting a timepoint.
    if (tpt->ringLeader == tpt) {
      // It's a leader
      for(std::list<TimepointId>::const_iterator it = tpt->ringFollowers.begin(); it != tpt->ringFollowers.end(); ++it)
	((*it).operator->())->ringLeader = TimepointId::noId();
    }
    else if (!tpt->ringLeader.isNoId()) // It's a follower
      tpt->ringLeader->ringFollowers.remove(tpt);
  }

  const TemporalNetworkId& TemporalNetwork::getId() const {
    return m_id;
  }

  const std::set<TimepointId>& TemporalNetwork::getUpdatedTimepoints() const {
    return m_updatedTimepoints;
  }

  void TemporalNetwork::handleNodeUpdate(const DnodeId& node){
    checkError(TimepointId::convertable(node), node);
    TimepointId tnode = node;
    if(node != getOrigin())
      m_updatedTimepoints.insert(tnode);
  }

  void TemporalNetwork::resetUpdatedTimepoints() { 
    m_updatedTimepoints.clear();
  }

  void TemporalNetwork::setConsistency(Bool c){
    condDebugMsg(!c && this->consistent, "TemporalNetwork:setConsistency", "Network is inconsistent");
    this->consistent = c;
  }

  Tnode::Tnode(TemporalNetwork* t) :  Dnode(), m_deletionMarker(true) , owner(t){
    lowerBound = NEG_INFINITY;
    upperBound = POS_INFINITY;
  }

  Tnode::~Tnode(){
    discard(false);
  }

  void Tnode::handleDiscard(){
    // Should always be cleared by now if we have synchronized correctly
    check_error(Entity::isPurging() || getExternalEntity().isNoId()); 

    Dnode::handleDiscard();
  }

  const TemporalConstraintId& Tnode::getBaseDomainConstraint() const { return m_baseDomainConstraint;}

  void Tnode::setBaseDomainConstraint(const TemporalConstraintId& constraint) {m_baseDomainConstraint = constraint;}

  const TemporalConstraintId& Tspec::getId() const {
    return m_id;
  }

  Tspec::~Tspec() {
    discard(false);
    m_id.remove();
  }

  void Tspec::handleDiscard(){
    // Should always be cleared by now if we have synchronized correctly
    check_error(Entity::isPurging() || getExternalEntity().isNoId()); 
    Entity::handleDiscard();
  }
} /* namespace Europa */


