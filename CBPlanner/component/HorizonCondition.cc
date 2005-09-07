#include "ConstrainedVariable.hh"
#include "HorizonCondition.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Debug.hh"

namespace EUROPA {

  HorizonCondition::HorizonCondition(const HorizonId& horizon, const DecisionManagerId& dm) : 
    Condition(dm), m_horizon(horizon), m_possiblyOutside(true) {
    check_error(m_horizon.isValid());
    check_error(m_id.isValid());
  }

  HorizonCondition::~HorizonCondition() {
    check_error(m_horizon.isValid());
  }

  void HorizonCondition::setNecessarilyOutsideHorizon() { 
    if (m_possiblyOutside) {
      m_possiblyOutside = false;
    }
  }

  void HorizonCondition::setPossiblyOutsideHorizon() { 
    if (!m_possiblyOutside) {
      m_possiblyOutside = true;
    }
  }

  bool HorizonCondition::isPossiblyOutsideHorizon() const { return m_possiblyOutside; }
  bool HorizonCondition::isNecessarilyOutsideHorizon() const { return !m_possiblyOutside;}

  bool HorizonCondition::testToken(const TokenId& tok) {
    const TempVarId& startVar = tok->getStart();
    const TempVarId& endVar = tok->getEnd();
    const AbstractDomain& tokStartDomain = startVar->lastDomain();
    const AbstractDomain& tokEndDomain = endVar->lastDomain();
    int start, end;
    m_horizon->getHorizon(start, end);
    bool passed(true);
    debugMsg("HorizonCondition:testToken", "Horizion is [" << start << ", " << end << "]");
    if (m_possiblyOutside) {
      debugMsg("HorizonCondition:testToken", "Testing possibly outside [" << tokStartDomain.getUpperBound() << ", " << tokEndDomain.getLowerBound() << "]");
      // apply possibly defintion.
      if (!tokStartDomain.isEmpty() && (tokStartDomain.getUpperBound() >= end)) {
	passed = false; 
        debugMsg("HorizionCondition:testToken", "Possible test failed start.upperbound >= end");
      }
      if (!tokEndDomain.isEmpty() && (tokEndDomain.getLowerBound() <= start)) {
	passed = false; 
       debugMsg("HorizionCondition:testToken", "Possible test failed end.lowerbound <= start");
      }
    }
    else {
      // apply necessary defintion.
       debugMsg("HorizonCondition:testToken", "Testing necessary outside [" << tokStartDomain.getLowerBound() << ", " << tokEndDomain.getUpperBound() << "]");
       if (!tokStartDomain.isEmpty() && (tokStartDomain.getLowerBound() > end)) {
	passed = false; 
        debugMsg("HorizionCondition:testToken", "Necessary test failed start.lowerbound > end");
       }
       if (!tokEndDomain.isEmpty() && (tokEndDomain.getUpperBound() < start)) {
	passed = false; 
        debugMsg("HorizionCondition:testToken", "Necessary test failed end.upperbound < start");
       }
    }
    return passed;
  }

  bool HorizonCondition::test(const EntityId& entity) {
    check_error(entity.isValid());
    TokenId tok;
    if (ConstrainedVariableId::convertable(entity)) {
      ConstrainedVariableId var(entity);
      EntityId parent(var->getParent());
      if (TokenId::convertable(parent)) {
	tok = parent;
      }
    }
    else if (TokenId::convertable(entity))
      tok = entity;

    if (!tok.isNoId())
      return testToken(tok);
    return true;
  }
}

