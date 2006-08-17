#include "Utils.hh"
#include "Debug.hh"
#include "ConstrainedVariable.hh"
#include "DecisionManager.hh"
#include "OpenDecisionManager.hh"
#include "HorizonCondition.hh"
#include "TokenDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include "Condition.hh"
#include "Debug.hh"

#include <string>
#include <sstream>

namespace EUROPA {


#define  publish(message){\
    check_error(!Entity::isPurging());\
    for(std::set<DecisionManagerListenerId>::const_iterator lit = m_listeners.begin(); lit != m_listeners.end(); ++lit) {\
      check_error((*lit).isValid());\
      (*lit)->message;\
    }\
  }

  std::string makeStr(const std::list<DecisionPointId>& decs){
    std::stringstream sstr;
    for (std::list<DecisionPointId>::const_iterator it = decs.begin(); it != decs.end(); ++it){
      DecisionPointId dec = *it;
      sstr << dec;
    }
    return sstr.str();
  }


  DecisionManager::DecisionManager(const PlanDatabaseId& db, const OpenDecisionManagerId& odm) 
    : m_id(this), m_db(db), m_odm(odm) {
    m_curDec = DecisionPointId::noId();
    m_retracting = false;
  }

  DecisionManager::~DecisionManager() {
    reset();
    // Clean up listeners
    cleanup(m_listeners);
    check_error(m_id.isValid());
    m_id.remove();
  }

  const DecisionManagerId& DecisionManager::getId() const {return m_id;}

  const PlanDatabaseId& DecisionManager::getPlanDatabase() const {return m_db;}

  const OpenDecisionManagerId& DecisionManager::getOpenDecisionManager() const {
    checkError(m_odm.isValid(), "Must be set in advance.");
    return m_odm;
  }

  const DecisionStack& DecisionManager::getClosedDecisions() const {
    return m_closedDecisions;
  }

  void DecisionManager::retract(){
    checkError(m_curDec.isNoId(), "Only valid for a completed search.");
    while(!getClosedDecisions().empty()){
      popLastDecision();
      m_curDec->retract();
      delete (DecisionPoint*) m_curDec;
      m_curDec = DecisionPointId::noId();
    }
    reset();
  }


  void DecisionManager::reset(){ 
    check_error(m_curDec.isNoId() || m_curDec.isValid());

    if(m_curDec.isId())
      delete (DecisionPoint*) m_curDec;

    cleanup(m_closedDecisions);

    m_curDec = DecisionPointId::noId();

    m_retracting = false;
  }

  unsigned int DecisionManager::getNumberOfDecisions() const {
    return getOpenDecisionManager()->getNumberOfDecisions();
  }

  bool DecisionManager::isVariableDecision(const ConstrainedVariableId& var) const{
    return getOpenDecisionManager()->isVariableDecision(var);
  }

  bool DecisionManager::isTokenDecision(const TokenId& token) const{
    return getOpenDecisionManager()->isTokenDecision(token);
  }

  bool DecisionManager::isObjectDecision(const TokenId& token) const{
    return getOpenDecisionManager()->isObjectDecision(token);
  }

  const DecisionPointId& DecisionManager::getCurrentDecision() const {
    // If we have a current id, it means that we failed to assign it the first time,
    // and so it was never placed on the closed decisions list.
    if(!m_curDec.isNoId())
      return m_curDec;

    if(m_curDec.isNoId() && !m_closedDecisions.empty())
      return m_closedDecisions.back();

    return DecisionPointId::noId();
  }

  bool DecisionManager::assignDecision(){
    static int sl_counter(0);
    sl_counter++;

    // Call initialization so it will get the base flaw set
    getOpenDecisionManager()->initializeIfNeeded();

    checkError(!m_retracting,
	       "Control logic should ensure we are never here if we are bactracking.");

    if (m_curDec.isNoId())
      m_curDec = getOpenDecisionManager()->getNextDecision();

    // If we have no decision available then there are no flaws found so exit with failure
    if(m_curDec.isNoId()) {
      debugMsg("DecisionManager:assignDecision", "Found no decision");
      return false;
    }

    publish(notifyAssignNextStarted(m_curDec));

    debugMsg("DecisionManager:assignNextDecision:assign",
             "Deciding to assign next decision " << m_curDec);

    // Assuming the worst
    m_retracting = true;
    bool hadRemainaingChoices = true;
    if (m_curDec->hasRemainingChoices()) {
      m_curDec->assign();
      if(propagate()){
	m_closedDecisions.push_back(m_curDec);
	publish(notifyAssignCurrentSucceeded(m_curDec));
	m_curDec = DecisionPointId::noId();
	m_retracting = false;
      }
    }
    else
      hadRemainaingChoices = false;
  
    // Now, if we are backtracking
    if(m_retracting)
      publish(notifyAssignCurrentFailed(m_curDec));

    // If we never had choices, then nuke the decision point. Nothing to retract.
    // Under these conditions, it will pop off stack next if it can.
    if(!hadRemainaingChoices){
      debugMsg("DecisionManager:assignNextDecision:abandon", m_curDec);
      delete (DecisionPoint*) m_curDec;
      m_curDec = DecisionPointId::noId();
    }

    return !m_retracting;
  }

  bool DecisionManager::retractDecision(unsigned int& retractCount) {
    static int sl_counter(0);
    sl_counter++;

    // The current decision will be valid if a failed assigment was made. This may occur
    // because the choice led to an inconsistency, or because there were no choices available
    check_error(m_curDec.isNoId() || m_curDec.isValid());

    // Handle the simple case first. We have a current decision to retract.
    if(!m_curDec.isNoId()){
      publish(notifyRetractStarted(m_curDec));

      debugMsg("DecisionManager:retractDecision:retract",
               "Deciding to retract current decision " << m_curDec);

      debugMsg("DecisionManager:retractDecision:retracting", "m_retracting = " << m_retracting);

      // We are done retracting if we have choices left to try. Conversely,
      // if there are no more choices to try, we should back out.
      debugMsg("DecisionManager:retractDecision:retracting", "current decision isOpen = " << m_curDec->isOpen());

      debugMsg("DecisionManager:retractDecision:retracting", 
	       "current decision hasRemainingChoices = " << m_curDec->hasRemainingChoices());

      if (!m_curDec->isOpen() || (m_curDec->isOpen() && !m_curDec->hasRemainingChoices())) {
        m_retracting = !m_curDec->hasRemainingChoices();

        debugMsg("DecisionManager:retractDecision:retracting", 
		 "m_curDec is closed, so recomputing retracting: m_retracting = " << m_retracting);

        m_curDec->retract();

        publish(notifyRetractSucceeded(m_curDec));

        debugMsg("DecisionManager:retractDecision:retract", 
                 "Succeeded in retracting current decision (" << m_curDec->getKey() << ")");

        // If we are still retracting, then nuke the current decision
        if(m_retracting){
          delete (DecisionPoint*) m_curDec;
          m_curDec = DecisionPointId::noId();
        }
        else
          return true;
      }
    }

    m_retracting = true;

    // The simple case in which to return is if there is no id, and the closed decisions list is empty
    if(m_curDec.isNoId() && m_closedDecisions.empty())
      return true;

    // If we are here, then we cannot actually retract a decision. this can arise because:
    // 1. When trying to assign the decision there were no choices to make, and thus nothing to retract
    // 2. In the event of multiple retractions, the current decision may be null and we have to obtain one from closed list
    // We care to clean up the current id and obtain the next one if available. Thus we pop the last decision
    // and go again.
    popLastDecision();

    // Now invoke in a recursive fashion to process this decision. Ensures we actually carry out the 'retract'
    return retractDecision(++retractCount);
  }

  void DecisionManager::add(const DecisionManagerListenerId& listener) {
    check_error(listener.isValid());
    m_listeners.insert(listener);
  }

  void DecisionManager::remove(const DecisionManagerListenerId& listener) {
    m_listeners.erase(listener);
  }

  // First, condition methods

  void DecisionManager::attach(const ConditionId& cond) {
    getOpenDecisionManager()->attach(cond);
    publish(notifyConditionAdded(cond));
    debugMsg("DecisionManager:attach", "Condition Added " << cond);
  }

  void DecisionManager::detach(const ConditionId& cond) {
    getOpenDecisionManager()->detach(cond);
    if (!Entity::isPurging()){
      publish(notifyConditionRemoved(cond));
      debugMsg("DecisionManager:detach", "Condition Removed " << cond);
    }
  }

  std::vector<ConditionId> DecisionManager::getConditions() const {
    return getOpenDecisionManager()->getConditions();
  }

  ConditionId DecisionManager::getCondition(const int pos) const {
    return getOpenDecisionManager()->getCondition(pos);
  }

  void DecisionManager::printOpenDecisions(std::ostream& os) {
    check_error(m_odm.isValid());
    getOpenDecisionManager()->printOpenDecisions(os);
  }

  void DecisionManager::printClosedDecisions(std::ostream& os) {
    DecisionStack::iterator dit = m_closedDecisions.begin();
    for (; dit != m_closedDecisions.end(); ++dit)
      os << *dit << std::endl;
  }

  void DecisionManager::print(std::ostream& os) {
    getOpenDecisionManager()->print(os);
  }

  bool DecisionManager::isRetracting() const {return m_retracting;}

  bool DecisionManager::hasDecisionToRetract() const{
    return m_curDec.isId() || !m_closedDecisions.empty();
  }

  void DecisionManager::popLastDecision(){
    // First handle the cleanup of the last decision.
    if(!m_curDec.isNoId())
      delete (DecisionPoint*) m_curDec;

    if(m_closedDecisions.empty())
      m_curDec = DecisionPointId::noId();
    else {
      m_curDec = m_closedDecisions.back();
      m_closedDecisions.pop_back();
    }
  }

  bool DecisionManager::isValid() {
    check_error(m_curDec.isNoId() || m_curDec.isValid());
    return true;
  }

  bool DecisionManager::propagate(){
    bool res = m_db->getClient()->propagate();
    return res;
  }

  void DecisionManager::plannerTimeoutReached() const {
    debugMsg("DecisionManager:plannerTimeoutReached", "Timeout. Plan Incomplete.");
    publish(notifyPlannerTimeout());
  }

  void DecisionManager::plannerSearchFinished() const {
    debugMsg("DecisionManager:plannerSearchFinished", "Search Finished. Plan Complete.");
    publish(notifySearchFinished());
  }
}
