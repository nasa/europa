#include "RulesEngineListener.hh"
#include "RulesEngine.hh"
#include "RulesEngineDefs.hh"
#include "Entity.hh"

namespace EUROPA {
  RulesEngineListener::RulesEngineListener(const RulesEngineId &rulesEngine) 
    : m_id(this), m_rulesEngine(rulesEngine) {
    check_error(m_rulesEngine.isValid());
    m_rulesEngine->add(m_id);
  }

  RulesEngineListener::~RulesEngineListener() {
    check_error(m_id.isValid());
    check_error(m_rulesEngine.isValid());
    m_rulesEngine->remove(m_id);
    m_id.remove();
  }

  const RulesEngineListenerId &RulesEngineListener::getId() const {
    return m_id;
  }
}
