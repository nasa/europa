#ifndef _H_Constraints
#define _H_Constraints

#include "Constraint.hh"

namespace Prototype
{
  class AddEqualConstraint: public Constraint
  {
  public:
    AddEqualConstraint(const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables,
		       const LabelStr& name = LabelStr("AddEqual"));

    void handleExecute();

    void handleExecute(const ConstrainedVariableId& variable, 
		       int argIndex, 
		       const DomainListener::ChangeType& changeType);

  private:
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int ARG_COUNT = 3;
  };

  class EqualConstraint: public Constraint
  {
  public:
    EqualConstraint(const ConstraintEngineId& constraintEngine,
		    const std::vector<ConstrainedVariableId>& variables,
		    const LabelStr& name = LabelStr("Equal"));

    void handleExecute();

    void handleExecute(const ConstrainedVariableId& variable, 
		       int argIndex, 
		       const DomainListener::ChangeType& changeType);

    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex, 
		   const DomainListener::ChangeType& changeType);
    /**
     * @brief Accessor required for EquilityConstraintPropagator
     */
    static AbstractDomain& getCurrentDomain(const ConstrainedVariableId& var);

  private:
    int m_lastNotified;
    static const int X = 0;
    static const int Y = 1;
    static const int ARG_COUNT = 2;
  };

  class SubsetOfConstraint: public Constraint{
  public:
    SubsetOfConstraint(const ConstraintEngineId& constraintEngine,
		       const ConstrainedVariableId& variable,
		       const AbstractDomain& superset,
		       const LabelStr& name = LabelStr("SubsetOf"));

    ~SubsetOfConstraint();

    void handleExecute();

    void handleExecute(const ConstrainedVariableId& variable, 
		       int argIndex,
		       const DomainListener::ChangeType& changeType);

    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex, 
		   const DomainListener::ChangeType& changeType);

    int executionCount() const;
  private:
    bool m_isDirty;
    AbstractDomain& m_currentDomain;
    AbstractDomain* m_superSetDomain;
    int m_executionCount;
  };
}
#endif
