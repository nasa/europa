#ifndef H_Constraints
#define H_Constraints

#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "Domains.hh"
#include "ConstraintType.hh"
#include "ConstraintTypeChecking.hh"

namespace EUROPA {

#define CREATE_FUNCTION_CONSTRAINT(cname)				\
  class cname##Constraint : public Constraint {				\
  public:								\
  cname##Constraint(const std::string& name,				\
		const std::string& propagatorName,				\
		const ConstraintEngineId constraintEngine,		\
		const std::vector<ConstrainedVariableId>& variables);	\
  void handleExecute();							\
  private:								\
  };

#define CREATE_FUNCTION_CONSTRAINT_ONE_ARG(cname, function, rt)		\
  cname##Constraint::cname##Constraint(const std::string& name,		\
				       const std::string& propagatorName,	\
				       const ConstraintEngineId constraintEngine, \
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
  cname##Constraint::cname##Constraint(const std::string& name,		\
				       const std::string& propagatorName,	\
				       const ConstraintEngineId constraintEngine, \
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
typedef And<NArgs<3>, All<Numeric> > TwoArgNumericFun;
typedef DataTypeCheck<MaxConstraint, TwoArgNumericFun> MaxCT;
CREATE_FUNCTION_CONSTRAINT(Min);
typedef DataTypeCheck<MinConstraint, TwoArgNumericFun> MinCT;
CREATE_FUNCTION_CONSTRAINT(Abs);
typedef And<NArgs<2>, All<Numeric> > OneArgNumericFun;
typedef DataTypeCheck<AbsConstraint, OneArgNumericFun> AbsCT;
CREATE_FUNCTION_CONSTRAINT(Pow);
typedef DataTypeCheck<PowConstraint, TwoArgNumericFun> PowCT;
CREATE_FUNCTION_CONSTRAINT(Sqrt);
typedef DataTypeCheck<SqrtConstraint, OneArgNumericFun> SqrtCT;
CREATE_FUNCTION_CONSTRAINT(Mod);
typedef DataTypeCheck<ModConstraint, TwoArgNumericFun> ModCT;
CREATE_FUNCTION_CONSTRAINT(Floor);
typedef DataTypeCheck<FloorConstraint, OneArgNumericFun> FloorCT;
CREATE_FUNCTION_CONSTRAINT(Ceil);
typedef DataTypeCheck<CeilConstraint, OneArgNumericFun> CeilCT;



/**
 * @brief AbsoluteValue(x, y) maintains the relation:
 * @li x.lb >= 0
 * @li x.ub = max(abs(y.lb), abs(y.ub))
 * @li y.lb >= -x.lb
 * @li y.ub <= x.ub
 */
class AbsoluteValue : public Constraint {
 public:
  AbsoluteValue(const std::string& name,
                const std::string& propagatorName,
                const ConstraintEngineId constraintEngine,
          const std::vector<ConstrainedVariableId>& variables);
  void handleExecute();
 private:
  IntervalDomain& m_x;
  IntervalDomain& m_y;
  static const unsigned int ARG_COUNT = 2;
};
typedef And<NArgs<2>, And<Mutually<Assignable<> >, All<Numeric> > > TwoAssignableNumeric;
typedef DataTypeCheck<AbsoluteValue, TwoAssignableNumeric>  AbsoluteValueCT;

class AddEqualConstraint : public Constraint {
 public:
  AddEqualConstraint(const std::string& name,
                     const std::string& propagatorName,
                     const ConstraintEngineId constraintEngine,
                     const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  // X + Y = Z
  Domain& m_x;
  Domain& m_y;
  Domain& m_z;

  static const unsigned int X = 0;
  static const unsigned int Y = 1;
  static const unsigned int Z = 2;
  static const unsigned int ARG_COUNT = 3;
};

//TODO: fix this
// typedef And<NArgs<3>, And<All<Numeric>, All<Assignable<Last<> > > > > ThreeNumericEq;
typedef And<NArgs<3>, And<All<Numeric>, All<Comparable<Last<> > > > > ThreeNumericEq;
typedef DataTypeCheck<AddEqualConstraint, ThreeNumericEq> AddEqualCT;

class MultEqualConstraint : public Constraint {
 public:
  MultEqualConstraint(const std::string& name,
                      const std::string& propagatorName,
                      const ConstraintEngineId constraintEngine,
                      const std::vector<ConstrainedVariableId>& variables);
  
  void handleExecute();
  
 private:
  static const unsigned int X = 0;
  static const unsigned int Y = 1;
  static const unsigned int Z = 2;
  static const unsigned int ARG_COUNT = 3;
};

typedef DataTypeCheck<MultEqualConstraint, ThreeNumericEq> MultEqualCT;


class DivEqualConstraint : public Constraint {
 public:
  DivEqualConstraint(const std::string& name,
                     const std::string& propagatorName,
                     const ConstraintEngineId constraintEngine,
                     const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  static const unsigned int X = 0;
  static const unsigned int Y = 1;
  static const unsigned int Z = 2;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<DivEqualConstraint, ThreeNumericEq> DivEqualCT;


/**
 * @class AllDiff
 * @brief A != B && A != C && B != C && A != D && B != D && ...
 */

class AllDiffConstraint : public Constraint {
 public:
  AllDiffConstraint(const std::string& name,
                    const std::string& propagatorName,
                    const ConstraintEngineId constraintEngine,
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
typedef DataTypeCheck<AllDiffConstraint, And<AtLeastNArgs<2>, Mutually<Assignable<> > > >
AllDiffCT;

  /**
   * @brief Calculate the euclidean distance in 2-d space between between 2 points
   */
class CalcDistanceConstraint : public Constraint {
 public:
  CalcDistanceConstraint(const std::string& name,
                         const std::string& propagatorName,
                         const ConstraintEngineId constraintEngine,
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

  Domain& m_distance;
  Domain& m_x1;
  Domain& m_y1;
  Domain& m_x2;
  Domain& m_y2;
};

typedef DataTypeCheck<CalcDistanceConstraint,
                      And<NArgs<5>, And<All<Numeric>,
                                        All<Assignable<First<> >, Second<>, Last<> > > > >
CalcDistanceCT;


class LessThanEqualConstraint : public Constraint {
  public:
    LessThanEqualConstraint(const std::string& name,
			    const std::string& propagatorName,
			    const ConstraintEngineId constraintEngine,
			    const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

    bool canIgnore(const ConstrainedVariableId variable,
		   unsigned int argIndex,
		   const DomainListener::ChangeType& changeType);

    static void propagate(Domain& domx, Domain& domy);

  private:
    bool testIsRedundant(const ConstrainedVariableId var = ConstrainedVariableId::noId()) const;

    Domain& m_x;
    Domain& m_y;
    static const unsigned int X = 0;
    static const unsigned int Y = 1;
    static const unsigned int ARG_COUNT = 2;
  };
typedef DataTypeCheck<LessThanEqualConstraint,
                      And<NArgs<2>, And<All<Numeric>, Mutually<Assignable<> > > > >
LessThanEqualCT;


    /**
   * @class CondAllDiff
   * @brief If A, then B != C && B != D && C != D && ... ; if not A, then !(B != C && B != D && C != D && ...).
   */

class CondAllDiffConstraint : public Constraint {
 public:
  CondAllDiffConstraint(const std::string& name,
                        const std::string& propagatorName,
                        const ConstraintEngineId constraintEngine,
                        const std::vector<ConstrainedVariableId>& variables);
  
    ~CondAllDiffConstraint() { }
  
  void handleExecute();
  
 private:
  const unsigned long ARG_COUNT;
};
template<typename Start = First<>, typename End = End>
struct AllAssignable : Mutually<Assignable<>, Start, End> {
  AllAssignable() : Mutually<Assignable<>, Start, End>() {}
  AllAssignable(type_iterator start, type_iterator end)
      : Mutually<Assignable<>, Start, End>(start, end){}
};
struct CondCondition : And<AtLeastNArgs<3>, First<Type<BoolDT> > > {
  CondCondition() : And<AtLeastNArgs<3>, First<Type<BoolDT> > >() {}
  CondCondition(type_iterator start, type_iterator end)
      : And<AtLeastNArgs<3>, First<Type<BoolDT> > >(start, end) {}
};
typedef And<CondCondition, AllAssignable<Second<>, End> > CondAllAssignableCondition;
typedef DataTypeCheck<CondAllDiffConstraint, CondAllAssignableCondition> CondAllDiffCT;

    /**
   * @class CondAllSame
   * @brief If A, then B == C && B == D && C == D && ... ; if not A, then !(B == C && B == D && C == D && ...).
   */

class CondAllSameConstraint : public Constraint {
 public:
  CondAllSameConstraint(const std::string& name,
                        const std::string& propagatorName,
                        const ConstraintEngineId constraintEngine,
                        const std::vector<ConstrainedVariableId>& variables);

  ~CondAllSameConstraint() { }

  void handleExecute();

 private:
  const unsigned long ARG_COUNT;
};
typedef DataTypeCheck<CondAllSameConstraint, CondAllAssignableCondition> CondAllSameCT;


template<typename Start = First<>, typename End = End>
struct EqThreeNumeric : public And<AtLeastNArgs<3>, And<All<Numeric>, All<Assignable<Start>, Start, End > > > {
  EqThreeNumeric() : And<AtLeastNArgs<3>, And<All<Numeric>, All<Assignable<Start>, Start, End > > >() {}
  EqThreeNumeric(type_iterator start, type_iterator end)
      : And<AtLeastNArgs<3>, And<All<Numeric>, All<Assignable<Start>, Start, End > > >(start, end) {}
};

class CountNonZeroesConstraint : public Constraint {
 public:
  CountNonZeroesConstraint(const std::string& name,
                           const std::string& propagatorName,
                           const ConstraintEngineId constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables);

  ~CountNonZeroesConstraint() {
    discard(false);
  }

  // All the work is done by the member constraints.
  inline void handleExecute() { }

  void handleDiscard(){
    Constraint::handleDiscard();
    m_zeroes.discard();
    m_otherVars.discard();
    m_superset.discard();
    m_addEqualConstraint.discard();
    m_countZeroesConstraint->discard();
    m_subsetConstraint->discard();
  }

 private:
  Variable<IntervalDomain> m_zeroes, m_otherVars,  m_superset;
  AddEqualConstraint m_addEqualConstraint;
  ConstraintId m_subsetConstraint;
  ConstraintId m_countZeroesConstraint;
};
typedef And<And<AtLeastNArgs<2>, All<Numeric> >, First<Assignable<IntDT> > > AtLeastTwoNumericFirstAssignable; 
typedef DataTypeCheck<CountNonZeroesConstraint,
                      AtLeastTwoNumericFirstAssignable>
CountNonZeroesCT;

  /**
   * @class CountZeroesConstraint
   * @brief First variable is the count of the rest that can be zero.
   * @note Supports boolean domains with the usual C/C++ convention of false
   * being zero and true being non-zero.
   */
class CountZeroesConstraint : public Constraint {
 public:
  CountZeroesConstraint(const std::string& name,
                        const std::string& propagatorName,
                        const ConstraintEngineId constraintEngine,
                        const std::vector<ConstrainedVariableId>& variables);

  ~CountZeroesConstraint() { }

  void handleExecute();
};
typedef DataTypeCheck<CountZeroesConstraint, AtLeastTwoNumericFirstAssignable> CountZeroesCT;


/**
 * @brief DistanceFromSquaresConstraint(x, y, a) maintains the relation
 * @li a = sqrt(x + y)
 * if x and y are singleton
 */

class DistanceFromSquaresConstraint : public Constraint {
 public:
  DistanceFromSquaresConstraint(const std::string& name,
                                const std::string& propagatorName,
                                const ConstraintEngineId constraintEngine,
                                const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  static const unsigned int V1 = 0;
  static const unsigned int V2 = 1;
  static const unsigned int RES = 2;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<DistanceFromSquaresConstraint,
                      And<And<NArgs<3>, All<Numeric> >, Last<Assignable<FloatDT> > > >
DistanceFromSquaresCT;

class EqualConstraint : public Constraint {
 public:
  EqualConstraint(const std::string& name,
                  const std::string& propagatorName,
                  const ConstraintEngineId constraintEngine,
                  const std::vector<ConstrainedVariableId>& variables);
  
  void handleExecute();
  
  /**
   * @brief Accessor required for EqualityConstraintPropagator.
   */
  static Domain& getCurrentDomain(const ConstrainedVariableId var);
  
 private:
  bool equate(const ConstrainedVariableId v1, const ConstrainedVariableId v2, bool& isEmpty);
  const unsigned long m_argCount;
};
typedef And<AtLeastNArgs<2>, Mutually<Assignable<> > > EqualCondition;
typedef DataTypeCheck<EqualConstraint,  EqualCondition> EqualCT;

/**
 * @class EqualMaximumConstraint
 * @brief First variable is the maximum value of the others.
 */

class EqualMaximumConstraint : public Constraint {
 public:
  EqualMaximumConstraint(const std::string& name,
                         const std::string& propagatorName,
                         const ConstraintEngineId constraintEngine,
                         const std::vector<ConstrainedVariableId>& variables);

  ~EqualMaximumConstraint() { }
  
  void handleExecute();
};
typedef DataTypeCheck<EqualMaximumConstraint, EqThreeNumeric<> > EqualMaximumCT;

  /**
   * @class EqualMinimumConstraint
   * @brief First variable is the minimum value of the others.
   */

class EqualMinimumConstraint : public Constraint {
  public:
  EqualMinimumConstraint(const std::string& name,
                         const std::string& propagatorName,
                         const ConstraintEngineId constraintEngine,
                         const std::vector<ConstrainedVariableId>& variables);
  
  ~EqualMinimumConstraint() { }
  
  void handleExecute();
};
typedef DataTypeCheck<EqualMinimumConstraint, EqThreeNumeric<> > EqualMinimumCT;

    /**
   * @class EqualProductConstraint
   * @brief A = B * C where B and C can each be products.
   * Converted into an AddEqualConstraint and/or two EqProductConstraints with fewer variables.
   */

class EqualProductConstraint : public Constraint {
 public:
  EqualProductConstraint(const std::string& name,
                         const std::string& propagatorName,
                         const ConstraintEngineId constraintEngine,
                         const std::vector<ConstrainedVariableId>& variables);

  ~EqualProductConstraint();

 private:

  // All the work is done by the member constraints
  inline void handleExecute() { }

  void handleDiscard();

  const unsigned long ARG_COUNT;

  ConstraintId m_eqProductC1, m_eqProductC2, m_eqProductC3, m_eqProductC4, m_eqProductC5;
  Variable<IntervalDomain> m_product1, m_product2, m_product3, m_product4;
};
typedef DataTypeCheck<EqualProductConstraint, EqThreeNumeric<> > EqualProductCT;

/**
 * @class EqualSumConstraint
 * @brief A = B + C where B and C can each be sums.
 * Converted into an AddEqualConstraint and/or two EqSumConstraints with fewer variables.
 */
class EqualSumConstraint : public Constraint {
 public:
  EqualSumConstraint(const std::string& name,
                     const std::string& propagatorName,
                     const ConstraintEngineId constraintEngine,
                     const std::vector<ConstrainedVariableId>& variables);

  ~EqualSumConstraint();

 private:

  // All the work is done by the member constraints
  inline void handleExecute() { }

  void handleDiscard();

  const unsigned long ARG_COUNT;

  ConstraintId m_eqSumC1, m_eqSumC2, m_eqSumC3, m_eqSumC4, m_eqSumC5;
  Variable<IntervalDomain> m_sum1, m_sum2, m_sum3, m_sum4;
};
typedef DataTypeCheck<EqualSumConstraint, EqThreeNumeric<> > EqualSumCT;


class LessThanConstraint : public Constraint {
 public:
  LessThanConstraint(const std::string& name,
                     const std::string& propagatorName,
                     const ConstraintEngineId constraintEngine,
                     const std::vector<ConstrainedVariableId>& variables);


  void handleExecute();

  bool canIgnore(const ConstrainedVariableId variable,
                 unsigned int argIndex,
                 const DomainListener::ChangeType& changeType);

  static void propagate(IntervalDomain& domx, IntervalDomain& domy);

 private:
  static const unsigned int X = 0;
  static const unsigned int Y = 1;
  static const unsigned int ARG_COUNT = 2;
};
typedef DataTypeCheck<LessThanConstraint, And<NArgs<2>, Mutually<Assignable<> > > > LessThanCT;


class LockConstraint : public Constraint {
 public:
  LockConstraint(const std::string& name,
                 const std::string& propagatorName,
                 const ConstraintEngineId constraintEngine,
                 const std::vector<ConstrainedVariableId>& variables);

  ~LockConstraint();

  void handleExecute();

  const Domain& getDomain() const;

 private:
  Domain& m_currentDomain;
  Domain& m_lockDomain;
};
typedef DataTypeCheck<LockConstraint, And<NArgs<2>, All<Assignable<First<> > > > > LockCT;

class NegateConstraint : public Constraint {
 public:
  NegateConstraint(const std::string& name,
                   const std::string& propagatorName,
                   const ConstraintEngineId constraintEngine,
                   const std::vector<ConstrainedVariableId>& variables);
  
  void handleExecute();
 private:
  static const unsigned int X=0;
  static const unsigned int Y=1;
};
typedef DataTypeCheck<NegateConstraint, TwoAssignableNumeric> NegateCT;

class NotEqualConstraint : public Constraint {
 public:
  NotEqualConstraint(const std::string& name,
                     const std::string& propagatorName,
                     const ConstraintEngineId constraintEngine,
                     const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

  bool canIgnore(const ConstrainedVariableId variable,
                 unsigned int argIndex,
                 const DomainListener::ChangeType& changeType);
  /**
   * @brief Helper method to do domain comparisons, and process removals if necessary
   */
  static bool checkAndRemove(const Domain& domx, Domain& domy);

 private:
  static const unsigned int X = 0;
  static const unsigned int Y = 1;
  static const unsigned int ARG_COUNT = 2;
};
typedef DataTypeCheck<NotEqualConstraint, EqualCondition> NotEqualCT;

  /**
   * Enforces the relation x < y
   */

    /**
   * @class MemberImplyConstraint
   * @brief If A is subset of B, then require that C is subset of D.
   */
class MemberImplyConstraint : public Constraint {
 public:
  MemberImplyConstraint(const std::string& name,
                        const std::string& propagatorName,
                        const ConstraintEngineId constraintEngine,
                        const std::vector<ConstrainedVariableId>& variables);
  
  ~MemberImplyConstraint() { }
  
  void handleExecute();
  
 private:
  const unsigned long ARG_COUNT;
};
//TODO: come up with better syntax for this
typedef DataTypeCheck<MemberImplyConstraint,
                      And<NArgs<4>, And<All<Assignable<First<> >, Second<>, Third<> >,
                                        All<Assignable<Third<> >, Fourth<>, Last<> > > > >
MemberImplyCT;


    /**
   * @class OrConstraint
   * @brief At least one of the variables must be true.
   * @note Supports numeric domains for all the variables with the
   * usual C/C++ convention of false being zero and true being
   * any non-zero value.
   */
class OrConstraint : public Constraint {
 public:
  OrConstraint(const std::string& name,
               const std::string& propagatorName,
               const ConstraintEngineId constraintEngine,
               const std::vector<ConstrainedVariableId>& variables);

  ~OrConstraint() {
    discard(false);
  }

 private:
  // All the work is done by the member constraints.
  inline void handleExecute() { }

  void handleDiscard(){
    Constraint::handleDiscard();
    m_nonZeroes.discard();
    m_superset.discard();
    m_subsetConstraint->discard();
    m_countNonZeroesConstraint->discard();
  }

  Variable<IntervalIntDomain> m_nonZeroes;
  Variable<IntervalIntDomain> m_superset;
  ConstraintId m_subsetConstraint;
  ConstraintId m_countNonZeroesConstraint;
};
typedef DataTypeCheck<OrConstraint, And<AtLeastNArgs<1>, All<Numeric> > > OrCT;


class RandConstraint : public Constraint {
 public:
  RandConstraint(const std::string& name,
                 const std::string& propagatorName,
                 const ConstraintEngineId constraintEngine,
                 const std::vector<ConstrainedVariableId>& variables);
  void handleExecute();
 private:
  unsigned int m_rvalue;
};
typedef DataTypeCheck<RandConstraint, And<NArgs<1>, All<Numeric> > > RandCT;

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
  RotateScopeRightConstraint(const std::string& name,
                             const std::string& propagatorName,
                             const ConstraintEngineId constraintEngine,
                             const std::vector<ConstrainedVariableId>& variables) __attribute__((noreturn))
  : Constraint(name, propagatorName, constraintEngine, variables), 
    m_otherConstraint() {
    // Called via REGISTER_NARY() macro's factory rather than via the
    //   REGISTER_ROTATED_NARY() macro's factory: not enough information
    //   to create the constraint.
    assertTrue(false);
  }

  RotateScopeRightConstraint(const std::string& name,
                             const std::string& propagatorName,
                             const ConstraintEngineId constraintEngine,
                             const std::vector<ConstrainedVariableId>& variables,
                             const std::string& otherName,
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

/**
 * @brief Computes the sine of a given variable. Varable is in degrees. The constraint is a function
 * rather than a relation. The range of the source variable must be in [0 90].
 */
class SineFunction : public Constraint {
 public:
  SineFunction(const std::string& name,
               const std::string& propagatorName,
               const ConstraintEngineId constraintEngine,
               const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  static const unsigned int ARG_COUNT = 2;
  Domain& m_target;
  Domain& m_source;
};
//TODO: Fix this syntax, too
typedef DataTypeCheck<SineFunction, And<NArgs<2>, And<First<Assignable<FloatDT> >,
                                                      Second<Assignable<FloatDT> > > > >
SineCT;

/**
 * @brief SquareOfDifference(x, y, a) maintains the relation:
 * @li a = (x - y)^2
 * if x and y are singleton.
 */

class SquareOfDifferenceConstraint : public Constraint {
 public:
  SquareOfDifferenceConstraint(const std::string& name,
                               const std::string& propagatorName,
                               const ConstraintEngineId constraintEngine,
                               const std::vector<ConstrainedVariableId>& variables);
  
  void handleExecute();
  
 private:
  static const unsigned int V1 = 0;
  static const unsigned int V2 = 1;
  static const unsigned int RES = 2;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<SquareOfDifferenceConstraint,
                      And<NArgs<3>, And<All<Assignable<Last<> >, First<>, End>,
                                        First<CanBePositive> > > >
SquareOfDifferenceCT;

/**
 * @brief Maintains a unary relation from a constant to a variable such that the variable
 * is a subset of the given constant.
 */

class SubsetOfConstraint : public Constraint {
 public:
  SubsetOfConstraint(const std::string& name,
                     const std::string& propagatorName,
                     const ConstraintEngineId constraintEngine,
                     const std::vector<ConstrainedVariableId>& variables);

  ~SubsetOfConstraint();

  void handleExecute();

  bool canIgnore(const ConstrainedVariableId variable,
                 unsigned int argIndex,
                 const DomainListener::ChangeType& changeType);

 private:
  Domain& m_currentDomain;
  Domain& m_superSetDomain;
};
//TODO: fix this syntax
typedef DataTypeCheck<SubsetOfConstraint, And<NArgs<2>,
                                              All<Comparable<Second<> >, First<>, Last<> > > >
SubsetOfCT;
                      

class SwapTwoVarsConstraint : public Constraint {
 public:
  SwapTwoVarsConstraint(const std::string& name,
                        const std::string& propagatorName,
                        const ConstraintEngineId constraintEngine,
                        const std::vector<ConstrainedVariableId>& variables) __attribute__((noreturn))
  : Constraint(name, propagatorName, constraintEngine, variables), m_otherConstraint() {
    // Called via REGISTER_NARY() macro's factory rather than via the
    //   REGISTER_SWAP_TWO_VARS_NARY() macro's factory: not enough information
    //   to create the constraint.
    assertTrue(false);
  }
  
    SwapTwoVarsConstraint(const std::string& name,
                          const std::string& propagatorName,
                          const ConstraintEngineId constraintEngine,
                          const std::vector<ConstrainedVariableId>& variables,
                          const std::string& otherName,
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
  TestAnd(const std::string& name,
          const std::string& propagatorName,
          const ConstraintEngineId constraintEngine,
          const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  Domain& m_test;
  Domain& m_arg1;
  Domain& m_arg2;
  static const unsigned int ARG_COUNT = 3;
};
struct TestCondition : And<NArgs<3>, First<Type<BoolDT> > > {
  TestCondition() : And<NArgs<3>, First<Type<BoolDT> > >() {}
  TestCondition(type_iterator start, type_iterator end)
      : And<NArgs<3>, First<Type<BoolDT> > >(start, end) {}
};

typedef And<NArgs<3>, All<Type<BoolDT> > > ThreeBooleanArgs;
typedef DataTypeCheck<TestAnd, And<TestCondition, ThreeBooleanArgs> > TestAndCT;

class TestEQ : public Constraint {
 public:
  TestEQ(const std::string& name,
         const std::string& propagatorName,
         const ConstraintEngineId constraintEngine,
         const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  Domain& m_test;
  Domain& m_arg1;
  Domain& m_arg2;
  static const unsigned int ARG_COUNT = 3;
};
typedef And<TestCondition, Mutually<Comparable<>, Second<>, End> > TestCompareCondition;
typedef DataTypeCheck<TestEQ, TestCompareCondition> TestEQCT;


class TestLessThan : public Constraint {
 public:
  TestLessThan(const std::string& name,
               const std::string& propagatorName,
               const ConstraintEngineId constraintEngine,
               const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  Domain& m_test;
  Domain& m_arg1;
  Domain& m_arg2;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<TestLessThan, TestCompareCondition> TestLessThanCT;

class TestLEQ : public Constraint {
 public:
  TestLEQ(const std::string& name,
          const std::string& propagatorName,
          const ConstraintEngineId constraintEngine,
          const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  Domain& m_test;
  Domain& m_arg1;
  Domain& m_arg2;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<TestLEQ, TestCompareCondition> TestLEQCT;

class TestNEQ : public Constraint {
 public:
  TestNEQ(const std::string& name,
          const std::string& propagatorName,
          const ConstraintEngineId constraintEngine,
          const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  Domain& m_test;
  Domain& m_arg1;
  Domain& m_arg2;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<TestNEQ, TestCompareCondition> TestNEQCT;

class TestOr : public Constraint {
 public:
  TestOr(const std::string& name,
         const std::string& propagatorName,
         const ConstraintEngineId constraintEngine,
         const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  Domain& m_test;
  Domain& m_arg1;
  Domain& m_arg2;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<TestOr, ThreeBooleanArgs> TestOrCT;

class TestSingleton : public Constraint {
 public:
  TestSingleton(const std::string& name,
                const std::string& propagatorName,
                const ConstraintEngineId constraintEngine,
                const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();
  const std::vector<ConstrainedVariableId>& getModifiedVariables() const;

 private:
  Domain& m_test;
  Domain& m_arg1;
  std::vector<ConstrainedVariableId> m_modifiedVariables;
  static const unsigned int ARG_COUNT = 2;
};
typedef DataTypeCheck<TestSingleton, And<NArgs<2>, First<Type<BoolDT> > > > TestSingletonCT;

class TestSpecified : public Constraint {
 public:
  TestSpecified(const std::string& name,
                const std::string& propagatorName,
                const ConstraintEngineId constraintEngine,
                const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  Domain& m_test;
  Domain& m_arg1;
  static const unsigned int ARG_COUNT = 2;
};
typedef DataTypeCheck<TestSpecified, And<NArgs<2>, First<Type<BoolDT> > > > TestSpecifiedCT;

class UnaryConstraint : public Constraint {
 public:
  /**
   * @brief Specialized constructor
   */
  UnaryConstraint(const Domain& dom, const ConstrainedVariableId var);

  /**
   * @brief Standard constructor
   */
  UnaryConstraint(const std::string& name,
                  const std::string& propagatorName,
                  const ConstraintEngineId constraintEngine,
                  const std::vector<ConstrainedVariableId>& variables);

  ~UnaryConstraint();

 private:
  UnaryConstraint(const UnaryConstraint&);
  UnaryConstraint& operator=(const UnaryConstraint&);
  void handleExecute();

  void handleDiscard();

  bool canIgnore(const ConstrainedVariableId variable,
                 unsigned int argIndex,
                 const DomainListener::ChangeType& changeType);

  void setSource(const ConstraintId sourceConstraint);

  Domain* m_x;
  Domain* m_y;
};
typedef DataTypeCheck<UnaryConstraint, NArgs<1> > UnaryCT;

  /**
   * @brief WithinBounds(x, y, z) maintains the relations:
   * @li x.lb >= y.lb
   * @li x.ub <= z.ub
   * @li y <= z
   */
class WithinBounds : public Constraint {
 public:
  WithinBounds(const std::string& name,
               const std::string& propagatorName,
               const ConstraintEngineId constraintEngine,
               const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();

 private:
  IntervalDomain& m_x;
  IntervalDomain& m_y;
  IntervalDomain& m_z;
  LessThanEqualConstraint m_leq;
  static const unsigned int ARG_COUNT = 3;
};
typedef DataTypeCheck<WithinBounds, And<NArgs<3>, And<All<Numeric>,
                                                      Mutually<Comparable<> > > > >
WithinBoundsCT;


class EqUnionConstraint : public Constraint {
 public:
  EqUnionConstraint(const std::string& name,
                    const std::string& propagatorName,
                    const ConstraintEngineId constraintEngine,
                    const std::vector<ConstrainedVariableId>& variables);

  void handleExecute();
  void handleExecuteInterval(IntervalDomain& dest,
                             std::vector<ConstrainedVariableId>::const_iterator start,
                             const std::vector<ConstrainedVariableId>::const_iterator end);
  void handleExecuteEnumerated(EnumeratedDomain& dest,
                               std::vector<ConstrainedVariableId>::const_iterator start,
                               const std::vector<ConstrainedVariableId>::const_iterator end);

};
typedef DataTypeCheck<EqUnionConstraint, And<AtLeastNArgs<2>,
                                             All<Assignable<First<> >, Second<>, End > > >
EqUnionCT;
}
#endif
