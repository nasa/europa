#include "PlanDatabaseListener.hh"
#include "PlanDatabase.hh"

namespace EUROPA {

PlanDatabaseListener::PlanDatabaseListener(const PlanDatabaseId planDatabase)
    :m_id(this), m_planDatabase() {
  setPlanDatabase(planDatabase);
}

PlanDatabaseListener::PlanDatabaseListener()
    :m_id(this), m_planDatabase() {
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

  void PlanDatabaseListener::setPlanDatabase(const PlanDatabaseId planDatabase) {
	  m_planDatabase = planDatabase;
	  check_error(m_planDatabase.isValid());
	  m_planDatabase->notifyAdded(m_id);
  }


  void PlanDatabaseListener::notifyAdded(const ObjectId){}

  void PlanDatabaseListener::notifyRemoved(const ObjectId){}

  void PlanDatabaseListener::notifyAdded(const TokenId){}

  void PlanDatabaseListener::notifyRemoved(const TokenId){}

  void PlanDatabaseListener::notifyActivated(const TokenId){}

  void PlanDatabaseListener::notifyDeactivated(const TokenId){}

  void PlanDatabaseListener::notifyMerged(const TokenId){}

  void PlanDatabaseListener::notifySplit(const TokenId){}

  void PlanDatabaseListener::notifyRejected(const TokenId){}

  void PlanDatabaseListener::notifyReinstated(const TokenId){}

  void PlanDatabaseListener::notifyConstrained(const ObjectId, const TokenId, const TokenId ){}

  void PlanDatabaseListener::notifyFreed(const ObjectId, const TokenId, const TokenId){}

  void PlanDatabaseListener::notifyAdded(const ObjectId, const TokenId){}

  void PlanDatabaseListener::notifyRemoved(const ObjectId, const TokenId){}

  void PlanDatabaseListener::notifyCommitted(const TokenId){}

  void PlanDatabaseListener::notifyTerminated(const TokenId){}

  const PlanDatabaseListenerId PlanDatabaseListener::getId() const{return m_id;}
}
