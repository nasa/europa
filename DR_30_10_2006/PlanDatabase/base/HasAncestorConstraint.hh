#ifndef _H_HasAncestorConstraint
#define _H_HasAncestorConstraint

#include "PlanDatabaseDefs.hh"
#include "Constraint.hh"

namespace EUROPA
{
  /**
   * @brief Requires that both objects are singletons before it will execute
   */
  class HasAncestorConstraint: public Constraint
  {
  public:

    HasAncestorConstraint(const LabelStr& name,
			const LabelStr& propagatorName,
			const ConstraintEngineId& constraintEngine,
			const std::vector<ConstrainedVariableId>& variables);

    ~HasAncestorConstraint();

    void handleExecute();

    void handleExecute(const ConstrainedVariableId& variable, 
		       int argIndex, 
		       const DomainListener::ChangeType& changeType);

    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex,
		   const DomainListener::ChangeType& changeType);
  private:
    void apply();
    ObjectDomain& m_first;
    const ObjectDomain& m_restrictions; /*!<Ancestors must be present in this set */
  };
}
#endif
