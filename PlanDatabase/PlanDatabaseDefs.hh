#ifndef _H_PlanDatabaseDefs
#define _H_PlanDatabaseDefs

#include "../ConstraintEngine/ConstraintEngineDefs.hh"
#include "../ConstraintEngine/Domain.hh"

namespace Prototype {
  class Object;
  typedef Id<Object> ObjectId;

  class Timeline;
  typedef Id<Timeline> TimelineId;

  class Token;
  typedef Id<Token> TokenId;

  class UnifyMemento;
  typedef Id<UnifyMemento> UnifyMementoId;

  class MergeMemento;
  typedef Id<MergeMemento> MergeMementoId;

  class StackMemento;
  typedef Id<StackMemento> StackMementoId;

  class PlanDatabaseListener;
  typedef Id<PlanDatabaseListener> PlanDatabaseListenerId;

  template<class DomainType> class TokenVariable;
  typedef Id< TokenVariable<EnumeratedDomain> > StateVarId;
  typedef Id< TokenVariable<IntervalIntDomain> > TempVarId;

  typedef Domain<ObjectId>  ObjectSet;
  typedef Id< TokenVariable<ObjectSet> > ObjectVarId;

  class Schema;
  typedef Id<Schema> SchemaId;

  class PlanDatabase;
  typedef Id<PlanDatabase> PlanDatabaseId;

  class RulesEngine;
  typedef Id<RulesEngine> RulesEngineId;

  class Rule;
  typedef Id<Rule> RuleId;

  class RuleContext;
  typedef Id<RuleContext> RuleContextId;

  class RuleInstance;
  typedef Id<RuleInstance> RuleInstanceId;
}

#endif
