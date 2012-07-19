#ifndef NODE_HEADER_FILE__
#define NODE_HEADER_FILE__

/**
 * @file Edge.hh
 * @author David Rijsman
 * @brief Defines the public interface for a node, part of directed graph
 * @date April 2006
 * @ingroup Resource
 */

#include "Types.hh"

namespace EUROPA
{
    /**
     * @brief
     */
    class Node
    {
      friend class Edge;
      friend class Graph;
      friend class EdgeIterator;
      friend class EdgeOutIterator;
    public:
      /**
       * @brief Constructor
       */
      Node( const NodeIdentity& identity );
      /**
       * @brief Destructor
       */
      ~Node();
      /**
       * @brief
       */
      inline const NodeIdentity& getIdentity() const;
      /**
       * @brief Returns true if the invoking node is enabled otherwise returns false.
       */
      inline bool isEnabled() const;
      /**
       * @brief
       */
      inline const EdgeList& getOutEdges() const;
      /**
       * @brief
       */
      inline const EdgeList& getInEdges() const;
      /**
       * @brief
       */
      inline void setDisabled();
      /**
       * @brief
       */
      inline void setEnabled();
      /**
       * @brief
       */
      inline int getVisit();
      /**
       * @brief
       */
      inline void setVisit( int v );
    private:
      /**
       * @brief Adds \a edge to the outgoing edges of the invoking node.
       */
      void addOutEdge( Edge* edge );
      /**
       * @brief Adds \a edge to the ingoing edges of the invoking node.
       */
      void addInEdge( Edge* edge );
      /**
       * @brief
       */
      void removeOutEdge( Edge* edge );
      /**
       * @brief
       */
      void removeInEdge( Edge* edge );

      bool m_Enabled;
      int m_Visit;
      NodeIdentity m_Identity;
      EdgeList m_InEdges;
      EdgeList m_OutEdges;
    };

    /**
     * @brief
     */
    std::ostream& operator<<( std::ostream& os, const Node& fn ) ;

    int Node::getVisit() {
      return m_Visit;
    }

    void Node::setVisit( int v ) {
      m_Visit = v;
    }

    const EdgeList& Node::getOutEdges() const {
      return m_OutEdges;
    }

    const EdgeList& Node::getInEdges() const {
      return m_InEdges;
    }

    const NodeIdentity& Node::getIdentity() const {
      return m_Identity;
    }

    bool Node::isEnabled() const {
      return m_Enabled;
    }

    void Node::setDisabled() {
      m_Enabled = false;
    }

    void Node::setEnabled()  {
      m_Enabled = true;
    }

}

#endif //NODE_HEADER_FILE__
