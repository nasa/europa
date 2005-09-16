#include "DynamicInfiniteRealCondition.hh"
#include "AbstractDomain.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"

namespace EUROPA {

  DynamicInfiniteRealCondition::DynamicInfiniteRealCondition(const DecisionManagerId& dm): Condition(dm) {
    check_error(m_id.isValid());
  }

  DynamicInfiniteRealCondition::~DynamicInfiniteRealCondition() { }

  bool DynamicInfiniteRealCondition::test(const EntityId& entity) {
    if (!ConstrainedVariableId::convertable(entity)) return true;
    ConstrainedVariableId var(entity); 
    check_error(var.isValid());
    bool passed(true);
    const AbstractDomain& dom = var->lastDomain();
    if (dom.isOpen() || dom.isInfinite())
      passed = false;

    debugMsg("DynamicInfiniteRealCondition:test", var->toString() << (passed ? " PASSED" : "FAILED"));

    return passed;
  } 
	      
  inline void DynamicInfiniteRealCondition::print (std::ostream& os) { os << "DYNAMIC_INFINITE_REAL_CONDITION"; }

} 
