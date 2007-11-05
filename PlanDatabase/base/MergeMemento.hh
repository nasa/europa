#ifndef _H_MergeMemento
#define _H_MergeMemento

#include "PlanDatabaseDefs.hh"
#include "Token.hh"
//#include "TokenVariable.hh"
#include "LabelStr.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"
#include <list>
#include <vector>

namespace EUROPA{

  class MergeMemento {
  public:
    virtual ~MergeMemento();
  protected:
    friend class UnifyMemento;
    MergeMemento(const TokenId& inactiveToken, const TokenId& activeToken);
    void undo(bool activeTokenDeleted);

    /**
     * @brief A constraint has been placed on the variable of a merged token. This will in practice occur if
     * constraints are posted in guards on slaves introduced outside of the guard
     */
    void handleAdditionOfInactiveConstraint(const ConstraintId& constraint);

    /**
     * @brief A Constraint may have been placed on a variable of an INACTIVE token priot to merging.
     * It must be possible to undo this decision, which will require removal of equivalent constraint which
     * was created on the active token.
     */
    void handleRemovalOfInactiveConstraint(const ConstraintId& constraint);

    /**
     * @brief Helper method to migrate a constraint to the active token. Will not migrate constraints impacting state variable
     */
    void migrateConstraint(const ConstraintId& constraint);

    const TokenId m_inactiveToken;
    const TokenId m_activeToken;

    std::list<ConstraintId> m_deactivatedConstraints;
    std::list<ConstraintId> m_newConstraints;
    bool m_undoing;
  };
}

#endif
