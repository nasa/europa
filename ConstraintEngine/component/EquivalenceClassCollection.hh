#ifndef _H_EquivalenceClassCollection
#define _H_EquivalenceClassCollection

/**
 * @file EquivalenceClassCollection.hh
 * @author Conor McGann
 * @date August, 2003
 * @brief Introduces the components reuired to manage equality relations among variables as a graph that can
 * be organized into disjoint sub-graphs (i.e. equivalence classes).
 */

#include "ConstraintEngineDefs.hh"
#include <set>
#include <map>

namespace Prototype{

  class Node;
  typedef Id<Node> NodeId;

  /**
   * @class Node
   * @brief A node in an Equivalence Graph
   * 
   * Neighbours are other nodes with whom an Equality constraint has been posted
   */
  class Node{
  public:
    /**
     * @brief Constructor. The node shadows exatly one variable.
     * @param variable The constrainedVariable represented in the graph
     */
    Node(const ConstrainedVariableId& variable);

    /**
     * @brief Return true of the nodes graph membership has been recomputed during the given cycle.
     * @param cycleCount The latest cycle for recomputing graph membership.
     */
    bool hasBeenUpdated(int cycleCount) const;

    /**
     * @brief Call to update the graph membership of this node and all connected nodes.
     * @param cycleCount The latest cycle for recomputing graph membership.
     * @param connectedVariables The working set of connected variable sin the graph. It is added to be each node visited.
     * @param graph The new graph key to become a member of.
     * @param oldGraphKeys The working set of old graph keys held by nodes visited. At the end these must be removed from the set of graphs.
     */
    void update(int cycleCount, std::set<ConstrainedVariableId>& connectedVariables, int graph, std::set<int>& oldGraphKeys);

    /**
     * @brief Synonomous to the addition of an equality constraint with the given node. Creates a link in the graph.
     * @param node The node to which a link is being created
     */
    void addNeighbour(const NodeId& node);

    /**
     * @brief Synonomous with removal of an equality constraint with the given node. Removes a link from the graph.
     * @param node The node from which a link is being removed
     * @see isAlone
     */
    void removeNeighbour(const NodeId& node);

    /**
     * @brief Accessor for the current graph to which this node belongs. All nodes with the same graph key form an equivalence class.
     */
    int getGraph() const;

    /**
     * @brief Detect if a node has no neighbours in the graph.
     * @return true if it has no neighbours, otherwise false. If true, then it will subsequently be removed.
     */
    bool isAlone() const;

  private:
    const ConstrainedVariableId m_variable;
    int m_graph;
    int m_lastUpdated;
    std::set<NodeId> m_neighbours;
  };

  /**
   * @class EquivalenceClassCollection
   * @brief Manager of the graph of nodes to organize them into disjoint sub-graphs which form equivalence classes
   */
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

    /**
     * @brief Remove a node if it has become disconnected from all neighbours
     * @see Node::isAlone()
     */
    void removeNode(const ConstrainedVariableId& variable);

    /**
     * @brief May recompute the set of graphs if necessary.
     * @return true if it was necessary to recompute all the graphs again (i..e. fullReprop required). Otherwise false.
     * @see m_requiresUpdate, recomputeSingleGraph
     */
    bool recomputeIfNecessary();

    /**
     * @brief Recompute the graph for this ndoe and all connected nodes. Accomplished by depth first search as in Europa code.
     * @param node The node to start from
     */
    void recomputeSingleGraph(const NodeId& node);

    /**
     * @brief Helper method to ensure integrity of the data
     */
    bool isValid() const;

    std::map<ConstrainedVariableId, NodeId> m_nodesByVar; /*!< Table to map constrained variables to their representative node in the graph */
    std::map<int, std::set<ConstrainedVariableId> > m_graphsByKey; /*!< Map of the graph key to the set of constrained variables which are 
								     inferred to be equivalent. This changes when constraints are added or removed. */
    bool m_requiresUpdate; /*!< Indicates of we must recompute all graps. True of a constraint has been removed. Made false by recomputing. */

    static int s_nextCycle; /*!< Monotonically increasing counter used to ensure we do not visit nodes more than once when recomputing a graph.*/
    static int s_nextGraph; /*!< Monotnically increasing counter used to allocate new graph keys when the need arises to recompute a graph. The
			      cases are when a constraint addition occurs, thus allowing for the possibility of graph merging, or a constraint 
			      removal occurs allowing for the possibility of graph splitting. */
  };
}

#endif
