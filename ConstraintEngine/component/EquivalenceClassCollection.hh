#ifndef _H_EquivalenceClassCollection
#define _H_EquivalenceClassCollection

#include "ConstraintEngineDefs.hh"
#include <set>
#include <map>

namespace Prototype{

  class Node;
  typedef Europa::Id<Node> NodeId;

  class Node{
  public:
    Node(const ConstrainedVariableId& variable);
    bool hasBeenUpdated(int cycleCount) const;
    void update(int cycleCount, std::set<ConstrainedVariableId>& connectedVariables, int graph, std::set<int>& oldGraphKeys);
    void addNeighbour(const NodeId& node);
    void removeNeighbour(const NodeId& node);
    int getGraph() const;
    bool isAlone() const;

  private:
    const ConstrainedVariableId m_variable;
    int m_graph;
    int m_lastUpdated;
    std::set<NodeId> m_neighbours;
  };

  class EquivalenceClassCollection{
  public:
    EquivalenceClassCollection();
    ~EquivalenceClassCollection();
    void addConnection(const ConstrainedVariableId& v1, const ConstrainedVariableId& v2);
    void removeConnection(const ConstrainedVariableId& v1, const ConstrainedVariableId& v2);
    bool updateRequired() const {return m_requiresUpdate;}

    int getGraphCount();
    int getGraphKey(const ConstrainedVariableId& variable);
    void getGraphKeys(std::set<int>& keys);
    const std::set<ConstrainedVariableId>& getGraphVariables(int key) const;
  private:
    const NodeId& getNode(const ConstrainedVariableId& variable);
    void removeNode(const ConstrainedVariableId& variable);
    void recomputeIfNecessary();
    void recomputeSingleGraph(const NodeId& node);
    std::map<ConstrainedVariableId, NodeId> m_nodesByVar;
    std::map<int, std::set<ConstrainedVariableId> > m_graphsByKey;
    bool m_requiresUpdate;

    static int s_nextCycle;
    static int s_nextGraph;
  };
}

#endif
