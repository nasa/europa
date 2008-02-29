#include "ConstraintEngineListener.hh"
#include "ConstraintEngine.hh"
#include "Entity.hh"

namespace EUROPA {

  ConstraintEngineListener::ConstraintEngineListener(const ConstraintEngineId& constraintEngine)
    :m_id(this), m_constraintEngine(constraintEngine){
    check_error(m_constraintEngine.isValid());
    m_constraintEngine->add(m_id);
  }

  ConstraintEngineListener::~ConstraintEngineListener(){
    check_error(m_id.isValid());
    check_error(m_constraintEngine.isValid());
    m_constraintEngine->remove(m_id);
    m_id.remove();
  }

  const ConstraintEngineListenerId& ConstraintEngineListener::getId() const {
    return(m_id);
  }
}
