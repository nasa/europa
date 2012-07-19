#include "ObjectTokenRelation.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include "Domains.hh"

#include "Debug.hh"
#include <iostream>
/**
 * @file ObjectTokenRelation.cc
 * @brief Provides the implementation for ObjectTokenRelation.
 */
namespace EUROPA {

  ObjectTokenRelation::ObjectTokenRelation(const LabelStr& name,
					   const LabelStr& propagatorName,
					   const ConstraintEngineId& constraintEngine,
					   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_token(variables[STATE_VAR]->parent()),
      m_currentDomain(static_cast<ObjectDomain&>(getCurrentDomain(variables[OBJECT_VAR]))){
    check_error(m_token.isValid());
    check_error(variables[OBJECT_VAR]->parent() == m_token);
    checkError(variables[OBJECT_VAR]->isClosed(),
	       "The Object Variable must be closed to correctly maintain the object token relationship through propagation.");
  }

  ObjectTokenRelation::~ObjectTokenRelation(){
    discard(false);
  }

  void ObjectTokenRelation::handleDiscard(){
    if(!Entity::isPurging()){
      check_error(m_token.isValid());
      notifyRemovals();
    }

    Constraint::handleDiscard();
  }

  void ObjectTokenRelation::handleExecute(){
    check_error(m_token.isValid());
    check_error(m_currentDomain.isOpen() || !m_currentDomain.isEmpty());

    // If it is not active, we can just do nothing, since it affects nothing (relaxations are handled when processing ignore).
    if(!m_token->isActive() || m_currentDomain.isOpen())
      return;

    if(m_notifiedObjects.empty()) { // Then we must notify
      notifyAdditions();
    }
    else { // It must have been active before, in which case we are processing restrictions
      notifyRemovals();
    }

    check_error(isValid());
  }

  void ObjectTokenRelation::handleExecute(const ConstrainedVariableId& variable,
					  int argIndex,
					  const DomainListener::ChangeType& changeType){
    handleExecute();
  }

  /**
   * Will handle changes immediately as long as the domain is open
   */
  bool ObjectTokenRelation::canIgnore(const ConstrainedVariableId& variable,
				      int argIndex,
				      const DomainListener::ChangeType& changeType){

    if(m_currentDomain.isOpen())
      return true;
    debugMsg("ObjectTokenRelation:canIgnore", m_token->toString() << " Received notification of change type " << changeType << " on variable " <<
             variable->toString());
    if(changeType == DomainListener::RESET || changeType == DomainListener::RELAXED){
      debugMsg("ObjectTokenRelation::canIgnore", "Evaluating relaxation event on " << variable->toLongString());
      if (m_token->isActive()){ // Still active so must have been object variable relaxed. Notify Additions.
	// Otherwise, handle possible notifications for new values in object domain.
	debugMsg("ObjectTokenRelation::canIgnore", "Handling addition");
	notifyAdditions();
      }
      else { // It is no longer active but we have outstanding objects to be notified of removal
	debugMsg("ObjectTokenRelation::canIgnore", "Handling removal");
	notifyRemovals();
      }
      check_error(isValid());
    }
    else if (m_token->isActive()){ // It is a restriction so handle it straight away
      debugMsg("ObjectTokenRelation::canIgnore", "Processing activation event on " << m_token->toString());
      handleExecute();
    }

    return true;
  }

  bool ObjectTokenRelation::isValid() const{
    return((!m_token->isActive() && m_notifiedObjects.empty()) ||
	   (m_token->isActive() && !m_notifiedObjects.empty()));
  }

  /**
   * All members of the current domain that are not members of the current set of notified objects should
   * be notifed of addition of a token
   */
  void ObjectTokenRelation::notifyAdditions() {
    std::list<double> values;
    m_currentDomain.getValues(values);
    check_error(!values.empty());

    for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it){
      ObjectId object = *it;
      check_error(object.isValid());
      if(m_notifiedObjects.find(object) == m_notifiedObjects.end()){
	m_notifiedObjects.insert(object);
        debugMsg("ObjectTokenRelation:notifyAdditions", "Adding " << m_token->toString() << " to " << object->toString());
	object->add(m_token);
      }
    }
  }

  /**
   * If it is not active, then notify all objects of removal and clear the set of stored notifications.
   * Otherwise, process the difference between the current domain, and prior notified objects
   */
  void ObjectTokenRelation::notifyRemovals() {
    static unsigned int sl_counter(0);
    sl_counter++;

    checkError(getId().isValid(), getId());

    // Remove token from objects where the domain has been restricted, and was previously notifed,
    // or where the Token is now inactive
    bool isActive = m_token->isActive();

    unsigned int startIndex = 0;

    debugMsg("ConstraintEngine", "[" << getKey() << "]");

    while (m_notifiedObjects.size() > startIndex) {
      std::set<ObjectId>::iterator it = m_notifiedObjects.begin();
      for (unsigned int i = 0; i < startIndex; i++) {
	it++;
      }

      ObjectId object = *it;

      if(!isActive || !m_currentDomain.isMember(object)){
        debugMsg("ObjectTokenRelation:notifyRemovals", "Removing " << m_token->toString() << " from " << object->toString());
	object->remove(m_token);
	m_notifiedObjects.erase(object);
      } else {
	startIndex++;
      }
      
    }
    

    /*while(it!=m_notifiedObjects.end()){
      ObjectId object = *it;
      if(!isActive || !m_currentDomain.isMember(object)){
	object->remove(m_token);
	m_notifiedObjects.erase(it++);
      }
      else
	++it;
	}*/
  }
}
