#ifndef _H_ResourceConstraint
#define _H_ResourceConstraint

#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"

namespace Prototype {

  class ResourceConstraint: public Constraint
  {
  public:
    ResourceConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex, 
		   const DomainListener::ChangeType& changeType);
  private:
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int ARG_COUNT = 3;
  };

} //namespace prototype

#endif
