#include "ComponentFactory.hh"
#include "Debug.hh"
#include "SolverUtils.hh"

/**
 * @file ComponentFactory.cc
 * @author Conor McGann
 * @brief Provides implementation for xml-based component factories.
 * @date April, 2005
 */

namespace EUROPA {
namespace SOLVERS {

Component::Component() : m_id(this) {}
    
Component::Component(const TiXmlElement&) : m_id(this) {}
    
Component::~Component() {
  m_id.remove();
}

ComponentId Component::getId() {return m_id;}

const ComponentId Component::getId() const {return m_id;}

const LabelStr& Component::getName() const {
  return m_name;
}

void Component::setName(const LabelStr& name){
  m_name = name;
}

ComponentId ComponentFactoryMgr::createComponentInstance(const TiXmlElement& configData) {
  LabelStr name = extractData(configData, "component");
  return FactoryMgr::createInstance(name,ComponentArgs(configData));
}

}
}
