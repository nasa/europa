#include "PlanDatabaseListener.hh"
#include "PlanDatabase.hh"

namespace EUROPA {

  PlanDatabaseListener::PlanDatabaseListener(const PlanDatabaseId& planDatabase)
    :m_id(this) {
	  setPlanDatabase(planDatabase);
  }

  PlanDatabaseListener::PlanDatabaseListener()
    :m_id(this) {
  }

  PlanDatabaseListener::~PlanDatabaseListener(){
    check_error(m_id.isValid());
    if(m_planDatabase.isId())
    {
    	check_error(m_planDatabase.isValid());
    	m_planDatabase->notifyRemoved(m_id);
    }
    m_id.remove();
  }

  void PlanDatabaseListener::setPlanDatabase(const PlanDatabaseId& planDatabase) {
	  m_planDatabase = planDatabase;
	  check_error(m_planDatabase.isValid());
	  m_planDatabase->notifyAdded(m_id);
  }


  void PlanDatabaseListener::notifyAdded(const ObjectId& object){}

  void PlanDatabaseListener::notifyRemoved(const ObjectId& object){}

  void PlanDatabaseListener::notifyAdded(const TokenId& token){}

  void PlanDatabaseListener::notifyRemoved(const TokenId& token){}

  void PlanDatabaseListener::notifyActivated(const TokenId& token){}

  void PlanDatabaseListener::notifyDeactivated(const TokenId& token){}

  void PlanDatabaseListener::notifyMerged(const TokenId& token){}

  void PlanDatabaseListener::notifySplit(const TokenId& token){}

  void PlanDatabaseListener::notifyRejected(const TokenId& token){}

  void PlanDatabaseListener::notifyReinstated(const TokenId& token){}

  void PlanDatabaseListener::notifyConstrained(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){}

  void PlanDatabaseListener::notifyFreed(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){}

  void PlanDatabaseListener::notifyAdded(const ObjectId& object, const TokenId& token){}

  void PlanDatabaseListener::notifyRemoved(const ObjectId& object, const TokenId& token){}

  void PlanDatabaseListener::notifyCommitted(const TokenId& token){}

  void PlanDatabaseListener::notifyTerminated(const TokenId& token){}

  const PlanDatabaseListenerId& PlanDatabaseListener::getId() const{return m_id;}
}
