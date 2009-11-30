#include "Context.hh"
#include "Error.hh"

namespace EUROPA {
  namespace SOLVERS {
    Context::Context(const LabelStr& name) : m_id(this), m_name(name) {}
    
    Context::~Context() {
      m_id.remove();
    }

    double Context::get(const LabelStr& key) const {
      std::map<edouble, double>::const_iterator it = m_map.find(key);
      checkError(it != m_map.end(), "Error: '" << key.toString()  << "' not in context '" << m_name.toString() << "'");
      return it->second;
    }

    void Context::remove(const LabelStr& key) {
      std::map<edouble, double>::const_iterator it = m_map.find(key);
      checkError(it != m_map.end(), "Error: '" << key.toString()  << "' not in context '" << m_name.toString() << "'");
      m_map.erase(key);
    }
  }
}
