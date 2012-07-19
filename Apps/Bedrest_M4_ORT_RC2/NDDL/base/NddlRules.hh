#ifndef NDDL_RULES_HH
#define NDDL_RULES_HH

#include "NddlDefs.hh"
#include "Rule.hh"
#include "RuleInstance.hh"

namespace NDDL {

  /**************** SUPPORT FOR INTEGRATION TO RULES ENGINE *********************/

#define localVar(domain, name, guarded) addVariable(domain, guarded, LabelStr(#name))

#define ruleVariable(domain) addVariable(domain, false, LabelStr("PSEUDO_VARIABLE"))

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
  const std::list<double>& listFromString(const std::string& str, bool isNumeric);

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
  ConstraintId c0 = ConstraintLibrary::createConstraint(LabelStr(#name),\
							m_planDatabase->getConstraintEngine(),\
							vars);\
  m_standardConstraints.insert(c0);\
}

#define rule_constraint(name, vars)\
{\
  addConstraint(LabelStr(#name), vars);\
}

/**
 * Macro definitions for temporal relations
 */

#define meets(origin, target)  relation(concurrent, origin, End, target, Start)
#define met_by(origin, target) relation(concurrent, target, End, origin, Start)
#define contains(origin, target)\
{\
  relation(precedes, origin, Start, target, Start)\
  relation(precedes, target, End, origin, End)\
}
#define contained_by(origin, target)\
{\
  relation(precedes, target, Start, origin, Start)\
  relation(precedes, origin, End, target, End)\
}
#define before(origin, target) relation(precedes, origin, End, target, Start)
#define after(origin, target) relation(precedes, target, End, origin, Start)
#define starts(origin, target) relation(concurrent, origin, Start, target, Start)
#define ends(origin, target) relation(concurrent, origin, End, target, End)
#define ends_after(origin, target) relation(precedes, target, End, origin, End)
#define ends_before(origin, target) ends_after(target, origin)
#define ends_after_start(origin, target) relation(precedes, target, Start, origin, End)
#define starts_before_end(origin, target) relation(precedes, origin, Start, target, End)
#define starts_during(origin, target)\
{\
   relation(precedes, target, Start, origin, Start)\
   relation(precedes, origin, Start, target, End)\
}

#define contains_start(origin, target)\
{\
  relation(precedes, origin, Start, target, Start)\
  relation(precedes, target, Start, origin, End)\
}
#define ends_during(origin, target)\
{\
  relation(precedes, target, Start, origin, End)\
  relation(precedes, origin, End, target, End)\
}
#define contains_end(origin, target)\
{\
  relation(precedes, origin, Start, target, End)\
  relation(precedes, target, End, origin, End)\
}
#define starts_after(origin, target) relation(precedes, target, Start, origin, Start)
#define starts_before(origin, target) relation(precedes, origin, Start, target, Start)
#define equals(origin, target)\
{\
  relation(concurrent, origin, Start, target, Start)\
  relation(concurrent, origin, End, target, End)\
}
#define equal(origin, target) equals(origin, target)

#define relation(relationname, origin, originvar, target, targetvar) {\
 std::vector<ConstrainedVariableId> origin##target##vars;\
 origin##target##vars.push_back(getSlave(LabelStr(#origin))->get##originvar());\
 origin##target##vars.push_back(getSlave(LabelStr(#target))->get##targetvar());\
 rule_constraint(relationname,origin##target##vars);\
  } 

} // namespace NDDL

#endif // NDDL_RULES_HH
