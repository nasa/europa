#include "BoostFlowProfileGraph.hh"
#include "Debug.hh"
#include "ConstrainedVariable.hh"
#include "Domain.hh"

#include <algorithm>
// #include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
// #include <boost/graph/edmonds_karp_max_flow.hpp>

namespace EUROPA {

BoostFlowProfileGraph::BoostFlowProfileGraph(const TransactionId source, 
                                             const TransactionId sink, 
                                             bool lowerLevel)
    : FlowProfileGraph(source, sink, lowerLevel), m_graph(), m_transactionToVertex(), 
      m_vertexToTransaction(), m_activeTransactions(), m_source(), m_sink(),
      m_recalculate(false) {
  using namespace boost;
  initializeGraph(source, sink);
}

void BoostFlowProfileGraph::initializeGraph(const TransactionId source, 
                                            const TransactionId sink) {

  m_graph.clear();
  m_transactionToVertex.clear();
  m_vertexToTransaction.clear();
  m_source = addNode(source);
  m_sink = addNode(sink);
  
}

BoostFlowProfileGraph::Vertex 
BoostFlowProfileGraph::addNode(const TransactionId t) {
  std::map<TransactionId, Vertex>::const_iterator found = m_transactionToVertex.find(t);
  if(found != m_transactionToVertex.end())
    return found->second;
  Vertex retval = add_vertex(m_graph);
  debugMsg("BoostFlowProfileGraph:addNode", "Added " << retval << " for " << t);
  m_transactionToVertex.insert(std::make_pair(t, retval));
  m_vertexToTransaction.insert(std::make_pair(retval, t));
  return retval;
}

BoostFlowProfileGraph::Vertex 
BoostFlowProfileGraph::getNode(const TransactionId t) const {
  std::map<TransactionId, Vertex>::const_iterator it = m_transactionToVertex.find(t);
  checkError(it != m_transactionToVertex.end(), 
             "Failed to find a vertex for " << t);
  return it->second;
}

TransactionId BoostFlowProfileGraph::getTransaction(const Vertex& v) const {
  std::map<Vertex, TransactionId>::const_iterator it = m_vertexToTransaction.find(v);
  checkError(it != m_vertexToTransaction.end(), 
             "Failed to find a transaction for " << v);
  return it->second;
}

BoostFlowProfileGraph::Edge BoostFlowProfileGraph::addEdge(const TransactionId t1,
                                                           const TransactionId t2,
                                                           const edouble capacity, 
                                                           const edouble reverseCapacity) {
  debugMsg("BoostFlowProfileGraph:addEdge", 
           (isLowerLevel() ? "<lower>" : "<upper>") << t1 << " -> " << t2 << " [" << 
           capacity << "]");
  debugMsg("BoostFlowProfileGraph:addEdge", 
           (isLowerLevel() ? "<lower>" : "<upper>") << t2 << " -> " << t1 << " [" << 
           reverseCapacity << "]");
  
  using namespace boost;
  property_map<Graph, edge_reverse_t>::type rev = get(edge_reverse, m_graph);
  Vertex v1 = getNode(t1);
  Vertex v2 = getNode(t2);
  
  std::pair<Edge, bool> found = edge(v1, v2, m_graph);
  if(found.second == true)
    return found.first;
  Edge e1 = add_edge(v1, v2, m_graph).first;
  Edge e2 = add_edge(v2, v1, m_graph).first;
           
 
  put(edge_capacity, m_graph, e1, cast_basis(capacity));
  put(edge_capacity, m_graph, e2, cast_basis(reverseCapacity));

  debugMsg("BoostFlowProfileGraph:addEdge",
           (isLowerLevel() ? "<lower>" : "<upper>") << e1 << " [" << 
           get(edge_capacity, m_graph, e1) << "]");
  debugMsg("BoostFlowProfileGraph:addEdge",
           (isLowerLevel() ? "<lower>" : "<upper>") << e2 << " [" << 
           get(edge_capacity, m_graph, e2) << "]");
 
  rev[e1] = e2;
  rev[e2] = e1;
  return e1;
}

void BoostFlowProfileGraph::enableAt(const TransactionId t1, const TransactionId t2) {
  if(m_transactionToVertex.find(t1) == m_transactionToVertex.end() ||
     m_transactionToVertex.find(t2) == m_transactionToVertex.end())
    return;
  m_recalculate = true;
  addEdge(t1, t2, PLUS_INFINITY, PLUS_INFINITY);
}

void BoostFlowProfileGraph::enableAtOrBefore(const TransactionId t1,
                                             const TransactionId t2) {
  if(m_transactionToVertex.find(t1) == m_transactionToVertex.end() ||
     m_transactionToVertex.find(t2) == m_transactionToVertex.end())
    return;
  m_recalculate = true;
  addEdge(t1, t2, 0, PLUS_INFINITY);
}

void BoostFlowProfileGraph::disable(const TransactionId transaction) {
  std::vector<TransactionId>::iterator it = 
      std::find(m_activeTransactions.begin(), m_activeTransactions.end(), transaction);
  if(it != m_activeTransactions.end())
    m_activeTransactions.erase(it);
}

void BoostFlowProfileGraph::addTransactionToGraph(const TransactionId t) {
  if(t == getTransaction(m_sink) || t == getTransaction(m_source))
    return;
  debugMsg("BoostFlowProfileGraph:addTransactionToGraph", 
           (isLowerLevel() ? "<lower>" : "<upper>") << "Adding " << t);

  edouble edgeCapacity = 0.0;
  TransactionId source, target;

  if(isLowerLevel() == t->isConsumer()) {
    source = getTransaction(m_source);
    target = t;
    edgeCapacity = t->quantity()->lastDomain().getUpperBound();
  }
  else {
    source = t;
    target = getTransaction(m_sink);
    edgeCapacity = t->quantity()->lastDomain().getLowerBound();
  }

  check_error(source != TransactionId::noId());
  check_error(target != TransactionId::noId());

  addNode(t);
  addEdge(source, target, edgeCapacity, 0.0);
}

void BoostFlowProfileGraph::enableTransaction(const TransactionId t,
                                              const InstantId inst,
                                              TransactionId2InstantId ) {
  debugMsg("BoostFlowProfileGraph:enableTransaction", 
           (isLowerLevel() ? "<lower>" : "<upper>") << "Enabling " << 
           (t->isConsumer() ? "consumer " : "producer ") << t << "" << t->toString() <<
           " at " << inst->getTime());

  // edouble edgeCapacity = 0.0;

  if(std::find(m_activeTransactions.begin(), m_activeTransactions.end(), t) ==
     m_activeTransactions.end()) {
    m_activeTransactions.push_back(t);
  }
  m_recalculate = true;
}

edouble BoostFlowProfileGraph::getResidualFromSource(const TransactionIdTransactionIdPair2Order& at,
                                                     const TransactionIdTransactionIdPair2Order& other) {
  using namespace boost;
  reset();
  for(std::vector<TransactionId>::const_iterator it = m_activeTransactions.begin();
      it != m_activeTransactions.end(); ++it) {
    addTransactionToGraph(*it);
  }
  for(TransactionIdTransactionIdPair2Order::const_iterator it = at.begin(); 
      it != at.end(); ++it) {
    enableAt(it->first.first, it->first.second);
  }
  for(TransactionIdTransactionIdPair2Order::const_iterator it = other.begin();
      it != other.end(); ++it) {
    debugMsg("BoostFlowProfileGraph:getResidualFromSource", 
             it->first.first << 
             (it->second == AFTER_OR_AT ? " after-or-at " :
              (it->second == BEFORE_OR_AT ? " before-or-at " :
               (it->second == NOT_ORDERED ? " unordered " :
                (it->second == STRICTLY_AT ? " at " : " unknown ")))) <<
             it->first.second);
    switch(it->second) {
      case AFTER_OR_AT:
        enableAtOrBefore(it->first.second, it->first.first);
        break;
      case BEFORE_OR_AT:
        enableAtOrBefore(it->first.first, it->first.second);
      case NOT_ORDERED:
        break;
      case STRICTLY_AT:
        enableAt(it->first.first, it->first.second);
      case UNKNOWN:
        break;
    }
  }
  return getResidualFromSource();
}

edouble BoostFlowProfileGraph::getResidualFromSource() {
  using namespace boost;
  debugMsg("BoostFlowProfileGraph:getResidualFromSource", 
           (isLowerLevel() ? "<lower>" : "<upper>") << "Getting residual...");
  edouble residual = 0.0;
  if(num_vertices(m_graph) <= 2) {
    m_recalculate = false;
    return residual;
  }
  if(m_recalculate) {
    double flow = boykov_kolmogorov_max_flow(m_graph, m_source, m_sink);
    debugMsg("BoostFlowProfileGraph:getResidualFromSource", "Total flow: " << flow);
    property_map<Graph, edge_flow_t>::type flowMap = get(edge_flow, m_graph);
    property_map<Graph, edge_capacity_t>::type capcityMap = get(edge_capacity, m_graph);
    Graph::edge_iterator it, end;
    for(tie(it, end) = edges(m_graph); it != end; ++it) {
      debugMsg("BoostFlowProfileGraph:getResidualFromSource",
               getTransaction(source(*it, m_graph)) << " -> " << 
               getTransaction(target(*it, m_graph)) << 
               " [" << flowMap[*it] << ":" << capcityMap[*it] << "]");
    }
    m_recalculate = false;
  }
  property_map<Graph, edge_residual_capacity_t>::type
      residualCapacity = get(edge_residual_capacity, m_graph);
  Graph::out_edge_iterator outIt, outEnd;
  tie(outIt, outEnd) = out_edges(m_source, m_graph);
  for(; outIt != outEnd; ++outIt) {
    residual += residualCapacity[*outIt];
    debugMsg("BoostFlowProfileGraph:getResidualFromSource",
             getTransaction(source(*outIt, m_graph)) << " -> " << 
             getTransaction(target(*outIt, m_graph)) << 
             " [" << residualCapacity[*outIt] << "]: " << residual);
  }
  
  return residual;
}

void BoostFlowProfileGraph::removeTransaction(const TransactionId id) {
  using namespace boost;
  debugMsg("BoostFlowProfileGraph:removeTransaction", 
           (isLowerLevel() ? "<lower>" : "<upper>") << "Removing " << id);
  disable(id);
  m_recalculate = true;
}

void BoostFlowProfileGraph::reset() {
  initializeGraph(getTransaction(m_source), getTransaction(m_sink));
}

}

