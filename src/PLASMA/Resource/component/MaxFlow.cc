//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software

#include "MaxFlow.hh"

namespace EUROPA
{
MaximumFlowAlgorithm::MaximumFlowAlgorithm( Graph* g, Node* source, Node* sink  ):
    m_CurrentOutEdgeOnNode(),
    m_EndOutEdgeOnNode(),
    m_ExcessOnNode(),
    m_DistanceOnNode(),
    m_OnEdge(),
    m_Nodes(),
    m_Graph( g ),
    m_Source( source ),
    m_Sink( sink ),
    m_NodeListIterator( m_Nodes.end() )
{
  checkError( g != 0, "Null not allowed as input for g" );
  checkError( source != 0, "Null not allowed as input for source" );
  checkError( sink != 0, "Null not allowed as input for sink" );
  checkError( g->getNode( source->getIdentity() ) != 0, "Source is not part of the graph" );
  checkError( g->getNode( sink->getIdentity() ) != 0, "Sink is not part of the graph");

  graphDebug("Maximum flow instance created with source "
             << *source << " and sink "
             << *sink );
}

void MaximumFlowAlgorithm::print( std::ostream& ) const
{

}

eint MaximumFlowAlgorithm::distanceOnNode(Node* n) const {
  Node2Long::const_iterator it = m_DistanceOnNode.find(n);
  checkError(it != m_DistanceOnNode.end(), "Failed to find distance for " << *n);
  return it->second;
}

//edouble MaximumFlowAlgorithm::flow(Edge* e) const {
//}

edouble MaximumFlowAlgorithm::getExcess(Node* n) const {
  Node2Double::const_iterator it = m_ExcessOnNode.find(n);
  checkError(it != m_ExcessOnNode.end(), "Failed to find excess for " << *n);
  return it->second;
}

EdgeList::const_iterator MaximumFlowAlgorithm::currentOutEdgeOnNode(Node* n) const {
  Node2EdgeListIteratorMap::const_iterator it = m_CurrentOutEdgeOnNode.find(n);
  checkError(it != m_CurrentOutEdgeOnNode.end(), 
             "Failed to find a current out-edge iterator for " << *n);
  return it->second;
}

EdgeList::const_iterator MaximumFlowAlgorithm::endOutEdgeOnNode(Node* n) const {
  Node2EdgeListIteratorMap::const_iterator it = m_EndOutEdgeOnNode.find(n);
  checkError(it != m_EndOutEdgeOnNode.end(), 
             "Failed to find an end out-edge iterator for " << *n);
  return it->second;
}

}
