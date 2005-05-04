#ifndef H_SolverDefs
#define H_SolverDefs

#include "PlanDatabaseDefs.hh"
#include "Entity.hh"

#define TIXML_USE_STL
#include "tinyxml.h"

#include <vector>

using namespace EUROPA;

namespace EUROPA {

  class Component;
  typedef Id<Component> ComponentId;

  class FlawManager;
  typedef Id<FlawManager> FlawManagerId;
  typedef std::set<FlawManagerId, EntityComparator<FlawManagerId> > FlawManagers;

  class DecisionPoint;
  typedef Id<DecisionPoint> DecisionPointId;

  class Solver;
  typedef Id<Solver> SolverId;

  typedef std::vector<DecisionPointId> DecisionStack;

  /** Specialized Factories **/
  class VariableDecisionPointFactory;
  typedef Id<VariableDecisionPointFactory> VariableDecisionPointFactoryId;

  class FilterCondition;
  typedef Id<FilterCondition> FilterConditionId;
}
#endif
