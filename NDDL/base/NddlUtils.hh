#ifndef H_NddlUtils
#define H_NddlUtils

/**
 * @file NddlUtils
 * @brief Provides support code for the NddlCompiler output coding patterns
 * @author Conor McGann
 * @date January, 2004
 */

#include "NddlDefs.hh"
#include "NddlToken.hh"
#include "NddlRules.hh"

#include "NumericDomain.hh"
#include "SymbolDomain.hh"
#include "StringDomain.hh"

#include "TypeFactory.hh"
#include "EnumeratedTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "intType.hh"
#include "floatType.hh"

namespace NDDL {

  /**
   * @brief Helper method to handle the conversion from NDDL language types to the registered
   * type names. The main issue is handling the NDDL primitives: int, float, bool, string
   * @see TypeFactory
   */
  /*
  LabelStr getType(const char* nddlType){
    static const std::string sl_nddlPrimitives(":int:float:string:bool:");

    // If it is a primitive, then return the type name obtained through the type factory
    if(sl_nddlPrimitives.find(nddlType) >= 0)
      return TypeFactory::baseDomain(nddlType).getTypeName();
    else
      return nddlType;
  }
*/
  /**
   * @brief Helper method to construct a singleton variable, and place it on a vector. 
   * Used in rule firing and generating pseudo variables on tokens. They cannot be specified.
   */ 
  template<class ELEMENT_TYPE>
  ConstrainedVariableId allocateVariable(const ConstraintEngineId& ce, 
						  std::vector<ConstrainedVariableId>& vars,
						  const ELEMENT_TYPE& domain,
						  const EntityId& parent){

    ConstrainedVariableId var = (new Variable< ELEMENT_TYPE >(ce, 
							      domain,
							      false,
							      Token::makePseudoVarName(),
							      parent))->getId();
    vars.push_back(var);
    return var;
  };

  /**
   * Should all be moved to the rule instance class.
   */
  TokenId tok(const RuleInstanceId& rule, const std::string name) ;
  ConstrainedVariableId var(const RuleInstanceId& entity, const std::string name) ;
  ConstrainedVariableId var(const TokenId& entity, const std::string name) ;

}

//------------------------------------------------------------------------------------------------------------------------------


#include "ObjectFactory.hh"

#define DECLARE_DEFAULT_OBJECT_FACTORY(Factory, Klass)\
class Factory: public ConcreteObjectFactory{\
public:\
  Factory(const LabelStr& name): ConcreteObjectFactory(name) {}\
private:\
  ObjectId createInstance(const PlanDatabaseId& planDb,\
			  const LabelStr& objectType, \
			  const LabelStr& objectName,\
			  const std::vector<const AbstractDomain*>& arguments) const {\
    check_error(arguments.empty());\
    Id<Klass> instance = (new Klass(planDb, objectType, objectName))->getId();\
    instance->handleDefaults();\
    return instance;\
  }\
};

#include "TokenFactory.hh"

/**
 * Declare a token factory - inline.
 */
#define DECLARE_TOKEN_FACTORY(klass, predicateName) \
class Factory: public ConcreteTokenFactory { \
public: \
  Factory() : ConcreteTokenFactory(LabelStr(#predicateName)) { \
  } \
private: \
  TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false, bool isFact = false) const { \
    TokenId token = (new klass(planDb, name, rejectable, isFact, true))->getId(); \
    return(token); \
  } \
  TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const { \
    TokenId token = (new klass(master, name, relation, true))->getId(); \
    return(token); \
  } \
};

#define REGISTER_TOKEN_FACTORY(klass) (new klass::Factory())

#define REGISTER_TYPE_FACTORY(klass, domain) \
  (new EnumeratedTypeFactory(#klass, #klass, domain))
	
#define REGISTER_ITYPE_FACTORY(klass, domain) \
  (new IntervalTypeFactory(#klass, domain))

#endif
