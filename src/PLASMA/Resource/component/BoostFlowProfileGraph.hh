#ifndef H_BoostFlowProfileGraph
#define H_BoostFlowProfileGraph

#include "FlowProfileGraph.hh"
#include "Types.hh"

#include <map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_traits.hpp>

namespace EUROPA {
class BoostFlowProfileGraph : public FlowProfileGraph {
 public:
  /**
   * @brief Creates a directed graph with a a source and a sink intended to calculate the
   * lower level envelope in case a lowerLevel is true otherwise intended to calculate the
   * upper level envelope.
   */
  BoostFlowProfileGraph(const TransactionId source, const TransactionId sink, bool lowerLevel);
  /**
   * @brief Destructor
   */
  ~BoostFlowProfileGraph() {}
  /**
   * @brief Creates bi-directional edge between \a t1 and \a t2 with infinite capacity
   * as a result of a concurrent constraint between the two transactions
   */
   void enableAt(const TransactionId t1, const TransactionId t2);
  /**
   * @brief Creates directed edge between \a t1 and \a t2 with infinite capacity
   * as a result of a before or at constraint between the two transactions (reverse
   * capacity set to zero)
   */
  void enableAtOrBefore(const TransactionId t1, const TransactionId t2);
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
  void enableTransaction(const TransactionId transaction, const InstantId inst,
                         TransactionId2InstantId contributions);
  /**
   * @brief Returns true if \a transaction is enabled in the invoking
   * instance
   */
  bool isEnabled(const TransactionId ) const {return true;}
  /**
   * @brief Disables \a transaction, if enabled, for the invoking instance
   */
  void disable( const TransactionId transaction );
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
  void pushFlow( const TransactionId  ) {return;}
  /**
   * @brief Returns the cummulative residual capacity originating from the source.
   *
   * Iterates over all outgoing edges from the source and sums the residual capicity of each
   * edge. Might trigger a maximum flow (re) calculation if required.
   */
  edouble getResidualFromSource();
  edouble getResidualFromSource(const TransactionIdTransactionIdPair2Order& at,
                                const TransactionIdTransactionIdPair2Order& other);

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
  edouble disableReachableResidualGraph( TransactionId2InstantId, const InstantId  ) {return 0.0;}
  /**
   * @brief Removes transaction \a id from the network.
   */
  void removeTransaction(const TransactionId id);
  /**
   * @brief Resets the invoking instance.
   *
   * Resetting entails disabling all the nodes, implying disabling all the edges, except for the
   * source and the sink.
   *
   */
  void reset();
  /**
   * @brief Restore flow invokes the maximum flow algorithm without resetting the existing distances
   * and existing flows on the nodes.
   *
   * Restore flows is done after extracting all the nodes that expire (go from pending to closed set
   * at an instant) from the network by pushing flow back.
   */
  void restoreFlow() {}
 private:

  typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS> Traits;
  // typedef boost::adjacency_list_traits<boost::listS, boost::listS, boost::directedS> Traits;
  typedef Traits::vertex_descriptor Vertex;
  typedef Traits::edge_descriptor Edge;
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
  // typedef boost::adjacency_list<boost::listS, boost::listS, boost::directedS,
                                boost::property<boost::vertex_index_t, long,
                                boost::property<boost::vertex_color_t, boost::default_color_type,
                                boost::property<boost::vertex_distance_t, long,
                                boost::property<boost::vertex_predecessor_t, Edge > > > >,

                                boost::property<boost::edge_flow_t, double,
                                boost::property<boost::edge_capacity_t, double,
                                boost::property<boost::edge_residual_capacity_t, double,
                                boost::property<boost::edge_reverse_t, Edge > > > > > Graph;

  // do these need to return references?
  Vertex addNode(const TransactionId t);
  Vertex getNode(const TransactionId t) const;
  TransactionId getTransaction(const Vertex& v) const;
  Edge addEdge(const TransactionId t1, const TransactionId t2, const edouble capacity,
               const edouble reverseCapacity);
  void initializeGraph(const TransactionId source, const TransactionId sink);
  void addTransactionToGraph(const TransactionId t);

  Graph m_graph;
  std::map<TransactionId, Vertex> m_transactionToVertex;
  std::map<Vertex, TransactionId> m_vertexToTransaction;
  std::vector<TransactionId> m_activeTransactions;
  Vertex m_source, m_sink;
  bool m_recalculate;
};
}
#endif
