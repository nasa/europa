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

    Component::~Component(){m_id.remove();}

    const ComponentId& Component::getId() const {return m_id;}

    Component::Component(const TiXmlElement& configData):
      m_id(this) {}

    /**
     * ALLOCATOR IMPLEMENTATION
     */
    ComponentId Component::AbstractFactory::allocate(const TiXmlElement& configData){
      LabelStr name = extractData(configData, "component");
      std::map< LabelStr, AbstractFactory* >::const_iterator it = factories().find(name);
      checkError(it != factories().end(), "No allocator registered for " << name.toString());
      debugMsg("Component:AbstractFactory:allocate", "Allocating component using " << name.toString());
      AbstractFactory* allocator = it->second;
      return allocator->create(configData);
    }

    void Component::AbstractFactory::purge(){
      factories().clear();
    }

    Component::AbstractFactory::~AbstractFactory(){
      debugMsg("Component:AbstractFactory:~AbstractFactory", "Deleting " << m_name.toString());
      remove(m_name);
    }

    const LabelStr& Component::AbstractFactory::getName() const {return m_name;}

    void Component::AbstractFactory::add(const LabelStr& name, AbstractFactory* allocator){
      factories().insert(std::pair<LabelStr, AbstractFactory*>(name, allocator));
    }

    void Component::AbstractFactory::remove(const LabelStr& name){
      factories().erase(name);
    }

    std::map< LabelStr, Component::AbstractFactory* >& Component::AbstractFactory::factories(){
      static std::map< LabelStr, AbstractFactory* > sl_collection;
      return sl_collection;
    }

    Component::AbstractFactory::AbstractFactory(const LabelStr& name): m_name(name){ 
      debugMsg("Component:AbstractFactory:AbstractFactory", "Creating " << m_name.toString());
      add(m_name, this);
    }
  }
}
