#ifndef _H_CBPlannerDefs
#define _H_CBPlannerDefs

#include "CommonDefs.hh"
#include "PlanDatabaseDefs.hh"
#include <cmath>

namespace Prototype {

  class Condition;
  typedef Id<Condition> ConditionId;

  class Horizon;
  typedef Id<Horizon> HorizonId;

  class HorizonCondition;
  typedef Id<HorizonCondition> HorizonConditionId;

  class TemporalVariableCondition;
  typedef Id<TemporalVariableCondition> TemporalVariableConditionId;

  class DynamicInfiniteRealCondition;
  typedef Id<DynamicInfiniteRealCondition> DynamicInfiniteRealConditionId;

  class DecisionManager;
  typedef Id<DecisionManager> DecisionManagerId;

  class Choice;
  typedef Id<Choice> ChoiceId;

  class DecisionPoint;
  typedef Id<DecisionPoint> DecisionPointId;

  class TokenDecisionPoint;
  typedef Id<TokenDecisionPoint> TokenDecisionPointId;
  typedef std::set<TokenDecisionPointId, EntityComparator<TokenDecisionPointId> > TokenDecisionSet;

  class ObjectDecisionPoint;
  typedef Id<ObjectDecisionPoint> ObjectDecisionPointId;
  class ObjectDecisionPointComparator;
  typedef std::set<ObjectDecisionPointId, ObjectDecisionPointComparator> ObjectDecisionSet;

  class ConstrainedVariableDecisionPoint;
  typedef Id<ConstrainedVariableDecisionPoint> ConstrainedVariableDecisionPointId;
  typedef std::set<ConstrainedVariableDecisionPointId, EntityComparator<ConstrainedVariableDecisionPointId> > VariableDecisionSet;

  class CBPlanner;
  typedef Id<CBPlanner> CBPlannerId;

  class DecisionManagerListener;
  typedef Id<DecisionManagerListener> DecisionManagerListenerId;

}

#endif
