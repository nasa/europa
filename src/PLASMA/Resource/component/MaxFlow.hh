#ifndef MAXIMUM_FLOW_ALGORITHM_HEADER_FILE_
#define MAXIMUM_FLOW_ALGORITHM_HEADER_FILE_

/**
 * @file Edge.hh
 * @author David Rijsman
 * @brief Defines the public interface for a maximum flow algorithm
 * @date April 2006
 * @ingroup Resource
 */

#include "Edge.hh"
#include "Graph.hh"
#include "Node.hh"
#include "NodeIterator.hh"
#include "EdgeIterator.hh"

#include <boost/unordered_map.hpp>

#ifndef LONG_MAX
// Would prefer to declare a static const variable, but that would be
// bad style inside a header, so there's another 'ifndef LONG_MAX'
// inside MaximumFlowAlgorithm::reLabel().
# include <limits>
#endif

namespace EUROPA
{
class MaximumFlowAlgorithm
{
private:
  MaximumFlowAlgorithm(const MaximumFlowAlgorithm&);
  MaximumFlowAlgorithm& operator=(const MaximumFlowAlgorithm&);
 public:
  MaximumFlowAlgorithm( Graph* g, Node* source, Node* sink  );
  Graph* getGraph() const { return m_Graph; }
  Node* getSource() const { return m_Source; }
  Node* getSink() const { return m_Sink; }
  inline void execute( bool reset = true );
  void print( std::ostream& os ) const;
  inline edouble getMaxFlow() const;
  inline edouble getFlow( Edge* edge ) const;
  inline void pushFlowBack( Node* node );
  inline edouble getResidual( Edge* edge ) const;
 private:

  eint distanceOnNode(Node* n) const;
  //inline edouble flow(Edge* e) const;
  edouble getExcess(Node* n) const;
  EdgeList::const_iterator currentOutEdgeOnNode(Node* n) const;
  EdgeList::const_iterator endOutEdgeOnNode(Node* n) const;

   inline void disCharge( Node* node );
   inline void initializePre( bool reset = true );
   inline bool isAdmissible( Edge* edge ) const;
   inline void push( Edge* edge );
   inline void reLabel( Node* n );
   inline Node* getNextInList();
   inline void resetToFront();

 #ifdef _MSC_VER
   typedef map< Node*, EdgeList::const_iterator > Node2EdgeListIteratorMap;
 #else
   typedef boost::unordered_map< Node*, EdgeList::const_iterator, NodeHash > Node2EdgeListIteratorMap;
 #endif //_MSC_VER

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

 edouble MaximumFlowAlgorithm::getMaxFlow() const
 {
   if( m_ExcessOnNode.find(  m_Sink ) == m_ExcessOnNode.end() )
     return 0.0;

   return m_ExcessOnNode.find(  m_Sink )->second;
 }

 edouble MaximumFlowAlgorithm::getFlow( Edge* edge  ) const
 {
   Edge2DoubleMap::const_iterator it = m_OnEdge.find(edge);
   checkError(it != m_OnEdge.end(), "Failed to find flow for edge " << *edge);
   return it->second;
 }

 edouble MaximumFlowAlgorithm::getResidual( Edge* edge ) const
 {
   return edge->getCapacity() - getFlow( edge );
 }


 Node* MaximumFlowAlgorithm::getNextInList()
 {
   Node* node = NULL;

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

   while( n != NULL )
   {
     eint oldDistance = distanceOnNode( n );

     disCharge( n );

     if( distanceOnNode( n ) > oldDistance )
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
   graphDebug("Start initializePre " << this);

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
                << *node << " of " << nodes.size());

     if( node->isEnabled() )
     {
       m_CurrentOutEdgeOnNode[ node ] = node->getOutEdges().begin();
       m_EndOutEdgeOnNode[ node ] = node->getOutEdges().end();

       while( m_CurrentOutEdgeOnNode[ node ] != m_EndOutEdgeOnNode[ node ] && 
              !(*m_CurrentOutEdgeOnNode[ node ])->getTarget()->isEnabled() )
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
  m_DistanceOnNode[ m_Source ] = static_cast<long>(m_Nodes.size());

  m_NodeListIterator = m_Nodes.end();

  EdgeOutIterator edgeOutIte( *m_Source );

  for( ; edgeOutIte.ok(); ++edgeOutIte )
  {
    Edge* edge = *edgeOutIte;

    // check edge is enabled

    edouble flow = getFlow(edge);
    edouble residual = edge->getCapacity() - flow;

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

      edouble excess = getExcess(target);

      m_ExcessOnNode[ target ] = excess + residual;

      graphDebug("Initializing flow on edge "
                 << *edge << " to be "
                 << getFlow(edge) << " (reverse flow "
                 << getFlow(m_Graph->getEdge( edge->getTarget(), edge->getSource() )) <<
                 ") and excess on node "
                 << *target << " to be "
                 << getExcess(target) );

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
             << getExcess(node) );

  while( getExcess(node) != 0 )
  {
    Edge* edge = NULL;

    EdgeList::const_iterator ite = currentOutEdgeOnNode(node);

    if( ite != endOutEdgeOnNode(node) )
      edge = (*ite);

    assert( edge != NULL );

    if( isAdmissible( edge ) )
    {
      push( edge );
    }
    else
    {
      reLabel( node );

      ++ite;

      if( ite == endOutEdgeOnNode(node) )
      {
        ite = node->getOutEdges().begin();
      }

      while(ite != endOutEdgeOnNode(node) && !(*ite)->getTarget()->isEnabled())
        ++ite;

      m_CurrentOutEdgeOnNode[node] = ite;
    }
  }

}

bool MaximumFlowAlgorithm::isAdmissible( Edge* edge ) const
{
  Edge2DoubleMap::const_iterator flowIt = m_OnEdge.find(edge);
  Node2Long::const_iterator sourceDistanceIt = 
      m_DistanceOnNode.find(edge->getSource());
  Node2Long::const_iterator targetDistanceIt =
      m_DistanceOnNode.find(edge->getTarget());
  checkError(flowIt != m_OnEdge.end(), 
             "Failed to find flow for " << *edge);
  checkError(sourceDistanceIt != m_DistanceOnNode.end(),
             "Failed to find distance for source node " << *(edge->getSource()));
  checkError(targetDistanceIt != m_DistanceOnNode.end(),
             "Failed to find distance for target node " << *(edge->getTarget()));
  graphDebug("Checking edge "
             << *edge << " if admissable, flow is "
             << flowIt->second << " distance of source is "
             << sourceDistanceIt->second << " distance of target is "
             << targetDistanceIt->second);


  return (flowIt->second < edge->getCapacity()) &&
      (sourceDistanceIt->second == targetDistanceIt->second + 1);
}

void MaximumFlowAlgorithm::pushFlowBack( Node* node )
{
  EdgeInIterator ite( *node );

  for( ; ite.ok(); ++ite )
  {
    Edge* edge = *ite;

    Node* source = edge->getSource();

    edouble flow_pushed_back = getFlow( edge );

    if( flow_pushed_back > 0 && edge->getCapacity() != 0 )
    {
      m_ExcessOnNode[ source ] = getExcess(source) + flow_pushed_back;
      m_OnEdge[ edge ] = 0.0;
      m_OnEdge[ m_Graph->getEdge( edge->getTarget(), edge->getSource() ) ] = 0.0;
    }
  }
}

void MaximumFlowAlgorithm::push( Edge* edge )
{
  Node* source = edge->getSource();
  Node* target = edge->getTarget();

  edouble excess = getExcess(source);
  edouble residual = edge->getCapacity() - getFlow(edge);

  assert( residual >= 0 );

  edouble delta = (excess > residual ) ? residual : excess;

  edouble newFlow = getFlow(edge) + delta;
  m_OnEdge[ edge ] = newFlow;
  m_OnEdge[ m_Graph->getEdge( edge->getTarget(), edge->getSource() ) ] = - newFlow;

  m_ExcessOnNode[ target ] = getExcess(target) + delta;
  m_ExcessOnNode[ source ] = getExcess(source) - delta;

  graphDebug("Pushed flow "
             << delta << " on edge "
             << *edge << " makes excess on node "
             << *source << " "
             << getExcess(source) << " and on "
             << *target << " "
             << getExcess(target) );


}

void MaximumFlowAlgorithm::reLabel( Node* n )
{
  graphDebug("Relabel node "
             << *n );

  eint minLabel = std::numeric_limits<eint>::max();

  EdgeOutIterator edgeOutIte( *n );

  for( ; edgeOutIte.ok(); ++edgeOutIte )
  {
    Edge* edge = *edgeOutIte;

    Node* target = edge->getTarget();

    graphDebug("Relabel node "
               << *n << " checking edge "
               << *edge << " to relabel, flow on edge is "
               << getFlow(edge) );
    //m_OnEdge[ edge ] < edge->getCapacity()
    if( getResidual( edge ) > 0  )
    {
      eint label = distanceOnNode( target );

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
             << distanceOnNode( n ) );
}
}

#endif //MAXIMUM_FLOW_ALGORITHM_HEADER_FILE_
