#ifndef _H_Constraints
#define _H_Constraints

#include "UnaryConstraint.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"

namespace Prototype {

  class AddEqualConstraint: public Constraint {
  public:
    AddEqualConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int ARG_COUNT = 3;
  };

  class EqualConstraint: public Constraint {
  public:
    EqualConstraint(const LabelStr& name,
		    const LabelStr& propagatorName,
		    const ConstraintEngineId& constraintEngine,
		    const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

    /**
     * @brief Accessor required for EquilityConstraintPropagator.
     */
    static AbstractDomain& getCurrentDomain(const ConstrainedVariableId& var);

  private:
    static const int X = 0;
    static const int Y = 1;
    static const int ARG_COUNT = 2;
  };

  class SubsetOfConstraint : public UnaryConstraint {
  public:
    SubsetOfConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const ConstrainedVariableId& variable,
		       const AbstractDomain& superset);

    ~SubsetOfConstraint();

    void handleExecute();

    bool canIgnore(const ConstrainedVariableId& variable,
		   int argIndex,
		   const DomainListener::ChangeType& changeType);

    int executionCount() const;

    const AbstractDomain& getDomain() const;

  private:
    bool m_isDirty;
    AbstractDomain& m_currentDomain;
    AbstractDomain* m_superSetDomain;
    int m_executionCount;
  };

  class LessThanEqualConstraint : public Constraint {
  public:
    LessThanEqualConstraint(const LabelStr& name,
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
    static const int ARG_COUNT = 2;
  };

  class NotEqualConstraint: public Constraint {
  public:
    NotEqualConstraint(const LabelStr& name,
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
    static const int ARG_COUNT = 2;
  };

  class MultEqualConstraint: public Constraint {
  public:
    MultEqualConstraint(const LabelStr& name,
			const LabelStr& propagatorName,
			const ConstraintEngineId& constraintEngine,
			const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int ARG_COUNT = 3;
  };

  class AddMultEqualConstraint: public Constraint {
  public:
    AddMultEqualConstraint(const LabelStr& name,
			   const LabelStr& propagatorName,
			   const ConstraintEngineId& constraintEngine,
			   const std::vector<ConstrainedVariableId>& variables);

    // All the work is done by the member constraints
    inline void handleExecute() { }

  private:
    static const int A = 0;
    static const int B = 1;
    static const int C = 2;
    static const int D = 3;
    static const int ARG_COUNT = 4;

    Variable<IntervalDomain> m_interimVariable;
    MultEqualConstraint m_multEqualConstraint;
    AddEqualConstraint m_addEqualConstraint;
  };

  /**
   * @class EqSumConstraint
   * @brief A = B + C where B and C can each be sums.
   * Converted into an AddEqualConstraint and/or two EqSumConstraints with fewer variables.
   */
  class EqualSumConstraint : public Constraint {
  public:
    EqualSumConstraint(const LabelStr& name,
                       const LabelStr& propagatorName,
                       const ConstraintEngineId& constraintEngine,
                       const std::vector<ConstrainedVariableId>& variables);

    ~EqualSumConstraint();

    // All the work is done by the member constraints
    inline void handleExecute() { }

  private:
    const int ARG_COUNT;

    ConstraintId m_eqSumC1, m_eqSumC2, m_eqSumC3, m_eqSumC4, m_eqSumC5;
    Variable<IntervalDomain> m_sum1, m_sum2, m_sum3, m_sum4;
  };

  class LessOrEqThanSumConstraint : public Constraint {
  public:
    LessOrEqThanSumConstraint(const LabelStr& name,
                              const LabelStr& propagatorName,
                              const ConstraintEngineId& constraintEngine,
                              const std::vector<ConstrainedVariableId>& variables);

    // All the work is done by the member constraints
    inline void handleExecute() { }

  private:
    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int ARG_COUNT = 3;

    Variable<IntervalDomain> m_interimVariable;
    LessThanEqualConstraint m_lessThanEqualConstraint;
    AddEqualConstraint m_addEqualConstraint;
  };
}
#endif
