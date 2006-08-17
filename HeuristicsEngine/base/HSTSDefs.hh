#ifndef H_HSTSDefs
#define H_HSTSDefs

#include "PlanDatabaseDefs.hh"


/**
 * @brief Forward declares classes and Id's
 * @author Conor McGann
 */
namespace EUROPA {

  typedef double Priority; /*!< Used to reference to the priority used in calculating heuristics. */

  class HeuristicsEngine;
  typedef Id<HeuristicsEngine> HeuristicsEngineId;

  class Heuristic;
  typedef Id<Heuristic> HeuristicId;

  class VariableHeuristic;
  typedef Id<VariableHeuristic> VariableHeuristicId;

  class TokenHeuristic;
  typedef Id<TokenHeuristic> TokenHeuristicId;

  class HeuristicInstance;
  typedef Id<HeuristicInstance> HeuristicInstanceId;

  /**
   * @brief Relation used when allocating a slave. Could be restricted by a class down the road.
   */
  typedef LabelStr MasterRelation;

  /**
   * @brief Used to store guard entry data - var index and expected value to match on.
   */
  typedef std::pair<unsigned int, double> GuardEntry;

}
#endif
