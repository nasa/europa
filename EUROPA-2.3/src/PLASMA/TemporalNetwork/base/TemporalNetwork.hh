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
#ifndef _H_TemporalNetwork
#define _H_TemporalNetwork

#include "TemporalNetworkDefs.hh"
#include "DistanceGraph.hh"
#include "Error.hh"
#include <list>

namespace EUROPA {

  class TemporalNetwork;
  class Tnode;
  class Tedge;

  class Tspec;

  class DispatchNode;

  typedef Id<Tnode> TimepointId;

  const TimepointId noTimepointId;

    /**
     * @class  TemporalNetwork
     * @author Paul H. Morris (with mods by Conor McGann)
     * @date   Mon Dec 27 2004
     * @brief  Implements a Simple Temporal Network (@ref stp "STN")
     *
     *         Provides incremental access algorithms for a Simple Temporal
     *         Network (@ref stp "STN"). This is a sublass of Distance Graph as it adds the fields
     *         required to implement STN accessors. The propagation strategy is
     *         to propergate added or narrowed constraints eagerly.
     *         This allows an efficient specialized cycle detection method because any new
     *         inconsistency must involve the added constraint, so we need only
     *         check for an effective propagation back to the start.
     * @ingroup TemporalNetwork
    */


  class TemporalNetwork : public DistanceGraph {


    Bool consistent;
    Bool hasDeletions;
    Int nodeCounter;
    /**
     * @breif Used for specialized cycle detection
     */
    TimepointId incrementalSource;

    friend Void keepEdge (DispatchNode* x, DispatchNode* y, Time d);
    public:

   // The following are provided for backward compatibility with previous
   // TemporalNetwork. They are not used internally.  Only the ones currently used
   // externally have been defined.  See DistanceGraph.hh for the macros
   // POS_INFINITY, etc.  These are defined to be symmetric under
   // negation, so that NEG_INFINITY = -POS_INFINITY.  Also note that
   // POS_INFINITY is defined so POS_INFINITY + 1 does not overflow.

   // g_noTime() is never returned by this Temporal Network;
   // instead, errors are generated for non-meaningful calls,
   // and infinity is returned where appropriate.

   // N.B. The bounds of added constraints are limited to being between
   // MIN_LENGTH and MAX_LENGTH, which might have considerably smaller
   // magnitude than g_maxFiniteTime().  Propagated distance values may range
   // up to g_maxFiniteTime(), however.  This is so the addition of a legal
   // length to a legal distance value can never overflow.

  /**
   * @brief Tests if a time t is outside of the temporal bounds of the network.
   * @param t the time you wish to query.
   * @return true iff t is outside of allowable bounds of the network.False otherwise.
   */

  inline Bool isOutOfBoundsTime(const Time t) {
    return (t < NEG_INFINITY || t > POS_INFINITY);
  }

  /**
   * @brief Tests if a time t is within the temporal bounds of the network.
   * @param t the time you wish to query.
   * @return true iff t is within the allowable bounds of the network. False otherwise.
   */
  inline Bool isFiniteTime(const Time t) {
    return (t >= MIN_FINITE_TIME && t <= MAX_FINITE_TIME);
  }

  // End of compatibility definitions.

     /**
     * @brief Accessor for the upper and lower bound times of a timepoint. The method
     *        will enforce consistentey.
     * @param id the timepoint for which you require this information.
     * @param lb return result giving lower bound of id.
     * @param ub return result giving upper bound of id.
     */
    Void getTimepointBounds(const TimepointId& id, Time& lb, Time& ub);

  /**
     * @brief Accessor for the upper and lower bound times of a timepoint. The method
     *        will <b>not</b>consistentey.
     * @param id the timepoint for which you require this information.
     * @param lb return result giving lower bound of id.
     * @param ub return result giving upper bound of id.
     */
    Void getLastTimepointBounds(const TimepointId& id, Time& lb, Time& ub);

   /**
     * @brief Accessor for the Lower Timepoint bound. The method
     *        will enforce consistentey.
     * @param id the timepoint for which you require this information.
     * @return lower bound of timepoint.
     */
    Time getLowerTimepointBound(const TimepointId&);

    /**
     * @brief Accessor for the Upper Timepoint bound. The method
     *        will enforce consistentey.
     * @param id the timepoint for which you require this information.
     * @return upper bound of timepoint.
     */
    Time getUpperTimepointBound(const TimepointId&);

    /**
     * @brief Test if the STN is consistent
     * @return true iff network is consistent. False otherwise.
     */
    //Bool isConsistent() const;

    /**
     * @brief Propagate the network.
     * @return true iff network is consistent. False otherwise.
     */
    Bool propagate();

    /**
     * @brief has the network processed all the changes?
     * @return true iff network is not fully propagated. False otherwise.
     */
    Bool updateRequired();

    /**
     * @brief Calculate the temporal distance between two timepoints.
     * @param src the start node in the network.
     * @param targ the end node in the network.
     * @param lb returns the lower bound of the distance between src and targ
     * @param ub returns the upper bound of the distnace between src and targ
     * @param exact if true use Dijkstra's algorithim to compute exact distnace. Approximate if false
     */
    Void calcDistanceBounds(const TimepointId& src, const TimepointId& targ, Time& lb, Time& ub, Bool exact=true);

    /**
     * @brief Calculate the (exact) temporal distance from one timepoint to others.  Much more efficient when many targs.
     * @param src the start node in the network.
     * @param targs the end nodes in the network.
     * @param lbs returns the lower bounds of the distances
     * @param ubs returns the upper bounds of the distances
     */
    Void calcDistanceBounds(const TimepointId& src,
                            const std::vector<TimepointId>& targs,
                            std::vector<Time>& lbs, std::vector<Time>& ubs);

     /**
     * @brief Similar to many-targ version of calcDistanceBounds
     * but only the signs of the "bounds" (that indicate precedences)
     * are meaningful.
     */
    Void calcDistanceSigns(const TimepointId& src,
                           const std::vector<TimepointId>& targs,
                           std::vector<Time>& lbs, std::vector<Time>& ubs);


    /**
     * @brief Identify the timepoints that mark the head and foot of a temporal constraint.
     * @param id temporal constraint of interest.
     * @result two time points - the head and foot of the constraint (in that order).
     */
    std::list<TimepointId> getConstraintScope(const TemporalConstraintId&);

    /**
     * @brief Get the upperbound on the time of a temporal constraint.
     * @param id temporal constraint of interest.
     * @result Upperbound time on constraint.
     */
    Time getConstraintUpperBound(const TemporalConstraintId&);

    /**
     * @brief Get the lowerbound on the time of a temporal constraint.
     * @param id temporal constraint of interest.
     * @result Lowerbound time on constraint.
     */
    Time getConstraintLowerBound(const TemporalConstraintId&);

    /**
     * @brief Identify the timepoints that mark the head and foot of a temporal constraint.
     * @param constraint temporal constraint of interest.
     * @param returns head of temporal constraint
     * @param returns foot of temporal constraint
     */
    void getConstraintScope(const TemporalConstraintId& constraint, TimepointId& source, TimepointId& target) const;

    /**
     * @brief Add temporal constraint to the network
     * @param src start or head of the constraint
     * @param targ finish or tail of the constraint
     * @param lb lower bound time
     * @param ub upper bound time
     * @param propagate iff true this constraint will be included in propagation.
     */
    TemporalConstraintId addTemporalConstraint(const TimepointId& src, const TimepointId& targ,
						const Time lb, const Time ub, bool propagate = true);
    /**
     * @brief Tighten the temporal constraint to new bounds iff they are tighter.
     * @param tcId Constraint to tighten
     * @param newLb New lower bound
     * @param newUb New Upper bound
     */
    Void narrowTemporalConstraint(const TemporalConstraintId& tcId, const Time newLb, const Time newUb);

    /**
     * @brief Remove a constraint from the temporal network
     * @param tcId Constraint to remove
     * @param markDeleted set to true iff you want the STN's state updated to indicate a deleteation has occured. False otherwise.
     */
    Void removeTemporalConstraint(const TemporalConstraintId& tcId, bool markDeleted = true);

    /**
     * @brief get the TimePointId of the origin of the STN.
     * @return the TimePointId of the origin of the STN.
     */
    TimepointId getOrigin();

    /**
     * @brief Create a new timepoint in the STN.
     * @return The TimepointId of the new timepoin.
     */
    TimepointId addTimepoint();

    /**
     * @brief Delete a timepoint from the STN. Note: this must not be the origin and it must be a valid timepoint.
     */
    Void deleteTimepoint(const TimepointId& tpId);

    /**
     * @brief Identify the set of timepoints that lead to an inconsistent network. Note the network must be inconsistent to call this method.
     * @return The set of timepoints behind the inconsistency.
     */
    std::list<TimepointId> getInconsistencyReason();

     /**
     * @brief Identify the set of edges that lead to an inconsistent network.
     * @return The set of edges behind the inconsistency if the network is inconsistent. An empty list if the network is consistent.
     */
    std::list<DedgeId> getEdgeNogoodList();

    /**
     * @brief Check if distance between two timepoints is less than a time bound
     * @param from start timepointId
     * @param to end timepointId
     * @param bound distance bound
     * @return true iff distance(form, to) < bound. False otherwise.
     */
    Bool isDistanceLessThan (const TimepointId& from, const TimepointId& to, Time bound);

    /**
     * @brief Check if distance between two timepoints is less or equal to a time bound
     * @param from start timepointId
     * @param to end timepointId
     * @param bound distance bound
     * @return true iff distance(form, to) <= bound. False otherwise.
     */
    Bool isDistanceLessThanOrEqual (const TimepointId& from, const TimepointId& to,
				    Time bound);

  /**
     * @brief An efficent approimate verion of isDistanceLessThan. It performans an
     *        unrolled recursion only to depth 1 with some extra checks involving upper/
     *        lower bounds of src/dest.
     * @param from start timepointId
     * @param to end timepointId
     * @param bound distance bound
     * @return true if distance(form, to) ~< bound. False otherwise.
     */
    Bool isDistancePossiblyLessThan (const TimepointId& from, const TimepointId& to,
				     Time bound);

     /**
     * @brief Clear the set of updated timepoints.
     */
    void resetUpdatedTimepoints();

     /**
     * @brief Add node to set of updated timepoints.
     * @param node node to add.
     */
    void handleNodeUpdate(const DnodeId& node);

    /**
     * @brief Returns the set of updated timepoints.
     * @return the set of updated timepoints.
     */
     const std::set<TimepointId>& getUpdatedTimepoints() const;

    /**
     * @brief Identify if timepoint is connected to the origin of the STN through edges in the network
     * @param timepoint
     * @return true iff timepoint has edge to the origin. False, otherwise.
     * @todo make const when we constify methods in the DistanceGraph
     */
    bool hasEdgeToOrigin(const TimepointId& timepoint);

    /**
     * @brief Get unique identifier for this STN instance
     * @return unique STN identifier
     */
    const TemporalNetworkId& getId() const;

    // For Dispatchability Processing

    /**
     * @brief
     * @return
     */
    std::list<TemporalConstraintId>    addDispatchConstraints();
    // Additional exec-oriented functions

     /**
     * @brief
     * @return
     */
    TimepointId getRingLeader(TimepointId tpId);

    /**
     * @brief
     * @return
     */
    std::list<TimepointId> getRingFollowers (TimepointId tpId);

    /**
     * @brief
     * @return
     */
    std::list<TimepointId> getRingPredecessors (TimepointId tpId);

    /**
     * @brief Constructor creates and empty STN
     */
    TemporalNetwork();

    /**
     * @brief Destructor
     */
    virtual  ~TemporalNetwork();

  private:
    /**
     * @brief Get the origin of the STN
     * @return  origin timepointId in the STN
     */
    TimepointId getOriginNode() const;

    /**
    * @brief propagate the entire STN
    */
    Void fullPropagate();

    /**
     * @brief propagate only edges between two points in the STN
     * @param src start point for propagation
     * @param targ end point for propagation
     */
    Void incPropagate(TimepointId src, TimepointId targ);

    /**
     * @brief For incremental propagation, determines whether a propagation
     *        should be tried from head to foot or vice versa.
     *
     *  The supplied
     * edge must be in the direction of propagation from head to foot.
     * There is another edge (not needed for the determination, so not
     * supplied) in the other direction.  Used with various
     * distance-like values.  Note that an effective prop in one
     * direction precludes one in the other direction.
     * @return node at which incremental propogation should be tried.
     */
    /**
     * @brief For incremental propagation, determines whether a propagation
     *        is started from head to foot or vice versa, and does first
     *        propagation.  (PHM: 06/21/2007 Recoded for efficiency.)
     * @return node from which to continue
     *        the propagation (or noId if first prop is ineffective).
     */
    DnodeId startNode (TimepointId head, Time& headDistance,
                       TimepointId foot, Time& footDistance,
                       bool forwards = true);

    /**
     * @brief Similar to the DistanceGraph Dijkstra, but propagates the
     * upper bound, and is incremental.
     */
    Void incDijkstraForward();

    /**
     * @brief Similar to the DistanceGraph Dijkstra, but propagates the
     * negation of the lower bound, and is incremental, and goes
     * in the reverse direction.
     */
    Void incDijkstraBackward();

    /**
     * @brief Propagates lower/upper distance bounds from src
     * using backward and forward Dijkstra propagations.
     */
    Void propagateBoundsFrom (const TimepointId& src);

    /**
     * @brief
     */
    Void maintainTEQ (Time lb, Time ub, TimepointId src, TimepointId targ);

    /**
     * @brief
     */
    Void cleanupTEQ(TimepointId tpt);

    /**
     * @brief check if node is valid
     * @return true iff node is valid.
     */
    Bool isValidId(const TimepointId& id);

    /**
     * @breif check if constraint is valid
     */
    Bool isValidId(const TemporalConstraintId& id);

    /**
     * @brief set the consistency flag.
     */
    void setConsistency(bool c);

    /**
     * @brief set of constraints in the temporal network
     */
    std::set<TemporalConstraintId> m_constraints;

    /**
     * @brief Unique ID of this temporal network instance
     */
    TemporalNetworkId m_id;

   protected:                          // Overridden virtual functions

   /**
     * @brief Creates a new node for the temporal network.
     * @return the new node.
     */
    DnodeId makeNode();

   /**
     * @brief Identify if the network has cycles.
     * @return returns true iff network contains cycles, false otherwise.
     */
    Bool cycleDetected (DnodeId next);

    /**
     * @brief Stores the changes made to nodes during propogation for more efficent incremental update
     */
    std::set<TimepointId> m_updatedTimepoints;
  };


  /**
     * @class  Tnode
     * @author Paul H. Morris (with mods by Conor McGann)
     * @date   Mon Dec 27 2004
     * @brief  Node in a temporal network.
     * @ingroup TemporalNetwork
    */

  class Tnode : public Dnode {
    friend class TemporalNetwork;
    friend Void keepEdge (DispatchNode* x, DispatchNode* y, Time d);
    Time lowerBound;
    Time upperBound;
    Int ordinal;
    TemporalConstraintId m_baseDomainConstraint; /*!< Constraint used to enforce timepoint bounds input.*/
    bool m_deletionMarker;
    void handleDiscard();
  public:
    Int index;          // PHM 5/9/2000 Used for matching TPs to dispatch nodes.
    TimepointId ringLeader;  // PHM 9/8/2000: Ptr to leading member of TEQ.
    std::list<TimepointId> ringFollowers;  // Does not include ringLeader.
    TemporalNetwork* owner;
  public:
    Tnode(TemporalNetwork* t);
    virtual ~Tnode();
    const TemporalConstraintId& getBaseDomainConstraint() const;
    void setBaseDomainConstraint(const TemporalConstraintId& constraint);
    inline void clearDeletionMarker() {m_deletionMarker = false;}
    inline bool getDeletionMarker() const { return m_deletionMarker; }
    inline const Time& getLowerBound() const {return lowerBound;}
    inline const Time& getUpperBound() const {return upperBound;}
    inline void getBounds(Time& lb, Time& ub) const {lb = lowerBound; ub = upperBound;}

  };

  /**
     * @class  Tspec
     * @author Paul H. Morris (with mods by Conor McGann)
     * @date   Mon Dec 27 2004
     * @brief  Temporal specification of edge between two timepoints
     * @ingroup TemporalNetwork
    */
  class Tspec : public Entity {
    friend class TemporalNetwork;
    Time lowerBound;
    Time upperBound;
    TimepointId head;
    TimepointId foot;
    TemporalNetwork* owner;
    TemporalConstraintId m_id;
    unsigned int m_edgeCount;
    void handleDiscard();

  public:
    /**
     * @brief constructor
     * @param t the temporal network to which this specification belongs
     * @param src start of the specification
     * @param targ end of the specification
     * @param lb lower bound
     * @param ub upper bound
     * @param edgeCount edge count
     */
    Tspec(TemporalNetwork* t, TimepointId src,TimepointId targ,Time lb,Time ub, unsigned short edgeCount)
      :owner(t), m_id(this), m_edgeCount(edgeCount)
    { head=src; foot=targ; lowerBound=lb; upperBound=ub;}

    virtual ~Tspec();

    /**
     * @brief return the temporal network that this Tspec is associated with
     * @return the temporal network id
     */
    const TemporalConstraintId& getId() const;

    /**
     * @brief get the upper and lower bounds of this Tspec
     * @param lb returns the lower bound
     * @param ub returns the upper bound
     */
    inline void getBounds(Time& lb, Time& ub) const {lb = lowerBound; ub = upperBound;}

    /**
     * @brief test if Tspec is complete
     * @return returns true if Tspec is complete, false otherwise.
     */
    inline bool isComplete() const {return m_edgeCount == 2;}
  };

} /* namespace Europa */

#endif
