#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstraintLibrary.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "Domain.hh"
#include "Utils.hh"
#include <cmath>

namespace Prototype {

  /**
   * Utility class that might get promoted later.
   */
  class Infinity {
  public:
    static double plus(double n1, double n2, double defaultValue) {
      // Why cast to int and use abs()?  Why not just use fabs()? --wedgingt 2004 Feb 24
      if (abs((int)n1) == PLUS_INFINITY || abs((int)n2) == PLUS_INFINITY)
	return(defaultValue);
      return(n1 + n2);
    }

    static double minus(double n1, double n2, double defaultValue) {
      // Why cast to int and use abs()?  Why not just use fabs()? --wedgingt 2004 Feb 24
      if (abs((int)n1) == PLUS_INFINITY || abs((int)n2) == PLUS_INFINITY)
	return(defaultValue);
      return(n1 - n2);
    }
  };

  AddEqualConstraint::AddEqualConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
    for (int i = 0; i < ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  void AddEqualConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domz = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

    check_error(AbstractDomain::canBeCompared(domx, domy));
    check_error(AbstractDomain::canBeCompared(domx, domz));
    check_error(AbstractDomain::canBeCompared(domz, domy));

    // Test preconditions for continued execution.
    // Should this be part of canIgnore() instead? --wedgingt 2004 Feb 24
    if (domx.isOpen() ||
        domy.isOpen() ||
        domz.isOpen())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty() && !domz.isEmpty());

    double xMin, xMax, yMin, yMax, zMin, zMax;
    domx.getBounds(xMin, xMax);
    domy.getBounds(yMin, yMax);
    domz.getBounds(zMin, zMax);

    // Process Z
    double xMax_plus_yMax = Infinity::plus(xMax, yMax, zMax);
    if (zMax > xMax_plus_yMax)
      zMax = domz.translateNumber(xMax_plus_yMax, false);

    double xMin_plus_yMin = Infinity::plus(xMin, yMin, zMin);
    if (zMin < xMin_plus_yMin)
      zMin = domz.translateNumber(xMin_plus_yMin, true);

    if (domz.intersect(zMin, zMax) && domz.isEmpty())
      return;

    // Process X
    double zMax_minus_yMin = Infinity::minus(zMax, yMin, xMax);
    if (xMax > zMax_minus_yMin)
      xMax = domx.translateNumber(zMax_minus_yMin, false);

    double zMin_minus_yMax = Infinity::minus(zMin, yMax, xMin);
    if (xMin < zMin_minus_yMax)
      xMin = domx.translateNumber(zMin_minus_yMax, true);

    if (domx.intersect(xMin, xMax) && domx.isEmpty())
      return;

    // Process Y
    double yMaxCandidate = Infinity::minus(zMax, xMin, yMax);
    if (yMax > yMaxCandidate)
      yMax = domy.translateNumber(yMaxCandidate, false);

    double yMinCandidate = Infinity::minus(zMin, xMax, yMin);
    if (yMin < yMinCandidate)
      yMin = domy.translateNumber(yMinCandidate, true);

    if (domy.intersect(yMin,yMax) && domy.isEmpty())
      return;

    /* Now, rounding issues from mixed numeric types can lead to the
     * following inconsistency, not yet caught.  We handle it here
     * however, by emptying the domain if the invariant post-condition
     * is not satisfied. The motivating case for this: A:Int[-10,10] +
     * B:Int[-10,10] == C:Real[0.01, 0.99].
     */
    if (!domz.isMember(Infinity::plus(yMax, xMin, zMin)) ||
        !domz.isMember(Infinity::plus(yMin, xMax, zMin)))
      domz.empty();
  }

  EqualConstraint::EqualConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables), m_argCount(variables.size()) {
    // Check the arguments.  Any that are not singleton must all be
    //   enumerations or must all be intervals.
    // Could support a mix of enumerations and intervals.  Not doing
    //   so might permit arbitrary and pointless search during planning
    //   in some problem domains. --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 21
    AbstractDomain& first = getCurrentDomain(m_variables[0]);
    bool requiresEnumeration = first.isEnumerated() && !first.isSingleton();
    for (unsigned int i = 1; i < m_argCount; i++) {
      const AbstractDomain& current = m_variables[i]->lastDomain();
      requiresEnumeration = requiresEnumeration || (current.isEnumerated() && !current.isSingleton());
      check_error(!requiresEnumeration || current.isEnumerated() || current.isSingleton());
      check_error(AbstractDomain::canBeCompared(first,
                                                getCurrentDomain(m_variables[i])));
      // This constraint has problems similar to CondAllSameConstraint's
      // related to minDelta().  @see CondAllSameConstraint::CondAllSameConstraint.
    }
  }

  /**
   * @brief Restrict all variables to the intersection of their domains.
   * @note Will only restrict closed domains.
   * @note In the worst case, this algorithm requires 2 passes over
   * the variables.
   * @note Should also restrict open domains (closing them and copying
   * the members of the intersection of the closed domains) if there
   * are any closed domains, but that is not supported elsewhere
   * presently, it seems.
   * --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 22
   */
  void EqualConstraint::handleExecute() {
    check_error(isActive());

    unsigned int i = 0;
    for ( ; i < m_argCount; i++)
      if (!getCurrentDomain(m_variables[i]).isOpen())
        break;
    if (i >= m_argCount)
      return;

    // Start from the first closed domain.
    AbstractDomain& closedDom = getCurrentDomain(m_variables[i]);
    check_error(!closedDom.isEmpty());
    bool changedOne = true;

    // This loop will run at most twice.
    while (changedOne) {
      changedOne = false;
      for (unsigned int j = i + 1; j < m_argCount; j++) {
        AbstractDomain& otherDom(getCurrentDomain(m_variables[j]));
        if (!otherDom.isOpen() && closedDom.equate(otherDom)) {
          if (closedDom.isEmpty() || otherDom.isEmpty())
            return;
          changedOne = true;
        }
      }
    }

  }

  AbstractDomain& EqualConstraint::getCurrentDomain(const ConstrainedVariableId& var) {
    return(Constraint::getCurrentDomain(var));
  }

  SubsetOfConstraint::SubsetOfConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_isDirty(true),
      m_currentDomain(getCurrentDomain(variables[0])),
      m_superSetDomain(getCurrentDomain(variables[1])),
      m_executionCount(0) {
    check_error(variables.size() == 2);
    check_error(AbstractDomain::canBeCompared(m_currentDomain, m_superSetDomain));
  }

  SubsetOfConstraint::~SubsetOfConstraint() {}

  void SubsetOfConstraint::handleExecute() {
    m_currentDomain.intersect(m_superSetDomain);
    m_isDirty = false;
    m_executionCount++;
  }

  bool SubsetOfConstraint::canIgnore(const ConstrainedVariableId& variable,
				     int argIndex,
				     const DomainListener::ChangeType& changeType) {
    check_error(argIndex == 0);
    return(changeType != DomainListener::RELAXED);
  }

  int SubsetOfConstraint::executionCount() const {
    return(m_executionCount);
  }

  LessThanEqualConstraint::LessThanEqualConstraint(const LabelStr& name,
						   const LabelStr& propagatorName,
						   const ConstraintEngineId& constraintEngine,
						   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void LessThanEqualConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));

    check_error(AbstractDomain::canBeCompared(domx, domy));

    // Discontinue if either domain is open.
    if (domx.isOpen() || domy.isOpen())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    // Discontinue if any domain is enumerated but not a singleton.
    // Would not have to do this if enumerations were sorted. --wedgingt 2004 Feb 24
    if (domx.isEnumerated() && !domx.isSingleton())
      return;
    if (domy.isEnumerated() && !domy.isSingleton())
      return;

    // Restrict X to be no larger than Y's max
    if (domx.intersect(domx.getLowerBound(), domy.getUpperBound()) && domx.isEmpty())
      return;

    // Restrict Y to be at least X's min
    domy.intersect(domx.getLowerBound(), domy.getUpperBound());
  }

  bool LessThanEqualConstraint::canIgnore(const ConstrainedVariableId& variable,
					  int argIndex,
					  const DomainListener::ChangeType& changeType) {
    return((argIndex == X &&
	    (changeType == DomainListener::UPPER_BOUND_DECREASED)) ||
	   (argIndex == Y &&
	    (changeType == DomainListener::LOWER_BOUND_INCREASED)));
  }

  NotEqualConstraint::NotEqualConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
  }

  void NotEqualConstraint::handleExecute() {
    AbstractDomain& domx = getCurrentDomain(m_variables[X]);
    AbstractDomain& domy = getCurrentDomain(m_variables[Y]);

    check_error(AbstractDomain::canBeCompared(domx, domy), "Cannot compare " + domx.toString() + " and " + domy.toString() + ".");

    // Discontinue if either domain is open.
    if (domx.isOpen() || domy.isOpen())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    if(!checkAndRemove(domx, domy))
      checkAndRemove(domy, domx);
  }

  bool NotEqualConstraint::checkAndRemove(const AbstractDomain& domx, AbstractDomain& domy){
    if(!domx.isSingleton())
      return false;

    double value = domx.getSingletonValue();

    // Not present, so nothing to remove
    if(!domy.isMember(value)){
      return false;
    }

    if(!domy.isEnumerated() && (!domy.isFinite() || domy.getSize() > 2))
      return false;

    // Now exectute the removal based on the data
    domy.remove(value);
    return false;
  }

  bool NotEqualConstraint::canIgnore(const ConstrainedVariableId& variable,
				     int argIndex,
				     const DomainListener::ChangeType& changeType) {
    if(changeType==DomainListener::RESET || changeType == DomainListener::RELAXED)
      return false;

    const AbstractDomain& domain = variable->lastDomain();

    if(domain.isSingleton() ||
       (domain.isInterval() && domain.isFinite() && domain.getSize() <=2 )) // Since this transition is key for propagation
       return false;

    return true;
  }

  LessThanConstraint::LessThanConstraint(const LabelStr& name,
                                         const LabelStr& propagatorName,
                                         const ConstraintEngineId& constraintEngine,
                                         const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_lessThanEqualConstraint(LabelStr("Internal:lessThanEqual"), propagatorName,
                                constraintEngine, m_variables),
      m_notEqualConstraint(LabelStr("Internal:notEqual"), propagatorName,
                                constraintEngine, m_variables) {
    check_error(m_variables.size() == 2);
  }

  MultEqualConstraint::MultEqualConstraint(const LabelStr& name,
                                           const LabelStr& propagatorName,
                                           const ConstraintEngineId& constraintEngine,
                                           const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
    for (int i = 0; i < ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  /**
   * @brief Return larger argument.
   * @note Shouldn't be here, but in a generic "arithmetic" class or system library.
   * @note max() is a macro in some compiler implementations. --wedgingt 2004 Feb 26
   */
  double max(double a, double b) {
    return (a > b ? a : b);
  }

  /**
   * @brief Return smaller argument.
   * @note Shouldn't be here, but in a generic "arithmetic" class or system library.
   * @note min() is a macro in some compiler implementations. --wedgingt 2004 Feb 26
   */
  double min(double a, double b) {
    return(a < b ? a : b);
  }

  /**
   * @brief Helper method to compute new bounds for both X and Y in X*Y == Z.
   * @return True if the target domain was modified.
   */
  bool updateMinAndMax(IntervalDomain& targetDomain,
		       double denomMin, double denomMax,
		       double numMin, double numMax) {
    double xMax = targetDomain.getUpperBound();
    double xMin = targetDomain.getLowerBound();
    double newMin = xMin;
    double newMax = xMax;

    // If the denominator could be zero, the result could be anything
    //   except for some sign related restrictions.
    if (denomMin <= 0.0 && denomMax >= 0.0) {
      if ((numMin >= 0.0 && denomMin > 0.0) ||
          (numMax <= 0.0 && denomMax < 0.0)) {
        if (targetDomain.intersect(0.0, xMax))
          return(true);
      } else {
        if ((numMax <= 0.0 && denomMin > 0.0) ||
            (numMin >= 0.0 && denomMax < 0.0)) {
          if (targetDomain.intersect(xMin, 0.0))
            return(true);
        }
      }
      return(false);
    }
    check_error(denomMin != 0.0 && denomMax != 0.0);

    // Otherwise we must examine min and max of all pairings to deal with signs correctly.
    newMax = max(max(numMax / denomMin, numMin / denomMin),
                 max(numMax / denomMax, numMin/ denomMax));
    newMin = min(min(numMax / denomMin, numMin / denomMin),
                 min(numMax / denomMax, numMin/ denomMax));

    if (xMax > newMax)
      xMax = targetDomain.translateNumber(newMax, false);
    if (xMin < newMin)
      xMin = targetDomain.translateNumber(newMin, true);

    return(targetDomain.intersect(xMin, xMax));
  }

  void MultEqualConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domz = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

    check_error(AbstractDomain::canBeCompared(domx, domy));
    check_error(AbstractDomain::canBeCompared(domx, domz));
    check_error(AbstractDomain::canBeCompared(domz, domy));

    /* Test preconditions for continued execution. */
    /* Could support one open domain, but only very messily due to REAL_ENUMERATED case. */
    if (domx.isOpen() ||
        domy.isOpen() ||
        domz.isOpen())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty() && !domz.isEmpty());

    double xMin, xMax, yMin, yMax, zMin, zMax;
    for (bool done = false; !done; ) {
      done = true;
      domx.getBounds(xMin, xMax);
      domy.getBounds(yMin, yMax);
      domz.getBounds(zMin, zMax);

      // Process Z
      double max_z = max(max(xMax * yMax, xMin * yMin), max(xMin * yMax, xMax * yMin));
      if (zMax > max_z)
        zMax = domz.translateNumber(max_z, false);

      double min_z = min(min(xMax * yMax, xMin * yMin), min(xMin * yMax, xMax * yMin));
      if (zMin < min_z)
        zMin = domz.translateNumber(min_z, true);

      if (domz.intersect(zMin, zMax) && domz.isEmpty())
        return;

      // Process X
      if (updateMinAndMax(domx, yMin, yMax, zMin, zMax)) {
        if (domx.isEmpty())
          return;
        else
          done = false;
      }

      // Process Y
      if (updateMinAndMax(domy, xMin, xMax, zMin, zMax)) {
        if (domy.isEmpty())
          return;
        else
          done = false;
      }
    }
  }

  /**
   * @class AddMultEqualConstraint
   * @brief A + (B*C) == D
   */
  AddMultEqualConstraint::AddMultEqualConstraint(const LabelStr& name,
                                                 const LabelStr& propagatorName,
                                                 const ConstraintEngineId& constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_interimVariable(constraintEngine, IntervalDomain(), false, LabelStr("InternalConstraintVariable"), getId()),
      m_multEqualConstraint(LabelStr("Internal::multEqual"), propagatorName, constraintEngine,
			    makeScope(m_variables[B], m_variables[C], m_interimVariable.getId())),
      m_addEqualConstraint(LabelStr("Internal:addEqual"), propagatorName, constraintEngine,
			   makeScope(m_interimVariable.getId(), m_variables[A], m_variables[D])) {
    check_error(m_variables.size() == (unsigned int) ARG_COUNT);
  }

  EqualSumConstraint::EqualSumConstraint(const LabelStr& name,
                                         const LabelStr& propagatorName,
                                         const ConstraintEngineId& constraintEngine,
                                         const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()),
      m_sum1(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqSumVariable"), getId()),
      m_sum2(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqSumVariable"), getId()),
      m_sum3(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqSumVariable"), getId()),
      m_sum4(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqSumVariable"), getId()) {
    check_error(ARG_COUNT > 2 && ARG_COUNT == (unsigned int)m_variables.size());
    std::vector<ConstrainedVariableId> scope;
    // B is always first and C is always second for the first set, so:
    scope.push_back(m_variables[1]); // B + ...
    scope.push_back(m_variables[2]); // ... C ...
    switch (ARG_COUNT) {
    case 3: // A = B + C
      scope.push_back(m_variables[0]); // ... = A
      m_eqSumC1 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 4: // A = (B + C) + D
      scope.push_back(m_sum1.getId()); // ... = (B + C)
      m_eqSumC1 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_sum1.getId()); // (B + C) ...
      scope.push_back(m_variables[3]); // ... + D = ...
      scope.push_back(m_variables[0]); // ... A
      m_eqSumC2 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 5: case 6: case 7:
      // 5: A = (B + C) + (D + E)
      // 6: A = (B + C) + (D + E + F)
      // 7: A = (B + C) + (D + E + F + G)
      // So, do (B + C) and (D + E ...) for all three:
      scope.push_back(m_sum1.getId()); // (B + C)
      m_eqSumC1 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_sum1.getId()); // (B + C) + ...
      scope.push_back(m_sum2.getId()); // (D + E ...) = ...
      scope.push_back(m_variables[0]); // A
      m_eqSumC2 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_variables[3]); // D + ...
      scope.push_back(m_variables[4]); // E ...
      switch (ARG_COUNT) {
      case 5:
        scope.push_back(m_sum2.getId()); // ... = (D + E)
        m_eqSumC3 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      case 6:
        scope.push_back(m_sum3.getId()); // ... = (D + E)
        m_eqSumC3 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_sum3.getId()); // (D + E) + ...
        scope.push_back(m_variables[5]); // ... F = ...
        scope.push_back(m_sum2.getId()); // ... (D + E + F)
        m_eqSumC4 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      case 7:
        scope.push_back(m_sum3.getId()); // ... = (D + E)
        m_eqSumC3 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_sum3.getId()); // (D + E) + ...
        scope.push_back(m_sum4.getId()); // ... (F + G) = ...
        scope.push_back(m_sum2.getId()); // (D + E + F + G)
        m_eqSumC4 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_variables[5]); // F + ...
        scope.push_back(m_variables[6]); // ... G = ...
        scope.push_back(m_sum4.getId()); // ... (F + G)
        m_eqSumC5 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      default:
        check_error(ALWAYS_FAILS);
        break;
      } /* switch (ARGCOUNT) 5, 6, 7 */
      break;
    default:
      { // A = first_half + second_half, recursively
        check_error(ARG_COUNT > 7);
        scope.clear(); // Was B + C for first set: those that only call AddEqual
        scope.push_back(m_sum1.getId()); // first_half + ...
        scope.push_back(m_sum2.getId()); // ... second_half = ...
        scope.push_back(m_variables[0]); // ... A
        m_eqSumC1 = (new AddEqualConstraint(LabelStr("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_sum1.getId()); // first_half = ...
        unsigned int half = ARG_COUNT/2;
        unsigned int i = 1;
        for ( ; i <= half; i++)
          scope.push_back(m_variables[i]); // ... X + ...
        m_eqSumC2 = (new EqualSumConstraint(LabelStr("EqualSum"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_sum2.getId()); // second_half = ...
        for ( ; i < ARG_COUNT; i++)
          scope.push_back(m_variables[i]); // ... Y + ...
        m_eqSumC3 = (new EqualSumConstraint(LabelStr("EqualSum"), propagatorName, constraintEngine, scope))->getId();
        break;
      }
      break;
    }
  }

  EqualSumConstraint::~EqualSumConstraint() {
    // Have to remove these before the variables they refer to
    //   and there's no other way to force the compiler to do
    //   these first. --wedgingt 2004 Feb 27
    if (!m_eqSumC5.isNoId())
      delete (Constraint*) m_eqSumC5;
    if (!m_eqSumC4.isNoId())
      delete (Constraint*) m_eqSumC4;
    if (!m_eqSumC3.isNoId())
      delete (Constraint*) m_eqSumC3;
    if (!m_eqSumC2.isNoId())
      delete (Constraint*) m_eqSumC2;
    if (!m_eqSumC1.isNoId())
      delete (Constraint*) m_eqSumC1;
  }

  EqualProductConstraint::EqualProductConstraint(const LabelStr& name,
                                                 const LabelStr& propagatorName,
                                                 const ConstraintEngineId& constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()),
      m_product1(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqProductVariable"), getId()),
      m_product2(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqProductVariable"), getId()),
      m_product3(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqProductVariable"), getId()),
      m_product4(constraintEngine, IntervalDomain(), false, LabelStr("InternalEqProductVariable"), getId()) {
    check_error(ARG_COUNT > 2 && ARG_COUNT == (unsigned int)m_variables.size());
    std::vector<ConstrainedVariableId> scope;
    // B is always first and C is always second for the first set, so:
    scope.push_back(m_variables[1]); // B * ...
    scope.push_back(m_variables[2]); // ... C ...
    switch (ARG_COUNT) {
    case 3: // A = B * C
      scope.push_back(m_variables[0]); // ... = A
      m_eqProductC1 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 4: // A = (B * C) * D
      scope.push_back(m_product1.getId()); // ... = (B * C)
      m_eqProductC1 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_product1.getId()); // (B * C) ...
      scope.push_back(m_variables[3]); // ... * D = ...
      scope.push_back(m_variables[0]); // ... A
      m_eqProductC2 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 5: case 6: case 7:
      // 5: A = (B * C) * (D * E)
      // 6: A = (B * C) * (D * E * F)
      // 7: A = (B * C) * (D * E * F * G)
      // So, do (B * C) and (D * E ...) for all three:
      scope.push_back(m_product1.getId()); // (B * C)
      m_eqProductC1 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_product1.getId()); // (B * C) * ...
      scope.push_back(m_product2.getId()); // (D * E ...) = ...
      scope.push_back(m_variables[0]); // A
      m_eqProductC2 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_variables[3]); // D * ...
      scope.push_back(m_variables[4]); // E ...
      switch (ARG_COUNT) {
      case 5:
        scope.push_back(m_product2.getId()); // ... = (D * E)
        m_eqProductC3 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      case 6:
        scope.push_back(m_product3.getId()); // ... = (D * E)
        m_eqProductC3 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product3.getId()); // (D * E) * ...
        scope.push_back(m_variables[5]); // ... F = ...
        scope.push_back(m_product2.getId()); // ... (D * E * F)
        m_eqProductC4 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      case 7:
        scope.push_back(m_product3.getId()); // ... = (D * E)
        m_eqProductC3 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product3.getId()); // (D * E) * ...
        scope.push_back(m_product4.getId()); // ... (F * G) = ...
        scope.push_back(m_product2.getId()); // (D * E * F * G)
        m_eqProductC4 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_variables[5]); // F * ...
        scope.push_back(m_variables[6]); // ... G = ...
        scope.push_back(m_product4.getId()); // ... (F * G)
        m_eqProductC5 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      default:
        check_error(ALWAYS_FAILS);
        break;
      } /* switch (ARGCOUNT) 5, 6, 7 */
      break;
    default:
      { // A = first_half * second_half, recursively
        check_error(ARG_COUNT > 7);
        scope.clear(); // Was B * C for first set: those that only call MultEqual
        scope.push_back(m_product1.getId()); // first_half * ...
        scope.push_back(m_product2.getId()); // ... second_half = ...
        scope.push_back(m_variables[0]); // ... A
        m_eqProductC1 = (new MultEqualConstraint(LabelStr("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product1.getId()); // first_half = ...
        unsigned int half = ARG_COUNT/2;
        unsigned int i = 1;
        for ( ; i <= half; i++)
          scope.push_back(m_variables[i]); // ... X * ...
        m_eqProductC2 = (new EqualProductConstraint(LabelStr("EqualProduct"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product2.getId()); // second_half = ...
        for ( ; i < ARG_COUNT; i++)
          scope.push_back(m_variables[i]); // ... Y * ...
        m_eqProductC3 = (new EqualProductConstraint(LabelStr("EqualProduct"), propagatorName, constraintEngine, scope))->getId();
        break;
      }
      break;
    }
  }

  EqualProductConstraint::~EqualProductConstraint() {
    // Have to remove these before the variables they refer to
    //   and there's no other way to force the compiler to do
    //   these first. --wedgingt 2004 Feb 27
    if (!m_eqProductC5.isNoId())
      delete (Constraint*) m_eqProductC5;
    if (!m_eqProductC4.isNoId())
      delete (Constraint*) m_eqProductC4;
    if (!m_eqProductC3.isNoId())
      delete (Constraint*) m_eqProductC3;
    if (!m_eqProductC2.isNoId())
      delete (Constraint*) m_eqProductC2;
    if (!m_eqProductC1.isNoId())
      delete (Constraint*) m_eqProductC1;
  }

  LessOrEqThanSumConstraint::LessOrEqThanSumConstraint(const LabelStr& name,
                                                       const LabelStr& propagatorName,
                                                       const ConstraintEngineId& constraintEngine,
                                                       const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_interimVariable(constraintEngine, IntervalDomain(), false, LabelStr("InternalConstraintVariable"), getId()),
      m_lessOrEqualConstraint(LabelStr("Internal:lessOrEqThanSum:lessOrEq"), propagatorName, constraintEngine,
                              makeScope(m_variables[0], m_interimVariable.getId())) {
    std::vector<ConstrainedVariableId> eqSumScope = m_variables;
    eqSumScope[0] = m_interimVariable.getId();
    m_eqSumConstraint = (new EqualSumConstraint(LabelStr("Internal:lessOrEqThanSum:eqSum"), propagatorName,
                                                constraintEngine, eqSumScope))->getId();
  }

  LessThanSumConstraint::LessThanSumConstraint(const LabelStr& name,
                                               const LabelStr& propagatorName,
                                               const ConstraintEngineId& constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_interimVariable(constraintEngine, IntervalDomain(), false, LabelStr("InternalConstraintVariable"), getId()),
      m_lessThanConstraint(LabelStr("Internal:lessThanSum:lessThan"), propagatorName, constraintEngine,
                           makeScope(m_variables[0], m_interimVariable.getId())) {
    std::vector<ConstrainedVariableId> eqSumScope = m_variables;
    eqSumScope[0] = m_interimVariable.getId();
    m_eqSumConstraint = (new EqualSumConstraint(LabelStr("Internal:lessThanSum:eqSum"), propagatorName,
                                                constraintEngine, eqSumScope))->getId();
    check_error(m_eqSumConstraint.isValid());
  }

  GreaterOrEqThanSumConstraint::GreaterOrEqThanSumConstraint(const LabelStr& name,
                                                       const LabelStr& propagatorName,
                                                       const ConstraintEngineId& constraintEngine,
                                                       const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_interimVariable(constraintEngine, IntervalDomain(), false, LabelStr("InternalConstraintVariable"), getId()),
      m_lessOrEqualConstraint(LabelStr("Internal:GreaterOrEqThanSum:lessThanEqual"), propagatorName, constraintEngine,
                              makeScope(m_interimVariable.getId(), m_variables[0])) {
    std::vector<ConstrainedVariableId> eqSumScope = m_variables;
    eqSumScope[0] = m_interimVariable.getId();
    m_eqSumConstraint = (new EqualSumConstraint(LabelStr("Internal:greaterOrEqThanSum:eqSum"), propagatorName,
                                                constraintEngine, eqSumScope))->getId();
  }

  GreaterThanSumConstraint::GreaterThanSumConstraint(const LabelStr& name,
                                               const LabelStr& propagatorName,
                                               const ConstraintEngineId& constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_interimVariable(constraintEngine, IntervalDomain(), false, LabelStr("InternalConstraintVariable"), getId()),
      m_lessThanConstraint(LabelStr("Internal:lessThanSum:lessOrEq"), propagatorName, constraintEngine,
                           makeScope(m_interimVariable.getId(), m_variables[0]))
  {
    std::vector<ConstrainedVariableId> eqSumScope = m_variables;
    eqSumScope[0] = m_interimVariable.getId();
    m_eqSumConstraint = (new EqualSumConstraint(LabelStr("Internal:greaterThanSum:eqSum"), propagatorName,
                                                constraintEngine, eqSumScope))->getId();
    check_error(m_eqSumConstraint.isValid());
  }

  CondAllSameConstraint::CondAllSameConstraint(const LabelStr& name,
                                               const LabelStr& propagatorName,
                                               const ConstraintEngineId& constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()) {
    check_error(ARG_COUNT > 2);
    check_error(getCurrentDomain(m_variables[0]).getType() == AbstractDomain::BOOL);
    for (unsigned int i = 2; i < ARG_COUNT; i++) {
      check_error(AbstractDomain::canBeCompared(getCurrentDomain(m_variables[1]),
                                                getCurrentDomain(m_variables[i])));

      // If this second condition is not enforced, the value used for
      // minDelta() in handleExecute() depends on the order of the
      // variables within the scope.  It should, in fact, probably be
      // enforced by AbstractDomain::canBeCompared().  Another
      // possibility would be to change the '==' here to '<=' ...
      // Or permit any pairing of numeric domains here since we're
      // doing equality tests, which don't really care about minDelta().
      // Presently, e.g., this precludes real and integer intervals
      // from appearing in the same constraint, which isn't really a problem.
      // --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 21
      check_error(getCurrentDomain(m_variables[1]).minDelta() ==
                  getCurrentDomain(m_variables[i]).minDelta());

    }
  }

  void CondAllSameConstraint::handleExecute() {
    BoolDomain& boolDom = static_cast<BoolDomain&>(getCurrentDomain(m_variables[0]));
    check_error(!boolDom.isOpen());

    if (!boolDom.isSingleton()) {
      // Condition is not singleton, so try to restrict it:
      // A. If all of the others are singleton and equal, the condition is true.
      // B. If the others have an empty intersection, the condition is false.
      // As with singleton false case, we can do nothing if any are open.
      bool canProveTrue = true;
      AbstractDomain* common = 0;
      double single = 0.0;
      for (unsigned int i = 1; !boolDom.isSingleton() && i < ARG_COUNT; i++) {
        AbstractDomain& current(getCurrentDomain(m_variables[i]));
        if (current.isOpen()) {
          canProveTrue = false;
          continue;
        }
        if (!current.isSingleton()) {
          canProveTrue = false;
          if (common == 0) {
            // First one: copy it to intersect with later ones.
            common = current.copy();

            // Skipping this check merely results in less efficient propagation.
            // check_error(common != 0);

          } else {
            // Intersect common with this one; if now empty, the other
            // variables cannot all have the same value, so remove true
            // from the condition.
            if (common->intersect(current) && common->isEmpty())
              boolDom.remove(true);
          }
          continue;
        } /* if !current.isSingleton() */
        if (canProveTrue) {
          if (i == 1)
            single = current.getSingletonValue();
          else
            if (fabs(single - current.getSingletonValue()) >= getCurrentDomain(m_variables[1]).minDelta()) {
              // Two singletons with different values: can't be all same, so:
              canProveTrue = false;
              boolDom.remove(true);
            }
        } /* if canProveTrue */
      } /* for i = 1; !boolDom.isSingleton && i < ARG_COUNT; i++ */
      if (canProveTrue)
        boolDom.remove(false);
      // Before it goes out of scope:
      if (common != 0)
        delete common;
    } /* if !boolDom.isSingleton */

    // Whether the condition was singleton on entry to this function
    // or became singleton just above, propagate the effects of that
    // to the other variables in the scope.
    if (boolDom.isSingleton()) {
      if (!boolDom.getSingletonValue()) {
        // Singleton false: ensure at least one other var can have a
        // value different than some third var.  If any are open
        // or more than one is not singleton, none can be restricted.

        unsigned int j = 1;
        while (j < ARG_COUNT - 1 && !getCurrentDomain(m_variables[j]).isOpen()
               && !getCurrentDomain(m_variables[j]).isSingleton())
          ++j;
        AbstractDomain& domj = getCurrentDomain(m_variables[j]);
        if (domj.isOpen() || !domj.isSingleton())
          return; // Can ignore relax events until condition var is relaxed.
        double single = domj.getSingletonValue();
        unsigned int foundOneToTrim = 0;
        for (unsigned int i = 1; i < ARG_COUNT; i++) {
          if (i == j)
            continue;
          AbstractDomain& domi = getCurrentDomain(m_variables[i]);
          if (domi.isOpen())
            return; // Can ignore relax events until condition var is relaxed.
          if (domi.isSingleton() && domi.getSingletonValue() != single)
            return; // Can ignore relax events until condition var is relaxed.
          if (domi.isSingleton())
            continue; // Identical to first singleton.
          if (!domi.isMember(single))
            return; // Can ignore relax events until condition var is relaxed.
          if (foundOneToTrim > 0) // Two that overlap single but aren't singleton, so ...
            return; // ... either might or might not overlap in the future.
          foundOneToTrim = i;
        }

        // No open domains and at most one that is not singleton with member single.
        if (foundOneToTrim == 0) {
          // All are singleton with member single, so condition cannot be false,
          //   provoking an inconsistency:
          boolDom.remove(false);
          return;
        }

        // Exactly one other var is not singleton with lone member single and
        // even that one var's domain contains single as a member, so remove
        // single to enforce the constraint.
        AbstractDomain& domToTrim = getCurrentDomain(m_variables[foundOneToTrim]);
        // But it can only be trimmed if it is enumerated (including boolean) or
        // if it is an integer interval with one of the end points equal to
        // single, which is painful to check.
        if (domToTrim.isEnumerated() || domToTrim.getType() == AbstractDomain::BOOL
            || (domToTrim.getType() == AbstractDomain::INT_INTERVAL
                && (domj.isMember(domToTrim.getLowerBound())
                    || domj.isMember(domToTrim.getUpperBound()))))
          domToTrim.remove(single);
        else
          ; // Can ignore relax events until condition var is relaxed.
        return;
      } else {

        // Singleton true: force all other vars in scope to be equated if _any_ of them are closed..
        unsigned int i = 1;
        for ( ; i < ARG_COUNT; i++)
          if (!getCurrentDomain(m_variables[i]).isOpen())
            break;
        if (i == ARG_COUNT) // All of them are open; can't reduce any.
          return; // Can ignore relax events until condition var is relaxed.
        AbstractDomain& dom1 = getCurrentDomain(m_variables[1]);
        for (bool changedOne = true; changedOne; ) {
          changedOne = false;
          for (i = 2; i < ARG_COUNT; i++) {
            changedOne = dom1.equate(getCurrentDomain(m_variables[i])) || changedOne;
            if (dom1.isEmpty())
              return; // inconsistent: cannot all be the same but condition var requires they are.
          }
        }
      } // else of if (!boolDom.getSingletonValue()): singleton false
    } // else of if (boolDom.isSingleton())
  } // end of CondAllSameConstraint::handleExecute()

  CondAllDiffConstraint::CondAllDiffConstraint(const LabelStr& name,
                                               const LabelStr& propagatorName,
                                               const ConstraintEngineId& constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()) {
    check_error(ARG_COUNT > 2);
    check_error(getCurrentDomain(m_variables[0]).getType() == AbstractDomain::BOOL);
    for (unsigned int i = 2; i < ARG_COUNT; i++)
      check_error(AbstractDomain::canBeCompared(getCurrentDomain(m_variables[1]),
                                                getCurrentDomain(m_variables[i])));
  }

  /**
   * @brief Helper function: add domToAdd to unionOfDomains "usefully".
   * Adds all of the members of domToAdd and, if needed and useful,
   * more values to unionOfDomains.
   * @param unionOfDomains Pointer to new'd AbstractDomain, which may
   * be delete'd and new'd with a different concrete class by this
   * function.
   * @param domToAdd Set of values to add to unionOfDomains' concrete
   * (C++) domain object.
   * @note Can add too much without affecting CondAllDiffConstraint
   * other than by delaying propagation, so add a disjoint interval by
   * simply returning a single larger interval that 'covers' both
   * original intervals.
   */
  static void addToUnion(AbstractDomain **unionOfDomains,
                         const AbstractDomain& domToAdd) {
    check_error(unionOfDomains != 0 && *unionOfDomains != 0);
    check_error(!(*unionOfDomains)->isEmpty() && !(*unionOfDomains)->isOpen());
    check_error(!domToAdd.isEmpty() && !domToAdd.isOpen());
    AbstractDomain *newUnion = 0;
    std::list<double> membersToAdd;
    std::list<double> newMembers;
    if (((*unionOfDomains)->isEnumerated() || (*unionOfDomains)->isSingleton())
        && (domToAdd.isEnumerated() || domToAdd.isSingleton())) {
      if (domToAdd.isEnumerated())
        domToAdd.getValues(membersToAdd);
      else
        membersToAdd.push_back(domToAdd.getSingletonValue());
      if ((*unionOfDomains)->isEnumerated())
        (*unionOfDomains)->getValues(newMembers);
      else
        newMembers.push_back((*unionOfDomains)->getSingletonValue());
      for (std::list<double>::const_iterator it = membersToAdd.begin();
           it != membersToAdd.end(); it++) {
        std::list<double>::const_iterator it2 = newMembers.begin();
        for ( ; it2 != newMembers.end(); it2++)
          if (*it == *it2)
            break;
        if (it2 == newMembers.end())
          newMembers.push_back(*it);
      }
      newUnion = new EnumeratedDomain(newMembers, /* closed = */ true,
                                      (*unionOfDomains)->getListener(),
                                      (*unionOfDomains)->isNumeric());
      // Could just add to current unionOfDomains rather than failing here, but
      //   very messy to implement using current interface to *Domain classes.
      assertFalse(newUnion == 0);
      delete *unionOfDomains;
      *unionOfDomains = newUnion;
      return;
    }
    // At least one is a non-singleton interval, so the result will be
    //   also be one.
    double toAddMin, toAddMax, newMin, newMax;
    domToAdd.getBounds(toAddMin, toAddMax);
    (*unionOfDomains)->getBounds(newMin, newMax);
    bool changing = false;
    if (toAddMin < newMin) {
      newMin = toAddMin;
      changing = true;
    }
    if (newMax < toAddMax) {
      newMax = toAddMax;
      changing = true;
    }
    if (changing) {
      if (domToAdd.getType() == AbstractDomain::REAL_INTERVAL
          || (*unionOfDomains)->getType() == AbstractDomain::REAL_INTERVAL)
        newUnion = new IntervalDomain(newMin, newMax,
                                      (*unionOfDomains)->getListener());
      if (domToAdd.getType() == AbstractDomain::INT_INTERVAL
          || (*unionOfDomains)->getType() == AbstractDomain::INT_INTERVAL)
        newUnion = new IntervalIntDomain((int)newMin, (int)newMax,
                                         (*unionOfDomains)->getListener());
      /* BOOL should be not get to here since both are non-singleton
       *   but then unionOfDomains "covers" domToAdd and changing
       *   would be false.
       * USER_DEFINED and REAL_ENUMERATION should not get to here
       *   since enumerations are dealt with above.
       * As above, a memory failure here could be dealt with, but
       *   messy to implement, but note that this also checks the
       *   assumptions/logic earlier in this comment.
       */
      assertFalse(newUnion == 0);
      delete *unionOfDomains;
      *unionOfDomains = newUnion;
      return;
    }
  }

  void CondAllDiffConstraint::handleExecute() {
    BoolDomain& boolDom = static_cast<BoolDomain&>(getCurrentDomain(m_variables[0]));
    check_error(!boolDom.isOpen());

    /* Whether the condition is singleton or not, try to restrict it:
     * A. If all pairs of the other's domains are disjoint, the
     *    condition is true.
     * B. If the union of any set of the others has cardinality less
     *    than their count, the condition is false.
     */
    bool canProveTrue = true;
    unsigned int firstNonDynamic = 0;
    unsigned int firstDynamic = 0;
    unsigned int i = 1;
    AbstractDomain* unionOfOthers = 0;
    for ( ; i < ARG_COUNT; i++) {
      AbstractDomain& current(getCurrentDomain(m_variables[i]));
      if (current.isOpen()) {
        canProveTrue = false;
        if (firstDynamic == 0)
          firstDynamic = i;
        continue;
      }
      if (firstNonDynamic == 0)
        firstNonDynamic = i;
      if (unionOfOthers == 0) {
        // First (non-dynamic) one: copy it to union with later ones.
        unionOfOthers = current.copy();

        // Skipping this check merely results in less efficient propagation.
        // check_error(unionOfOthers != 0);

      } else {
        /* Intersect current with union of priors: if non-empty, the
           condition cannot be proven true since further restrictions
           could make two of the variables have the same singleton values.
        */
        if (canProveTrue && unionOfOthers->intersects(current))
          canProveTrue = false;
        // Add members of current to unionOfOthers "usefully".
        addToUnion(&unionOfOthers, current);
        if (unionOfOthers->isFinite() && (unsigned int)(unionOfOthers->getSize()) < i) {
          // At least two of the variables must have same value.
          boolDom.remove(true);
          canProveTrue = false;
          if (boolDom.isEmpty())
            break; // Would return except for delete of unionOfOthers.
        }
      }
    } // for ( ; i < ARG_COUNT; i++)
    // Don't need this below, so:
    if (unionOfOthers != 0)
      delete unionOfOthers;
    if (canProveTrue)
      boolDom.remove(false);

    /* Whether the condition was singleton on entry to this function
     * or became singleton just above, propagate the effects of that
     * to the other variables in the scope.  But nothing can be done
     * if there are no other non-dynamic domains.
     */
    if (!boolDom.isSingleton() || firstNonDynamic == 0)
      return; // Cannot restrict any of the other vars.

    if (!boolDom.getSingletonValue()) {
      /* Condition var is singleton false: at least two other vars
       * must have same value.  Any dynamic domain could potentially
       * be narrowed to any single value, so there's nothing further
       * to do if there are any dynamic domains.
       */
      if (firstDynamic > 0)
        return;
      for (i = 1; i < ARG_COUNT - 1; i++) {
        AbstractDomain& iDom = getCurrentDomain(m_variables[i]);
        for (unsigned int j = i; ++j < ARG_COUNT; ) {
          if (iDom.intersects(getCurrentDomain(m_variables[j])))
            // The two vars have overlapping domains, so constraint is satisfied.
            return;
        }
      }
      /* Could not find two vars that might have same value, so
       * the condition var must be true and this constraint is
       * violated.
       */
      boolDom.remove(false);
      return;
    }
    /* Then condition var is singleton true: force all other vars to be distinct.
     * If no other var is singleton, no (easy) way to restrict other
     *   any other var, so find singletons and remove them from the other
     *   (non-dynamic & finite) domains where the removal can be done
     *   without splitting intervals.
     * Harder: look for x vars that have at most y values where x >= y.
     *   Similar checks already made above, so effect here would be minimal.
     */
    for (bool changedOne = true; changedOne; ) {
      changedOne = false;
      // For each var ...
      for (i = firstNonDynamic; i < ARG_COUNT; i++) {
        // ... that has a singleton domain ...
        if (getCurrentDomain(m_variables[i]).isOpen() ||
            !getCurrentDomain(m_variables[i]).isSingleton())
          continue;
        AbstractDomain& singletonDom = getCurrentDomain(m_variables[i]);
        // ... go thru the other vars ...
        for (unsigned int j = firstNonDynamic; j < ARG_COUNT; j++) {
          if (j == i)
            continue;
          AbstractDomain& jDom = getCurrentDomain(m_variables[j]);
          // ... that have non-dynamic and finite domains ...
          if (!jDom.isOpen() && jDom.isFinite() &&
              // ... looking for one that contains singletonDom's value.
              // In an enumeration, any member can be removed:
              ((jDom.isEnumerated() && jDom.isMember(singletonDom.getSingletonValue()))
               // In an interval, can only remove an endpoint:
               || (!jDom.isEnumerated() && (singletonDom.isMember(jDom.getLowerBound())
                                            || singletonDom.isMember(jDom.getUpperBound()))))) {
            // Found one: remove singletonDom's value.
            jDom.remove(singletonDom.getSingletonValue());
            if (jDom.isEmpty())
              return;
            // No point in going thru again if we're just starting.
            changedOne = (i > firstNonDynamic);
          } // if !jDom.isOpen ...
        } // for unsigned int j = firstNonDynamic; ...
      } // for i = firstNonDynamic; i < ARG_COUNT; i++
    } // for changedOne = true; changedOne;
  } // end of CondAllDiffConstraint::handleExecute()

  AllDiffConstraint::AllDiffConstraint(const LabelStr& name,
                                       const LabelStr& propagatorName,
                                       const ConstraintEngineId& constraintEngine,
                                       const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_condVar(constraintEngine, BoolDomain(true), false, LabelStr("Internal:AllDiff:cond"), getId())
  {
    std::vector<ConstrainedVariableId> condAllDiffScope;
    condAllDiffScope.reserve(m_variables.size() + 1);
    condAllDiffScope.push_back(m_condVar.getId());
    condAllDiffScope.insert(condAllDiffScope.end(), m_variables.begin(), m_variables.end());
    check_error(m_variables.size() + 1 == condAllDiffScope.size());
    m_condAllDiffConstraint = (new CondAllDiffConstraint(LabelStr("Internal:AllDiff:condAllDiff"), propagatorName,
                                                         constraintEngine, condAllDiffScope))->getId();
  }

  MemberImplyConstraint::MemberImplyConstraint(const LabelStr& name,
                                               const LabelStr& propagatorName,
                                               const ConstraintEngineId& constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()) {
    check_error(ARG_COUNT == 4);
    check_error(AbstractDomain::canBeCompared(getCurrentDomain(m_variables[0]),
                                              getCurrentDomain(m_variables[1])));
    check_error(AbstractDomain::canBeCompared(getCurrentDomain(m_variables[2]),
                                              getCurrentDomain(m_variables[3])));
  }

  void MemberImplyConstraint::handleExecute() {
    AbstractDomain& domA(getCurrentDomain(m_variables[0]));
    AbstractDomain& domB(getCurrentDomain(m_variables[1]));
    AbstractDomain& domC(getCurrentDomain(m_variables[2]));
    AbstractDomain& domD(getCurrentDomain(m_variables[3]));

    assertFalse(domA.isEmpty() || domB.isEmpty() || domC.isEmpty() || domD.isEmpty());
    
    if (domA.isOpen() || domB.isOpen() || domD.isOpen())
      return;
    if (domA.isSubsetOf(domB))
      (void)domC.intersect(domD);
  }

  CountZerosConstraint::CountZerosConstraint(const LabelStr& name,
                                             const LabelStr& propagatorName,
                                             const ConstraintEngineId& constraintEngine,
                                             const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    for (unsigned int i = 0; i < m_variables.size(); i++)
      check_error(getCurrentDomain(m_variables[i]).isNumeric());
  }

  void CountZerosConstraint::handleExecute() {
    unsigned int i = 1;

    // Count the other vars that must be zero ...
    unsigned int minZeros = 0;
    // ... and that could be zero.
    unsigned int maxZeros = 0;
    for ( ; i < m_variables.size(); i++) {
      AbstractDomain& other = getCurrentDomain(m_variables[i]);
      if (other.isMember(0.0)) {
        ++maxZeros;
        if (other.isSingleton())
          ++minZeros;
      }
    }

    // The count of zeros is the first variable.
    AbstractDomain& countDom = getCurrentDomain(m_variables[0]);

    // If all that could be zero must be zero to get the count high
    // enough, set all that could be zero to zero.
    if (minZeros < countDom.getLowerBound() &&
        maxZeros == countDom.getLowerBound()) {
      // Find those that could be zero but might not be
      // and restrict them to 0.
      for (i = 1; i < m_variables.size(); i++) {
        AbstractDomain& other = getCurrentDomain(m_variables[i]);
        if (other.isMember(0.0) && !other.isSingleton())
          other.set(0.0);
      }
    }

    // If all that might be zero are needed to be non-zero to get the
    // count low enough, restrict all that might be zero to not be
    // zero.
    if (maxZeros > countDom.getUpperBound() &&
        minZeros == countDom.getUpperBound()) {
      // Find those that could be zero but might not be and restrict
      // them to not be 0.
      for (i = 1; i < m_variables.size(); i++) {
        AbstractDomain& other = getCurrentDomain(m_variables[i]);
        if (other.isMember(0.0) && !other.isSingleton()) {
          // Can only remove 0.0 from integer interval domains if it
          // is an endpoint and can only remove 0.0 from real interval
          // domains if it is singleton ... which it can't be to get
          // here.
          if (other.isEnumerated())
            other.remove(0.0);
          else
            if (other.getType() != AbstractDomain::REAL_INTERVAL) {
              // Unfortunately, the easiest way to test correctly (due
              // to IntervalIntDomain::minDelta()) is rather messy:
              const IntervalDomain zeroDom(0.0);
              if (zeroDom.isMember(other.getLowerBound()) ||
                  zeroDom.isMember(other.getUpperBound()))
                other.remove(0.0);
            }
        }
      }
    }

    // If the counts seen restrict the count variable, do so.
    if (minZeros > countDom.getLowerBound() ||
        countDom.getUpperBound() > maxZeros)
      countDom.intersect(IntervalIntDomain(minZeros, maxZeros));
  }

  CountNonZerosConstraint::CountNonZerosConstraint(const LabelStr& name,
                                                   const LabelStr& propagatorName,
                                                   const ConstraintEngineId& constraintEngine,
                                                   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_zeros(constraintEngine, IntervalDomain(), false, LabelStr("InternalCountNonZerosVar"), getId()),
      m_otherVars(constraintEngine, IntervalDomain(), false, LabelStr("InternalCountNonZerosOtherVars"), getId()),
      m_superset(constraintEngine, IntervalDomain(variables.size() - 1), false, LabelStr("InternalCountNonZerosSuperset"), getId()),
      m_addEqualConstraint(LabelStr("Internal:CountNonZeros:addEqual"), propagatorName, constraintEngine,
                           makeScope(m_zeros.getId(), m_variables[0], m_otherVars.getId()))
  {
    m_subsetConstraint = (new SubsetOfConstraint(LabelStr("Internal:CountNonZeros:subSet"), propagatorName, constraintEngine,
                                                 makeScope(m_otherVars.getId(), m_superset.getId())))->getId();
    std::vector<ConstrainedVariableId> cZCScope = m_variables;
    cZCScope[0] = m_zeros.getId();
    check_error(m_variables.size() == cZCScope.size());
    m_countZerosConstraint = (new CountZerosConstraint(LabelStr("Internal:CountNonZeros:countZeros"),
                                                       propagatorName, constraintEngine, cZCScope))->getId();
  }

  CardinalityConstraint::CardinalityConstraint(const LabelStr& name,
                                               const LabelStr& propagatorName,
                                               const ConstraintEngineId& constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_nonZeros(constraintEngine, IntervalIntDomain(0, PLUS_INFINITY), false, LabelStr("InternalCardinalityVar"), getId()),
      m_lessThanEqualConstraint(LabelStr("Internal:CountNonZeros:lessThanEqual"), propagatorName,
                                constraintEngine, makeScope(m_nonZeros.getId(), m_variables[0]))
  {
    std::vector<ConstrainedVariableId> cCScope = m_variables;
    cCScope[0] = m_nonZeros.getId();
    check_error(m_variables.size() == cCScope.size());
    m_countNonZerosConstraint = (new CountNonZerosConstraint(LabelStr("Internal:Cardinality:countNonZeros"),
                                                             propagatorName, constraintEngine, cCScope))->getId();
  }

  OrConstraint::OrConstraint(const LabelStr& name,
                             const LabelStr& propagatorName,
                             const ConstraintEngineId& constraintEngine,
                             const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_nonZeros(constraintEngine, IntervalIntDomain(1, PLUS_INFINITY), false, LabelStr("InternalVar:Or:nonZeros"), getId()),
      m_superset(constraintEngine, IntervalIntDomain(1, variables.size()), false, LabelStr("InternalVar:Or:superset"), getId())
  {
    m_subsetConstraint = (new SubsetOfConstraint(LabelStr("Internal:CountNonZeros:subSet"), propagatorName, constraintEngine,
                                                 makeScope(m_nonZeros.getId(), m_superset.getId())))->getId();
    std::vector<ConstrainedVariableId> cNZCScope;
    cNZCScope.reserve(m_variables.size() + 1);
    cNZCScope.push_back(m_nonZeros.getId());
    cNZCScope.insert(cNZCScope.end(), m_variables.begin(), m_variables.end());
    check_error(m_variables.size() + 1 == cNZCScope.size());
    m_countNonZerosConstraint = (new CountNonZerosConstraint(LabelStr("Internal:Or:countNonZeros"), propagatorName,
                                                             constraintEngine, cNZCScope))->getId();
  }

  EqualMinimumConstraint::EqualMinimumConstraint(const LabelStr& name,
                                                 const LabelStr& propagatorName,
                                                 const ConstraintEngineId& constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(m_variables.size() > 1);
    for (unsigned int i = 0; i < m_variables.size(); i++)
      check_error(getCurrentDomain(m_variables[i]).isNumeric());
    // Should probably call AbstractDomain::canBeCompared() and check
    // minDelta() as well.
  }

  // If EqualMinConstraint::handleExecute's contributors were a class
  // data member, then EqualMinConstraint::canIgnore() could be quite
  // specific about events to ignore.  If the event(s) were also
  // available to handleExecute(), it could focus on just the changed
  // vars.

  void EqualMinimumConstraint::handleExecute() {
    AbstractDomain& minDom = getCurrentDomain(m_variables[0]);
    AbstractDomain& firstDom = getCurrentDomain(m_variables[1]);
    double minSoFar = firstDom.getLowerBound();
    double maxSoFar = firstDom.getUpperBound();
    std::set<unsigned int> contributors; /**< Set of var indices that "affect" minimum, or are affected by minDom's minimum. */
    contributors.insert(1);
    std::vector<ConstrainedVariableId>::iterator it = m_variables.begin();
    unsigned int i = 2;
    for (it++, it++; it != m_variables.end(); it++, i++) {
      AbstractDomain& curDom = getCurrentDomain(*it);
      if (maxSoFar < curDom.getLowerBound()) {
        // This variable doesn't "contribute" ...
        // ... but it might need to be trimmed by minDom; check:
        if (curDom.getLowerBound() < minDom.getLowerBound())
          contributors.insert(i);
        continue;
      }
      if (curDom.getUpperBound() < minSoFar) {
        // This variable completely "dominates" so far.
        minSoFar = curDom.getLowerBound();
        maxSoFar = curDom.getUpperBound();
        contributors.clear();
        contributors.insert(i);
        continue;
      }
      if (curDom.getUpperBound() < maxSoFar) {
        // This variable restricts the min's largest value.
        maxSoFar = curDom.getUpperBound();
        contributors.insert(i);
      }
      if (curDom.getLowerBound() < minSoFar) {
        // This variable contains the smallest values seen so far.
        minSoFar = curDom.getLowerBound();
        contributors.insert(i);
        continue;
      }
      if (curDom.getLowerBound() <= minDom.getUpperBound())
        contributors.insert(i);
    }
    if (minDom.intersect(IntervalDomain(minSoFar, maxSoFar)) && minDom.isEmpty())
      return;
    double minimum = minDom.getLowerBound();
    if (contributors.size() == 1) {
      // If there is only one other var that has a value in minDom,
      // it needs to be restricted to minDom to satisfy the constraint.
      i = *(contributors.begin());
      check_error(i > 0);
      AbstractDomain& curDom = getCurrentDomain(m_variables[i]);
      IntervalDomain restriction(minimum, minDom.getUpperBound());
      curDom.intersect(restriction);
      return; // No other contributors, so cannot be more to do.
    }
    while (!contributors.empty()) {
      i = *(contributors.begin());
      check_error(i > 0);
      contributors.erase(contributors.begin());
      AbstractDomain& curDom = getCurrentDomain(m_variables[i]);
      if (minimum < curDom.getLowerBound())
        continue;
      IntervalDomain restriction(minimum, curDom.getUpperBound());
      if (curDom.intersect(restriction) && curDom.isEmpty())
        return;
    }
  }

  EqualMaximumConstraint::EqualMaximumConstraint(const LabelStr& name,
                                                 const LabelStr& propagatorName,
                                                 const ConstraintEngineId& constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(m_variables.size() > 1);
    for (unsigned int i = 0; i < m_variables.size(); i++)
      check_error(getCurrentDomain(m_variables[i]).isNumeric());
    // Should probably call AbstractDomain::canBeCompared() and check
    // minDelta() as well.
  }

  // If EqualMaxConstraint::handleExecute's contributors were a class
  // data member, then EqualMaxConstraint::canIgnore() could be quite
  // specific about events to ignore.  If the event(s) were also
  // available to handleExecute(), it could focus on just the changed
  // vars.

  void EqualMaximumConstraint::handleExecute() {
    AbstractDomain& maxDom = getCurrentDomain(m_variables[0]);
    AbstractDomain& firstDom = getCurrentDomain(m_variables[1]);
    double minSoFar = firstDom.getLowerBound();
    double maxSoFar = firstDom.getUpperBound();
    std::set<unsigned int> contributors; /**< Set of var indices that "affect" maximum, or are affected by maxDom's maximum. */
    contributors.insert(1);
    std::vector<ConstrainedVariableId>::iterator it = m_variables.begin();
    unsigned int i = 2;
    for (it++, it++; it != m_variables.end(); it++, i++) {
      AbstractDomain& curDom = getCurrentDomain(*it);
      if (minSoFar > curDom.getUpperBound()) {
        // This variable doesn't "contribute" ...
        // ... but it might need to be trimmed by maxDom; check:
        if (curDom.getUpperBound() > maxDom.getUpperBound())
          contributors.insert(i);
        continue;
      }
      if (curDom.getLowerBound() > maxSoFar) {
        // This variable completely "dominates" so far.
        minSoFar = curDom.getLowerBound();
        maxSoFar = curDom.getUpperBound();
        contributors.clear();
        contributors.insert(i);
        continue;
      }
      if (curDom.getLowerBound() > minSoFar) {
        // This variable restricts the max's smallest value.
        minSoFar = curDom.getLowerBound();
        contributors.insert(i);
      }
      if (curDom.getUpperBound() > maxSoFar) {
        // This variable contains the largest values seen so far.
        maxSoFar = curDom.getUpperBound();
        contributors.insert(i);
        continue;
      }
      if (curDom.getUpperBound() >= maxDom.getLowerBound())
        contributors.insert(i);
    }
    if (maxDom.intersect(IntervalDomain(minSoFar, maxSoFar)) && maxDom.isEmpty())
      return;
    double maximum = maxDom.getUpperBound();
    if (contributors.size() == 1) {
      // If there is only one other var that has a value in maxDom,
      // it needs to be restricted to maxDom to satisfy the constraint.
      i = *(contributors.begin());
      check_error(i > 0);
      AbstractDomain& curDom = getCurrentDomain(m_variables[i]);
      IntervalDomain restriction(maxDom.getLowerBound(), maximum);
      curDom.intersect(restriction);
      return; // No other contributors, so cannot be more to do.
    }
    while (!contributors.empty()) {
      i = *(contributors.begin());
      check_error(i > 0);
      contributors.erase(contributors.begin());
      AbstractDomain& curDom = getCurrentDomain(m_variables[i]);
      if (maximum > curDom.getUpperBound())
        continue;
      IntervalDomain restriction(curDom.getLowerBound(), maximum);
      if (curDom.intersect(restriction) && curDom.isEmpty())
        return;
    }
  }

  CondEqualSumConstraint::CondEqualSumConstraint(const LabelStr& name,
                                                 const LabelStr& propagatorName,
                                                 const ConstraintEngineId& constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_sumVar(constraintEngine, IntervalDomain(), false, LabelStr("InternalConstraintVariable"), getId()),
      m_condAllSameConstraint(LabelStr("Internal:CondEqualSum:condAllSame"), propagatorName, constraintEngine,
                              makeScope(m_variables[0], m_variables[1], m_sumVar.getId()))
  {
    check_error(m_variables.size() > 2);
    std::vector<ConstrainedVariableId> eqSumScope;
    eqSumScope.reserve(m_variables.size() - 1);
    eqSumScope.push_back(m_sumVar.getId());
    std::vector<ConstrainedVariableId>::iterator it = m_variables.begin();
    ++it; ++it;
    eqSumScope.insert(eqSumScope.end(), it, m_variables.end());
    check_error(m_variables.size() - 1 == eqSumScope.size());
    m_eqSumConstraint = (new EqualSumConstraint(LabelStr("Internal:CondEqualSum:eqSum"), propagatorName,
                                                constraintEngine, eqSumScope))->getId();
    check_error(m_eqSumConstraint.isValid());
  }

  RotateScopeRightConstraint::RotateScopeRightConstraint(const LabelStr& name,
                                                         const LabelStr& propagatorName,
                                                         const ConstraintEngineId& constraintEngine,
                                                         const std::vector<ConstrainedVariableId>& variables,
                                                         const LabelStr& otherName,
                                                         const int& rotateCount)
    : Constraint(name, propagatorName, constraintEngine, variables)
  {
    check_error((unsigned) abs(rotateCount) < m_variables.size());
    std::vector<ConstrainedVariableId> otherScope;
    otherScope.reserve(m_variables.size());
    unsigned int i;
    if (rotateCount > 0) {
      // Rotate to right: last var becomes first, pushing others to the right.
      for (i = rotateCount; i > 0; i--)
        otherScope.push_back(m_variables[m_variables.size() - i]);
      for (i = 0; i < m_variables.size() - rotateCount; i++)
        otherScope.push_back(m_variables[i]);
    } else {
      // Rotate to left: first var becomes last, pushing others to the left.
      for (i = (unsigned) abs(rotateCount); i < m_variables.size(); i++)
        otherScope.push_back(m_variables[i]);
      for (i = 0; i < (unsigned) abs(rotateCount); i++)
        otherScope.push_back(m_variables[i]);
    }
    check_error(m_variables.size() == otherScope.size());
    m_otherConstraint = ConstraintLibrary::createConstraint(otherName, constraintEngine, otherScope);
  }

  SwapTwoVarsConstraint::SwapTwoVarsConstraint(const LabelStr& name,
                                               const LabelStr& propagatorName,
                                               const ConstraintEngineId& constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables,
                                               const LabelStr& otherName,
                                               int firstIndex, int secondIndex)
    : Constraint(name, propagatorName, constraintEngine, variables)
  {
    check_error((unsigned) abs(firstIndex) < m_variables.size());
    check_error((unsigned) abs(secondIndex) < m_variables.size());
    check_error(firstIndex != secondIndex);
    if (firstIndex < 0)
      firstIndex = m_variables.size() - firstIndex;
    if (secondIndex < 0)
      secondIndex = m_variables.size() - secondIndex;
    std::vector<ConstrainedVariableId> otherScope(m_variables);
    otherScope[firstIndex] = m_variables[secondIndex];
    otherScope[secondIndex] = m_variables[firstIndex];
    m_otherConstraint = ConstraintLibrary::createConstraint(otherName, constraintEngine, otherScope);
  }


  LockConstraint::LockConstraint(const LabelStr& name,
				 const LabelStr& propagatorName,
				 const ConstraintEngineId& constraintEngine,
				 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_currentDomain(getCurrentDomain(variables[0])),
      m_lockDomain(getCurrentDomain(variables[1])){
    check_error(variables.size() == 2);
  }

  LockConstraint::~LockConstraint() {
  }

  void LockConstraint::handleExecute() {
    // Only need to do something if they are not equal. So skip out.
    if(m_lockDomain == m_currentDomain)
      return;

    // If the current domain is a superset, then restrict it. 
    if(m_lockDomain.isSubsetOf(m_currentDomain))
       m_currentDomain.intersect(m_lockDomain);
    else
      m_currentDomain.empty(); // Otherwise, the lock is enforced by forcing a relaxation
  }

  // Enforces X >=0, Y<=0, X+Y==0
  NegateConstraint::NegateConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == 2);
    check_error(!variables[0]->baseDomain().isEnumerated());
    check_error(!variables[1]->baseDomain().isEnumerated());
  }

  void NegateConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));

    check_error(AbstractDomain::canBeCompared(domx, domy));
    check_error(!domx.isEmpty() && !domy.isEmpty());

    double xMin, xMax, yMin, yMax;
    domx.getBounds(xMin, xMax);
    domy.getBounds(yMin, yMax);

    // Prune immediately to enforce X >= 0 && Y <= 0.
    xMin = (xMin >= 0 ? xMin : 0);
    xMax = (xMax >= xMin ? xMax : xMin);
    yMax = (yMax <= 0 ? yMax : 0);
    yMin = (yMin <= yMax ? yMin : 0);

    if(domx.intersect(-yMax, -yMin) && domx.isEmpty())
      return;

    domy.intersect(-xMax, -xMin);
  }

  TestEqConstraint::TestEqConstraint(const LabelStr& name,
				     const LabelStr& propagatorName,
				     const ConstraintEngineId& constraintEngine,
				     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == (unsigned int) ARG_COUNT);
    AbstractDomain& domX = getCurrentDomain(variables[X]);
    AbstractDomain& domY = getCurrentDomain(variables[Y]);
    check_error(AbstractDomain::canBeCompared(domX, domY));                                                
  }
  
  void TestEqConstraint::handleExecute(){
    AbstractDomain& domX = getCurrentDomain(m_variables[X]);
    AbstractDomain& domY = getCurrentDomain(m_variables[Y]);
    AbstractDomain& domZ = getCurrentDomain(m_variables[Z]);

    check_error(!domX.isEmpty() && !domY.isEmpty() && !domZ.isEmpty());

    // If neither one is not a singleton, just ignore. Could do more, perhaps!
    if(!domX.isSingleton() || !domY.isSingleton())
      return;

    bool result = (domX == domY);
    domZ.set(result);
  }


  void initConstraintLibrary() {
    static bool s_runAlready(false);
    
    if (!s_runAlready) {
      // Register constraint Factories
      REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");
      REGISTER_CONSTRAINT(LockConstraint, "Lock", "Default");
      REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
      REGISTER_CONSTRAINT(AddEqualConstraint, "AddEqual", "Default");
      REGISTER_CONSTRAINT(AddMultEqualConstraint, "AddMultEqual", "Default");
      REGISTER_CONSTRAINT(AllDiffConstraint, "AllDiff", "Default");
      REGISTER_CONSTRAINT(CardinalityConstraint, "Cardinality", "Default");
      REGISTER_CONSTRAINT(CondAllDiffConstraint, "CondAllDiff", "Default");
      REGISTER_CONSTRAINT(CondAllSameConstraint, "CondAllSame", "Default");
      REGISTER_CONSTRAINT(CondEqualSumConstraint, "CondEqualSum", "Default");
      REGISTER_CONSTRAINT(CountNonZerosConstraint, "CountNonZeros", "Default");
      REGISTER_CONSTRAINT(CountZerosConstraint, "CountZeros", "Default");
      REGISTER_CONSTRAINT(OrConstraint, "Or", "Default");
      REGISTER_CONSTRAINT(EqualMaximumConstraint, "EqualMaximum", "Default");
      REGISTER_CONSTRAINT(EqualMinimumConstraint, "EqualMinimum", "Default");
      REGISTER_CONSTRAINT(EqualProductConstraint, "EqualProduct", "Default");
      REGISTER_CONSTRAINT(EqualSumConstraint, "EqualSum", "Default");
      REGISTER_CONSTRAINT(GreaterThanSumConstraint, "GreaterThanSum", "Default");
      REGISTER_CONSTRAINT(GreaterOrEqThanSumConstraint, "GreaterOrEqThanSum", "Default");
      REGISTER_CONSTRAINT(LessOrEqThanSumConstraint, "LessOrEqThanSum", "Default");
      REGISTER_CONSTRAINT(LessThanConstraint, "LessThan", "Default");
      REGISTER_CONSTRAINT(LessThanEqualConstraint, "LessThanEqual", "Default");
      REGISTER_CONSTRAINT(LessThanSumConstraint, "LessThanSum", "Default");
      REGISTER_CONSTRAINT(MemberImplyConstraint, "MemberImply", "Default");
      REGISTER_CONSTRAINT(MultEqualConstraint, "MultEqual", "Default");
      REGISTER_CONSTRAINT(NotEqualConstraint, "NotEqual", "Default");
      REGISTER_CONSTRAINT(TestEqConstraint, "TestEqual", "Default");

      // Europa (NewPlan/ConstraintNetwork) names for the same constraints:
      REGISTER_CONSTRAINT(AddEqualConstraint, "addeq", "Default");
      REGISTER_CONSTRAINT(NegateConstraint, "neg", "Default");
      REGISTER_CONSTRAINT(AddMultEqualConstraint, "addmuleq", "Default");
      REGISTER_CONSTRAINT(AllDiffConstraint, "adiff", "Default"); // all different
      REGISTER_CONSTRAINT(EqualConstraint, "asame", "Default"); // all same
      REGISTER_CONSTRAINT(CardinalityConstraint, "card", "Default"); // cardinality not more than
      REGISTER_CONSTRAINT(CountNonZerosConstraint, "cardeq", "Default"); // cardinality equals
      REGISTER_CONSTRAINT(CondAllSameConstraint, "condeq", "Default");
      REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
      REGISTER_CONSTRAINT(EqualConstraint, "fasame", "Default"); // flexible all same
      REGISTER_CONSTRAINT(OrConstraint, "for", "Default"); // flexible or
      REGISTER_CONSTRAINT(LessThanEqualConstraint, "leq", "Default");
      REGISTER_CONSTRAINT(LessOrEqThanSumConstraint, "leqsum", "Default");
      REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
      REGISTER_CONSTRAINT(MemberImplyConstraint, "memberImply", "Default");
      REGISTER_CONSTRAINT(NotEqualConstraint, "neq", "Default");
      REGISTER_CONSTRAINT(OrConstraint, "or", "Default");
      REGISTER_CONSTRAINT(EqualProductConstraint, "product", "Default");
      REGISTER_CONSTRAINT(EqualSumConstraint, "sum", "Default");

      // Rotate scope right one (last var moves to front) to ...
      // ... change addleq constraint to GreaterOrEqThan constraint:
      REGISTER_ROTATED_CONSTRAINT("addleq", "Default", "GreaterOrEqThanSum", 1);
      // ... change addlt constraint to GreaterThanSum constraint:
      REGISTER_ROTATED_CONSTRAINT("addlt", "Default", "GreaterThanSum", 1);
      // ... change max constraint to EqualMaximum constraint:
      REGISTER_ROTATED_CONSTRAINT("max", "Default", "EqualMaximum", 1);
      // ... change min constraint to EqualMinimum constraint:
      REGISTER_ROTATED_CONSTRAINT("min", "Default", "EqualMinimum", 1);

      // But addeqcond is harder, requiring two "steps":
      REGISTER_SWAP_TWO_VARS_CONSTRAINT("eqcondsum", "Default", "CondEqualSum", 0, 1);
      REGISTER_ROTATED_CONSTRAINT("addeqcond", "Default", "eqcondsum", 2);

      s_runAlready = true;
    }
  }
} // end namespace Prototype
