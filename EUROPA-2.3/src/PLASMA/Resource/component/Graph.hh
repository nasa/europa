#ifndef GRAPH_HEADER_FILE_
#define GRAPH_HEADER_FILE_

/**
 * @file Graph.hh
 * @author David Rijsman
 * @brief Defines the public interface for a directed graph
 * @date April 2006
 * @ingroup Resource
 */

#include "Edge.hh"
#include "Node.hh"
#include "Types.hh"

namespace EUROPA
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
      inline Edge* getEdge( Node* source, Node* target ) const;
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

    Edge* Graph::getEdge( Node* source, Node* target ) const
    {
      checkError( 0 != source, "Null not allowed as input for source" );
      checkError( 0 != target, "Null not allowed as input for target" );

      const EdgeList& outEdges = source->getOutEdges();

      EdgeList::const_iterator fIte = outEdges.begin();
      EdgeList::const_iterator fEnd = outEdges.end();

      for( ; fIte != fEnd; ++fIte )
	{
	  Edge* edge = *fIte;

	  if( edge->getTarget() == target )
	    {
	      return edge;

	      break;
	    }
	}

      return 0;
    }
}
#endif //GRAPH_HEADER_FILE_
