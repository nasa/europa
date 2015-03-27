#ifndef H_HasAncestorConstraint
#define H_HasAncestorConstraint

#include "PlanDatabaseDefs.hh"
#include "Constraint.hh"
#include "ConstraintTypeChecking.hh"

namespace EUROPA
{
class ObjectDomain;
/**
 * @brief Requires that both objects are singletons before it will execute
 */
class HasAncestorConstraint: public Constraint
{
 public:

  HasAncestorConstraint(const std::string& name,
			const std::string& propagatorName,
			const ConstraintEngineId constraintEngine,
			const std::vector<ConstrainedVariableId>& variables);

  ~HasAncestorConstraint();

  void handleExecute();

  void handleExecute(const ConstrainedVariableId variable, 
                     unsigned int argIndex, 
                     const DomainListener::ChangeType& changeType);

  bool canIgnore(const ConstrainedVariableId variable, 
                 unsigned int argIndex,
                 const DomainListener::ChangeType& changeType);
 private:
  void apply();
  ObjectDomain& m_first;
  const ObjectDomain& m_restrictions; /*!<Ancestors must be present in this set */
};
typedef DataTypeCheck<HasAncestorConstraint, And<NArgs<2>, All<IsEntity> > > HasAncestorCT;
}
#endif
