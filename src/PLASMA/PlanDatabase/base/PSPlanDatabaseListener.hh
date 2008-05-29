#ifndef _H_PSPlanDatabaseListener
#define _H_PSPlanDatabaseListener

#include "PlanDatabaseListener.hh"
#include "PSPlanDatabase.hh"

namespace EUROPA {

 	/* Inheritance here is backwards from most PS classes (here PSPlanDatabaseListener inherits from PlanDatabaseListener instead
 	 * of the other way around).  We couldn't come up with a clean way to make this work the other way.
 	 *  
 	 * See notes in PlanDatabaseListener.hh for details on each method.
 	 * 
 	 * Note the ugliness - two signatures involved for each notification (this class basically translates from one
 	 * to the other).
 	 * 
 	 * We were prevented by swig from:
 	 * a) Having a protected constructor
 	 * b) Privately inheriting from PlanDatabaseListener (so all it's methods aren't accessible to any subclasses)
 	 */

  class PSPlanDatabaseListener : public PlanDatabaseListener {
  public:
	  virtual ~PSPlanDatabaseListener() {}
	  
	  virtual void notifyAdded(PSObject* object) {}
	  virtual void notifyRemoved(PSObject* object) {}
	  virtual void notifyAdded(PSToken* token) {}
	  virtual void notifyRemoved(PSToken* token) {}
	  virtual void notifyActivated(PSToken* token) {}
	  virtual void notifyDeactivated(PSToken* token) {}
	  virtual void notifyMerged(PSToken* token) {}
	  virtual void notifySplit(PSToken* token) {}
	  virtual void notifyRejected(PSToken* token) {}
	  virtual void notifyReinstated(PSToken* token) {}
	  virtual void notifyConstrained(PSObject* object, PSToken* predecessor, PSToken* successor) {}
	  virtual void notifyFreed(PSObject* object, PSToken* predecessor, PSToken* successor) {}
	  virtual void notifyAdded(PSObject* object, PSToken* token) {}
	  virtual void notifyRemoved(PSObject* object, PSToken* token) {}
	  virtual void notifyCommitted(PSToken* token) {}
	  virtual void notifyTerminated(PSToken* token) {}
	  
  private:

	  /* These are the methods called by the plan database.  We override those methods here 
	   *  to call the PS interface versions of the same methods above.
	   */
	  virtual void notifyAdded(const ObjectId& object);
	  virtual void notifyRemoved(const ObjectId& object);
	  virtual void notifyAdded(const TokenId& token);
	  virtual void notifyRemoved(const TokenId& token);
	  virtual void notifyActivated(const TokenId& token);
	  virtual void notifyDeactivated(const TokenId& token);
	  virtual void notifyMerged(const TokenId& token);
	  virtual void notifySplit(const TokenId& token);
	  virtual void notifyRejected(const TokenId& token);
	  virtual void notifyReinstated(const TokenId& token);
	  virtual void notifyConstrained(const ObjectId& object, const TokenId& predecessor, const TokenId& successor);
	  virtual void notifyFreed(const ObjectId& object, const TokenId& predecessor, const TokenId& successor);
	  virtual void notifyAdded(const ObjectId& object, const TokenId& token);
	  virtual void notifyRemoved(const ObjectId& object, const TokenId& token);
	  virtual void notifyCommitted(const TokenId& token);
	  virtual void notifyTerminated(const TokenId& token);
  };	  
	  
}

#endif // _H_PSPlanDatabaseListener
