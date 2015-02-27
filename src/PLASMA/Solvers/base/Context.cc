#include "Context.hh"
#include "Error.hh"

namespace EUROPA {
namespace SOLVERS {
Context::Context(const std::string& name) : m_id(this), m_name(name), m_map() {}
    
    Context::~Context() {
      m_id.remove();
    }

    double Context::get(const std::string& key) const {
      std::map<std::string, double>::const_iterator it = m_map.find(key);
      checkError(it != m_map.end(), "Error: '" << key  << "' not in context '" << m_name << "'");
      return it->second;
    }

    void Context::remove(const std::string& key) {
      std::map<std::string, double>::const_iterator it = m_map.find(key);
      checkError(it != m_map.end(), "Error: '" << key  << "' not in context '" << m_name << "'");
      m_map.erase(key);
    }
  }
}
