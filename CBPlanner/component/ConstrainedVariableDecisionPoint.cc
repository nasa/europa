#include "ConstrainedVariableDecisionPoint.hh"
#include "ConstrainedVariable.hh"
#include "DbClient.hh"
#include "ValueChoice.hh"
#include "AbstractDomain.hh"
#include <iostream>

namespace Prototype {

  ConstrainedVariableDecisionPoint::ConstrainedVariableDecisionPoint(const DbClientId& dbClient, const ConstrainedVariableId& var)
    : DecisionPoint(dbClient, var) {  m_var = var; }

  ConstrainedVariableDecisionPoint::~ConstrainedVariableDecisionPoint() { }

  const bool ConstrainedVariableDecisionPoint::assign(const ChoiceId& choice) {
    check_error(!choice.isNoId());
    check_error(Id<ValueChoice>::convertable(choice));
    m_dbClient->specify(m_var, Id<ValueChoice>(choice)->getValue()); 
    return DecisionPoint::assign(choice);
  }

  const bool ConstrainedVariableDecisionPoint::retract() {
    m_dbClient->reset(m_var);
    return DecisionPoint::retract();
  }

  std::list<ChoiceId>& ConstrainedVariableDecisionPoint::getChoices() {
    check_error(m_id.isValid());
    std::list<ChoiceId>::iterator cit = m_choices.begin();
    for(; cit != m_choices.end(); cit++) {
      delete (Choice*) (*cit);
    }
    m_choices.clear();
    Choice::makeChoices(m_id, m_var->lastDomain(), m_choices);
    return DecisionPoint::getChoices();
  }

  const ConstrainedVariableId& ConstrainedVariableDecisionPoint::getVariable() const { return m_var;  }

  void ConstrainedVariableDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    if (!m_var.isNoId()) {
      os << "(" << getKey() << ") Variable (" << m_entityKey << ") ";
      if (!m_var->getParent().isNoId())
	os << " Parent: " << m_var->getParent()->getKey();
      if (m_var->lastDomain().isOpen()) {
	check_error(false);
      }
      else if (m_var->lastDomain().isInfinite())
	os << " DerivedDomain size: Infinity";
      else
	os << " DerivedDomain: " << m_var->lastDomain();
    }
    else 
      os << "(" << getKey() << ") Variable (" << m_entityKey << ") [deleted] ";
    os << " Current Choice: " << m_current;
    os << " Discarded: " << m_discarded.size();
  }

  std::ostream& operator <<(std::ostream& os, const Id<ConstrainedVariableDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
