#ifndef _H_UnifyMemento
#define _H_UnifyMemento

#include "PlanDatabaseDefs.hh"
//#include "TokenVariable.hh"
#include "MergeMemento.hh"
#include "StackMemento.hh"
#include "LabelStr.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {

  class UnifyMemento {
  public:
    virtual ~UnifyMemento();
  protected:
    friend class Token;

    enum UnifyMethod {
      mergeMethod = 0,
      stackMethod
    };

    UnifyMemento();
    UnifyMemento(const TokenId& inactiveToken, const TokenId& activeToken);

    void undo(bool activeTokenDeleted);

    void handleAdditionOfInactiveConstraint(const ConstraintId& constraint);
    void handleRemovalOfInactiveConstraint(const ConstraintId& constraint);

  private:
    UnifyMethod m_method;
    MergeMementoId m_mm;
    StackMementoId m_sm;
  };
}

#endif
