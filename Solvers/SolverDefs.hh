#ifndef H_SolverDefs
#define H_SolverDefs

#include "PlanDatabaseDefs.hh"
#include "Entity.hh"

#include <vector>

using namespace EUROPA;

namespace EUROPA {
  namespace SOLVERS {

    class Component;
    typedef Id<Component> ComponentId;

    class FlawManager;
    typedef Id<FlawManager> FlawManagerId;
    typedef std::list<FlawManagerId> FlawManagers;

    class MatchingRule;
    typedef Id<MatchingRule> MatchingRuleId;

    class DecisionPoint;
    typedef Id<DecisionPoint> DecisionPointId;

    class Solver;
    typedef Id<Solver> SolverId;

    typedef std::vector<DecisionPointId> DecisionStack;

    class FilterCondition;
    typedef Id<FilterCondition> FilterConditionId;

  }
}
#endif
