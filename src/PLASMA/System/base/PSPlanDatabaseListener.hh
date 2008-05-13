#ifndef _H_PSPlanDatabaseListener
#define _H_PSPlanDatabaseListener

#include "PlanDatabaseListener.hh"
#include "PSEngine.hh"

namespace EUROPA {

 	/* Private inheritance from PlanDatabaseListener so that:
 	 *  a) We receive the usual notifications, but 
 	 *  b) Any class that inherits from this one only cares/knows about the PS versions of methods
 	 * 
 	 *  TODO:  Figure out how to have a protected constructor (swig issue)
 	 *  TODO:  Figure out how privately inherit from PlanDatabaseListener (so all it's methods aren't accessible to any subclasses)
 	 *  TODO:  Reverse the inheritance once the other classes are prepared for it (see #)
 	 */

  class PSPlanDatabaseListener : public PlanDatabaseListener {
  public:
	  virtual ~PSPlanDatabaseListener() {}
	  
	  virtual void notifyAdded(PSObject* obj) {}
	  
//  protected:	(alas, SWIG won't wrap a protected constructor) 
	  PSPlanDatabaseListener(PSEngine* engine);
	  
  private:
	  
	  virtual void notifyAdded(const ObjectId& object);
	  
	  PSEngine& m_psengine;
  };	  
	  
}

#endif // _H_PSPlanDatabaseListener
