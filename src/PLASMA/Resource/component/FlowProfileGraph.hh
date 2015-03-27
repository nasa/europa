#ifndef H_FlowProfileGraph
#define H_FlowProfileGraph


#include "Types.hh"

namespace EUROPA {

class Graph;
class MaximumFlowAlgorithm;
class Node;

/**
 * @brief Graph structure to determine the subset of pending transactions at time T
 * which has the largest contribution to an envelope (see FlowProfile)
 */
class FlowProfileGraph
{
 public:
  /**
   * @brief Creates a directed graph with a source and a sink intended to calculate the
   * lower level envelope in case lowerLevel is true otherwise intended to calculate the
   * upper level envelope.
   */
  FlowProfileGraph( const TransactionId source, const TransactionId sink, bool lowerLevel );
  /**
   * @brief Destructor
   */
  virtual ~FlowProfileGraph() {}
  /**
   * @brief Creates bi-directional edge between \a t1 and \a t2 with infinite capacity
   * as a result of a concurrent constraint between the two transactions
   */
  virtual void enableAt( const TransactionId t1, const TransactionId t2 ) = 0;
  /**
   * @brief Creates directed edge between \a t1 and \a t2 with infinite capacity
   * as a result of a before or at constraint between the two transactions (reverse
   * capacity set to zero)
   */
  virtual void enableAtOrBefore( const TransactionId t1, const TransactionId t2 ) = 0;
  /**
   * @brief Creates a node in the network and creates an edge:
   *
   * \verbatim
   *                               |            lower level               |            upper level
   *   ---------------------------------------------------------------------------------------------------------
   *   transaction is consumer     |  <source, transaction> w = q.upper   |  <transaction, sink> w = q.lower
   *   ---------------------------------------------------------------------------------------------------------
   *   transaction is producer     |  <transaction, sink > w = q.lower    |  <source, transaction> w = q.upper
   *   ---------------------------------------------------------------------------------------------------------
   * \endverbatim
   */
  virtual void enableTransaction( const TransactionId transaction, const InstantId inst, TransactionId2InstantId contributions ) = 0;
  /**
   * @brief Returns true if \a transaction is enabled in the invoking
   * instance
   */
  virtual bool isEnabled(  const TransactionId transaction ) const = 0;
  /**
   * @brief Disables \a transaction, if enabled, for the invoking instance
   */
  virtual void disable( const TransactionId transaction ) = 0;
  /**
   * @brief Will push any flow wich flows through the node corresponding with \a transaction
   * back to the source of the edge the flow originates from.
   *
   * When we move from one instant to another we retract all the transactions whose end time
   * is equal to the new instant time and the transaction is still in the network. This method
   * will push any flow going through this node back to where it is coming from after which we
   * try again to push the flow to the sink.

   * Will error out if no node corresponding to \a transaction is in the network or if the node
   * is not enabled.
   *
   * If a recalculation of the maximum flow is required this method will do nothing.
   * \todo verify if this is really required or perhaps we should error out?
   */
  virtual void pushFlow( const TransactionId transaction ) = 0;
  /**
   * @brief Returns the cummulative residual capacity originating from the source.
   *
   * Iterates over all outgoing edges from the source and sums the residual capicity of each
   * edge. Might trigger a maximum flow (re) calculation if required.
   */
  virtual edouble getResidualFromSource() = 0;
  virtual edouble getResidualFromSource(const TransactionIdTransactionIdPair2Order& at,
                                        const TransactionIdTransactionIdPair2Order& other) = 0;
  /**
   * @brief Disables every node reachable from the source in the residual network. Returns the sum
   * of the contribution of each disabled node. The contribution is determined as following:
   * \verbatim
   *                               |  lower level |  upper level
   *   ---------------------------------------------------------------------------------------------------------
   *   transaction is consumer     |    q.upper   |   q.lower
   *   ---------------------------------------------------------------------------------------------------------
   *   transaction is producer     |    q.lower   |   q.upper
   *   ---------------------------------------------------------------------------------------------------------
   * \endverbatim
   *
   * where q is the quantity variable associated with the transactions associated with the node. The parameter \a
   * contributions, which maps a TransactionId to a InstantId is maps every transaction associated with a disabled
   * node to \a instant.
   */
  virtual edouble disableReachableResidualGraph( TransactionId2InstantId contributions, const InstantId instant  ) = 0;
  /**
   * @brief Returns true if the invoking instance calculates the lower level, otherwise returns false which indicates
   * the invoking instance is calculating the upper level.
   */
  bool isLowerLevel() const { return m_lowerLevel; }
  /**
   * @brief Removes transaction \a id from the network.
   */
  virtual void removeTransaction( const TransactionId id ) = 0;
  /**
   * @brief Resets the invoking instance.
   *
   * Resetting entails disabling all the nodes, implying disabling all the edges, except for the
   * source and the sink.
   *
   */
  virtual void reset() = 0;
  /**
   * @brief Restore flow invokes the maximum flow algorithm without resetting the existing distances
   * and existing flows on the nodes.
   *
   * Restore flows is done after extracting all the nodes that expire (go from pending to closed set
   * at an instant) from the network by pushing flow back.
   */
  virtual void restoreFlow() = 0;
 protected:
  /*!
   * @brief Boolean indicating if the instance is intended to calculate the lower level
   */
  bool m_lowerLevel;
  /*!
   * @brief Boolean indicating if the maximum flow solution needs to be recalculated
   */
  bool m_recalculate;
};

class FlowProfileGraphImpl : public FlowProfileGraph {
private:
  FlowProfileGraphImpl(const FlowProfileGraphImpl&);
  FlowProfileGraphImpl& operator=(const FlowProfileGraphImpl&);
 public:
  FlowProfileGraphImpl(const TransactionId source, const TransactionId sink, bool lowerLevel);
  ~FlowProfileGraphImpl();
  void enableAt( const TransactionId t1, const TransactionId t2 );
  void enableAtOrBefore(const TransactionId t1, const TransactionId t2);
  void enableTransaction(const TransactionId transaction, const InstantId inst,
                         TransactionId2InstantId contributions);
  bool isEnabled(const TransactionId transaction) const;
  void disable(const TransactionId transaction);
  void pushFlow( const TransactionId transaction );
  edouble getResidualFromSource();
  edouble getResidualFromSource(const TransactionIdTransactionIdPair2Order&,
                                const TransactionIdTransactionIdPair2Order&) {
    return getResidualFromSource();
  }

  edouble disableReachableResidualGraph(TransactionId2InstantId contributions, const InstantId instant);
  void removeTransaction(const TransactionId id);
  void reset();
  void restoreFlow();
 private:
  /**
   * @brief Helper function for disableReachableResidualGraph
   */
  void visitNeighbors( const Node* node, edouble& residual, Node2Bool& visited, TransactionId2InstantId contributions, const InstantId instant  );

  MaximumFlowAlgorithm* m_maxflow;
  /*!
   * @brief Bi directional graph datastructure
   */
  Graph* m_graph;
  /*!
   * @brief Source for the maximum flow problem
   */
  Node* m_source;
  /*!
   * @brief Sink for the maximum flow problem
   */
  Node* m_sink;

};

}

#endif
