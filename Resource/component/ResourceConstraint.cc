#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "Domain.hh"
#include "Utils.hh"
#include "ResourceConstraint.hh"

namespace Prototype
{
  
  ResourceConstraint::ResourceConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);
    //@todo add type checking of each variable in the constraint
  }

  void ResourceConstraint::handleExecute()
  {
  }

  bool ResourceConstraint::canIgnore(const ConstrainedVariableId& variable, 
				     int argIndex, 
				     const DomainListener::ChangeType& changeType){
    // if it is a restriction, but not a singleton, then we can ignore it.
    if(changeType != DomainListener::RESET && changeType != DomainListener::RELAXED)
      return !getCurrentDomain(variable).isSingleton();
  }

}//namespace prototype
