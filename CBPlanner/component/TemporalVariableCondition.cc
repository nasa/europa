#include "PlanIdentificationDefs.hh"
#include "TemporalVariableCondition.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "IntervalIntDomain.hh"
#include "ConstrainedVariable.hh"

namespace Prototype {

  TemporalVariableCondition::TemporalVariableCondition(const HorizonId& horizon, const DecisionManagerId& dm): Condition(dm),
    m_start(true), m_end(true), m_duration(true),m_allowOverlap(false), m_horizon(horizon) {
    check_error(m_horizon.isValid());
    check_error(m_id.isValid());
  }

  TemporalVariableCondition::~TemporalVariableCondition() {
    check_error(m_horizon.isValid());
  }

  bool TemporalVariableCondition::isStartIgnored() const { return m_start; }
  bool TemporalVariableCondition::isEndIgnored() const { return m_end; } 
  bool TemporalVariableCondition::isDurationIgnored() const { return m_duration; } 
  bool TemporalVariableCondition::isTemporalIgnored() const { return (m_start && m_end && m_duration); }

  void TemporalVariableCondition::setIgnoreStart(const bool& start) { 
    if (m_start != start) {
      m_start = start; 
      notifyChanged();
    }
  }

  void TemporalVariableCondition::setIgnoreEnd(const bool& end) { 
    if (m_end != end) {
      m_end = end;
      notifyChanged();
    }
  } 

  void TemporalVariableCondition::setIgnoreDuration(const bool& duration) { 
    if (m_duration != duration) {
      m_duration = duration; 
      notifyChanged();
    }
  }

  void TemporalVariableCondition::setIgnoreTemporal(const bool& value) { 
    setIgnoreStart(value);
    setIgnoreEnd(value);
    setIgnoreDuration(value);
  }

  void TemporalVariableCondition::allowHorizonOverlap() { 
    if (!m_allowOverlap) {
      m_allowOverlap = true; 
      notifyChanged();
    }
  } 

  void TemporalVariableCondition::disallowHorizonOverlap() { 
    if (m_allowOverlap) {
      m_allowOverlap = false; 
      notifyChanged();
    }
  }

  bool TemporalVariableCondition::isHorizonOverlapAllowed() const { return m_allowOverlap; }

  bool TemporalVariableCondition::domainContainsHorizonEnd (const ConstrainedVariableId& var) {
    check_error (m_horizon.isValid());
    int start,end; 
    m_horizon->getHorizon(start,end);
    
    // we know because the check is of whether it is convertable is done in
    // test method which is the only one calling this method.
    const TempVarId& tmpVar = var;
    const AbstractDomain& varDom = tmpVar->lastDomain();
    if ((varDom.getLowerBound() <= end-1) && 
	(varDom.getUpperBound() >= end))
      return true;
    return false;
  }

    bool TemporalVariableCondition::test(const EntityId& entity) {
      check_error(entity.isValid());
      if (!TempVarId::convertable(entity)) return true;
      bool passed(true);
      TempVarId var(entity);
      EntityId parent(var->getParent());
      check_error(parent.isValid());
      if (TokenId::convertable(parent)) {
	TokenId tok(parent);
	int key(var->getKey());
	if (key == tok->getEnd()->getKey()) {
	  if (m_end && m_allowOverlap) {
	    if (!domainContainsHorizonEnd(var))
	      passed = false;
	  }
	  else 
	    passed = !m_end;
	}
	else if (key == tok->getStart()->getKey()) {
	  passed = !m_start;
	}
	else if (key == tok->getDuration()->getKey()) {
	  passed = !m_duration;
	}
      }

      return passed;
    } 
	      
    void TemporalVariableCondition::print (std::ostream& os) { os << "TEMPORAL_VARIABLE_CONDITION"; }

}
