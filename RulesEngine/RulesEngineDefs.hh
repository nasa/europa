#ifndef _H_RulesEngineDefs
#define _H_RulesEngineDefs

#include "PlanDatabaseDefs.hh"

namespace PLASMA {

  class Rule;
  typedef Id<Rule> RuleId;

  class RulesEngine;
  typedef Id<RulesEngine> RulesEngineId;

  class RuleInstance;
  typedef Id<RuleInstance> RuleInstanceId;

  class RuleContext;
  typedef Id<RuleContext> RuleContextId;

  class RuleVariableListener;
  typedef Id<RuleVariableListener> RuleVariableListenerId;

  class RulesEngineListener;
  typedef Id<RulesEngineListener> RulesEngineListenerId;

}
#endif
