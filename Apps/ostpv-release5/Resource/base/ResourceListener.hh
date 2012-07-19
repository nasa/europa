#ifndef _H_ResourceListener
#define _H_ResourceListener

#include "ResourceDefs.hh"
#include "Resource.hh"

namespace EUROPA {

  class ResourceListener {
  public:
    ResourceListener(const ResourceId& resource ) : 
	  m_id(this), m_resource(resource) {
	  m_resource->add(m_id);
	}

	virtual ~ResourceListener() {	
	  check_error(m_id.isValid());
	  m_id.remove();
	}

	const ResourceId& getResource() const { return m_resource; }
	const ResourceListenerId& getId() const { return m_id; }

  protected:
	friend class Resource;

	// This should eventually take a more informed double argument
	// instead of bool
	virtual void notifyFlawState( bool hasFlawsNow ) = 0;

	// My own ID
	ResourceListenerId m_id;

	// The one and only resource we listen to
	const ResourceId m_resource;
  };

}

#endif
