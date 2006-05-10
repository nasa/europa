#ifndef GRAPH_HEADER_FILE_
#define GRAPH_HEADER_FILE_

/**
 * @file SAVH_Graph.hh
 * @author David Rijsman
 * @brief Defines the public interface for a directed graph
 * @date April 2006
 * @ingroup Resource
 */

#include "SAVH_Types.hh"
#include "SAVH_Edge.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
    /**
     * @brief
     */
    class Graph
    {
      friend class NodeIterator;
    public:
      /**
       * @brief
       */
      Graph();
      /**
       * @brief
       */
      ~Graph();
      /**
       * @brief
       */
      Node* getNode( const NodeIdentity& identity ) const;
      /**
       * @brief
       */
      Node* createNode( const NodeIdentity& identity, bool enabled = true );
      /**
       * @brief
       */
      void removeNode( const NodeIdentity& identity );
      /**
       * @brief
       */
      Edge* getEdge( Node* source, Node* target ) const;    
      /**
       * @brief
       */
      void createEdge( const NodeIdentity& source, const NodeIdentity& target, double capacity, bool enabled = true );
      /**
       * @brief
       */
      inline const NodeIdentity2Node& getNodes() const;
      /*!
       * @brief Disables all the nodes and edges of the invoking graph
       */
      void setDisabled();
    private:
      /**
       * @brief
       */
      Edge* createEdge( Node* source, Node* target, double capacity, bool enabled = true );

      NodeIdentity2Node m_Nodes;
    };

    const NodeIdentity2Node& Graph::getNodes() const
    {
      return m_Nodes;
    }
  }
}
#endif //GRAPH_HEADER_FILE_
