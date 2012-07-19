#ifndef _H_DbClientListener
#define _H_DbClientListener

#include "PlanDatabaseDefs.hh"
#include "DbClient.hh"
#include <vector>

/**
 * @file Provides the listener interface for client transactions on the PlanDatabase.
 * @author Conor McGann, March, 2004.
 */
namespace EUROPA {

  /**
   * @brief Listener definition. Similar to PlanDatabaseListener but is only concerned with
   * external operations rather than internal database events. Furthermore, it includes events for
   * constraint creation and variable specification.
   * @see PlanDatabaseListener
   */
  class DbClientListener {
  public:
    virtual ~DbClientListener(){
      m_client->notifyRemoved(m_id);
      m_id.remove();
    }

    const DbClientListenerId& getId() const {return m_id;}

    virtual void notifyObjectCreated(const ObjectId& object) {}
    virtual void notifyObjectCreated(const ObjectId& object, const std::vector<const AbstractDomain*>& arguments){}
    virtual void notifyObjectDeleted(const ObjectId& object) {}
    virtual void notifyClosed(){}
    virtual void notifyClosed(const LabelStr& objectType){}
    virtual void notifyTokenCreated(const TokenId& token){}
    virtual void notifyTokenDeleted(const TokenId& token, const std::string& name){}
    virtual void notifyConstrained(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){}
    virtual void notifyFreed(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){}
    virtual void notifyActivated(const TokenId& token){}
    virtual void notifyMerged(const TokenId& token, const TokenId& activeToken){}
    virtual void notifyMerged(const TokenId& token){}
    virtual void notifyRejected(const TokenId& token){}
    virtual void notifyCancelled(const TokenId& token){}
    virtual void notifyConstraintCreated(const ConstraintId& constraint){}
    virtual void notifyConstraintDeleted(const ConstraintId& constraint){}
    virtual void notifyVariableCreated(const ConstrainedVariableId& variable){}
    virtual void notifyVariableDeleted(const ConstrainedVariableId& variable){}
    virtual void notifyVariableSpecified(const ConstrainedVariableId& variable){}
    virtual void notifyVariableRestricted(const ConstrainedVariableId& variable){}
    virtual void notifyVariableClosed(const ConstrainedVariableId& variable){}
    virtual void notifyVariableReset(const ConstrainedVariableId& variable){}

  private:
    DbClientListenerId m_id;

  protected:
    DbClientListener(const DbClientId& client):m_id(this), m_client(client){m_client->notifyAdded(m_id);}
    DbClientId m_client;  };
}
#endif
