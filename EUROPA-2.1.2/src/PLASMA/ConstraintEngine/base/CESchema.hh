#ifndef _H_CESchema
#define _H_CESchema

#include "ConstraintEngineDefs.hh"
#include "AbstractDomain.hh"
#include "ConstrainedVariable.hh"
#include "LabelStr.hh"
#include "Engine.hh"
#include "TypeFactory.hh"
#include "ConstraintFactory.hh"
#include <map>
#include <string>

/**
 * @file Class to manage all metadata for Constraint engine (variable data types, constraint types, etc).
 * @author Javier Barreiro, May 2008
 */

namespace EUROPA {

  class CESchema;
  typedef Id<CESchema> CESchemaId;

  class CESchema : public EngineComponent
  {
    public:
      CESchema();
      virtual ~CESchema();
  
      const CESchemaId& getId() const;
      
      // Methods to Manage Type Factories
      /**
       * @brief Add a factory to provide instantiation of particular concrete types based on a label.
       */
      void registerFactory(const TypeFactoryId& factory);

      /**
       * @brief Obtain the factory based on the type name
       */ 
      TypeFactoryId getFactory(const char* typeName);

      /**
       * @brief Return the base domain
       */
      const AbstractDomain & baseDomain(const char* typeName);

      void purgeTypeFactories();

      // Methods to Manage Constraint Factories
      void registerConstraintFactory(ConstraintFactory* factory);
      void registerConstraintFactory(ConstraintFactory* factory, const LabelStr& name);

      const ConstraintFactoryId& getConstraintFactory(const LabelStr& name);

      bool isConstraintFactoryRegistered(const LabelStr& name, const bool& warn = false);

      bool isConstraintFactoryNotRegistered(const LabelStr& name);      

      void purgeConstraintFactories();
      
      /**
       * @brief Delete all factory instances stored. 
       */
      void purgeAll();
      

    protected:
      CESchemaId m_id;
      std::map<double, TypeFactoryId> m_typeFactories;   
      std::map<double, ConstraintFactoryId > m_constraintFactories;      
  };
  
} // namespace EUROPA

#endif // _H_CESchema
