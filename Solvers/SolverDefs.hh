#ifndef H_SolverDefs
#define H_SolverDefs

#include "PlanDatabaseDefs.hh"
#include "Entity.hh"

#define TIXML_USE_STL
#include "tinyxml.h"

#include <vector>

using namespace EUROPA;

namespace EUROPA {
  namespace SOLVERS {

    class Component;
    typedef Id<Component> ComponentId;

    class FlawManager;
    typedef Id<FlawManager> FlawManagerId;
    typedef std::list<FlawManagerId> FlawManagers;

    class VariableMatchingRule;
    typedef Id<VariableMatchingRule> VariableMatchingRuleId;

    class TokenMatchingRule;
    typedef Id<TokenMatchingRule> TokenMatchingRuleId;

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
