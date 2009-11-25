#ifndef _H_Constraints
#define _H_Constraints

#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "Domains.hh"

namespace EUROPA {

#define CREATE_CONSTRAINT_BASE(basename)				\
  class basename : public ConstraintType {				\
  public:								\
    basename(const LabelStr& name,					\
	     const LabelStr& propagatorName,				\
	     bool systemDefined = false)				\
      : ConstraintType(name,propagatorName,systemDefined) { m_name = name.c_str(); } \
    									\
    virtual ~basename() {}						\
    									\
    virtual ConstraintId createConstraint( \
                      const ConstraintEngineId constraintEngine, \
					  const std::vector<ConstrainedVariableId>& scope, \
					  const char* violationExpl) = 0; \
    									\
    virtual void checkArgTypes(const std::vector<DataTypeId>& argTypes) const; \
  protected:								\
    std::string m_name;							\
  };

  CREATE_CONSTRAINT_BASE(TwoSameArgumentsCT);
  CREATE_CONSTRAINT_BASE(TwoSameNumericArgumentsCT);
  CREATE_CONSTRAINT_BASE(TestTwoSameArgumentsCT);
  CREATE_CONSTRAINT_BASE(TestTwoSameNumericArgumentsCT);
  CREATE_CONSTRAINT_BASE(TwoBooleanArgumentsCT);
  CREATE_CONSTRAINT_BASE(ThreeBooleanArgumentsCT);
  CREATE_CONSTRAINT_BASE(AllSameArgumentsCT);
  CREATE_CONSTRAINT_BASE(AllSameNumericArgumentsCT);
  CREATE_CONSTRAINT_BASE(TestOneArgumentCT);


#define CREATE_CONSTRAINT_TYPE(base, name, constraint) \
  class name : public base {			       \
  public:					       \
  name(const LabelStr& name,			       \
       const LabelStr& propagatorName,		       \
       bool systemDefined = false)					\
    : base(name,propagatorName,systemDefined) {}			\
  ~name() {}							\
  virtual ConstraintId createConstraint(const ConstraintEngineId constraintEngine, \
					const std::vector<ConstrainedVariableId>& scope,\
					const char* violationMsg) \
    {									\
      return makeConstraintInstance<constraint>(m_name, m_propagatorName, constraintEngine, scope, violationMsg); \
    } \
  };


#define CREATE_FUNCTION_CONSTRAINT(cname)				\
  class cname##Constraint : public Constraint {				\
  public:								\
  cname##Constraint(const LabelStr& name,				\
		const LabelStr& propagatorName,				\
		const ConstraintEngineId& constraintEngine,		\
		const std::vector<ConstrainedVariableId>& variables);	\
  void handleExecute();							\
  private:								\
  };

#define CREATE_FUNCTION_CONSTRAINT_ONE_ARG(cname, function, rt)		\
  cname##Constraint::cname##Constraint(const LabelStr& name,		\
				       const LabelStr& propagatorName,	\
				       const ConstraintEngineId& constraintEngine, \
				       const std::vector<ConstrainedVariableId>& variables) \
  : Constraint(name, propagatorName, constraintEngine, variables) {}	\
  void cname##Constraint::handleExecute() {				\
    check_error(isActive());						\
    if (getCurrentDomain(m_variables[1]).isSingleton()) {		\
      rt value = function(getCurrentDomain(m_variables[1]).getSingletonValue()); \
      getCurrentDomain(m_variables[0]).intersect(value, value);		\
    }									\
  }

#define CREATE_FUNCTION_CONSTRAINT_TWO_ARG(cname, function, rt)		\
  cname##Constraint::cname##Constraint(const LabelStr& name,		\
				       const LabelStr& propagatorName,	\
				       const ConstraintEngineId& constraintEngine, \
				       const std::vector<ConstrainedVariableId>& variables) \
  : Constraint(name, propagatorName, constraintEngine, variables) {}	\
  void cname##Constraint::handleExecute() {				\
    check_error(isActive());						\
    if (getCurrentDomain(m_variables[1]).isSingleton()			\
	&& getCurrentDomain(m_variables[2]).isSingleton()) {		\
      rt value = function(getCurrentDomain(m_variables[1]).getSingletonValue(), \
			  getCurrentDomain(m_variables[2]).getSingletonValue()); \
      getCurrentDomain(m_variables[0]).intersect(value, value);		\
    }									\
  }



  CREATE_FUNCTION_CONSTRAINT(Max);
  CREATE_FUNCTION_CONSTRAINT(Min);
  CREATE_FUNCTION_CONSTRAINT(Abs);
  CREATE_FUNCTION_CONSTRAINT(Pow);
  CREATE_FUNCTION_CONSTRAINT(Sqrt);
  CREATE_FUNCTION_CONSTRAINT(Mod);
  CREATE_FUNCTION_CONSTRAINT(Floor);
  CREATE_FUNCTION_CONSTRAINT(Ceil);



  /**
   * @brief AbsoluteValue(x, y) maintains the relation:
   * @li x.lb >= 0
   * @li x.ub = max(abs(y.lb), abs(y.ub))
   * @li y.lb >= -x.lb
   * @li y.ub <= x.ub
   */
  class AbsoluteValue : public Constraint {
  public:
    AbsoluteValue(const LabelStr& name,
          const LabelStr& propagatorName,
          const ConstraintEngineId& constraintEngine,
          const std::vector<ConstrainedVariableId>& variables);
    void handleExecute();
  private:
    IntervalDomain& m_x;
    IntervalDomain& m_y;
    static const unsigned int ARG_COUNT = 2;
  };

  class AbsoluteValueCT : public ConstraintType {
  public:
      AbsoluteValueCT(const LabelStr& name,
              const LabelStr& propagatorName,
              bool systemDefined = false)
      : ConstraintType(name,propagatorName,systemDefined) {}

      virtual ~AbsoluteValueCT() {}

      virtual ConstraintId createConstraint(
              const ConstraintEngineId constraintEngine,
              const std::vector<ConstrainedVariableId>& scope,
              const char* violationExpl)
      {
          return makeConstraintInstance<AbsoluteValue>(m_name, m_propagatorName, constraintEngine, scope, violationExpl);
      }

      virtual void checkArgTypes(const std::vector<DataTypeId>& argTypes) const;
  };

  class AddEqualConstraint : public Constraint {
  public:
    AddEqualConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:

    AbstractDomain& m_x;
    AbstractDomain& m_y;
    AbstractDomain& m_z;

    static const int X = 0;
    static const int Y = 1;
    static const int Z = 2;
    static const int ARG_COUNT = 3;
  };

  class AddEqualCT : public ConstraintType {
  public:
      AddEqualCT(const LabelStr& name,
              const LabelStr& propagatorName,
              bool systemDefined = false)
      : ConstraintType(name,propagatorName,systemDefined) {}

      virtual ~AddEqualCT() {}

      virtual ConstraintId createConstraint(const ConstraintEngineId constraintEngine,
              const std::vector<ConstrainedVariableId>& scope,
              const char* violationExpl)
      {
          return makeConstraintInstance<AddEqualConstraint>(m_name, m_propagatorName, constraintEngine, scope, violationExpl);
      }

      virtual void checkArgTypes(const std::vector<DataTypeId>& argTypes) const;
  };

  class MultEqualConstraint : public Constraint {
  public:
    MultEqualConstraint(const LabelStr& name,
			const LabelStr& propagatorName,
			const ConstraintEngineId& constraintEngine,
			const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

    /**
     * @brief Helper method to compute new bounds for both X and Y in X*Y == Z.
     * @return True if the target domain was modified.
     */
    static bool updateMinAndMax(IntervalDomain& targetDomain,
				edouble denomMin, edouble denomMax,
				edouble numMin, edouble numMax);
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

    ~AddMultEqualConstraint();
  private:
    // All the work is done by the member constraints
    inline void handleExecute() { }
    void handleDiscard();

    static const int A = 0;
    static const int B = 1;
    static const int C = 2;
    static const int D = 3;
    static const int ARG_COUNT = 4;

    Variable<IntervalDomain> m_interimVariable;
    MultEqualConstraint m_multEqualConstraint;
    AddEqualConstraint m_addEqualConstraint;
  };

  class AllDiffConstraint : public Constraint {
  public:
    AllDiffConstraint(const LabelStr& name,
                      const LabelStr& propagatorName,
                      const ConstraintEngineId& constraintEngine,
                      const std::vector<ConstrainedVariableId>& variables);

    ~AllDiffConstraint() {
			discard(false);
    }

  private:
    void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_condVar.discard();
      m_condAllDiffConstraint->discard();
    }

    Variable<BoolDomain> m_condVar;
    ConstraintId m_condAllDiffConstraint;
  };

class CalcDistanceConstraint : public Constraint {
  public:
    CalcDistanceConstraint(const LabelStr& name,
			   const LabelStr& propagatorName,
			   const ConstraintEngineId& constraintEngine,
			   const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

    /**
     * Calculates the actual distance
     */
    static edouble compute(edouble x1, edouble y1, edouble x2, edouble y2);

    /**
     * Calculates the hypotenuse w. pythagaras
     */
    static edouble compute(edouble a, edouble b);

  private:

    static const unsigned int ARG_COUNT = 5;
    static const unsigned int DISTANCE = 0;
    static const unsigned int X1 = 1;
    static const unsigned int Y1 = 2;
    static const unsigned int X2 = 3;
    static const unsigned int Y2 = 4;

    AbstractDomain& m_distance;
    AbstractDomain& m_x1;
    AbstractDomain& m_y1;
    AbstractDomain& m_x2;
    AbstractDomain& m_y2;
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

    static void propagate(AbstractDomain& domx, AbstractDomain& domy);

  private:
    bool testIsRedundant(const ConstrainedVariableId& var = ConstrainedVariableId::noId()) const;

    AbstractDomain& m_x;
    AbstractDomain& m_y;
    static const int X = 0;
    static const int Y = 1;
    static const int ARG_COUNT = 2;
  };
  CREATE_CONSTRAINT_TYPE(TwoSameNumericArgumentsCT, LessThanEqualCT, LessThanEqualConstraint);

class CardinalityConstraint : public Constraint {
  public:
    CardinalityConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~CardinalityConstraint() {
      discard(false);
    }

  private:
    // All the work is done by the member constraints.
    inline void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_nonZeros.discard();
      m_lessThanEqualConstraint.discard();
      m_countNonZerosConstraint->discard();
    }

    Variable<IntervalIntDomain> m_nonZeros;
    LessThanEqualConstraint m_lessThanEqualConstraint;
    ConstraintId m_countNonZerosConstraint;
  };

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

class CondEqualSumConstraint : public Constraint {
  public:
    CondEqualSumConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~CondEqualSumConstraint() {
      discard(false);
    }

  private:
    // All the work is done by the member constraints.
    inline void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_sumVar.discard();
      m_condAllSameConstraint.discard();
      m_eqSumConstraint->discard();
    }

    Variable<IntervalDomain> m_sumVar;
    CondAllSameConstraint m_condAllSameConstraint;
    ConstraintId m_eqSumConstraint;
  };

class CountNonZerosConstraint : public Constraint {
  public:
    CountNonZerosConstraint(const LabelStr& name,
                            const LabelStr& propagatorName,
                            const ConstraintEngineId& constraintEngine,
                            const std::vector<ConstrainedVariableId>& variables);

    ~CountNonZerosConstraint() {
      discard(false);
    }

    // All the work is done by the member constraints.
    inline void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_zeros.discard();
      m_otherVars.discard();
      m_superset.discard();
      m_addEqualConstraint.discard();
      m_countZerosConstraint->discard();
      m_subsetConstraint->discard();
    }

  private:
    Variable<IntervalDomain> m_zeros, m_otherVars,  m_superset;
    AddEqualConstraint m_addEqualConstraint;
    ConstraintId m_subsetConstraint;
    ConstraintId m_countZerosConstraint;
  };

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
    class DistanceFromSquaresConstraint : public Constraint {
  public:
    DistanceFromSquaresConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int V1 = 0;
    static const int V2 = 1;
    static const int RES = 2;
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
    bool equate(const ConstrainedVariableId& v1, const ConstrainedVariableId& v2, bool& isEmpty);
    const unsigned int m_argCount;
  };
  CREATE_CONSTRAINT_TYPE(TwoSameArgumentsCT, EqualCT, EqualConstraint);

      class EqualMaximumConstraint : public Constraint {
  public:
    EqualMaximumConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~EqualMaximumConstraint() { }

    void handleExecute();
  };

class EqualMinimumConstraint : public Constraint {
  public:
    EqualMinimumConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~EqualMinimumConstraint() { }

    void handleExecute();
  };

class EqualProductConstraint : public Constraint {
  public:
    EqualProductConstraint(const LabelStr& name,
                           const LabelStr& propagatorName,
                           const ConstraintEngineId& constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

    ~EqualProductConstraint();

  private:

    // All the work is done by the member constraints
    inline void handleExecute() { }

    void handleDiscard();

    const unsigned int ARG_COUNT;

    ConstraintId m_eqProductC1, m_eqProductC2, m_eqProductC3, m_eqProductC4, m_eqProductC5;
    Variable<IntervalDomain> m_product1, m_product2, m_product3, m_product4;
  };

class EqualSumConstraint : public Constraint {
  public:
    EqualSumConstraint(const LabelStr& name,
                       const LabelStr& propagatorName,
                       const ConstraintEngineId& constraintEngine,
                       const std::vector<ConstrainedVariableId>& variables);

    ~EqualSumConstraint();

  private:

    // All the work is done by the member constraints
    inline void handleExecute() { }

    void handleDiscard();

    const unsigned int ARG_COUNT;

    ConstraintId m_eqSumC1, m_eqSumC2, m_eqSumC3, m_eqSumC4, m_eqSumC5;
    Variable<IntervalDomain> m_sum1, m_sum2, m_sum3, m_sum4;
  };

class GreaterOrEqThanSumConstraint : public Constraint {
  public:
    GreaterOrEqThanSumConstraint(const LabelStr& name,
                                 const LabelStr& propagatorName,
                                 const ConstraintEngineId& constraintEngine,
                                 const std::vector<ConstrainedVariableId>& variables);

    ~GreaterOrEqThanSumConstraint() {
      discard(false);
    }

  private:
    // All the work is done by the member constraints
    inline void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_interimVariable.discard();
      m_lessOrEqualConstraint.discard();
      m_eqSumConstraint->discard();
    }

    Variable<IntervalDomain> m_interimVariable;
    LessThanEqualConstraint m_lessOrEqualConstraint;
    ConstraintId m_eqSumConstraint;
  };

class LessThanConstraint : public Constraint {
  public:
    LessThanConstraint(const LabelStr& name,
                       const LabelStr& propagatorName,
                       const ConstraintEngineId& constraintEngine,
                       const std::vector<ConstrainedVariableId>& variables);


    void handleExecute();

    bool canIgnore(const ConstrainedVariableId& variable,
		   int argIndex,
		   const DomainListener::ChangeType& changeType);

    static void propagate(IntervalDomain& domx, IntervalDomain& domy);

  private:
    static const int X = 0;
    static const int Y = 1;
    static const int ARG_COUNT = 2;
  };
  CREATE_CONSTRAINT_TYPE(TwoSameNumericArgumentsCT, LessThanCT, LessThanConstraint);

class GreaterThanSumConstraint : public Constraint {
  public:
    GreaterThanSumConstraint(const LabelStr& name,
                             const LabelStr& propagatorName,
                             const ConstraintEngineId& constraintEngine,
                             const std::vector<ConstrainedVariableId>& variables);

    ~GreaterThanSumConstraint() {
      discard(false);
    }

  private:
    // All the work is done by the member constraints
    inline void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_interimVariable.discard();
      m_lessThanConstraint.discard();
      m_eqSumConstraint->discard();
    }

    Variable<IntervalDomain> m_interimVariable;
    LessThanConstraint m_lessThanConstraint;
    ConstraintId m_eqSumConstraint;
  };

class LessOrEqThanSumConstraint : public Constraint {
  public:
    LessOrEqThanSumConstraint(const LabelStr& name,
                              const LabelStr& propagatorName,
                              const ConstraintEngineId& constraintEngine,
                              const std::vector<ConstrainedVariableId>& variables);

    ~LessOrEqThanSumConstraint();

  private:
    void handleExecute();
    void handleDiscard();
    Variable<IntervalDomain> m_interimVariable;
    LessThanEqualConstraint m_lessOrEqualConstraint;
    ConstraintId m_eqSumConstraint;
  };

  class LessThanSumConstraint : public Constraint {
  public:
    LessThanSumConstraint(const LabelStr& name,
                          const LabelStr& propagatorName,
                          const ConstraintEngineId& constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables);

    ~LessThanSumConstraint() {
      discard(false);
    }

  private:

    // All the work is done by the member constraints
    inline void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_interimVariable.discard();
      m_lessThanConstraint.discard();
      m_eqSumConstraint->discard();
    }

    Variable<IntervalDomain> m_interimVariable;
    LessThanConstraint m_lessThanConstraint;
    ConstraintId m_eqSumConstraint;
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

  /**
   * Enforces the relation x < y
   */

        /**
   * @class EqualSumConstraint
   * @brief A = B + C where B and C can each be sums.
   * Converted into an AddEqualConstraint and/or two EqSumConstraints with fewer variables.
   */
    /**
   * @class EqualProductConstraint
   * @brief A = B * C where B and C can each be products.
   * Converted into an AddEqualConstraint and/or two EqProductConstraints with fewer variables.
   */
    /**
   * @class LessOrEqThanSumConstraint
   * @brief A <= B + C + ...
   * Converted into two constraints: A <= temp and temp equal to the sum of the rest.
   */
    /**
   * @class LessThanSumConstraint
   * @brief A < B + C + ...
   * Converted into two constraints: A < temp and temp equal to the sum of the rest.
   */
    /**
   * @class GreaterOrEqThanSumConstraint
   * @brief A >= B + C + ...
   * Converted into two constraints: A >= temp and temp equal to the sum of the rest.
   */
    /**
   * @class GreaterThanSumConstraint
   * @brief A > B + C + ...
   * Converted into two constraints: A < temp and temp equal to the sum of the rest.
   */
    /**
   * @class CondAllSame
   * @brief If A, then B == C && B == D && C == D && ... ; if not A, then !(B == C && B == D && C == D && ...).
   */
    /**
   * @class CondAllDiff
   * @brief If A, then B != C && B != D && C != D && ... ; if not A, then !(B != C && B != D && C != D && ...).
   */
    /**
   * @class AllDiff
   * @brief A != B && A != C && B != C && A != D && B != D && ...
   */
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
  /**
   * @class CardinalityConstraint
   * @brief First variable must be greater than or equal the count of the
   * other variables that are true.
   * @note Supports numeric domains for the other variables with the
   * usual C/C++ convention of false being zero and true being
   * any non-zero value.
   */
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
      discard(false);
    }

  private:
    // All the work is done by the member constraints.
    inline void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_nonZeros.discard();
      m_superset.discard();
      m_subsetConstraint->discard();
      m_countNonZerosConstraint->discard();
    }

    Variable<IntervalIntDomain> m_nonZeros;
    Variable<IntervalIntDomain> m_superset;
    ConstraintId m_subsetConstraint;
    ConstraintId m_countNonZerosConstraint;
  };

  class RandConstraint : public Constraint {
  public:
    RandConstraint(const LabelStr& name,
          const LabelStr& propagatorName,
          const ConstraintEngineId& constraintEngine,
          const std::vector<ConstrainedVariableId>& variables);
    void handleExecute();
  private:
    unsigned int m_rvalue;
  };

  /**
   * @class EqualMinimumConstraint
   * @brief First variable is the minimum value of the others.
   */
    /**
   * @class EqualMaximumConstraint
   * @brief First variable is the maximum value of the others.
   */
    /**
   * @class CondEqualSumConstraint
   * @brief If A is true, then B = C + D ...; if A is false, B != C + D ...
   * Converted into two constraints: CondAllSame(A, B, sum) and EqualSum(sum, C, D, ...).
   */
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
      discard(false);
    }

  private:
    void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_otherConstraint->discard();
    }

    ConstraintId m_otherConstraint;
  };

  /**
   * @class SwapTwoVarsConstraint
   * @brief Swap two variables in the scope and call the otherName
   * constraint.
   */
  class SineFunction : public Constraint {
  public:
    SineFunction(const LabelStr& name,
		 const LabelStr& propagatorName,
		 const ConstraintEngineId& constraintEngine,
		 const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const unsigned int ARG_COUNT = 2;
    AbstractDomain& m_target;
    AbstractDomain& m_source;
  };

class SquareOfDifferenceConstraint : public Constraint {
  public:
    SquareOfDifferenceConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int V1 = 0;
    static const int V2 = 1;
    static const int RES = 2;
    static const int ARG_COUNT = 3;
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
      discard(false);
    }

  private:
    void handleExecute() { }

    void handleDiscard(){
      Constraint::handleDiscard();
      m_otherConstraint->discard();
    }

    ConstraintId m_otherConstraint;
  };

  // Enforce X+Y=0. X >=0. Y <=0.
    class TestAnd : public Constraint {
  public:
    TestAnd(const LabelStr& name,
	    const LabelStr& propagatorName,
	    const ConstraintEngineId& constraintEngine,
	    const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    AbstractDomain& m_arg2;
    static const unsigned int ARG_COUNT = 3;
  };
  CREATE_CONSTRAINT_TYPE(ThreeBooleanArgumentsCT, TestAndCT, TestAnd);

  class TestEQ : public Constraint {
  public:
    TestEQ(const LabelStr& name,
	   const LabelStr& propagatorName,
	   const ConstraintEngineId& constraintEngine,
	   const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    AbstractDomain& m_arg2;
    static const unsigned int ARG_COUNT = 3;
  };
  CREATE_CONSTRAINT_TYPE(TestTwoSameArgumentsCT, TestEQCT, TestEQ);

  class TestLessThan : public Constraint {
  public:
    TestLessThan(const LabelStr& name,
		 const LabelStr& propagatorName,
		 const ConstraintEngineId& constraintEngine,
		 const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    AbstractDomain& m_arg2;
    static const unsigned int ARG_COUNT = 3;
  };
  CREATE_CONSTRAINT_TYPE(TestTwoSameNumericArgumentsCT, TestLessThanCT, TestLessThan);

  class TestLEQ : public Constraint {
  public:
    TestLEQ(const LabelStr& name,
	    const LabelStr& propagatorName,
	    const ConstraintEngineId& constraintEngine,
	    const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    AbstractDomain& m_arg2;
    static const unsigned int ARG_COUNT = 3;
  };
  CREATE_CONSTRAINT_TYPE(TestTwoSameNumericArgumentsCT, TestLEQCT, TestLEQ);


  /**
   * @brief Calculate the euclidean distance in 2-d space between between 2 points
   */
    /**
   * @brief Computes the sign of a given variable. Varable is in degrees. The constraint is a function
   * rather than a relation. The range of the source variable must be in [0 90].
   */
    /**
   * @brief SquareOfDifference(x, y, a) maintains the relation:
   * @li a = (x - y)^2
   * if x and y are singleton.
   */
    /**
   * @brief DistanceFromSquaresConstraint(x, y, a) maintains the relation
   * @li a = sqrt(x + y)
   * if x and y are singleton
   */
    /**
   * @brief Maintains a unary relation from a constant to a variable such that the variable
   * is a subset of the given constant.
   */
  class TestNEQ : public Constraint {
  public:
    TestNEQ(const LabelStr& name,
	    const LabelStr& propagatorName,
	    const ConstraintEngineId& constraintEngine,
	    const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    AbstractDomain& m_arg2;
    static const unsigned int ARG_COUNT = 3;
  };
  CREATE_CONSTRAINT_TYPE(TestTwoSameArgumentsCT, TestNEQCT, TestNEQ);

class TestOr : public Constraint {
  public:
    TestOr(const LabelStr& name,
	   const LabelStr& propagatorName,
	   const ConstraintEngineId& constraintEngine,
	   const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    AbstractDomain& m_arg2;
    static const unsigned int ARG_COUNT = 3;
  };
  CREATE_CONSTRAINT_TYPE(ThreeBooleanArgumentsCT, TestOrCT, TestOr);


class TestSingleton : public Constraint {
  public:
    TestSingleton(const LabelStr& name,
	   const LabelStr& propagatorName,
	   const ConstraintEngineId& constraintEngine,
	   const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    static const unsigned int ARG_COUNT = 2;
  };

  CREATE_CONSTRAINT_TYPE(TestOneArgumentCT, TestSingletonCT, TestSingleton);

class TestSpecified : public Constraint {
  public:
    TestSpecified(const LabelStr& name,
	   const LabelStr& propagatorName,
	   const ConstraintEngineId& constraintEngine,
	   const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    AbstractDomain& m_test;
    AbstractDomain& m_arg1;
    static const unsigned int ARG_COUNT = 2;
  };
  CREATE_CONSTRAINT_TYPE(TestOneArgumentCT, TestSpecifiedCT, TestSpecified);

class UnaryConstraint : public Constraint {
  public:
    /**
     * @brief Specialized constructor
     */
    UnaryConstraint(const AbstractDomain& dom, const ConstrainedVariableId& var);

    /**
     * @brief Standard constructor
     */
    UnaryConstraint(const LabelStr& name,
            const LabelStr& propagatorName,
            const ConstraintEngineId& constraintEngine,
            const std::vector<ConstrainedVariableId>& variables);

    ~UnaryConstraint();

  private:

    void handleExecute();

    void handleDiscard();

    bool canIgnore(const ConstrainedVariableId& variable,
           int argIndex,
           const DomainListener::ChangeType& changeType);

    void setSource(const ConstraintId& sourceConstraint);

    AbstractDomain* m_x;
    AbstractDomain* m_y;
  };

  /**
   * @brief WithinBounds(x, y, z) maintains the relations:
   * @li x.lb >= y.lb
   * @li x.ub <= z.ub
   * @li y <= z
   */
  class WithinBounds : public Constraint {
  public:
    WithinBounds(const LabelStr& name,
         const LabelStr& propagatorName,
         const ConstraintEngineId& constraintEngine,
         const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    IntervalDomain& m_x;
    IntervalDomain& m_y;
    IntervalDomain& m_z;
    LessThanEqualConstraint m_leq;
    static const unsigned int ARG_COUNT = 3;
  };


}
#endif
