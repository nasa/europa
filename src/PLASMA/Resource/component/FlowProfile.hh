#ifndef FLOW_PROFILE_HEADER__
#define FLOW_PROFILE_HEADER__

/**
 * @file Profile.hh
 * @author David Rijsman
 * @brief Defines the public interface for a maximum flow algorithm
 * @date April 2006
 * @ingroup Resource
 */

#include "DomainListener.hh"
#include "MaxFlow.hh"
#include "Profile.hh"
#include "ResourceDefs.hh"
#include "Types.hh"
#include "TemporalPropagator.hh"
#include "FlowProfileGraph.hh"
#ifdef _MSC_VER
#  include <map>
#endif

namespace EUROPA
{
class Graph;
class MaximumFlowAlgorithm;
class Node;

    /**
     * @brief Calculates the lower and upper level envelope of a resource.
     *
     * Given a set of transactions on a resource and temperol relations between
     * the transactions this algorithm calculates the tightest bounds possible.
     * The algorithm calculates the envelopes at any possible time the envelope
     * can change (at each instant).
     *
     * At each instant T the algorith determines the sets of transactions which
     * have to contribute to the envelope level, closed set, the set which can
     * contribute to the envelopes, the pending set, and the set of transaction
     * which can not contribute to the envelopes, the open set.
     *
     * The closed set are all transactions whose time upperbound is lower or
     * equal to T. The pending set are the transaction whose lower time bound
     * is lower or equal to T and the upper time bound is greater than T. The
     * open set of transactions are transaction whose lower time bound is greater
     * than T.
     *
     * To calculate the upper level envelope L(T) at time T we do the following:
     * -1- Initialize L(T) = 0
     * -2- Take the sum of the upper bound of the quantity of the production
     *     transactaction in the closed set and add to L(T)
     * -3- Take the sum of the lower bound of the quantity of the consumer
     *     transactaction in the closed set and add to L(T)
     * -4- Of all the transactions in the pending set at time T determine the
     *     the set whose sum of production and consumptions is larger than any
     *     other set without violating any of the temporal relations (if two
     *     transactions need to be executed at the same time they always need
     *     to be in the same set). Add the sum of this set to L(T)
     *
     * To determine the set in step -4- we solve a maximum flow algorithm on
     * the pending set. See FlowProfileGraph on how we construct a maximum
     * flow graph out of the pending set.
     *
     * See 'N Muscettola. Computing the Envelope for Stepwise-Constant
     * Resource Allocations. CP 2002, LNCS 2470, pp 139-154, 2002'
     */
class FlowProfile: public Profile {
private:
  FlowProfile(const FlowProfile&);
  FlowProfile& operator=(const FlowProfile&);
 public:
  /**
   * @brief Constructor
   */
  FlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector);
  /**
   * @brief Destructor
   */
  virtual ~FlowProfile();
  /**
   * @brief Retrieves the first (earliest) instant the transaction starts contributing to
   * lower level. Will return true in case a instant has been associated with a transaction
   * and will in that case store the instant in parameter. Returns false if no instant
   * has been associated with transaction t.
   */
  bool getEarliestLowerLevelInstant( const TransactionId t, InstantId& i );
  /**
   * @brief Retrieves the first (earliest) instant the transaction starts contributing to
   * upper level. Will return true in case a instant has been associated with a transaction
   * and will in that case store the instant in parameter. Returns false if no instant
   * has been associated with transaction t.
   */
  bool getEarliestUpperLevelInstant( const TransactionId t, InstantId& i );
  /**
   * @brief Deletes pre-existing FlowProfileGraphs for the lower and upper level and allocates new ones.
   */
  template<typename FlowGraphType>
  void initializeGraphs() {
    if(m_lowerLevelGraph != NULL)
      delete m_lowerLevelGraph;
    m_lowerLevelGraph = new FlowGraphType(m_dummySourceTransaction,
                                          m_dummySinkTransaction, true);

    if(m_upperLevelGraph != NULL)
      delete m_upperLevelGraph;
    m_upperLevelGraph = new FlowGraphType(m_dummySourceTransaction,
                                          m_dummySinkTransaction, false);
  }


 protected:
  virtual void postHandleRecompute(const eint& endTime, const std::pair<edouble,edouble>& endDiff);
  /**
   * @brief Enables a transaction t. A transaction is enabled a time T to calculate the
   * envelopes for instant at time T if the lower bound of the time equals T (this is assuming
   * envelopes are calculated from earliest to latest instant). Another way of formulating this
   * is that a transaction is enabled at the time it moves from the open set to the pending set
   * or straight to the closed set if the time is a singleton.
   */
  void enableTransaction( const TransactionId t, const InstantId i );
  /**
   * @brief Helper method for subclasses to respond to a temporal constraint being added between two transactions.
   */
  void handleTemporalConstraintAdded(const TransactionId predecessor,
                                     const unsigned int preArgIndex,
                                     const TransactionId successor,
                                     const unsigned int sucArgIndex);

  /**
   * @brief Helper method for subclasses to respond to a temporal constraint being removed between two transactions.
   */
  void handleTemporalConstraintRemoved(const TransactionId predecessor,
                                       const unsigned int preArgIndex,
                                       const TransactionId successor,
                                       const unsigned int sucArgIndex);
  /**
   * @brief Updates the maximum flow graphs in case transactions t1 and t2 are now strictly ordered.
   */
  void handleOrderedAt( const TransactionId t1, const TransactionId t2 );
  /**
   * @brief Updates the maximum flow graphs in case transactions t1 and t2 are now weakly ordered.
   */
  void handleOrderedAtOrBefore( const TransactionId t1, const TransactionId t2 );

  void handleTransactionAdded( const TransactionId t);
  void handleTransactionRemoved( const TransactionId t);
  void handleTransactionTimeChanged( const TransactionId t,
                                     const DomainListener::ChangeType& type );
  void handleTransactionQuantityChanged( const TransactionId t,
                                         const DomainListener::ChangeType& type );
  void initRecompute(InstantId inst);
  void initRecompute();
  Order getOrdering( const TransactionId t1, const TransactionId t2 );
  void recomputeLevels(InstantId prev, InstantId inst);

  typedef std::pair< eint, eint > IntIntPair;
#ifdef _MSC_VER
  typedef std::map< TransactionId, IntIntPair > TransactionId2IntIntPair;
#else
  typedef boost::unordered_map< TransactionId, IntIntPair, TransactionIdHash > TransactionId2IntIntPair;
#endif //_MSC_VER

  TransactionId2IntIntPair m_previousTimeBounds;

  TransactionId m_dummySourceTransaction;
  TransactionId m_dummySinkTransaction;

  FlowProfileGraph* m_lowerLevelGraph;
  FlowProfileGraph* m_upperLevelGraph;

  edouble m_lowerClosedLevel;
  edouble m_upperClosedLevel;

  bool m_recalculateLowerLevel;
  bool m_recalculateUpperLevel;


  TransactionIdTransactionIdPair2Order m_orderings;
  TransactionIdTransactionIdPair2Order m_orderedAt;

  TransactionId2InstantId m_lowerLevelContribution;
  TransactionId2InstantId m_upperLevelContribution;
};
}

#endif //FLOW_PROFILE_HEADER__
