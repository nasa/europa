#ifndef EDGE_HEADER_FLOW_HEADER_
#define EDGE_HEADER_FLOW_HEADER_

/**
 * @file Edge.hh
 * @author David Rijsman
 * @brief Defines the public interface for an edge part of directed graph
 * @date April 2006
 * @ingroup Resource
 */

#include "Types.hh"
#include "Node.hh"

namespace EUROPA
{
    class Edge
    {
      friend class Graph;
      friend class Node;
    public:
      /**
       * @brief Constructor
       * @arg source
       * @arg target
       * @arg capacity
       * @arg enabled
       *
       */
      Edge( Node* source, Node* target, double capacity, bool enabled = true );
      /**
       * @brief Destructor
       *
       */
      ~Edge();
      /*!
       * @brief Returns the maximum capacity of an Edge (infinity)
       */
      static double getMaxCapacity();
      /**
       * @brief Returns the capacity of the invoking edge
       *
       */
      inline double getCapacity() const;
      /**
       * @brief Sets the capacity of the invoking edge
       * @arg cap The new capacity of the invoking edge
       *
       */
      inline void setCapacity( double cap );
      /**
       * @brief Returns the source of the invoking edge
       *
       */
      inline Node* getSource() const;
      /**
       * @brief Returns the target of the invoking edge
       */
      inline Node* getTarget() const;
      /**
       * @brief Enables the invoking edge
       *
       */
      void setEnabled();
      /**
       * @brief Disables the invoking edge
       *
       */
      void setDisabled();
      /**
       * @brief Returns true of the invoking edge is enabled otherwise returns false
       *
       */
      inline bool isEnabled() const;
      /**
       * @brief Returns the identity of the invoking edge
       *
       */
      EdgeIdentity getIdentity() const;
      /**
       * @brief Creates a unique identity for an edge going from \a source to \a target
       * @arg source Source of the edge
       * @arg target Target of the edge
       * @par: Errors:
       * @li source equals 0
       * @li target equals 0
       */
      static EdgeIdentity getIdentity( Node* source, Node* target );
    private:

      double m_Capacity;
      bool m_Enabled;

      Node* m_Source;
      Node* m_Target;
    };

    /**
     * @brief
     */
    std::ostream& operator<<( std::ostream& os, const Edge& fe );

    double Edge::getCapacity() const
    {
      return m_Capacity;
    }

    void Edge::setCapacity( double  cap )
    {
      m_Capacity = cap;
    }

    Node* Edge::getSource() const
    {
      return m_Source;
    }

    Node* Edge::getTarget() const
    {
      return m_Target;
    }

    bool Edge::isEnabled() const
    {
      return m_Enabled && m_Target->isEnabled();
    }

} //namespace EUROPA

#endif //EDGE_HEADER_FLOW_HEADER_
