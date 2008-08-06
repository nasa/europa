#ifndef H_ConfigurationFactory
#define H_ConfigurationFactory

#include "SolverDefs.hh"
#include "XMLUtils.hh"

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
    class Component {
    public:
      virtual ~Component();
      const ComponentId& getId() const;

      /**
       * @brief Defines a base class for factories that take xml configuration data.
       */
      class AbstractFactory {
      public:
        /**
         * @brief Abstract creation method.
         * @param configData An xml element that may or may not have children. It must have an attribute of 'name'
         * @return An instance of a component.
         */
        static ComponentId allocate(const TiXmlElement& configData);

	/**
	 * @brief Deallocate all elements
	 */
	static void purge();

        virtual ~AbstractFactory();

        /**
         * @brief Access the logical name used for lookup.
         * @see allocate
         */
        const LabelStr& getName() const;

      protected:
        /**
         * @brief Register factory by name.
         */
        static void add(const LabelStr& name, AbstractFactory* factory);

        /**
         * @brief Remove registered factory
         */
        static void remove(const LabelStr& name);

        static std::map< LabelStr, AbstractFactory* >& factories(); /*!< Stores factories by name */

        /**
         * @brief Implement this in derived classes
         * @see ConcreteFactory::create
         */
        virtual ComponentId create(const TiXmlElement& configData) const = 0;

        AbstractFactory(const LabelStr& name);

      private:
        const LabelStr m_name; /*!< Name used for lookup */
      };

      /**
       * @brief Provides concrete allocation using a template.
       */
      template<class T>
      class ConcreteFactory: public AbstractFactory {
      public:
        ConcreteFactory(const LabelStr& name) : AbstractFactory(name) {}

      private:
        ComponentId create(const TiXmlElement& config) const {
          ComponentId component = (new T(config))->getId();
          return component;
        }
      };

    protected:
      Component(const TiXmlElement& configData);
      Component() : m_id(this) {}
    private:

      Id<Component> m_id;
    };

    /**
     * @brief Use to register factory classes.
     */
#define REGISTER_COMPONENT_FACTORY(CLASS, NAME) new EUROPA::SOLVERS::Component::ConcreteFactory< CLASS >(#NAME);
  }
} // namespace
#endif
