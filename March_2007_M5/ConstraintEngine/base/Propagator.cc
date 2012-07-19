#include "Propagator.hh"
#include "ConstraintEngine.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA
{
  Propagator::Propagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Entity(), m_id(this), m_name(name), m_constraintEngine(constraintEngine), m_enabled(true){
    check_error(!m_constraintEngine.isNoId() && m_constraintEngine.isValid());
    m_constraintEngine->add(m_id);
  }

  Propagator::~Propagator(){
    discard(false);
    m_id.remove();
  }

  const PropagatorId& Propagator::getId() const {return m_id;}

  const LabelStr& Propagator::getName() const {return m_name;}

  const ConstraintEngineId& Propagator::getConstraintEngine() const {return m_constraintEngine;}

  bool Propagator::isEnabled() const {return m_enabled;}

  void Propagator::enable(){m_enabled = true;}

  void Propagator::disable(){m_enabled = false;}

  const std::list<ConstraintId>& Propagator::getConstraints() const {return m_constraints;}

  void Propagator::execute(const ConstraintId& constraint){
    check_error(constraint.isValid());
    m_constraintEngine->execute(constraint);
  }

  void Propagator::addConstraint(const ConstraintId& constraint){
    check_error(constraint->getPropagator().isNoId());
    checkError(!constraint->isDiscarded(), constraint);
    m_constraints.push_back(constraint);
    handleConstraintAdded(constraint);
  }

  void Propagator::removeConstraint(const ConstraintId& constraint){
    check_error(constraint->getPropagator() == m_id);
    m_constraints.remove(constraint);
    handleConstraintRemoved(constraint);
  }

  AbstractDomain& Propagator::getCurrentDomain(const ConstrainedVariableId& var) {
    check_error(var.isValid());
    return var->getCurrentDomain();
  }
}
