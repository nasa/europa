#ifndef _H_DMResourceListener
#define _H_DMResourceListener

#include "ResourceDefs.hh"
#include "ResourceListener.hh"
#include "ResourceOpenDecisionManager.hh"

// RODM in comments stands for ResourceOpenDecisionManager

namespace Prototype {

  class DMResourceListener : public ResourceListener {
  public:
    DMResourceListener(const ResourceOpenDecisionManagerId& rodm, const ResourceId& resource );
	virtual ~DMResourceListener();

  protected:
	friend class Resource;

	virtual void notifyFlawState( bool hasFlawsNow );

  private:
	// the RODM to notify about changes
	ResourceOpenDecisionManagerId m_rodm;

	// ----------------------------------------------------------------------------
	// Saving information about the previous state of the resource

	// is true iff the resource is flawed
	bool m_flawed;

  };

}

#endif
