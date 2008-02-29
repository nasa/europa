#ifndef MAXIMUM_FLOW_ALGORITHM_HEADER_FILE_
#define MAXIMUM_FLOW_ALGORITHM_HEADER_FILE_

/**
 * @file SAVH_Edge.hh
 * @author David Rijsman
 * @brief Defines the public interface for a maximum flow algorithm
 * @date April 2006
 * @ingroup Resource
 */

#include "SAVH_Edge.hh"
#include "SAVH_Graph.hh"
#include "SAVH_Node.hh"
#include "SAVH_NodeIterator.hh"
#include "SAVH_EdgeIterator.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
    /**
     * @brief
     */
    class MaximumFlowAlgorithm
    {
      public:
        /**
         * @brief
         */
        MaximumFlowAlgorithm( Graph* g, Node* source, Node* sink  );
        /**
         * @brief
         */
        Graph* getGraph() const { return m_Graph; }
        /**
         * @brief
         */
        Node* getSource() const { return m_Source; }
        /**
         * @brief
         */
        Node* getSink() const { return m_Sink; }
        /**
         * @brief
         */
        inline void execute( bool reset = true );
        /**
         * @brief
         */
        void print( std::ostream& os ) const;
        /**
         * @brief
         */
        inline double getMaxFlow() const;
        /**
         * @brief
         */
        inline double getFlow( Edge* edge ) const;
        /**
         * @brief
         */
        inline void pushFlowBack( Node* node );
        /**
         * @brief
         */
        inline double getResidual( Edge* edge ) const;
      private:
        /**
         * @brief
         */
        inline void disCharge( Node* node );
        /**
         * @brief
         */
        inline void initializePre( bool reset = true );
        /**
         * @brief
         */
        inline bool isAdmissible( Edge* edge );
        /**
         * @brief
         */
        inline void push( Edge* edge );
        /**
         * @brief
         */
        inline void reLabel( Node* n );
        /**
         * @brief
         */
        inline Node* getNextInList();
        /**
         * @brief
         */
        inline void resetToFront();

        typedef hash_map< Node*, EdgeList::const_iterator, NodeHash > Node2EdgeListIteratorMap;

        Node2EdgeListIteratorMap m_CurrentOutEdgeOnNode;
        Node2EdgeListIteratorMap m_EndOutEdgeOnNode;

        Node2Double m_ExcessOnNode;
        Node2Long m_DistanceOnNode;
        Edge2DoubleMap m_OnEdge;

        NodeList m_Nodes;
        Graph* m_Graph;
        Node* m_Source;
        Node* m_Sink;

        NodeList::iterator m_NodeListIterator;
    };

    double MaximumFlowAlgorithm::getMaxFlow() const
    {
      if( m_ExcessOnNode.find(  m_Sink ) == m_ExcessOnNode.end() )
        return 0.0;

      return m_ExcessOnNode.find(  m_Sink )->second;
    }

    double MaximumFlowAlgorithm::getFlow( Edge* edge  ) const
    {
      return m_OnEdge.find( edge )->second;
    }

    double MaximumFlowAlgorithm::getResidual( Edge* edge ) const
    {
      return edge->getCapacity() - getFlow( edge );
    }


    Node* MaximumFlowAlgorithm::getNextInList()
    {
      Node* node = 0;

      if( m_NodeListIterator == m_Nodes.end() )
      {
        m_NodeListIterator = m_Nodes.begin();
      }
      else
      {
        ++m_NodeListIterator;
      }

      NodeList::iterator end = m_Nodes.end();

      while( m_NodeListIterator != end &&  
          !(*m_NodeListIterator)->isEnabled() )
      {
        ++m_NodeListIterator;
      }

      if( m_NodeListIterator != end )
        node = *m_NodeListIterator;

      return node;
    }

    void MaximumFlowAlgorithm::resetToFront()
    {
      if( m_NodeListIterator != m_Nodes.end() )
      {
        Node* n = (*m_NodeListIterator);
        m_Nodes.erase( m_NodeListIterator );
        m_Nodes.push_front( n );

        m_NodeListIterator = m_Nodes.end();
      }
    }


    void MaximumFlowAlgorithm::execute( bool reset )
    {
      graphDebug("Start execute, reset is " << std::boolalpha << reset );

      initializePre( reset );

      Node* n = getNextInList();

      while( n != 0 )
      {
        long oldDistance = m_DistanceOnNode[ n ];

        disCharge( n );

        if( m_DistanceOnNode[ n ] > oldDistance )
        {
          resetToFront();
        }

        n = getNextInList();
      }

      graphDebug("End execute, max flow: " 
          << getMaxFlow() );
    }

    void MaximumFlowAlgorithm::initializePre( bool reset )
    {
      graphDebug("Start initializePre");

      checkError( m_Source->isEnabled(),"Source '" << *m_Source << "' is not enabled.");
      checkError( m_Sink->isEnabled(),"Sink '" << *m_Sink << "' is not enabled." );

      if( reset )
      {
        m_CurrentOutEdgeOnNode.clear();
        m_EndOutEdgeOnNode.clear();
        m_DistanceOnNode.clear();
        m_ExcessOnNode.clear();

        m_OnEdge.clear();

        m_Nodes.clear();
      }

      const NodeIdentity2Node& nodes = m_Graph->getNodes();

      NodeIdentity2Node::const_iterator nIte = nodes.begin();
      NodeIdentity2Node::const_iterator nEnd = nodes.end();

      for( ; nIte != nEnd; ++nIte )
      {
        Node* node = (*nIte).second;

        graphDebug("Initializing node " 
            << *node );

        if( node->isEnabled() )
        {
          m_CurrentOutEdgeOnNode[ node ] = node->getOutEdges().begin();
          m_EndOutEdgeOnNode[ node ] = node->getOutEdges().end();

          while( m_CurrentOutEdgeOnNode[ node ] != m_EndOutEdgeOnNode[ node ] && !(*m_CurrentOutEdgeOnNode[ node ])->getTarget()->isEnabled() )
            ++m_CurrentOutEdgeOnNode[ node ];

          if( reset )
          {
            if( node != m_Source && node != m_Sink )
              m_Nodes.push_back( node );

            graphDebug("Setting distance and excess for node " 
                << *node << " to 1 and 0.0");

            m_DistanceOnNode[ node ] = 1;
            m_ExcessOnNode[ node ] = 0.0;

            const EdgeList& outEdges = node->getOutEdges();

            EdgeList::const_iterator fIte = outEdges.begin();
            EdgeList::const_iterator fEnd = outEdges.end();

            for( ; fIte != fEnd; ++fIte )
            {
              Edge* edge = *fIte;

              if( edge->isEnabled() )
              {
                graphDebug("Initializing flow on edge " 
                    << *edge << " to be 0");

                m_OnEdge[ edge ] = 0.0;

                checkError( 0 !=  m_Graph->getEdge( edge->getTarget(), edge->getSource() )
                    &&
                    m_Graph->getEdge( edge->getTarget(), edge->getSource() )->isEnabled(),
                    "No (enabled) reverse edge for edge '" << *edge << "'");

                m_OnEdge[ m_Graph->getEdge( edge->getTarget(), edge->getSource() ) ] = 0.0;
              }
            }
          }
        }
      }

      m_DistanceOnNode[ m_Sink ] = 0;
      m_DistanceOnNode[ m_Source ] = (long) m_Nodes.size();

      m_NodeListIterator = m_Nodes.end();

      EdgeOutIterator edgeOutIte( *m_Source );

      for( ; edgeOutIte.ok(); ++edgeOutIte )
      {
        Edge* edge = *edgeOutIte;

        // check edge is enabled

        double flow = m_OnEdge[ edge ];
        double residual = edge->getCapacity() - flow;

        graphDebug("Initializing flow from source "
            << *m_Source << " for edge " 
            << *edge << ", flow is "
            << flow << " residual is "
            << residual );

        if( residual != 0 )
        {
          m_OnEdge[ edge ] = flow + residual;
          m_OnEdge[ m_Graph->getEdge( edge->getTarget(), edge->getSource() ) ] = - (flow + residual);


          Node* target = edge->getTarget();

          double excess = m_ExcessOnNode[ target ];

          m_ExcessOnNode[ target ] = excess + residual;

          graphDebug("Initializing flow on edge " 
              << *edge << " to be " 
              << m_OnEdge[ edge ] << " (reverse flow "
              << m_OnEdge[ m_Graph->getEdge( edge->getTarget(), edge->getSource() ) ] << ") and excess on node "
              << *target << " to be " 
              << m_ExcessOnNode[ target ] );

        }
      }

      graphDebug("End initializePre");
    }

    void MaximumFlowAlgorithm::disCharge( Node* node )
    {
      checkError( 0 != node, "Node is null, null is not allowed as input");
      checkError( node->isEnabled(), "Node '" << *node << "' not enabled.");

      graphDebug("Discharge invoked for node " 
          << *node << " witch excess " 
          << m_ExcessOnNode[ node ] );

      while( m_ExcessOnNode[ node ] != 0 )
      {
        Edge* edge = 0;

        EdgeList::const_iterator ite = m_CurrentOutEdgeOnNode[ node ];

        if( ite != m_EndOutEdgeOnNode[ node ] )
          edge = (*ite);

        assert( edge != 0 );

        if( isAdmissible( edge ) )
        {
          push( edge );
        }
        else
        {
          reLabel( node );

          ++ite;

          if( ite == m_EndOutEdgeOnNode[ node ] )
          {
            ite = node->getOutEdges().begin();
          }

          m_CurrentOutEdgeOnNode[node] = ite;
        }
      }

    }

    bool MaximumFlowAlgorithm::isAdmissible( Edge* edge )
    {
      graphDebug("Checking edge "
          << *edge << " if admissable, flow is "
          << m_OnEdge[ edge ] << " distance of source is "
          << m_DistanceOnNode[ edge->getSource() ] << " distance of target is " 
          << m_DistanceOnNode[ edge->getTarget() ] );


      return ( m_OnEdge[ edge ] < edge->getCapacity() )
        &&  
        ( m_DistanceOnNode[ edge->getSource() ] ==  m_DistanceOnNode[ edge->getTarget() ] + 1 );
    }

    void MaximumFlowAlgorithm::pushFlowBack( Node* node )
    {
      EdgeInIterator ite( *node );

      for( ; ite.ok(); ++ite )
      {
        Edge* edge = *ite;

        Node* source = edge->getSource();

        double flow_pushed_back = getFlow( edge );

        if( flow_pushed_back > 0 && edge->getCapacity() != 0 )
        {
          m_ExcessOnNode[ source ] = m_ExcessOnNode[ source ] + flow_pushed_back;
          m_OnEdge[ edge ] = 0.0;
          m_OnEdge[ m_Graph->getEdge( edge->getTarget(), edge->getSource() ) ] = 0.0;
        }
      }
    }

    void MaximumFlowAlgorithm::push( Edge* edge )
    {
      Node* source = edge->getSource();
      Node* target = edge->getTarget();

      double excess = m_ExcessOnNode[ source ];
      double residual = edge->getCapacity() - m_OnEdge[ edge ];

      assert( residual >= 0 );

      double delta = (excess > residual ) ? residual : excess;

      double newFlow = m_OnEdge[ edge ] + delta;
      m_OnEdge[ edge ] = newFlow;
      m_OnEdge[ m_Graph->getEdge( edge->getTarget(), edge->getSource() ) ] = - newFlow;

      m_ExcessOnNode[ target ] = m_ExcessOnNode[ target ] + delta;
      m_ExcessOnNode[ source ] = m_ExcessOnNode[ source ] - delta;

      graphDebug("Pushed flow "
          << delta << " on edge "
          << *edge << " makes excess on node "
          << *source << " " 
          << m_ExcessOnNode[ source ] << " and on "
          << *target << " "
          << m_ExcessOnNode[ target ] );


    }

    void MaximumFlowAlgorithm::reLabel( Node* n )
    {
      graphDebug("Relabel node "
          << *n );

      long minLabel = LONG_MAX;

      EdgeOutIterator edgeOutIte( *n );

      for( ; edgeOutIte.ok(); ++edgeOutIte )
      {
        Edge* edge = *edgeOutIte;

        Node* target = edge->getTarget();

        graphDebug("Relabel node "
                   << *n << " checking edge "
                   << *edge << " to relabel, flow on edge is "
                   << m_OnEdge[ edge ] );
        //m_OnEdge[ edge ] < edge->getCapacity()
        if( getResidual( edge ) > 0  )
        {
          long label = m_DistanceOnNode[ target ];

          if( minLabel >= label )
          {
            graphDebug("Node " 
                << *target << " is labeled with label "
                << label );

            minLabel = label;
          }
        }
      }

      //at this point all distance-labels for the connected nodes are smaller or equal to node n
      m_DistanceOnNode[ n ] = minLabel + 1;

      graphDebug("(Re)labeled node "
          << *n << " to have distance "
          << m_DistanceOnNode[ n ] );
    }
  }
}

#endif //MAXIMUM_FLOW_ALGORITHM_HEADER_FILE_
