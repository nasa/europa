#ifndef _H_Constraints
#define _H_Constraints

#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"

namespace EUROPA {

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

  private:
    const unsigned int m_argCount;
  };

  class SubsetOfConstraint : public Constraint {
  public:
    SubsetOfConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    ~SubsetOfConstraint();

    void handleExecute();

    bool canIgnore(const ConstrainedVariableId& variable,
		   int argIndex,
		   const DomainListener::ChangeType& changeType);

  private:
    AbstractDomain& m_currentDomain;
    AbstractDomain& m_superSetDomain;
  };

  class LockConstraint : public Constraint {
  public:
    LockConstraint(const LabelStr& name,
		   const LabelStr& propagatorName,
		   const ConstraintEngineId& constraintEngine,
		   const std::vector<ConstrainedVariableId>& variables);

    ~LockConstraint();

    void handleExecute();

    const AbstractDomain& getDomain() const;

  private:
    AbstractDomain& m_currentDomain;
    AbstractDomain& m_lockDomain;
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
    /**
     * @brief Helper method to do domain comparisons, and process removals if necessary
     */
    static bool checkAndRemove(const AbstractDomain& domx, AbstractDomain& domy);

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
      delete (EqualSumConstraint*) m_eqSumConstraint;
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
      delete (EqualSumConstraint*) m_eqSumConstraint;
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
      delete (EqualSumConstraint*) m_eqSumConstraint;
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
      delete (EqualSumConstraint*) m_eqSumConstraint;
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
    Variable<IntervalDomain> m_zeros, m_otherVars,  m_superset;
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
    Variable<IntervalIntDomain> m_superset;
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
   * @class EqualMaximumConstraint
   * @brief First variable is the maximum value of the others.
   */
  class EqualMaximumConstraint : public Constraint {
  public:
    EqualMaximumConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~EqualMaximumConstraint() { }

    void handleExecute();
  };

  /**
   * @class CondEqualSumConstraint
   * @brief If A is true, then B = C + D ...; if A is false, B != C + D ...
   * Converted into two constraints: CondAllSame(A, B, sum) and EqualSum(sum, C, D, ...).
   */
  class CondEqualSumConstraint : public Constraint {
  public:
    CondEqualSumConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~CondEqualSumConstraint() {
      delete (EqualSumConstraint*) m_eqSumConstraint;
    }

    // All the work is done by the member constraints.
    inline void handleExecute() { }

  private:
    Variable<IntervalDomain> m_sumVar;
    CondAllSameConstraint m_condAllSameConstraint;
    ConstraintId m_eqSumConstraint;
  };

  /**
   * @class RotateScopeRightConstraint
   * @brief Rotate the scope right rotateCount places and call the otherName
   * constraint.
   * @note "Rotating right" comes from last variable moving to the start,
   * "pushing" all of the other variables to the right.
   * @note Negative and zero values for rotateCount are supported.
   */
  class RotateScopeRightConstraint : public Constraint {
  public:
    RotateScopeRightConstraint(const LabelStr& name,
                               const LabelStr& propagatorName,
                               const ConstraintEngineId& constraintEngine,
                               const std::vector<ConstrainedVariableId>& variables)
      : Constraint(name, propagatorName, constraintEngine, variables) {
      // Called via REGISTER_NARY() macro's factory rather than via the
      //   REGISTER_ROTATED_NARY() macro's factory: not enough information
      //   to create the constraint.
      assertTrue(false);
    }

    RotateScopeRightConstraint(const LabelStr& name,
                               const LabelStr& propagatorName,
                               const ConstraintEngineId& constraintEngine,
                               const std::vector<ConstrainedVariableId>& variables,
                               const LabelStr& otherName,
                               const int& rotateCount);

    ~RotateScopeRightConstraint() {
      assertTrue(m_otherConstraint.isValid());
      delete (Constraint*) m_otherConstraint;
    }

    void handleExecute() { }

  private:
    ConstraintId m_otherConstraint;
  };

  /**
   * @class SwapTwoVarsConstraint
   * @brief Swap two variables in the scope and call the otherName
   * constraint.
   */
  class SwapTwoVarsConstraint : public Constraint {
  public:
    SwapTwoVarsConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables)
      : Constraint(name, propagatorName, constraintEngine, variables) {
      // Called via REGISTER_NARY() macro's factory rather than via the
      //   REGISTER_SWAP_TWO_VARS_NARY() macro's factory: not enough information
      //   to create the constraint.
      assertTrue(false);
    }

    SwapTwoVarsConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables,
                          const LabelStr& otherName,
                          int firstIndex, int secondIndex);

    ~SwapTwoVarsConstraint() {
      assertTrue(m_otherConstraint.isValid());
      delete (Constraint*) m_otherConstraint;
    }

    void handleExecute() { }

  private:
    ConstraintId m_otherConstraint;
  };

  // Enforce X+Y=0. X >=0. Y <=0.
  class NegateConstraint : public Constraint {
  public:
    NegateConstraint(const LabelStr& name,
		    const LabelStr& propagatorName,
		    const ConstraintEngineId& constraintEngine,
		    const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();
  private:
    static const int X=0;
    static const int Y=1;
  };

  //Test if X==Y and return the result in Z
  class TestEqConstraint: public Constraint
  {
  public:

    TestEqConstraint(const LabelStr& name,
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

  extern void initConstraintLibrary();
}
#endif
