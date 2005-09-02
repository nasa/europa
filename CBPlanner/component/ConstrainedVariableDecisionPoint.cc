#include "ConstrainedVariableDecisionPoint.hh"
#include "ConstrainedVariable.hh"
#include "OpenDecisionManager.hh"
#include "DbClient.hh"
#include "AbstractDomain.hh"
#include <iostream>

namespace EUROPA {

  ConstrainedVariableDecisionPoint::ConstrainedVariableDecisionPoint(const DbClientId& dbClient, const ConstrainedVariableId& var, const OpenDecisionManagerId& odm)
    : DecisionPoint(dbClient, var) {  
      m_var = var; 
      m_choiceIndex = 0;
      m_odm = odm;
    }

  ConstrainedVariableDecisionPoint::~ConstrainedVariableDecisionPoint() { }

  unsigned int ConstrainedVariableDecisionPoint::getNrChoices() {
    check_error(m_var.isValid());
    if (m_var->lastDomain().isNumeric()) 
      if (!m_var->lastDomain().isFinite() || m_var->lastDomain().getSize() > 50) 
	return (unsigned int) (m_choices[1] - m_choices[0]);
    return m_choices.size();
  }

  std::vector<double>& ConstrainedVariableDecisionPoint::getChoices() {
    return m_choices;
  }

  const double ConstrainedVariableDecisionPoint::getChoiceValue(const unsigned int index) const {
    if (m_var->lastDomain().isNumeric() && m_var->lastDomain().getSize() > 50) 
      return m_choices[0]+index;
    else
      return m_choices[index];
  }

  bool ConstrainedVariableDecisionPoint::assign() {
    checkError(m_choiceIndex < getNrChoices() && getNrChoices() > 0 && !m_choices.empty(),
	       "Cannor be assigning if we have no choices. A control loop bug.");
    check_error(m_var.isValid());
    check_error(!m_var->lastDomain().isOpen() && m_var->lastDomain().isFinite(),
		"Cannot assign a variable with open or infinite domain");

    m_open = false;

    m_dbClient->specify(m_var, getChoiceValue(m_choiceIndex)); 
    m_choiceIndex++;
    return true;
  }

  bool ConstrainedVariableDecisionPoint::retract() {
    m_dbClient->reset(m_var);

    m_open = true;

    if (!hasRemainingChoices()) 
      return false;
    return true;
  }

  bool ConstrainedVariableDecisionPoint::hasRemainingChoices() {
    if (m_choiceIndex == 0) return true; // we have never assigned this decision  or initialized choices
    return m_choiceIndex < getNrChoices();
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
      os << " Current Choice: ";
      if (m_choiceIndex == 0) os << " No Choice ";
      else {
	double valueAsDouble = getChoiceValue(m_choiceIndex-1);
	if (m_var->lastDomain().isNumeric()) os << valueAsDouble;
	else if (LabelStr::isString(valueAsDouble)) os << LabelStr(valueAsDouble).toString();
	else if (m_var->lastDomain().isBool()) {
	  if (valueAsDouble == 0) os << "false";
	  else os << "true";
	}
	else {
	  EntityId entity(valueAsDouble);
	  os << entity->getName().toString() << " (" << entity->getKey() << ")";
	}
      }
      os << " Discarded: " << m_choiceIndex;
    }
    else 
      os << "(" << getKey() << ") Variable (" << m_entityKey << ") [deleted]  ";
  }

  std::ostream& operator <<(std::ostream& os, const Id<ConstrainedVariableDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
