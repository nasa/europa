#include "DMLogger.hh"
#include "Condition.hh"
#include "DecisionPoint.hh"
#include "DecisionManager.hh"
#include "Choice.hh"

namespace PLASMA {

  DMLogger::DMLogger(std::ostream& os, const DecisionManagerId& dm) : DecisionManagerListener(dm), m_os(os) {}

  DMLogger::~DMLogger() {}

  void DMLogger::notifyConditionAdded(const ConditionId& cond){
    m_os << "DMLogger: Condition Added ";
    cond->print(m_os);
    m_os << std::endl;
  }

  void DMLogger::notifyConditionRemoved(const ConditionId& cond) {
    m_os << "DMLogger: Condition Removed ";
    cond->print(m_os);
    m_os << std::endl;
  }

  void DMLogger::notifyConditionsChanged() {
    m_os << "DMLogger: Conditions Changed " << std::endl;
  }

  void DMLogger::notifyPassed(const EntityId& entity, const ConditionId& cond) {
    m_os << "DMLogger: (" << entity->getKey() << ") Passed ";
    cond->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyFailed(const EntityId& entity, const ConditionId& cond) {
    m_os << "DMLogger: (" << entity->getKey() << ") Failed ";
    cond->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyNewUnitDecision(const DecisionPointId& dec) {
    m_os << "DMLogger: New Unit Decision ";
    dec->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyNewDecision(const DecisionPointId& dec) {
    m_os << "DMLogger: New Decision ";
    dec->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyRemovedDecision(const EntityId& entity) {
    m_os << "DMLogger: Removed Decision corresponding to (" << entity->getKey() << ")" << std::endl;
  }

  void DMLogger::notifyAssignNextStarted(const DecisionPointId& dec) { // previous dec
    if (dec.isNoId())
      m_os << "DMLogger: Deciding to assign next decision with current decision noId" << std::endl;       
    else {
      m_os << "DMLogger: Deciding to assign next decision with current decision ";
      dec->print(m_os);
      m_os << std::endl; 
    }
    m_os << "DMLogger: Open Decisions [" << m_dm->getNumberOfDecisions() << "]:" << std::endl;
    m_dm->printOpenDecisions(m_os);
    m_os << "DMLogger: Closed Decisions [" << m_dm->getClosedDecisions().size() << "]:" << std::endl;
    m_dm->printClosedDecisions(m_os);
  }
  void DMLogger::notifyAssignNextFailed(const DecisionPointId& dec) { // current dec
    if (dec.isNoId())
      m_os << "DMLogger: Failed to assign next decision noId" << std::endl;   
    else {
      m_os << "DMLogger: Failed to assign " << dec->getCurrent() << " next decision (" << dec->getKey() << ") from the following choices ";
      std::list<ChoiceId>::const_iterator it = m_dm->getCurrentDecisionChoices().begin();
      for ( ; it != m_dm->getCurrentDecisionChoices().end(); ++it) {
	(*it)->print(m_os);
      }
      m_os << std::endl;   
    }
  }
  void DMLogger::notifyAssignNextSucceeded(const DecisionPointId& dec) { // current dec
    check_error(dec.isValid());
    m_os << "DMLogger: Succeeded in assigning " << dec->getCurrent() << " next decision (" << dec->getKey() << ") from the following choices "; 
    std::list<ChoiceId>::const_iterator it = m_dm->getCurrentDecisionChoices().begin();
    for ( ; it != m_dm->getCurrentDecisionChoices().end(); ++it)
       (*it)->print(m_os);
    m_os << std::endl;   

  }
  void DMLogger::notifyAssignCurrentStarted(const DecisionPointId& dec) { // current dec
    check_error(dec.isValid());
    m_os << "DMLogger: Deciding to assign current decision ";
    dec->print(m_os);
    m_os << std::endl;   
  }
  void DMLogger::notifyAssignCurrentFailed(const DecisionPointId& dec) { // current dec
    check_error(dec.isValid());
    m_os << "DMLogger: Failed to assign " << dec->getCurrent() << " current decision (" << dec->getKey() << ") from the following choices ";   
    std::list<ChoiceId>::const_iterator it = m_dm->getCurrentDecisionChoices().begin();
    for ( ; it != m_dm->getCurrentDecisionChoices().end(); ++it)
      (*it)->print(m_os);
    m_os << std::endl;   
  }
  void DMLogger::notifyAssignCurrentSucceeded(const DecisionPointId& dec) { // current dec
    check_error(dec.isValid());
    m_os << "DMLogger: Succeeded in assigning " << dec->getCurrent() << " current decision (" << dec->getKey() << ") from the following choices ";   
    std::list<ChoiceId>::const_iterator it = m_dm->getCurrentDecisionChoices().begin();
    for ( ; it != m_dm->getCurrentDecisionChoices().end(); ++it)
      (*it)->print(m_os);
    m_os << std::endl;   
  }
  void DMLogger::notifyRetractStarted(const DecisionPointId& dec) { // current dec
    check_error(dec.isValid());
    m_os << "DMLogger: Deciding to retract current decision ";
    dec->print(m_os);
    m_os << std::endl;   
    m_os << "DMLogger: Closed Decisions [" << m_dm->getClosedDecisions().size() << "]:" << std::endl;
    m_dm->printClosedDecisions(m_os);
  }
  void DMLogger::notifyRetractFailed(const DecisionPointId& dec) {  // current dec
    if (dec.isNoId())
      m_os << "DMLogger: Failed to retract current decision noId" << std::endl;   
    else
      m_os << "DMLogger: Failed to retract current decision (" << dec->getKey() << ")" << std::endl;   
  }
  void DMLogger::notifyRetractSucceeded(const DecisionPointId& dec) {  // current dec
    check_error(dec.isValid());
    m_os << "DMLogger: Succeeded in retracting current decision (" << dec->getKey() << ")" << std::endl;   
  }
  void DMLogger::notifySearchFinished() {  
    m_os << "DMLogger: Search Finished. Plan Complete " << std::endl;   
  }
  void DMLogger::notifyPlannerTimeout() {  
    m_os << "DMLogger: Timeout. Plan Not Complete " << std::endl;   
  }
}
