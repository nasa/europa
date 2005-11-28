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

    class MatchingEngine;
    typedef Id<MatchingEngine> MatchingEngineId;

    class MatchingRule;
    typedef Id<MatchingRule> MatchingRuleId;

    class FlawFilter;
    typedef Id<FlawFilter> FlawFilterId;

    class FlawManager;
    typedef Id<FlawManager> FlawManagerId;
    typedef std::list<FlawManagerId> FlawManagers;

    class DecisionPoint;
    typedef Id<DecisionPoint> DecisionPointId;

    class Solver;
    typedef Id<Solver> SolverId;

    typedef std::vector<DecisionPointId> DecisionStack;

    /**
     * @brief Defines a configuration class
     */
    class SolverConfig{
    public:
      SolverConfig();
    };
  }
}
#endif
