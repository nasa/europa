#ifndef _H_ResourceTransactionConstraint
#define _H_ResourceTransactionConstraint

#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"

namespace Prototype {

  class ResourceTransactionConstraint: public Constraint
  {
  public:
    ResourceTransactionConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex, 
		   const DomainListener::ChangeType& changeType);
  private:
    static const int TO = 0;
    static const int TH = 1;
    static const int TQ = 2;
    static const int ARG_COUNT = 3;
  };

} //namespace prototype

#endif
