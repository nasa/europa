#ifndef _H_Constraints
#define _H_Constraints

#include "UnaryConstraint.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"

namespace Prototype {

  class AddEqualConstraint : public Constraint {
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

  class EqualConstraint : public Constraint {
  public:
    EqualConstraint(const LabelStr& name,
		    const LabelStr& propagatorName,
		    const ConstraintEngineId& constraintEngine,
		    const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

    /**
     * @brief Accessor required for EqualityConstraintPropagator.
     */
    static AbstractDomain& getCurrentDomain(const ConstrainedVariableId& var);
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

  class NotEqualConstraint : public Constraint {
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

  class MultEqualConstraint : public Constraint {
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

  class AddMultEqualConstraint : public Constraint {
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
   * @class GreaterOrEqThanSumConstraint
   * @brief A >= B + C + ...
   * Converted into two constraints: A >= temp and temp equal to the sum of the rest.
   */
  class GreaterOrEqThanSumConstraint : public Constraint {
  public:
    GreaterOrEqThanSumConstraint(const LabelStr& name,
                                 const LabelStr& propagatorName,
                                 const ConstraintEngineId& constraintEngine,
                                 const std::vector<ConstrainedVariableId>& variables);

    ~GreaterOrEqThanSumConstraint() {
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
   * @class GreaterThanSumConstraint
   * @brief A > B + C + ...
   * Converted into two constraints: A < temp and temp equal to the sum of the rest.
   */
  class GreaterThanSumConstraint : public Constraint {
  public:
    GreaterThanSumConstraint(const LabelStr& name,
                             const LabelStr& propagatorName,
                             const ConstraintEngineId& constraintEngine,
                             const std::vector<ConstrainedVariableId>& variables);

    ~GreaterThanSumConstraint() {
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
   * @class AddLessThanConstraint
   * @brief A + B < C
   * Converted into GreaterThanSumConstraint.
   */
  class AddLessThanConstraint : public Constraint {
  public:
    AddLessThanConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~AddLessThanConstraint() {
      delete (Constraint*) m_greaterThanSumConstraint;
    }

    // All the work is done by the member constraint
    inline void handleExecute() { }

  private:
    ConstraintId m_greaterThanSumConstraint;
  };

  /**
   * @class AddLessOrEqThanConstraint
   * @brief A + B <= C
   * Converted into GreaterOrEqThanSumConstraint.
   */
  class AddLessOrEqThanConstraint : public Constraint {
  public:
    AddLessOrEqThanConstraint(const LabelStr& name,
                              const LabelStr& propagatorName,
                              const ConstraintEngineId& constraintEngine,
                              const std::vector<ConstrainedVariableId>& variables);

    ~AddLessOrEqThanConstraint() {
      delete (Constraint*) m_greaterOrEqThanSumConstraint;
    }

    // All the work is done by the member constraint
    inline void handleExecute() { }

  private:
    ConstraintId m_greaterOrEqThanSumConstraint;
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
   * @class AllDiff
   * @brief A != B && A != C && B != C && A != D && B != D && ...
   */
  class AllDiffConstraint : public Constraint {
  public:
    AllDiffConstraint(const LabelStr& name,
                      const LabelStr& propagatorName,
                      const ConstraintEngineId& constraintEngine,
                      const std::vector<ConstrainedVariableId>& variables);

    ~AllDiffConstraint() {
      delete (CondAllDiffConstraint *) m_condAllDiffConstraint;
    }

    void handleExecute() { }

  private:
    Variable<BoolDomain> m_condVar;
    ConstraintId m_condAllDiffConstraint;
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

  /**
   * @class CountZerosConstraint
   * @brief First variable is the count of the rest that can be zero.
   * @note Supports boolean domains with the usual C/C++ convention of false
   * being zero and true being non-zero.
   */
  class CountZerosConstraint : public Constraint {
  public:
    CountZerosConstraint(const LabelStr& name,
                         const LabelStr& propagatorName,
                         const ConstraintEngineId& constraintEngine,
                         const std::vector<ConstrainedVariableId>& variables);

    ~CountZerosConstraint() { }

    void handleExecute();
  };

  /**
   * @class CountNonZerosConstraint
   * @brief First variable is the count of the rest that can be non-zero.
   * @note Supports boolean domains with the usual C/C++ convention of false
   * being zero and true being non-zero.
   */
  class CountNonZerosConstraint : public Constraint {
  public:
    CountNonZerosConstraint(const LabelStr& name,
                            const LabelStr& propagatorName,
                            const ConstraintEngineId& constraintEngine,
                            const std::vector<ConstrainedVariableId>& variables);

    ~CountNonZerosConstraint() {
      delete (CountZerosConstraint*) m_countZerosConstraint;
      delete (SubsetOfConstraint*) m_subsetConstraint;
    }

    // All the work is done by the member constraints.
    inline void handleExecute() { }

  private:
    Variable<IntervalDomain> m_zeros, m_otherVars;
    AddEqualConstraint m_addEqualConstraint;
    ConstraintId m_subsetConstraint;
    ConstraintId m_countZerosConstraint;
  };

  /**
   * @class CardinalityConstraint
   * @brief First variable must be greater than or equal the count of the
   * other variables that are true.
   * @note Supports numeric domains for the other variables with the
   * usual C/C++ convention of false being zero and true being
   * any non-zero value.
   */
  class CardinalityConstraint : public Constraint {
  public:
    CardinalityConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~CardinalityConstraint() {
      delete (CountNonZerosConstraint*) m_countNonZerosConstraint;
    }

    // All the work is done by the member constraints.
    inline void handleExecute() { }

  private:
    Variable<IntervalIntDomain> m_nonZeros;
    LessThanEqualConstraint m_lessThanEqualConstraint;
    ConstraintId m_countNonZerosConstraint;
  };

  /**
   * @class OrConstraint
   * @brief At least one of the variables must be true.
   * @note Supports numeric domains for all the variables with the
   * usual C/C++ convention of false being zero and true being
   * any non-zero value.
   */
  class OrConstraint : public Constraint {
  public:
    OrConstraint(const LabelStr& name,
                 const LabelStr& propagatorName,
                 const ConstraintEngineId& constraintEngine,
                 const std::vector<ConstrainedVariableId>& variables);

    ~OrConstraint() {
      delete (CountNonZerosConstraint*) m_countNonZerosConstraint;
      delete (SubsetOfConstraint*) m_subsetConstraint;
    }

    // All the work is done by the member constraints.
    inline void handleExecute() { }

  private:
    Variable<IntervalIntDomain> m_nonZeros;
    ConstraintId m_subsetConstraint;
    ConstraintId m_countNonZerosConstraint;
  };

  /**
   * @class EqualMinimumConstraint
   * @brief First variable is the minimum value of the others.
   */
  class EqualMinimumConstraint : public Constraint {
  public:
    EqualMinimumConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~EqualMinimumConstraint() { }

    void handleExecute();
  };

  /**
   * @class MinimumEqualConstraint
   * @brief Last variable is the minimum value of the others.
   * @note Same as Europa (NewPlan) 'min' constraint.
   * @note The general "move last var to front" constraint might be fairly
   * easy to implement using ConstraintLibrary::createConstraint().
   */
  class MinimumEqualConstraint : public Constraint {
  public:
    MinimumEqualConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~MinimumEqualConstraint() {
      delete (EqualMinimumConstraint*) m_eqMinConstraint;
    }

    void handleExecute() { }

  private:
    ConstraintId m_eqMinConstraint;
  };
}
#endif
