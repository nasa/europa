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

#include "SAVH_MaxFlow.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
      MaximumFlowAlgorithm::MaximumFlowAlgorithm( Graph* g, Node* source, Node* sink  ):
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

    void MaximumFlowAlgorithm::print( std::ostream& os ) const
    {
    
    }
  }
}
