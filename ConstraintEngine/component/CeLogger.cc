#include "CeLogger.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"
#include "AbstractDomain.hh"
#include <string>
#include <iostream>

namespace Prototype {

  const std::string VARIABLE_INDENT("");

  std::string getName(const ConstrainedVariableId& variable) {
    return(variable->getName().toString());
  }

  CeLogger::CeLogger(std::ostream& os, const ConstraintEngineId& ce)
    : ConstraintEngineListener(ce), m_os(os) { }

  void CeLogger::notifyPropagationCommenced() {
    m_os << "CeLogger: PROPAGATION COMMENCED" << std::endl;
  }

  void CeLogger::notifyPropagationCompleted() {
    m_os << "CeLogger: PROPAGATION COMPLETED" << std::endl;
  }

  void CeLogger::notifyPropagationPreempted() {
    m_os << "CeLogger: PROPAGATION PREEMPTED" << std::endl;
  }

  void CeLogger::notifyAdded(const ConstraintId& constraint) {
    m_os << "CeLogger: CONSTRAINT ADDED "
         << constraint->getName().toString() << '('
         << constraint->getKey() << ')' << std::endl;
  }

  void CeLogger::notifyRemoved(const ConstraintId& constraint) {
    m_os << "CeLogger: CONSTRAINT REMOVED "
         << constraint->getName().toString() << '('
         << constraint->getKey() << ')' << std::endl;
  }

  void CeLogger::notifyExecuted(const ConstraintId& constraint) {
    m_os << "CeLogger:  CONSTRAINT EXECUTED "
         << constraint->getName().toString() << '('
         << constraint->getKey() << ')' << std::endl;
  }

  void CeLogger::notifyAdded(const ConstrainedVariableId& variable) {
    m_os << "CeLogger: " << VARIABLE_INDENT << "VARIABLE ADDED "
         << getName(variable) << '(' << variable->getKey() <<  ')' << std::endl;
  }

  void CeLogger::notifyRemoved(const ConstrainedVariableId& variable) {
    m_os << "CeLogger: " << VARIABLE_INDENT << "VARIABLE REMOVED "
         << getName(variable) << '(' << variable->getKey() <<  ')' << std::endl;
  }

  void CeLogger::notifyChanged(const ConstrainedVariableId& variable,
                               const DomainListener::ChangeType& changeType) {
    m_os << "CeLogger: " << VARIABLE_INDENT << "VARIABLE "
         << getName(variable) << " - ";
    switch (changeType) {
    case DomainListener::UPPER_BOUND_DECREASED:
      m_os << "UPPER_BOUND_DECREASED";
      break;
    case DomainListener::LOWER_BOUND_INCREASED:
      m_os << "LOWER_BOUND_INCREASED";
      break;
    case DomainListener::BOUNDS_RESTRICTED:
      m_os << "BOUNDS_RESTRICTED";
      break;
    case DomainListener::VALUE_REMOVED:
      m_os << "VALUE_REMOVED";
      break;
    case DomainListener::RESTRICT_TO_SINGLETON:
      m_os << "RESTRICT_TO_SINGLETON";
      break;
    case DomainListener::SET:
      m_os << "SET";
      break;
    case DomainListener::SET_TO_SINGLETON:
      m_os << "SET_TO_SINGLETON";
      break;
    case DomainListener::RESET:
      m_os << "RESET";
      break;
    case DomainListener::RELAXED:
      m_os << "RELAXED";
      break;
    case DomainListener::CLOSED:
      m_os << "CLOSED";
      break;
    case DomainListener::EMPTIED:
      m_os << "EMPTIED";
      break;
    default:
      check_error(ALWAYS_FAILS);
      break;
    }
    m_os << " (" << variable->getKey() << ':';
    variable->lastDomain() >> m_os;
    m_os << ')' << std::endl;
  }
}
