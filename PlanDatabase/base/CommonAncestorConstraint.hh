#ifndef _H_CommonAncestorConstraint
#define _H_CommonAncestorConstraint

/**
 * @file CommonAncestor.hh
 * @author Conor McGann
 * @date Spring, 2004
 */

#include "PlanDatabaseDefs.hh"
#include "Constraint.hh"

namespace EUROPA
{
  /**
   * @brief Requires that the first and second objects have a common ancestor that is
   * contained in the restricted set.
   *
   * The semantics for this are such that:
   * exists f in m_first AND
   * exists s in m_second AND
   * exists r in m_restrictions
   * such that:
   * ancestor(r, f) AND
   * ancestor(r, s).
   */
  class CommonAncestorConstraint: public Constraint
  {
  public:

    CommonAncestorConstraint(const LabelStr& name,
			const LabelStr& propagatorName,
			const ConstraintEngineId& constraintEngine,
			const std::vector<ConstrainedVariableId>& variables);

    ~CommonAncestorConstraint();

    void handleExecute();

    void handleExecute(const ConstrainedVariableId& variable, 
		       int argIndex, 
		       const DomainListener::ChangeType& changeType);

    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex,
		   const DomainListener::ChangeType& changeType);
  private:
    void apply(ObjectDomain& singleton, ObjectDomain& other);
    ObjectDomain& m_first; /*!< The first object set */
    ObjectDomain& m_second; /*!< The second object set */
    const ObjectDomain& m_restrictions; /*!<Ancestors must be present in this restricted set */
  };
}
#endif
