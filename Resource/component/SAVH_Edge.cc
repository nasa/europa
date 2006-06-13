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

#include "float.h"
#include "SAVH_Edge.hh"
#include "SAVH_Node.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
    Edge::Edge( Node* source, Node* target, double capacity, bool enabled ):
      m_Capacity( capacity ),
      m_Enabled( enabled ),
      m_Source( source ),
      m_Target( target )
    {
      checkError( 0 != source, "Null not allowed as input for source" );
      checkError( 0 != target, "Null not allowed as input for target" );
      checkError( capacity <= getMaxCapacity(),"Edge constructor invoked with capacity '" 
		  << capacity << "' larger than maximum capacity '"
		  << getMaxCapacity() << "'");

      m_Source->addOutEdge( this );
      m_Target->addInEdge( this );
    }

    Edge::~Edge()
    {
    }

    double Edge::getMaxCapacity()
    {
      return DBL_MAX;
    }

    EdgeIdentity Edge::getIdentity() const
    {
      return getIdentity( m_Source, m_Target );
    }

    EdgeIdentity Edge::getIdentity( Node* source, Node* target )
    {
      checkError( 0 != source, "Null not allowed as input for source" );
      checkError( 0 != target, "Null not allowed as input for target" );

      return std::make_pair( source->getIdentity(), target->getIdentity() );
    }

    void Edge::setEnabled()
    {
      m_Enabled = true;

      graphDebug("Enabled edge " 
		 << *this );
    }
   
    void Edge::setDisabled()
    {
      m_Enabled = false;

      graphDebug("Disabled edge " 
		 << *this );
    }

    bool Edge::isEnabled() const
    {
      return m_Enabled && m_Target->isEnabled();
    }

    std::ostream& operator<<( std::ostream& os, const Edge& fe )
    {
      os << fe.getIdentity() << " {" << fe.getCapacity() << "} (enabled: " << std::boolalpha << fe.isEnabled() << ")";

      return os;
    }
  }
}
