#include "ResourceDefs.hh"
#include "DMResourceListener.hh"
#include "Resource.hh"
#include "ResourceOpenDecisionManager.hh"

#define NOISE 1

namespace EUROPA {

  DMResourceListener::DMResourceListener( const ResourceOpenDecisionManagerId& rodm,
										  const ResourceId& resource ) :
    ResourceListener(resource), m_rodm(rodm) {}

  DMResourceListener::~DMResourceListener() {}


  //===================================================================================
  void DMResourceListener::notifyFlawState( bool new_flawed ) {
#if (NOISE > 5 ) 
	std::cout << "RL: State of resource "<<m_resource<<" is still "<<(new_flawed?"":"NOT ")<<
	  "FLAWED"<<std::endl;
#endif
	m_flawed = new_flawed;	

	// Tell RODM about the current status, change or not
	m_rodm->notifyResourceState( m_id, new_flawed );
  }

}
