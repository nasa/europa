#include "Generator.hh"

namespace PLASMA {

  Generator::Generator(const LabelStr& name) : Entity(), m_id(this), m_name(name) { }

  Generator::~Generator() { 
    check_error(m_id.isValid()); 
    m_id.remove(); 
  }
  const GeneratorId& Generator::getId() const { return m_id; }
  const LabelStr& Generator::getName() const { return m_name; }
  void Generator::getAllValues(std::list<double>& values) { }
  void Generator::getRemainingValues(std::list<double>& values) { }
  void Generator::getNextValue(double next, const double last) { }
}
