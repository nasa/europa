#ifndef H_ConfigurationFactory
#define H_ConfigurationFactory

#include "SolverDefs.hh"
#include "XMLUtils.hh"
#include "Factory.hh"

/**
 * @brief Declares classes for binding components to xml based configuration data.
 *
 * This follows an AbstractFactory pattern. The main feature of this over other factories is the
 * provision for passing XML elements to pass configuration information as they are constructed.
 * @author Conor McGann
 * @date 03/05
 */

namespace EUROPA {
  namespace SOLVERS {

    class Component;
    typedef Id<Component> ComponentId;

    /**
     * @brief A base class for components that can be allocated within this framework.
     * data in xml.
     */
    class Component : public FactoryObj {
    public:
        virtual ~Component();
        ComponentId& getId();
        const ComponentId& getId() const;

    protected:
        ComponentId m_id;
        
        Component(const TiXmlElement& configData);
        Component();
    };

    class ComponentArgs : public FactoryArgs {
    public:
        const TiXmlElement& configData;
        ComponentArgs(const TiXmlElement& cd) : configData(cd) {}
    };
    
    /**
      * @brief Defines a base class for factories that take xml configuration data.
      */    
    class ComponentFactoryMgr : public FactoryMgr {       
    public:
       virtual ComponentId createInstance(const TiXmlElement& configData);
    };

    /**
    * @brief Provides concrete allocation using a template.
    */
    template<class ComponentType>
    class ComponentFactory: public Factory 
    {
    public:
        ComponentFactory(const LabelStr& name) : Factory(name) {}

        virtual EUROPA::FactoryObjId& createInstance(const EUROPA::FactoryArgs& fa) {
            const ComponentArgs& args = (const ComponentArgs&)fa;
            return (EUROPA::FactoryObjId&)(new ComponentType(args.configData))->getId();
        }
    };

   /**
     * @brief Use to register factory classes.
     */
#define REGISTER_COMPONENT_FACTORY(MGR,CLASS, NAME) \
    (MGR->registerFactory((new EUROPA::SOLVERS::ComponentFactory< CLASS >(#NAME))->getId()));
  }
} // namespace
#endif
