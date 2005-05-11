#include "Filters.hh"
#include "Resource.hh"
#include "SolverUtils.hh"
#include "Debug.hh"
#include "PlanDatabase.hh"

namespace EUROPA {
  namespace SOLVERS {

    SingletonFilter::SingletonFilter(const TiXmlElement& configData)
      : VariableMatchingRule(configData) {}

    bool SingletonFilter::matches(const ConstrainedVariableId& var, const LabelStr& objectType, const LabelStr& predicate) const{
      if(!VariableMatchingRule::matches(var, objectType, predicate))
	return false;
  
      debugMsg("SingletonFilter:matches", "Evaluating " << var->toString() << " for singleton filter.");

      // Indicate a match if it is not a singleton
      return !var->lastDomain().isSingleton();
    }

  }
}
