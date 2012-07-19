#include "EquivalenceClassCollection.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA{

  ConstraintNode::ConstraintNode(const ConstrainedVariableId& variable)
    : m_variable(variable), m_graph(0), m_lastUpdated(0){}

  bool ConstraintNode::hasBeenUpdated(int nextCycle) const {
    check_error(nextCycle >= m_lastUpdated);
    return (nextCycle == m_lastUpdated);
  }

  int ConstraintNode::getGraph() const {return m_graph;}

  bool ConstraintNode::isAlone() const {return m_neighbours.empty();}

  void ConstraintNode::update(int nextCycle, std::set<ConstrainedVariableId>& connectedVariables, int graph,  std::set<int>& oldGraphKeys){
    check_error(nextCycle > m_lastUpdated);
    // Update local data and add self to set
    m_lastUpdated = nextCycle;
    connectedVariables.insert(m_variable);
    oldGraphKeys.insert(m_graph);
    m_graph = graph;

    // Iterate over any neighbours not yet touched
    for(std::set<ConstraintNodeId>::iterator it = m_neighbours.begin(); it != m_neighbours.end(); ++it)
      if(!(*it)->hasBeenUpdated(nextCycle))
	 (*it)->update(nextCycle, connectedVariables, graph, oldGraphKeys);
  }

  void ConstraintNode::addNeighbour(const ConstraintNodeId& node){
    check_error(m_neighbours.find(node) == m_neighbours.end());
    m_neighbours.insert(node);
  }

  void ConstraintNode::removeNeighbour(const ConstraintNodeId& node){
    check_error(m_neighbours.find(node) != m_neighbours.end());
    m_neighbours.erase(node);
  }

  EquivalenceClassCollection::EquivalenceClassCollection()
    :m_requiresUpdate(false){}

  EquivalenceClassCollection::~EquivalenceClassCollection(){
    for(std::map<ConstrainedVariableId, ConstraintNodeId>::iterator it = m_nodesByVar.begin(); it != m_nodesByVar.end(); ++it)
      it->second.release();
  }

  void EquivalenceClassCollection::addConnection(const ConstrainedVariableId& v1, const ConstrainedVariableId& v2){
    const ConstraintNodeId n1 = getNode(v1);
    const ConstraintNodeId n2 = getNode(v2);
    n1->addNeighbour(n2);
    n2->addNeighbour(n1);
    if(!recomputeIfNecessary()){ // Most commonly this will be true, meaning no full reprop was required.
      s_nextCycle++;
      recomputeSingleGraph(n1);
    }
  }

  void EquivalenceClassCollection::removeConnection(const ConstrainedVariableId& v1, const ConstrainedVariableId& v2){
    ConstraintNodeId n1 = getNode(v1);
    ConstraintNodeId n2 = getNode(v2);

    check_error(n1->getGraph() == n2->getGraph());
    int graph = n1->getGraph();
    m_graphsByKey.erase(graph);

    n1->removeNeighbour(n2);
    n2->removeNeighbour(n1);

    if(n1->isAlone()){
      removeNode(v1);
      n1.release();
    }

    if(n2->isAlone()){
      removeNode(v2);
      n2.release();
    }

    m_requiresUpdate = true;
  }

  int EquivalenceClassCollection::getGraphCount(){
    recomputeIfNecessary();
    return m_graphsByKey.size();
  }

  int EquivalenceClassCollection::getGraphKey(const ConstrainedVariableId& variable){
    recomputeIfNecessary();
    const ConstraintNodeId& node = getNode(variable);
    return node->getGraph();
  }

  void EquivalenceClassCollection::getGraphKeys(std::set<int>& keys){
    keys.clear();
    recomputeIfNecessary();
    for(std::map<int, std::set<ConstrainedVariableId> >::const_iterator it = m_graphsByKey.begin(); it != m_graphsByKey.end(); ++it)
      keys.insert(it->first);
  }


  const std::set<ConstrainedVariableId>& EquivalenceClassCollection::getGraphVariables(int key) const{
    static const std::set<ConstrainedVariableId> sl_emptySet;
    std::map<int, std::set<ConstrainedVariableId> >::const_iterator it = m_graphsByKey.find(key);
    if(it == m_graphsByKey.end())
      return sl_emptySet;
    else
      return it->second;
  }

  bool EquivalenceClassCollection::recomputeIfNecessary(){
    if(!m_requiresUpdate)
      return false;

    // Reset the flag and previously cached results
    m_requiresUpdate = false;

    // Increment the cycle count to force all nodes to be tested again.
    s_nextCycle++;

    // Iterate over all nodes
    for(std::map<ConstrainedVariableId, ConstraintNodeId>::iterator it = m_nodesByVar.begin(); it != m_nodesByVar.end(); ++it){
      const ConstraintNodeId& node = it->second;
      check_error(node.isValid());
      if(!node->hasBeenUpdated(s_nextCycle)) // means we have a new graph to build.
	recomputeSingleGraph(node);
    }

    return true;
  }

  void EquivalenceClassCollection::recomputeSingleGraph(const ConstraintNodeId& node){
    s_nextGraph++;
    // Initialize new graph with an empty set and obtain the reference to fill it up
    std::set<ConstrainedVariableId> emptySet;
    std::map<int, std::set<ConstrainedVariableId> >::iterator  newGraphEntry =
      m_graphsByKey.insert(std::pair<int, std::set<ConstrainedVariableId> >(s_nextGraph, emptySet)).first;

    check_error(newGraphEntry->first == s_nextGraph);
    std::set<ConstrainedVariableId>& newGraph = newGraphEntry->second;

    // Now fill it up
    std::set<int> graphKeysToRemove;
    node->update(s_nextCycle, newGraph, s_nextGraph, graphKeysToRemove);
    check_error(graphKeysToRemove.size() <= 2);
    for(std::set<int>::iterator it = graphKeysToRemove.begin(); it != graphKeysToRemove.end(); ++it)
      m_graphsByKey.erase(*it);
  }

  const ConstraintNodeId& EquivalenceClassCollection::getNode(const ConstrainedVariableId& variable){
    std::map<ConstrainedVariableId, ConstraintNodeId>::iterator it = m_nodesByVar.find(variable);

    if (it == m_nodesByVar.end()){ // Not present yet, so create a new entry
      ConstraintNodeId node(new ConstraintNode(variable));
      it = m_nodesByVar.insert(std::pair<ConstrainedVariableId, ConstraintNodeId>(variable, node)).first;
    }

    check_error(it != m_nodesByVar.end());
    check_error(it->second.isValid());

    return it->second;
  }

  void EquivalenceClassCollection::removeNode(const ConstrainedVariableId& variable){
    check_error(m_nodesByVar.find(variable) != m_nodesByVar.end());
    m_nodesByVar.erase(variable);
  }

  bool EquivalenceClassCollection::isValid() const{
    // Make sure all constrained variables in the map are valid.

    // Make sure all constrained variables in the graphs are valid
    return true;
  }

  int EquivalenceClassCollection::s_nextCycle(0);
  int EquivalenceClassCollection::s_nextGraph(0);
}
