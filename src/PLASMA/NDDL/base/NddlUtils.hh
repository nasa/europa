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

#include "Domains.hh"

namespace NDDL {

  /**
   * @brief Helper method to construct a singleton variable, and place it on a vector.
   * Used in rule firing and generating pseudo variables on tokens. They cannot be specified.
   */
  template<class ELEMENT_TYPE>
  ConstrainedVariableId allocateVariable(const ConstraintEngineId& ce,
						  std::vector<ConstrainedVariableId>& vars,
						  const ELEMENT_TYPE& domain,
						  const EntityId& parent){

    std::stringstream sstr;
    sstr << "PSEDUO_VARIABLE_" << vars.size();
    ConstrainedVariableId var = (new Variable< ELEMENT_TYPE >(ce,
							      domain,
							      false,
							      false,
							      LabelStr(sstr.str()),
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
class Factory: public ObjectFactory{\
public:\
  Factory(const LabelStr& name): ObjectFactory(name) {}\
private:\
  ObjectId createInstance(const PlanDatabaseId& planDb,\
			  const LabelStr& objectType, \
			  const LabelStr& objectName,\
			  const std::vector<const Domain*>& arguments) const {\
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
class Factory: public TokenFactory { \
public: \
  Factory() : TokenFactory(LabelStr(#predicateName)) { \
  } \
  TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false, bool isFact = false) const { \
    TokenId token = (new klass(planDb, name, rejectable, isFact, true))->getId(); \
    return(token); \
  } \
  TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const { \
    TokenId token = (new klass(master, name, relation, true))->getId(); \
    return(token); \
  } \
};

#define REGISTER_TOKEN_FACTORY(schema_id,klass) \
  schema_id->registerTokenFactory((new klass::Factory())->getId())

#define REGISTER_TYPE_FACTORY(typeFactoryMgr, klass, domain) \
  typeFactoryMgr->registerFactory((new EnumeratedTypeFactory(#klass, #klass, domain))->getId())

#define REGISTER_ITYPE_FACTORY(typeFactoryMgr, klass, domain) \
  typeFactoryMgr->registerFactory((new IntervalTypeFactory(#klass, domain))->getId())

#endif
