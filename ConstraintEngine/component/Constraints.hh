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

  class LessThanConstraint : public Constraint {
  public:
    LessThanConstraint(const LabelStr& name,
                       const LabelStr& propagatorName,
                       const ConstraintEngineId& constraintEngine,
                       const std::vector<ConstrainedVariableId>& variables);

    // All the work is done by the member constraints
    inline void handleExecute() { }

  private:
    LessThanEqualConstraint m_lessThanEqualConstraint;
    NotEqualConstraint m_notEqualConstraint;
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
   * @class EqualSumConstraint
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
    const unsigned int ARG_COUNT;

    ConstraintId m_eqSumC1, m_eqSumC2, m_eqSumC3, m_eqSumC4, m_eqSumC5;
    Variable<IntervalDomain> m_sum1, m_sum2, m_sum3, m_sum4;
  };

  /**
   * @class EqualProductConstraint
   * @brief A = B * C where B and C can each be products.
   * Converted into an AddEqualConstraint and/or two EqProductConstraints with fewer variables.
   */
  class EqualProductConstraint : public Constraint {
  public:
    EqualProductConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~EqualProductConstraint();

    // All the work is done by the member constraints
    inline void handleExecute() { }

  private:
    const unsigned int ARG_COUNT;

    ConstraintId m_eqProductC1, m_eqProductC2, m_eqProductC3, m_eqProductC4, m_eqProductC5;
    Variable<IntervalDomain> m_product1, m_product2, m_product3, m_product4;
  };

  /**
   * @class LessOrEqThanSumConstraint
   * @brief A <= B + C + ...
   * Converted into two constraints: A <= temp and temp equal to the sum of the rest.
   */
  class LessOrEqThanSumConstraint : public Constraint {
  public:
    LessOrEqThanSumConstraint(const LabelStr& name,
                              const LabelStr& propagatorName,
                              const ConstraintEngineId& constraintEngine,
                              const std::vector<ConstrainedVariableId>& variables);

    ~LessOrEqThanSumConstraint() {
      delete (Constraint*) m_eqSumConstraint;
    }

    // All the work is done by the member constraints
    inline void handleExecute() { }

  private:
    Variable<IntervalDomain> m_interimVariable;
    LessThanEqualConstraint m_lessOrEqualConstraint;
    ConstraintId m_eqSumConstraint;
  };

  /**
   * @class LessThanSumConstraint
   * @brief A < B + C + ...
   * Converted into two constraints: A < temp and temp equal to the sum of the rest.
   */
  class LessThanSumConstraint : public Constraint {
  public:
    LessThanSumConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~LessThanSumConstraint() {
      delete (Constraint*) m_eqSumConstraint;
    }

    // All the work is done by the member constraints
    inline void handleExecute() { }

  private:
    Variable<IntervalDomain> m_interimVariable;
    LessThanConstraint m_lessThanConstraint;
    ConstraintId m_eqSumConstraint;
  };

  /**
   * @class CondAllSame
   * @brief If A, then B == C && B == D && C == D && ... ; if not A, then !(B == C && B == D && C == D && ...).
   */
  class CondAllSameConstraint : public Constraint {
  public:
    CondAllSameConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~CondAllSameConstraint() { }

    void handleExecute();

  private:
    const unsigned int ARG_COUNT;
  };

  /**
   * @class CondAllDiff
   * @brief If A, then B != C && B != D && C != D && ... ; if not A, then !(B != C && B != D && C != D && ...).
   */
  class CondAllDiffConstraint : public Constraint {
  public:
    CondAllDiffConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~CondAllDiffConstraint() { }

    void handleExecute();

  private:
    const unsigned int ARG_COUNT;
  };

  /**
   * @class MemberImplyConstraint
   * @brief If A is subset of B, then require that C is subset of D.
   */
  class MemberImplyConstraint : public Constraint {
  public:
    MemberImplyConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~MemberImplyConstraint() { }

    void handleExecute();

  private:
    const unsigned int ARG_COUNT;
  };
}
#endif
