#ifndef _H_PlanDatabaseDefs
#define _H_PlanDatabaseDefs

#include "../ConstraintEngine/ConstraintEngineDefs.hh"
#include "../ConstraintEngine/Domain.hh"

namespace Prototype {
  class Object;
  typedef Europa::Id<Object> ObjectId;

  class Timeline;
  typedef Europa::Id<Timeline> TimelineId;

  class Token;
  typedef Europa::Id<Token> TokenId;

  class UnifyMemento;
  typedef Europa::Id<UnifyMemento> UnifyMementoId;

  class MergeMemento;
  typedef Europa::Id<MergeMemento> MergeMementoId;

  class StackMemento;
  typedef Europa::Id<StackMemento> StackMementoId;

  class PlanDatabaseListener;
  typedef Europa::Id<PlanDatabaseListener> PlanDatabaseListenerId;

  template<class DomainType> class TokenVariable;
  typedef Europa::Id< TokenVariable<EnumeratedDomain> > StateVarId;
  typedef Europa::Id< TokenVariable<IntervalIntDomain> > TempVarId;

  typedef Domain<ObjectId>  ObjectSet;
  typedef Europa::Id< TokenVariable<ObjectSet> > ObjectVarId;

  class Schema;
  typedef Europa::Id<Schema> SchemaId;

  class PlanDatabase;
  typedef Europa::Id<PlanDatabase> PlanDatabaseId;

  class RulesEngine;
  typedef Europa::Id<RulesEngine> RulesEngineId;

  class Rule;
  typedef Europa::Id<Rule> RuleId;

  class RuleContext;
  typedef Europa::Id<RuleContext> RuleContextId;

  class RuleInstance;
  typedef Europa::Id<RuleInstance> RuleInstanceId;
}

#endif
