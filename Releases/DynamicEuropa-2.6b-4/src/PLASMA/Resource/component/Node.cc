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

#include "Edge.hh"
#include "Node.hh"

namespace EUROPA
{
    Node::Node( const NodeIdentity& identity ):
      m_Enabled( true ),
      m_Visit( -1 ),
      m_Identity( identity )
    {
    }

    Node::~Node()
    {
      {
	EdgeList::iterator ite = m_OutEdges.begin();
	EdgeList::iterator end = m_OutEdges.end();

	for( ; ite != end; ++ite )
	  {
	    Edge* edge = (*ite );

	    edge->getTarget()->removeInEdge( edge );

	    delete edge;
	  }
      }

      {
	EdgeList::iterator ite = m_InEdges.begin();
	EdgeList::iterator end = m_InEdges.end();

	for( ; ite != end; ++ite )
	  {
	    Edge* edge = (*ite );

	    edge->getSource()->removeOutEdge( edge );

	    delete edge;
	  }
      }
    }

    void Node::addOutEdge( Edge* edge )
    {
      checkError( 0 != edge, "Null not allowed as input for edge");

      m_OutEdges.push_back( edge );
    }

    void Node::removeOutEdge( Edge* edge )
    {
      checkError( 0 != edge, "Null not allowed as input for edge" );

      EdgeList::iterator ite = std::find( m_OutEdges.begin(), m_OutEdges.end(), edge );

      if( ite != m_OutEdges.end() )
	m_OutEdges.erase( ite );
    }

    void Node::addInEdge( Edge* edge )
    {
      checkError( 0 != edge, "Null not allowed as input for edge");

      m_InEdges.push_back( edge );
    }

    void Node::removeInEdge( Edge* edge )
    {
      checkError( 0 != edge, "Null not allowed as input for edge" );

      EdgeList::iterator ite = std::find( m_InEdges.begin(), m_InEdges.end(), edge );

      if( ite != m_InEdges.end() )
	m_InEdges.erase( ite );
    }


    std::ostream& operator<<( std::ostream& os, const Node& fn )
    {
      os << "[" << fn.getIdentity() << "]";

      return os;
    }
}
