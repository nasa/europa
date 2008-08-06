#ifndef _H_PSPlanDatabaseListener
#define _H_PSPlanDatabaseListener

#include "PlanDatabaseListener.hh"
#include "PSPlanDatabase.hh"

namespace EUROPA {

 	/*
 	 * NOTES:
 	 * - We implement a subset of the PlanDatabaseListener notifications that are
 	 *   likely to be useful to a user (the rest are hidden by making them private here)
 	 * - Inheritance here is backwards from most PS classes (here PSPlanDatabaseListener
 	 *   inherits from PlanDatabaseListener instead of the other way around).  We couldn't
 	 *   come up with a clean way to make this work the other way.
 	 * - See notes in PlanDatabaseListener.hh for details on each method.
 	 * - For the public methods, note the ugliness - two signatures involved for each notification
 	 *   (this class basically translates from one to the other).
 	 * - We were prevented by swig from:
 	 *   a) Having a protected constructor
 	 *   b) Privately inheriting from PlanDatabaseListener (so all it's methods aren't accessible to any subclasses)
 	 *
 	 *  WARNING:  For some reason, I was getting weird static_cast compile errors until I included
 	 *  Constraint.hh and ConstrainedVariable.hh above (forewarned is forearmed!)
 	 */

  class PSPlanDatabaseListener : public PlanDatabaseListener {
  public:
	  virtual ~PSPlanDatabaseListener() {}

	  /* The subset of notifications available through PSEngine interface */
	  virtual void notifyAdded(PSObject* object) {}
	  virtual void notifyRemoved(PSObject* object) {}
	  virtual void notifyActivated(PSToken* token) {}
	  virtual void notifyDeactivated(PSToken* token) {}
	  virtual void notifyRejected(PSToken* token) {}
	  virtual void notifyMerged(PSToken* token) {}
	  virtual void notifySplit(PSToken* token) {}
	  virtual void notifyAdded(PSObject* object, PSToken* token) {}
	  virtual void notifyRemoved(PSObject* object, PSToken* token) {}

  private:

	  /* We override these base class methods (called by PlanDatabase)
	   * to call the above PS interface versions of the same methods.
	   */
	  virtual void notifyAdded(const ObjectId& object);
	  virtual void notifyRemoved(const ObjectId& object);
	  virtual void notifyActivated(const TokenId& token);
	  virtual void notifyDeactivated(const TokenId& token);
	  virtual void notifyRejected(const TokenId& token);
	  virtual void notifyMerged(const TokenId& token);
	  virtual void notifySplit(const TokenId& token);
	  virtual void notifyAdded(const ObjectId& object, const TokenId& token);
	  virtual void notifyRemoved(const ObjectId& object, const TokenId& token);

	  /* These methods are likely unnecessary to a user.  We override the base
	   * class version only to make private (they still don't do anything)
	   */
	  virtual void notifyAdded(const TokenId& token) {}
	  virtual void notifyRemoved(const TokenId& token) {}
	  virtual void notifyReinstated(const TokenId& token) {}
	  virtual void notifyConstrained(const ObjectId& object, const TokenId& predecessor, const TokenId& successor) {}
	  virtual void notifyFreed(const ObjectId& object, const TokenId& predecessor, const TokenId& successor) {}
	  virtual void notifyCommitted(const TokenId& token) {}
	  virtual void notifyTerminated(const TokenId& token) {}
  };

}

#endif // _H_PSPlanDatabaseListener
