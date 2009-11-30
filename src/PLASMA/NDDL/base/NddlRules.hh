#ifndef NDDL_RULES_HH
#define NDDL_RULES_HH

#include "NddlDefs.hh"
#include "Rule.hh"
#include "RuleInstance.hh"

namespace NDDL {

  /**************** SUPPORT FOR INTEGRATION TO RULES ENGINE *********************/

#define localVar(domain, name, guarded) addVariable(domain, guarded, LabelStr(#name))

#define ruleVariable(domain) addVariable(domain, false, makeImplicitVariableName())

  /**
   * @brief Allocates a local loop variable in code for 'foreach' loop
   */
#define loopVar(domain, value)\
{\
  ObjectDomain loop_##t(#domain);\
  loop_##t.insert(value);\
  loop_##t.close();\
  addVariable(loop_##t, false, #value);\
}

#define predicateVariable(domain) allocateVariable(getPlanDatabase()->getConstraintEngine(), m_pseudoVariables, domain, getId())

  /**
   * @brief Function to detokenize a delimited list into a list. List is an internal static
   * and is reset each time.
   */
  const std::list<edouble>& listFromString(const std::string& str, bool isNumeric);

  /**
   * @brief Function to allocate a token on the same object as the master
   */
  TokenId allocateOnSameObject(const TokenId& parent, const LabelStr& predicateSuffix, const LabelStr& relationToMaster);

  /**
   * Macro declaration and definition for the Rule which provides a factory
   * for apparopriate RuleInstances
   */
#define DECLARE_AND_DEFINE_RULE(RuleName, RuleInstanceName, Predicate, Source)\
  class RuleName: public Rule {\
  public:\
    friend class RuleInstanceName;\
    RuleName(): Rule(LabelStr(#Predicate), LabelStr(#Source)){}\
    RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb, const RulesEngineId &rulesEngine) const{\
      RuleInstanceName *foo = new RuleInstanceName(m_id, token, planDb);\
      foo->setRulesEngine(rulesEngine);\
      return foo->getId();\
    }\
  };

/**
 * Called within the context of a rule
 */
#define slave(klass, nddl_type, name, relation) addSlave(new klass(m_token, LabelStr(#nddl_type), relation, true), LabelStr(#name));

#define localSlave(predicate, name, relation) addSlave(allocateOnSameObject(m_token, #predicate, relation), LabelStr(#name));

/**
 * Called within the context of a rule, for a subgoal to be restricted to the same object as the parent
 */

#define sameObject(objectVar, label)  {\
 std::vector<ConstrainedVariableId> objectVar##label##vars;\
 objectVar##label##vars.push_back(var(getId(), #objectVar));\
 objectVar##label##vars.push_back(getSlave(LabelStr(#label))->getObject());\
 rule_constraint(eq,objectVar##label##vars);\
}

#define constrainObject(objectVar, suffix, label)  {\
 std::vector<ConstrainedVariableId> objectVar##label##vars;\
 objectVar##label##vars.push_back(varFromObject(std::string(#objectVar), std::string(#suffix)));\
 objectVar##label##vars.push_back(getSlave(LabelStr(#label))->getObject());\
 rule_constraint(eq,objectVar##label##vars);\
}

/**
 * Shorthand to access the singleton object from a variable whose base domain must be a singleton/
 */
#define singleton(var) var->baseDomain().getSingletonValue()

/**
 * Shortand for allocating an object variable as a local rule variable
 */
#define objectVar(type, name, guarded, leaveOpen)\
ConstrainedVariableId name = addVariable(ObjectDomain(#type), guarded, #name);\
getPlanDatabase()->makeObjectVariableFromType(#type, name, !guarded && leaveOpen);

/**
 * Shorthand for populating and synchronizing a parmater variable of objects
 */
#define completeObjectParam(type, name)\
{\
  getPlanDatabase()->makeObjectVariableFromType(LabelStr(#type), name);\
}

/**
 * For constraints invoked in the predicate definition
 */
#define token_constraint(name, vars)\
{\
  ConstraintId c0 = m_planDatabase->getConstraintEngine()->createConstraint(LabelStr(#name),vars);\
  m_standardConstraints.insert(c0);\
}

#define rule_constraint(name, vars)\
{\
  addConstraint(LabelStr(#name), vars);\
}

/**
 * Macro definitions for temporal relations
 */

#define meets(origin, target)  relation(concurrent, origin, end, target, start)
#define met_by(origin, target) relation(concurrent, target, end, origin, start)
#define contains(origin, target)\
{\
  relation(precedes, origin, start, target, start)\
  relation(precedes, target, end, origin, end)\
}
#define contained_by(origin, target)\
{\
  relation(precedes, target, start, origin, start)\
  relation(precedes, origin, end, target, end)\
}
#define before(origin, target) relation(precedes, origin, end, target, start)
#define after(origin, target) relation(precedes, target, end, origin, start)
#define starts(origin, target) relation(concurrent, origin, start, target, start)
#define ends(origin, target) relation(concurrent, origin, end, target, end)
#define ends_after(origin, target) relation(precedes, target, end, origin, end)
#define ends_before(origin, target) ends_after(target, origin)
#define ends_after_start(origin, target) relation(precedes, target, start, origin, end)
#define starts_before_end(origin, target) relation(precedes, origin, start, target, end)
#define starts_during(origin, target)\
{\
   relation(precedes, target, start, origin, start)\
   relation(precedes, origin, start, target, end)\
}

#define contains_start(origin, target)\
{\
  relation(precedes, origin, start, target, start)\
  relation(precedes, target, start, origin, end)\
}
#define ends_during(origin, target)\
{\
  relation(precedes, target, start, origin, end)\
  relation(precedes, origin, end, target, end)\
}
#define contains_end(origin, target)\
{\
  relation(precedes, origin, start, target, end)\
  relation(precedes, target, end, origin, end)\
}
#define starts_after(origin, target) relation(precedes, target, start, origin, start)
#define starts_before(origin, target) relation(precedes, origin, start, target, start)
#define equals(origin, target)\
{\
  relation(concurrent, origin, start, target, start)\
  relation(concurrent, origin, end, target, end)\
}
#define equal(origin, target) equals(origin, target)

#define relation(relationname, origin, originvar, target, targetvar) {\
 std::vector<ConstrainedVariableId> origin##target##vars;\
 origin##target##vars.push_back(getSlave(LabelStr(#origin))->originvar());\
 origin##target##vars.push_back(getSlave(LabelStr(#target))->targetvar());\
 rule_constraint(relationname,origin##target##vars);\
  } 

} // namespace NDDL

#endif // NDDL_RULES_HH
