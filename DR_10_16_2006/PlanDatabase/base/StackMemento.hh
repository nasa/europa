#ifndef _H_StackMemento
#define _H_StackMemento

#include "PlanDatabaseDefs.hh"
#include "Token.hh"
//#include "TokenVariable.hh"
#include "LabelStr.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"
#include <list>
#include <vector>

namespace EUROPA {
  class StackMemento {
  public:
    virtual ~StackMemento();
  private:
    friend class UnifyMemento;
    StackMemento(const TokenId& activeTokenToStack, const TokenId& activeToken);
    void undo(bool activeTokenDeleted);

    void handleRemovalOfInactiveConstraint(const ConstraintId& constraint);
    const TokenId m_activeTokenToStack;
    const TokenId m_activeToken;
    std::list<ConstraintId> m_stackConstraints;
  };

  typedef Id<StackMemento> StackMementoId;
}

#endif
