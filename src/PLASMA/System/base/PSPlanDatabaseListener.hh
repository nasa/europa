#ifndef _H_PSPlanDatabaseListener
#define _H_PSPlanDatabaseListener

#include "PlanDatabaseListener.hh"

namespace EUROPA {

 	/* Inheritance here is backwards from most PS classes (here PSPlanDatabaseListener inherits from PlanDatabaseListener instead
 	 * of the other way around).  We couldn't come up with a clean way to make this work the other way.
 	 *  
 	 * We were prevented by swig from:
 	 * a) Having a protected constructor
 	 * b) Privately inheriting from PlanDatabaseListener (so all it's methods aren't accessible to any subclasses)
 	 */

  class PSPlanDatabaseListener : public PlanDatabaseListener {
  public:
	  PSPlanDatabaseListener();
	  virtual ~PSPlanDatabaseListener() {}
	  
	  virtual void notifyAdded(PSObject* obj) {std::cout << "WRONG ONE " << std::endl;}
	  
  private:
	  
	  virtual void notifyAdded(const ObjectId& object);
  };	  
	  
}

#endif // _H_PSPlanDatabaseListener
