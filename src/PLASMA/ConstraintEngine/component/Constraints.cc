#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstraintType.hh"
#include "ConstrainedVariable.hh"
#include "Domains.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "CESchema.hh"
#include <algorithm>
#include <cmath>

namespace EUROPA {

  UnaryConstraint::UnaryConstraint(const Domain& dom,
				   const ConstrainedVariableId var)
    : Constraint("UNARY", "Default", var->getConstraintEngine(), makeScope(var)),
      m_x(dom.copy()),
      m_y(static_cast<Domain*>(& (getCurrentDomain(var)))) {
  }

  UnaryConstraint::UnaryConstraint(const std::string& name,
				   const std::string& propagatorName,
				   const ConstraintEngineId constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_x(0),
      m_y(static_cast<Domain*>(& (getCurrentDomain(variables[0])))) {
    checkError(variables.size() == 1, "Invalid arg count. " << toString());
  }

  /**
   * @brief Destructor will initiate custom discard.
   */
  UnaryConstraint::~UnaryConstraint(){
    discard(false);
  }

  void UnaryConstraint::handleExecute() {
    checkError(m_x != 0, "Source not set for " << toString());
    m_y->intersect(*m_x);
  }

  void UnaryConstraint::handleDiscard() {
    Constraint::handleDiscard();
    delete m_x;
    m_x = 0;
  }

  bool UnaryConstraint::canIgnore(const ConstrainedVariableId,
				  unsigned int argIndex,
				  const DomainListener::ChangeType& changeType){
    checkError(argIndex == 0, "Cannot have more than one variable in scope.");

    // Can ignore if this is a restriction of the variable which we can assume has already been
    // restricted by the constraint by initial execution of the constraint
    if(changeType == DomainListener::RESET || changeType == DomainListener::RELAXED)
      return false;
    else
      return true;
  }

  void UnaryConstraint::setSource(const ConstraintId sourceConstraint){
    checkError(m_x == 0, "Already set domain for " << toString() << " and not using " << sourceConstraint->toString());
    UnaryConstraint* source = id_cast<UnaryConstraint>(sourceConstraint);
    m_x = source->m_x->copy();
  }


  AddEqualConstraint::AddEqualConstraint(const std::string& name,
					 const std::string& propagatorName,
					 const ConstraintEngineId constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_x(getCurrentDomain(m_variables[X])),
      m_y(getCurrentDomain(m_variables[Y])),
      m_z(getCurrentDomain(m_variables[Z]))
  {
    check_error(variables.size() ==  ARG_COUNT);
  }


  /**
   * Utility class that might get promoted later.
   */
  class Infinity {
  public:
    static edouble plus(edouble n1, edouble n2, edouble defaultValue) {
      if (std::abs(n1) >= PLUS_INFINITY || std::abs(n2) >= PLUS_INFINITY)
	return(defaultValue);
      edouble retval = n1 + n2;
      if(std::abs(retval) >= PLUS_INFINITY)
	return defaultValue;
      return retval;
    }

    static edouble minus(edouble n1, edouble n2, edouble defaultValue) {
      if (std::abs(n1) >= PLUS_INFINITY || std::abs(n2) >= PLUS_INFINITY)
	return(defaultValue);
      edouble retval = n1 - n2;
      if(std::abs(retval) >= PLUS_INFINITY)
	return defaultValue;
      return retval;
    }
  };

  void AddEqualConstraint::handleExecute() {
    static unsigned int sl_counter(0);
    sl_counter++;
    debugMsg("AddEqualConstraint:handleExecute", toString() << " counter == " << sl_counter);
    check_error(Domain::canBeCompared(m_x, m_y));
    check_error(Domain::canBeCompared(m_x, m_z));
    check_error(Domain::canBeCompared(m_z, m_y));

    // Test preconditions for continued execution.
    if (m_x.isOpen() ||
        m_y.isOpen() ||
        m_z.isOpen())
      return;

    edouble xMin, xMax, yMin, yMax, zMin, zMax;
    m_x.getBounds(xMin, xMax);
    m_y.getBounds(yMin, yMax);
    m_z.getBounds(zMin, zMax);

    // Process Z
    edouble xMax_plus_yMax = Infinity::plus(xMax, yMax, zMax);
    if (zMax > xMax_plus_yMax)
      zMax = m_z.translateNumber(xMax_plus_yMax, false);

    edouble xMin_plus_yMin = Infinity::plus(xMin, yMin, zMin);
    if (zMin < xMin_plus_yMin)
      zMin = m_z.translateNumber(xMin_plus_yMin, true);

    if (m_z.intersect(zMin, zMax) && m_z.isEmpty())
      return;

    // Process X
    edouble zMax_minus_yMin = Infinity::minus(zMax, yMin, xMax);
    if (xMax > zMax_minus_yMin)
      xMax = m_x.translateNumber(zMax_minus_yMin, false);

    edouble zMin_minus_yMax = Infinity::minus(zMin, yMax, xMin);
    if (xMin < zMin_minus_yMax)
      xMin = m_x.translateNumber(zMin_minus_yMax, true);

    if (m_x.intersect(xMin, xMax) && m_x.isEmpty())
      return;

    // Process Y
    edouble yMaxCandidate = Infinity::minus(zMax, xMin, yMax);
    if (yMax > yMaxCandidate)
      yMax = m_y.translateNumber(yMaxCandidate, false);

    edouble yMinCandidate = Infinity::minus(zMin, xMax, yMin);
    if (yMin < yMinCandidate)
      yMin = m_y.translateNumber(yMinCandidate, true);

    if (m_y.intersect(yMin,yMax) && m_y.isEmpty())
      return;


    /* Now, rounding issues from mixed numeric types can lead to the
     * following inconsistency, not yet caught.  We handle it here
     * however, by emptying the domain if the invariant post-condition
     * is not satisfied. The motivating case for this: A:Int[-10,10] +
     * B:Int[-10,10] == C:Real[0.01, 0.99].
     */
    if (m_z.isInterval() &&
	(!m_z.isMember(Infinity::plus(yMax, xMin, zMin)) ||
	 !m_z.isMember(Infinity::plus(yMin, xMax, zMin))))
      m_z.empty();
  }

  /*********** MultEqualConstraint: X*Y = Z *************/
  MultEqualConstraint::MultEqualConstraint(const std::string& name,
                                           const std::string& propagatorName,
                                           const ConstraintEngineId constraintEngine,
                                           const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() ==  ARG_COUNT);
    for (unsigned int i = 0; i < ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }
/*
  bool MultEqualConstraint::updateMinAndMax(IntervalDomain& targetDomain,
              edouble denomMin, edouble denomMax,
              edouble numMin, edouble numMax) {
    edouble xMax = targetDomain.getUpperBound();
    edouble xMin = targetDomain.getLowerBound();
    edouble newMin = xMin;
    edouble newMax = xMax;

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
    newMax = std::max(std::max(numMax / denomMin, numMin / denomMin),
                 std::max(numMax / denomMax, numMin/ denomMax));
    newMin = std::min(std::min(numMax / denomMin, numMin / denomMin),
                 std::min(numMax / denomMax, numMin/ denomMax));

    if (xMax > newMax)
      xMax = targetDomain.translateNumber(newMax, false);
    if (xMin < newMin)
      xMin = targetDomain.translateNumber(newMin, true);

    // Update the bounds of targetDomain
    return(targetDomain.intersect(xMin, xMax));
  }

  void MultEqualConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domz = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

    check_error(Domain::canBeCompared(domx, domy));
    check_error(Domain::canBeCompared(domx, domz));
    check_error(Domain::canBeCompared(domz, domy));

    // Test preconditions for continued execution.
    // Could support one open domain, but only very messily due to REAL_ENUMERATED case.
    if (domx.isOpen() ||
        domy.isOpen() ||
        domz.isOpen())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty() && !domz.isEmpty());

    edouble xMin, xMax, yMin, yMax, zMin, zMax;
    for (bool done = false; !done; ) {
      done = true;
      domx.getBounds(xMin, xMax);
      domy.getBounds(yMin, yMax);
      domz.getBounds(zMin, zMax);

      // Process Z = X * Y
      edouble max_z = std::max(std::max(xMax * yMax, xMin * yMin), std::max(xMin * yMax, xMax * yMin));
      if (zMax > max_z)
        zMax = domz.translateNumber(max_z, false);

      edouble min_z = std::min(std::min(xMax * yMax, xMin * yMin), std::min(xMin * yMax, xMax * yMin));
      if (zMin < min_z)
        zMin = domz.translateNumber(min_z, true);

      // Minh: Why doesn't output error when dom(x or y or z) is empty (after update)?
      if (domz.intersect(zMin, zMax) && domz.isEmpty())
        return;

      // Process X = Z / Y
      if (updateMinAndMax(domx, yMin, yMax, zMin, zMax)) {
        if (domx.isEmpty())
          return;
        else
          done = false;
      }

      // Process Y = Z / X
      if (updateMinAndMax(domy, xMin, xMax, zMin, zMax)) {
        if (domy.isEmpty())
          return;
        else
          done = false;
      }
    }
  }
*/

  /*
   * Some helper functions to help with the bound updates
   */
  // Update Z's domain bounds using domains of X and Y in: X * Y = Z.
  // Return TRUE if we have new bounds for Z
namespace {
bool updateMultBounds(IntervalDomain& domZ,
                      const IntervalDomain& domX,
                      const IntervalDomain& domY)
{
  // Can not do any update if the domain of any variable is empty
  if(domX.isEmpty() || domY.isEmpty() || domZ.isEmpty())
    return false;

  edouble xMin, yMin, xMax, yMax, zMin, zMax, a, b, c, d;
  domX.getBounds(xMin, xMax);
  domY.getBounds(yMin, yMax);

  a = xMin * yMin;
  b = xMin * yMax;
  c = xMax * yMin;
  d = xMax * yMax;

  zMin = std::min(std::min(a,b),std::min(c,d));
  zMax = std::max(std::max(a,b),std::max(c,d));

  return domZ.intersect(zMin,zMax);
}

// Update Z's domain bounds using domains of X and Y in: X / Y = Z.
// Return TRUE if we have new bounds for Z
// NOTE: given the division-by-zero problem, there will be many different scenarios that need
// special treatments
bool updateDivBounds(IntervalDomain& domZ,
                     const IntervalDomain& domX,
                     const IntervalDomain& domY)
{
  // Can not do any update if the domain of any variable is empty
  if(domX.isEmpty() || domY.isEmpty() || domZ.isEmpty())
    return false;

  edouble xMin, yMin, xMax, yMax, zMin, zMax, a, b, c, d;
  domX.getBounds(xMin, xMax);
  domY.getBounds(yMin, yMax);


  // if [Y] = {0} (singleton 0 value)
  if(yMin == 0.0 && yMax == 0.0) {
    // If [X] doesn't contain 0 then inconsistent -> empty Z domain
    // NOTE: alternatively, [X] can be emptied instead of [Z] but it doesn't matter
    // with the current mechanism in Europa
    if((xMin > 0.0 || xMax < 0.0)) {
      domZ.empty();
      return true;
    } else {
      // If 0 \in [X] then no bound tightening (because 0/0 can be anything)
      return false;
    }
  }

  // If 0 \in [Y] but 0 is not one of the bounds then no tightening
  // NOTE: regardless of the bounds of X because [Y] contains both positive & negative values
  if(yMin < 0.0 && yMax > 0.0)
    return false;

  // If [Y] doesn't contain 0. Safely compute the two possible new bounds
  if(yMin > 0.0 || yMax < 0.0) {
    a = xMin / yMin;
    b = xMin / yMax;
    c = xMax / yMin;
    d = xMax / yMax;

    zMin = std::min(std::min(a,b),std::min(c,d));
    zMax = std::max(std::max(a,b),std::max(c,d));
    return domZ.intersect(zMin,zMax);
  }

  // if lb[Y] = 0 OR ub[Y] = 0 and ub[Y] > lb[Y] (i.e., 0 is one but not both bounds of [Y])
  // and 0 \in [X] then there is no bound tightening (because 0/0 can be anything)
  if((yMin == 0.0 || yMax == 0.0) && (xMin <= 0.0 && xMax >= 0.0))
    return false;

  /*
   * if lb[Y] = 0 OR ub[Y] = 0 and [X] doesn't contain 0, then we can compute one of the bounds
   */
  // One of the bounds will be \infinity so we will initialize both of the potential
  // new [Z]'s bounds with the existing bounds on [Z]
  domZ.getBounds(zMin, zMax);

  // Case 1 [+,+] / [0,+]: new bounds [xMin / yMax, +infinity]
  if(yMin == 0.0 && xMin > 0.0)
    zMin = xMin / yMax;

  // Case 2 [-,-] / [0,+] : new bounds [-infinity, xMax / yMax]
  if(yMin == 0.0 && xMax < 0.0)
    zMax = xMax / yMax;

  // Case 3 [+,+] / [-,0]: new bounds [-infinity, xMin /yMin]
  if(yMax == 0.0 && xMin > 0.0)
    zMax = xMin / yMin;

  // Case 4 [-,-] / [-,0]: new bounds [xMax / yMin, +infinity ]
  if(yMax == 0.0 && xMax < 0.0)
    zMin = xMax / yMin;

  // Set the new bounds
  return domZ.intersect(zMin,zMax);
}
}
  void MultEqualConstraint::handleExecute() {
    IntervalDomain& domX = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domY = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domZ = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

    check_error(Domain::canBeCompared(domX, domY));
    check_error(Domain::canBeCompared(domX, domZ));
    check_error(Domain::canBeCompared(domZ, domY));

    // Do not handle cases where one of the domains is open
    if (domX.isOpen() || domY.isOpen() || domZ.isOpen())
      return;

    // Domain should not be empty when this function is called
    check_error(!domX.isEmpty() && !domY.isEmpty() && !domZ.isEmpty());

    bool xChanged = true, yChanged = true, zChanged = true;

    // Repeat the update process until none of the three variables changed their (bound) values
    while(xChanged || yChanged || zChanged) {
      // Process Z = X * Y
      zChanged = updateMultBounds(domZ,domX,domY);

      // Process X = Z / Y
      xChanged = updateDivBounds(domX,domZ,domY);

      // Process Y = Z / X
      yChanged = updateDivBounds(domY,domZ,domX);
    }
  }


  /*********** DivEqualConstraint: X/Y = Z *************/
  DivEqualConstraint::DivEqualConstraint(const std::string& name,
                                           const std::string& propagatorName,
                                           const ConstraintEngineId constraintEngine,
                                           const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables)
  {
    check_error(variables.size() == ARG_COUNT);
    for (unsigned int i = 0; i < ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  void DivEqualConstraint::handleExecute()
  {
    IntervalDomain& domX = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domY = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domZ = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

    check_error(Domain::canBeCompared(domX, domY));
    check_error(Domain::canBeCompared(domX, domZ));
    check_error(Domain::canBeCompared(domZ, domY));

    // Test preconditions for continued execution.
    // Could support one open domain, but only very messily due to REAL_ENUMERATED case.
    if (domX.isOpen() ||  domY.isOpen() || domZ.isOpen())
      return;

    // Domain should not be empty when this function is called
    check_error(!domX.isEmpty() && !domY.isEmpty() && !domZ.isEmpty());

    bool xChanged = true, yChanged = true, zChanged = true;

    // Repeat the update process until none of the three variables changed their (bound) values
    while(xChanged || yChanged || zChanged) {
      // Process Z = X / Y
      zChanged = updateDivBounds(domZ,domX,domY);

      // Process X = Y * Z
      xChanged = updateMultBounds(domX,domY,domZ);

      // Process Y = X / Z
      yChanged = updateDivBounds(domY,domX,domZ);
    }
  }

  /*********** EqualConstraint *************/
  EqualConstraint::EqualConstraint(const std::string& name,
				   const std::string& propagatorName,
				   const ConstraintEngineId constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables), m_argCount(variables.size()) {}

  /**
   * @brief Restrict all variables to the intersection of their domains.
   * @see equate(const ConstrainedVariableId v1, const ConstrainedVariableId v2, bool& isEmpty) for details of handling
   * issues with open and closed domains.
   */
  void EqualConstraint::handleExecute() {
    check_error(isActive());

    bool changed = false;
    for(unsigned int i = 1; i < m_argCount; i++) {
      ConstrainedVariableId v1 = m_variables[i-1];
      ConstrainedVariableId v2 = m_variables[i];
      bool isEmpty = false;
      changed = equate(v1, v2, isEmpty) || changed;
      if(isEmpty)
        return;
    }

    //if the previous process changed the domains, we need to make sure that
    //the change occurs everywhere.  fortunately, since the n-1st variable
    //is now equal to the intersection of all of the variables,
    //we can just equate backwards and they should all be equal
    if(changed && m_argCount > 2) {
      for(unsigned long i = m_argCount - 2; i >= 1; i--) {
	ConstrainedVariableId v1 = m_variables[i];
	ConstrainedVariableId v2 = m_variables[i-1];
	bool isEmpty = false;
	equate(v1, v2, isEmpty);
	if(isEmpty)
	  return;
      }
    }
  }

  bool EqualConstraint::equate(const ConstrainedVariableId v1, const ConstrainedVariableId v2, bool& isEmpty){
    checkError(isEmpty == false, "Should be initially false.");
    Domain& d1 = getCurrentDomain(v1);
    Domain& d2 = getCurrentDomain(v2);

    bool changed = false;
    if((d1.isClosed() && d2.isClosed()) || (d1.isEnumerated() && d2.isEnumerated())){
      debugMsg("EqualConstraint:equate","before equate " << v1->toString() << " --- " << v2->toString());
      changed = d1.equate(d2);
      if(changed && (d1.isEmpty() || d2.isEmpty())) {
	debugMsg("EqualConstraint:equate","emptied variable " << v1->toString() << " --- " << v2->toString());
	      isEmpty = true;
      }
    }
    else {
      checkError(!d1.isInterval() && !d2.isInterval(),
		 v1->toString() << " should not be equated with " << v2->toString());

      std::list<edouble> d1_values;
      d1.getValues(d1_values);
      const Domain& d2_base = v2->baseDomain();
      while(!isEmpty && !d1_values.empty()){
	// if it not a member of d2 BUT a member of the base domain of v2, then we should exclude from d1.
	edouble value = d1_values.front();
	if(!d2.isMember(value) && (d2_base.isMember(value) || d2.isClosed())){
	  d1.remove(value);
	  changed = true;
	  isEmpty = d1.isEmpty();
	}
	d1_values.pop_front();
      }

      if(!isEmpty){
	std::list<edouble> d2_values;
	d2.getValues(d2_values);
	const Domain& d1_base = v1->baseDomain();
	while(!isEmpty && !d2_values.empty()){
	  // if it not a member of d2 BUT a member of the base domain of v2, then we should exclude from d1.
	  edouble value = d2_values.front();
	  if(!d1.isMember(value) && (d1_base.isMember(value) || d1.isClosed())){
	    d2.remove(value);
	    changed = true;
	    isEmpty = d2.isEmpty();
	  }
	  d2_values.pop_front();
	}
      }
    }

    return changed;
  }

  Domain& EqualConstraint::getCurrentDomain(const ConstrainedVariableId var) {
    return(Constraint::getCurrentDomain(var));
  }

  /************ SubsetOfConstraint ******************/
  SubsetOfConstraint::SubsetOfConstraint(const std::string& name,
					 const std::string& propagatorName,
					 const ConstraintEngineId constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_currentDomain(getCurrentDomain(variables[0])),
      m_superSetDomain(getCurrentDomain(variables[1])){
    check_error(variables.size() == 2);
    check_error(Domain::canBeCompared(m_currentDomain, m_superSetDomain));
  }

  SubsetOfConstraint::~SubsetOfConstraint() {}

  void SubsetOfConstraint::handleExecute() {
    m_currentDomain.intersect(m_superSetDomain);
  }

bool SubsetOfConstraint::canIgnore(const ConstrainedVariableId,
                                   unsigned int argIndex,
                                   const DomainListener::ChangeType& changeType){
  // If not a relaxation, and if it is the first argument, then we can ignore it as it will already be a subset
  if(changeType == DomainListener::RESET ||
     changeType == DomainListener::RELAXED ||
     argIndex == 1)
    return false;
  else
    return true;
}

  /*********** LessThanEqualConstraint *************/
  LessThanEqualConstraint::LessThanEqualConstraint(const std::string& name,
						   const std::string& propagatorName,
						   const ConstraintEngineId constraintEngine,
						   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_x(getCurrentDomain(variables[X])),
      m_y(getCurrentDomain(variables[Y])){
    checkError(variables.size() == ARG_COUNT, toString());
    checkError(m_x.isNumeric(), variables[X]->toString());
    checkError(m_y.isNumeric(), variables[Y]->toString());
  }

  void LessThanEqualConstraint::handleExecute() {
    propagate(m_x, m_y);
  }

  void LessThanEqualConstraint::propagate(Domain& m_x, Domain& m_y){
    check_error(Domain::canBeCompared(m_x, m_y));

    // Discontinue if either domain is open.
    if (m_x.isOpen() || m_y.isOpen())
      return;

    check_error(!m_x.isEmpty() && !m_y.isEmpty());

    // Restrict X to be no larger than Y's max
    debugMsg("LessThanEqualConstraint:handleExecute",
	     "Intersecting " << m_x.toString() << " with [" <<
	     m_x.getLowerBound() << " " << m_y.getUpperBound() << "]");

    if (m_x.intersect(m_x.getLowerBound(), m_y.getUpperBound()) && m_x.isEmpty())
      return;

    // Restrict Y to be at least X's min
    m_y.intersect(m_x.getLowerBound(), m_y.getUpperBound());
  }

bool LessThanEqualConstraint::canIgnore(const ConstrainedVariableId,
                                        unsigned int argIndex,
                                        const DomainListener::ChangeType& changeType) {
  return((argIndex == X &&
          (changeType == DomainListener::UPPER_BOUND_DECREASED)) ||
         (argIndex == Y &&
          (changeType == DomainListener::LOWER_BOUND_INCREASED)));
}

bool LessThanEqualConstraint::testIsRedundant(const ConstrainedVariableId var) const{
  if(Constraint::testIsRedundant(var))
    return true;
  
  if(getScope()[X]->baseDomain().getUpperBound() <= getScope()[Y]->baseDomain().getLowerBound())
    return true;
  
  return false;
}


  /*********** NotEqualConstraint *************/
  NotEqualConstraint::NotEqualConstraint(const std::string& name,
					 const std::string& propagatorName,
					 const ConstraintEngineId constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == ARG_COUNT);
  }

  void NotEqualConstraint::handleExecute() {
    Domain& domx = getCurrentDomain(m_variables[X]);
    Domain& domy = getCurrentDomain(m_variables[Y]);
    check_error(Domain::canBeCompared(domx, domy), "Cannot compare " + domx.toString() + " and " + domy.toString() + ".");
    // Discontinue if either domain is open.
    //if (domx.isOpen() || domy.isOpen())
    //  return;
    check_error(!domx.isEmpty() && !domy.isEmpty());

    if (!checkAndRemove(domx, domy))
      checkAndRemove(domy, domx);
  }

  bool NotEqualConstraint::checkAndRemove(const Domain& domx, Domain& domy) {
    if (!domx.isSingleton())
      return(false);
    edouble value = domx.getSingletonValue();
    // Not present, so nothing to remove.
    if (!domy.isMember(value))
      return(false);
    // If enumerated, remove it and be done with it.
    if (domy.isEnumerated()) {
      domy.remove(value);
      return(true);
    }
    // Since it is an interval, and it does contain the value, empty it if a singleton.
    if (domy.isSingleton()) {
	domy.empty();
	return(true);
    }
    // If it is a Boolean domain then set it to be the alternate
    if (domy.isBool()) {
      domy.set(!value);
      return(true);
    }

    if (domx.compareEqual(domx.getSingletonValue(), domy.getLowerBound())) {
      edouble low = domx.getSingletonValue() + domx.minDelta();
      domy.intersect(IntervalDomain(low, domy.getUpperBound()));
      return(true);
    }
    if (domx.compareEqual(domx.getSingletonValue(), domy.getUpperBound())) {
      edouble hi = domx.getSingletonValue() - domx.minDelta();
      domy.intersect(IntervalDomain(domy.getLowerBound(), hi));
      return(true);
    }
    /** COULD SPECIAL CASE INTERVAL INT DOMAIN, BUT NOT WORTH IT PROBABLY **/
    // Otherwise, we would have to split the interval, so do not propagate it
    return(false);
  }

bool NotEqualConstraint::canIgnore(const ConstrainedVariableId variable,
                                   unsigned int,
                                   const DomainListener::ChangeType& changeType) {
  if(changeType==DomainListener::RESET || changeType == DomainListener::RELAXED)
    return false;

  const Domain& domain = variable->lastDomain();

  if(domain.isSingleton() ||
     (domain.isInterval() && domain.isFinite() && domain.getSize() <=2 )) // Since this transition is key for propagation
    return false;

  return true;
}

  /*********** LessThanConstraint *************/
  LessThanConstraint::LessThanConstraint(const std::string& name,
                                         const std::string& propagatorName,
                                         const ConstraintEngineId constraintEngine,
                                         const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == ARG_COUNT);
  }

  void LessThanConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    propagate(domx, domy);
  }

  /**
   * @brief factored out propagation algorithm to allow re-use
   */
  void LessThanConstraint::propagate(IntervalDomain& domx, IntervalDomain& domy){
    check_error(Domain::canBeCompared(domx, domy), "Cannot compare " + domx.toString() + " and " + domy.toString() + ".");

    // Discontinue if either domain is open.
    if (domx.isOpen() || domy.isOpen())
      return;

    debugMsg("LessThanConstraint:handleExecute", "Computing " << domx.toString() << " < " << domy.toString() << " x.minDelta = " <<
	     domx.minDelta() << " y.minDelta = " << domy.minDelta());
    if(domx.getUpperBound() >= domy.getUpperBound() &&
       domy.getUpperBound() < PLUS_INFINITY &&
       domx.intersect(domx.getLowerBound(), domy.getUpperBound() - domx.minDelta()) &&
       domx.isEmpty())
      return;

    if(domy.getLowerBound() <= domx.getLowerBound() &&
       domx.getLowerBound() > MINUS_INFINITY &&
       domy.intersect(domx.getLowerBound() + domy.minDelta(), domy.getUpperBound()) &&
       domy.isEmpty())
      return;

    // Special handling for singletons, which could be infinite
    if(domx.isSingleton() && domy.isSingleton() && domx.getSingletonValue() >= domy.getSingletonValue()){
      domx.empty();
      return;
    }
  }

bool LessThanConstraint::canIgnore(const ConstrainedVariableId,
                                   unsigned int argIndex,
                                   const DomainListener::ChangeType& changeType) {
  return((argIndex == X &&
          (changeType == DomainListener::UPPER_BOUND_DECREASED)) ||
         (argIndex == Y &&
          (changeType == DomainListener::LOWER_BOUND_INCREASED)));
}

  /*********** EqualSumConstraint *************/
EqualSumConstraint::EqualSumConstraint(const std::string& name,
                                       const std::string& propagatorName,
                                       const ConstraintEngineId constraintEngine,
                                       const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()),
      m_eqSumC1(), m_eqSumC2(), m_eqSumC3(), m_eqSumC4(), m_eqSumC5(),
      m_sum1(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqSumVariable"), getId()),
      m_sum2(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqSumVariable"), getId()),
      m_sum3(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqSumVariable"), getId()),
      m_sum4(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqSumVariable"), getId()) {
  check_error(ARG_COUNT > 2 && ARG_COUNT == m_variables.size());
  std::vector<ConstrainedVariableId> scope;
  // B is always first and C is always second for the first set, so:
  scope.push_back(m_variables[1]); // B + ...
  scope.push_back(m_variables[2]); // ... C ...
  switch (ARG_COUNT) {
    case 3: // A = B + C
      scope.push_back(m_variables[0]); // ... = A
      m_eqSumC1 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 4: // A = (B + C) + D
      scope.push_back(m_sum1.getId()); // ... = (B + C)
      m_eqSumC1 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_sum1.getId()); // (B + C) ...
      scope.push_back(m_variables[3]); // ... + D = ...
      scope.push_back(m_variables[0]); // ... A
      m_eqSumC2 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 5: case 6: case 7:
      // 5: A = (B + C) + (D + E)
      // 6: A = (B + C) + (D + E + F)
      // 7: A = (B + C) + (D + E + F + G)
      // So, do (B + C) and (D + E ...) for all three:
      scope.push_back(m_sum1.getId()); // (B + C)
      m_eqSumC1 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_sum1.getId()); // (B + C) + ...
      scope.push_back(m_sum2.getId()); // (D + E ...) = ...
      scope.push_back(m_variables[0]); // A
      m_eqSumC2 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_variables[3]); // D + ...
      scope.push_back(m_variables[4]); // E ...
      switch (ARG_COUNT) {
        case 5:
          scope.push_back(m_sum2.getId()); // ... = (D + E)
          m_eqSumC3 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
          break;
        case 6:
          scope.push_back(m_sum3.getId()); // ... = (D + E)
          m_eqSumC3 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
          scope.clear();
          scope.push_back(m_sum3.getId()); // (D + E) + ...
          scope.push_back(m_variables[5]); // ... F = ...
          scope.push_back(m_sum2.getId()); // ... (D + E + F)
          m_eqSumC4 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
          break;
        case 7:
          scope.push_back(m_sum3.getId()); // ... = (D + E)
          m_eqSumC3 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
          scope.clear();
          scope.push_back(m_sum3.getId()); // (D + E) + ...
          scope.push_back(m_sum4.getId()); // ... (F + G) = ...
          scope.push_back(m_sum2.getId()); // (D + E + F + G)
          m_eqSumC4 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
          scope.clear();
          scope.push_back(m_variables[5]); // F + ...
          scope.push_back(m_variables[6]); // ... G = ...
          scope.push_back(m_sum4.getId()); // ... (F + G)
          m_eqSumC5 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
          break;
        default:
          check_error(ALWAYS_FAILS);
          break;
      } // switch (ARGCOUNT) 5, 6, 7
      break;
    default:
      { // A = first_half + second_half, recursively
        check_error(ARG_COUNT > 7);
        scope.clear(); // Was B + C for first set: those that only call AddEqual
        scope.push_back(m_sum1.getId()); // first_half + ...
        scope.push_back(m_sum2.getId()); // ... second_half = ...
        scope.push_back(m_variables[0]); // ... A
        m_eqSumC1 = (new AddEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_sum1.getId()); // first_half = ...
        unsigned long half = ARG_COUNT/2;
        unsigned long i = 1;
        for ( ; i <= half; i++)
          scope.push_back(m_variables[i]); // ... X + ...
        m_eqSumC2 = (new EqualSumConstraint(std::string("EqualSum"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_sum2.getId()); // second_half = ...
        for ( ; i < ARG_COUNT; i++)
          scope.push_back(m_variables[i]); // ... Y + ...
        m_eqSumC3 = (new EqualSumConstraint(std::string("EqualSum"), propagatorName, constraintEngine, scope))->getId();
        break;
      }
  }
}

  EqualSumConstraint::~EqualSumConstraint() {
    discard(false);
  }

  void EqualSumConstraint::handleDiscard(){
    // Process for this constraint first
    Constraint::handleDiscard();

    // Further unwind for contained variables    Constraint::handleDiscard();
    m_sum1.discard();
    m_sum2.discard();
    m_sum3.discard();
    m_sum4.discard();
  }

  /*********** EqualProductConstraint *************/
  EqualProductConstraint::EqualProductConstraint(const std::string& name,
                                                 const std::string& propagatorName,
                                                 const ConstraintEngineId constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()),
      m_eqProductC1(), m_eqProductC2(), m_eqProductC3(), m_eqProductC4(), m_eqProductC5(),
      m_product1(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqProductVariable"), getId()),
      m_product2(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqProductVariable"), getId()),
      m_product3(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqProductVariable"), getId()),
      m_product4(constraintEngine, IntervalDomain(), true, false, std::string("InternalEqProductVariable"), getId()) {
    check_error(ARG_COUNT > 2 && ARG_COUNT == m_variables.size());
    std::vector<ConstrainedVariableId> scope;
    // B is always first and C is always second for the first set, so:
    scope.push_back(m_variables[1]); // B * ...
    scope.push_back(m_variables[2]); // ... C ...
    switch (ARG_COUNT) {
    case 3: // A = B * C
      scope.push_back(m_variables[0]); // ... = A
      m_eqProductC1 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 4: // A = (B * C) * D
      scope.push_back(m_product1.getId()); // ... = (B * C)
      m_eqProductC1 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_product1.getId()); // (B * C) ...
      scope.push_back(m_variables[3]); // ... * D = ...
      scope.push_back(m_variables[0]); // ... A
      m_eqProductC2 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      break;
    case 5: case 6: case 7:
      // 5: A = (B * C) * (D * E)
      // 6: A = (B * C) * (D * E * F)
      // 7: A = (B * C) * (D * E * F * G)
      // So, do (B * C) and (D * E ...) for all three:
      scope.push_back(m_product1.getId()); // (B * C)
      m_eqProductC1 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_product1.getId()); // (B * C) * ...
      scope.push_back(m_product2.getId()); // (D * E ...) = ...
      scope.push_back(m_variables[0]); // A
      m_eqProductC2 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
      scope.clear();
      scope.push_back(m_variables[3]); // D * ...
      scope.push_back(m_variables[4]); // E ...
      switch (ARG_COUNT) {
      case 5:
        scope.push_back(m_product2.getId()); // ... = (D * E)
        m_eqProductC3 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      case 6:
        scope.push_back(m_product3.getId()); // ... = (D * E)
        m_eqProductC3 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product3.getId()); // (D * E) * ...
        scope.push_back(m_variables[5]); // ... F = ...
        scope.push_back(m_product2.getId()); // ... (D * E * F)
        m_eqProductC4 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      case 7:
        scope.push_back(m_product3.getId()); // ... = (D * E)
        m_eqProductC3 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product3.getId()); // (D * E) * ...
        scope.push_back(m_product4.getId()); // ... (F * G) = ...
        scope.push_back(m_product2.getId()); // (D * E * F * G)
        m_eqProductC4 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_variables[5]); // F * ...
        scope.push_back(m_variables[6]); // ... G = ...
        scope.push_back(m_product4.getId()); // ... (F * G)
        m_eqProductC5 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        break;
      default:
        check_error(ALWAYS_FAILS);
        break;
      } // switch (ARGCOUNT) 5, 6, 7
      break;
    default:
      { // A = first_half * second_half, recursively
        check_error(ARG_COUNT > 7);
        scope.clear(); // Was B * C for first set: those that only call MultEqual
        scope.push_back(m_product1.getId()); // first_half * ...
        scope.push_back(m_product2.getId()); // ... second_half = ...
        scope.push_back(m_variables[0]); // ... A
        m_eqProductC1 = (new MultEqualConstraint(std::string("MultEqual"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product1.getId()); // first_half = ...
        unsigned long half = ARG_COUNT/2;
        unsigned long i = 1;
        for ( ; i <= half; i++)
          scope.push_back(m_variables[i]); // ... X * ...
        m_eqProductC2 = (new EqualProductConstraint(std::string("EqualProduct"), propagatorName, constraintEngine, scope))->getId();
        scope.clear();
        scope.push_back(m_product2.getId()); // second_half = ...
        for ( ; i < ARG_COUNT; i++)
          scope.push_back(m_variables[i]); // ... Y * ...
        m_eqProductC3 = (new EqualProductConstraint(std::string("EqualProduct"), propagatorName, constraintEngine, scope))->getId();
        break;
      }
    }
  }

  EqualProductConstraint::~EqualProductConstraint(){
    discard(false);
  }

  void EqualProductConstraint::handleDiscard(){
    // Process for this constraint first
    Constraint::handleDiscard();

    // Further unwind for contained variables
    m_product1.discard();
    m_product2.discard();
    m_product3.discard();
    m_product4.discard();
  }

  /*********** CondAllSameConstraint *************/
  CondAllSameConstraint::CondAllSameConstraint(const std::string& name,
                                               const std::string& propagatorName,
                                               const ConstraintEngineId constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()) {
    check_error(ARG_COUNT > 2);
    check_error(getCurrentDomain(m_variables[0]).isBool());
    for (unsigned int i = 2; i < ARG_COUNT; i++) {
      check_error(Domain::canBeCompared(getCurrentDomain(m_variables[1]),
                                                getCurrentDomain(m_variables[i])));
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
      Domain* common = 0;
      edouble single = 0.0;
      for (unsigned int i = 1; !boolDom.isSingleton() && i < ARG_COUNT; i++) {
        Domain& current(getCurrentDomain(m_variables[i]));
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
        } // if !current.isSingleton()
        if (canProveTrue) {
          if (i == 1)
            single = current.getSingletonValue();
          else
            if (std::abs(single - current.getSingletonValue()) >= getCurrentDomain(m_variables[1]).minDelta()) {
              // Two singletons with different values: can't be all same, so:
              canProveTrue = false;
              boolDom.remove(true);
            }
        } // if canProveTrue
      } // for i = 1; !boolDom.isSingleton && i < ARG_COUNT; i++
      if (canProveTrue)
        boolDom.remove(false);
      // Before it goes out of scope:
      if (common != 0)
        delete common;
    } //if !boolDom.isSingleton

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
        Domain& domj = getCurrentDomain(m_variables[j]);
        if (domj.isOpen() || !domj.isSingleton())
          return; // Can ignore relax events until condition var is relaxed.
        edouble single = domj.getSingletonValue();
        unsigned int foundOneToTrim = 0;
        for (unsigned int i = 1; i < ARG_COUNT; i++) {
          if (i == j)
            continue;
          Domain& domi = getCurrentDomain(m_variables[i]);
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
        Domain& domToTrim = getCurrentDomain(m_variables[foundOneToTrim]);
        // But it can only be trimmed if it is enumerated (including boolean) or
        // if it is an integer interval with one of the end points equal to
        // single, which is painful to check.
        if (domToTrim.isEnumerated() || domToTrim.isBool()
            || (domToTrim.minDelta() == 1.0
                && (domj.isMember(domToTrim.getLowerBound())
                    || domj.isMember(domToTrim.getUpperBound()))))
          domToTrim.remove(single);
        else {} // Can ignore relax events until condition var is relaxed.
        return;
      } else {

        // Singleton true: force all other vars in scope to be equated if _any_ of them are closed..
        unsigned int i = 1;
        for ( ; i < ARG_COUNT; i++)
          if (!getCurrentDomain(m_variables[i]).isOpen())
            break;
        if (i == ARG_COUNT) // All of them are open; can't reduce any.
          return; // Can ignore relax events until condition var is relaxed.
        Domain& dom1 = getCurrentDomain(m_variables[1]);
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


  /*********** CondAllDiffConstraint *************/
  CondAllDiffConstraint::CondAllDiffConstraint(const std::string& name,
                                               const std::string& propagatorName,
                                               const ConstraintEngineId constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()) {
    check_error(ARG_COUNT > 2);
    check_error(getCurrentDomain(m_variables[0]).isBool());
    for (unsigned int i = 2; i < ARG_COUNT; i++)
      check_error(Domain::canBeCompared(getCurrentDomain(m_variables[1]),
                                                getCurrentDomain(m_variables[i])));
  }

namespace {
  /**
   * @brief Helper function: add domToAdd to unionOfDomains "usefully".
   * Adds all of the members of domToAdd and, if needed and useful,
   * more values to unionOfDomains.
   * @param unionOfDomains Pointer to new'd Domain, which may
   * be delete'd and new'd with a different concrete class by this
   * function.
   * @param domToAdd Set of values to add to unionOfDomains' concrete
   * (C++) domain object.
   * @note Can add too much without affecting CondAllDiffConstraint
   * other than by delaying propagation, so add a disjoint interval by
   * simply returning a single larger interval that 'covers' both
   * original intervals.
   */
void addToUnion(Domain **unionOfDomains,
                const Domain& domToAdd) {
  check_error(unionOfDomains != 0 && *unionOfDomains != 0);
  check_error(!(*unionOfDomains)->isEmpty() && !(*unionOfDomains)->isOpen());
  check_error(!domToAdd.isEmpty() && !domToAdd.isOpen());
  Domain *newUnion = 0;
  std::list<edouble> membersToAdd;
  std::list<edouble> newMembers;
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
    for (std::list<edouble>::const_iterator it = membersToAdd.begin();
         it != membersToAdd.end(); it++) {
      std::list<edouble>::const_iterator it2 = newMembers.begin();
      for ( ; it2 != newMembers.end(); it2++)
        if (*it == *it2)
          break;
      if (it2 == newMembers.end())
        newMembers.push_back(*it);
    }
    newUnion = new EnumeratedDomain(
        (*unionOfDomains)->getDataType(),
        newMembers);

    // Could just add to current unionOfDomains rather than failing here, but
    //   very messy to implement using current interface to *Domain classes.
    assertFalse(newUnion == NULL);
    delete *unionOfDomains;
    *unionOfDomains = newUnion;
    return;
  }
  // At least one is a non-singleton interval, so the result will be
  //   also be one.
  edouble toAddMin, toAddMax, newMin, newMax;
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
    if((*unionOfDomains)->minDelta() < 1.0)
      newUnion = new IntervalDomain(newMin, newMax);
    else
      newUnion = new IntervalIntDomain(static_cast<eint>(newMin), static_cast<eint>(newMax));

    /* BOOL should be not get to here since both are non-singleton
     *   but then unionOfDomains "covers" domToAdd and changing
     *   would be false.
     * USER_DEFINED and REAL_ENUMERATION should not get to here
     *   since enumerations are dealt with above.
     * As above, a memory failure here could be dealt with, but
     *   messy to implement, but note that this also checks the
     *   assumptions/logic earlier in this comment.
     */
    checkError(newUnion != 0, "Failed to allocate memory.");
    delete *unionOfDomains;
    *unionOfDomains = newUnion;
    return;
  }
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
    Domain* unionOfOthers = 0;
    for ( ; i < ARG_COUNT; i++) {
      Domain& current(getCurrentDomain(m_variables[i]));
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
        if (unionOfOthers->isFinite() && (unionOfOthers->getSize()) < i) {
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
        Domain& iDom = getCurrentDomain(m_variables[i]);
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
        Domain& singletonDom = getCurrentDomain(m_variables[i]);
        // ... go thru the other vars ...
        for (unsigned int j = firstNonDynamic; j < ARG_COUNT; j++) {
          if (j == i)
            continue;
          Domain& jDom = getCurrentDomain(m_variables[j]);
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

AllDiffConstraint::AllDiffConstraint(const std::string& name,
                                     const std::string& propagatorName,
                                     const ConstraintEngineId constraintEngine,
                                     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_condVar(constraintEngine, BoolDomain(true), true, false, std::string("Internal:AllDiff:cond"), getId()),
      m_condAllDiffConstraint() {
    std::vector<ConstrainedVariableId> condAllDiffScope;
    condAllDiffScope.reserve(m_variables.size() + 1);
    condAllDiffScope.push_back(m_condVar.getId());
    condAllDiffScope.insert(condAllDiffScope.end(), m_variables.begin(), m_variables.end());
    check_error(m_variables.size() + 1 == condAllDiffScope.size());
    m_condAllDiffConstraint = (new CondAllDiffConstraint(std::string("CondAllDiff"), propagatorName,
                                                         constraintEngine, condAllDiffScope))->getId();
  }

  MemberImplyConstraint::MemberImplyConstraint(const std::string& name,
                                               const std::string& propagatorName,
                                               const ConstraintEngineId constraintEngine,
                                               const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      ARG_COUNT(variables.size()) {
    check_error(ARG_COUNT == 4);
    check_error(Domain::canBeCompared(getCurrentDomain(m_variables[0]),
                                              getCurrentDomain(m_variables[1])));
    check_error(Domain::canBeCompared(getCurrentDomain(m_variables[2]),
                                              getCurrentDomain(m_variables[3])));
  }
  /**
   * @brief if B in C then restrict A to D:
   */
  void MemberImplyConstraint::handleExecute() {
    Domain& domA(getCurrentDomain(m_variables[0]));
    Domain& domB(getCurrentDomain(m_variables[1]));
    Domain& domC(getCurrentDomain(m_variables[2]));
    Domain& domD(getCurrentDomain(m_variables[3]));

    assertFalse(domA.isEmpty() || domB.isEmpty() || domC.isEmpty() || domD.isEmpty());

    if (domA.isOpen() || domB.isOpen() || domD.isOpen())
      return;
    if (domB.isSubsetOf(domC))
      (void) domA.intersect(domD);
  }

  CountZeroesConstraint::CountZeroesConstraint(const std::string& name,
                                             const std::string& propagatorName,
                                             const ConstraintEngineId constraintEngine,
                                             const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    for (unsigned int i = 0; i < m_variables.size(); i++)
      check_error(getCurrentDomain(m_variables[i]).isNumeric() || getCurrentDomain(m_variables[i]).isBool());
  }

  void CountZeroesConstraint::handleExecute() {
    unsigned int i = 1;

    // Count the other vars that must be zero ...
    unsigned int minZeroes = 0;
    // ... and that could be zero.
    unsigned int maxZeroes = 0;
    for ( ; i < m_variables.size(); i++) {
      Domain& other = getCurrentDomain(m_variables[i]);
      if (other.isMember(0.0)) {
        ++maxZeroes;
        if (other.isSingleton())
          ++minZeroes;
      }
    }

    // The count of zeroes is the first variable.
    Domain& countDom = getCurrentDomain(m_variables[0]);

    // If all that could be zero must be zero to get the count high
    // enough, set all that could be zero to zero.
    if (minZeroes < countDom.getLowerBound() &&
        maxZeroes == countDom.getLowerBound()) {
      // Find those that could be zero but might not be
      // and restrict them to 0.
      for (i = 1; i < m_variables.size(); i++) {
        Domain& other = getCurrentDomain(m_variables[i]);
        if (other.isMember(0.0) && !other.isSingleton()) {
          Domain* zero = other.copy();
          zero->set(0.0);
          other.intersect(*zero);
          delete zero;
        }
      }
    }

    // If all that might be zero are needed to be non-zero to get the
    // count low enough, restrict all that might be zero to not be
    // zero.
    if (maxZeroes > countDom.getUpperBound() &&
        minZeroes == countDom.getUpperBound()) {
      // Find those that could be zero but might not be and restrict
      // them to not be 0.
      for (i = 1; i < m_variables.size(); i++) {
        Domain& other = getCurrentDomain(m_variables[i]);
        if (other.isMember(0.0) && !other.isSingleton()) {
          // Can only remove 0.0 from integer interval domains if it
          // is an endpoint and can only remove 0.0 from real interval
          // domains if it is singleton ... which it can't be to get
          // here.
          if (other.isEnumerated())
            other.remove(0.0);
          else {
            const IntervalDomain zeroDom(0.0);
              if (zeroDom.isMember(other.getLowerBound()) ||
                  zeroDom.isMember(other.getUpperBound()))
                other.remove(0.0);
            }
        }
      }
    }

    // If the counts seen restrict the count variable, do so.
    if (minZeroes > countDom.getLowerBound() ||
        countDom.getUpperBound() > maxZeroes)
      countDom.intersect(make_int_int(minZeroes, maxZeroes));
  }

CountNonZeroesConstraint::CountNonZeroesConstraint(const std::string& name,
                                                 const std::string& propagatorName,
                                                 const ConstraintEngineId constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_zeroes(constraintEngine, IntervalDomain(), true, false, std::string("InternalCountNonZeroesVar"), getId()),
      m_otherVars(constraintEngine, IntervalDomain(), true, false, std::string("InternalCountNonZeroesOtherVars"), getId()),
    m_superset(constraintEngine, IntervalDomain(edouble(variables.size() - 1)), true, false, std::string("InternalCountNonZeroesSuperset"), getId()),
    m_addEqualConstraint(std::string("AddEqual"), propagatorName, constraintEngine,
                         makeScope(m_zeroes.getId(), m_variables[0], m_otherVars.getId())),
    m_subsetConstraint(), m_countZeroesConstraint() {
  m_subsetConstraint = (new SubsetOfConstraint(std::string("SubsetOf"), propagatorName, constraintEngine,
                                               makeScope(m_otherVars.getId(), m_superset.getId())))->getId();
  std::vector<ConstrainedVariableId> cZCScope = m_variables;
  cZCScope[0] = m_zeroes.getId();
  check_error(m_variables.size() == cZCScope.size());
  m_countZeroesConstraint = (new CountZeroesConstraint(std::string("CountZeroes"),
                                                     propagatorName, constraintEngine, cZCScope))->getId();
}


  OrConstraint::OrConstraint(const std::string& name,
                             const std::string& propagatorName,
                             const ConstraintEngineId constraintEngine,
                             const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_nonZeroes(constraintEngine, IntervalIntDomain(1, PLUS_INFINITY), true, false, std::string("InternalVar:Or:nonZeroes"), getId()),
      m_superset(constraintEngine, IntervalIntDomain(1, eint(variables.size())), true, false, std::string("InternalVar:Or:superset"), getId()),
                     m_subsetConstraint(), m_countNonZeroesConstraint()
  {
    m_subsetConstraint = (new SubsetOfConstraint(std::string("SubsetOf"), propagatorName, constraintEngine,
                                                 makeScope(m_nonZeroes.getId(), m_superset.getId())))->getId();
    std::vector<ConstrainedVariableId> cNZCScope;
    cNZCScope.reserve(m_variables.size() + 1);
    cNZCScope.push_back(m_nonZeroes.getId());
    cNZCScope.insert(cNZCScope.end(), m_variables.begin(), m_variables.end());
    check_error(m_variables.size() + 1 == cNZCScope.size());
    m_countNonZeroesConstraint = (new CountNonZeroesConstraint(std::string("CountNonZeroes"), propagatorName,
                                                             constraintEngine, cNZCScope))->getId();
  }

  EqualMinimumConstraint::EqualMinimumConstraint(const std::string& name,
                                                 const std::string& propagatorName,
                                                 const ConstraintEngineId constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(m_variables.size() > 1);
    for (unsigned int i = 0; i < m_variables.size(); i++)
      check_error(getCurrentDomain(m_variables[i]).isNumeric());
    // Should probably call Domain::canBeCompared() and check
    // minDelta() as well.
  }

  // If EqualMinConstraint::handleExecute's contributors were a class
  // data member, then EqualMinConstraint::canIgnore() could be quite
  // specific about events to ignore.  If the event(s) were also
  // available to handleExecute(), it could focus on just the changed
  // vars.

  void EqualMinimumConstraint::handleExecute() {
    Domain& minDom = getCurrentDomain(m_variables[0]);
    Domain& firstDom = getCurrentDomain(m_variables[1]);
    edouble minSoFar = firstDom.getLowerBound();
    edouble maxSoFar = firstDom.getUpperBound();
    std::set<unsigned int> contributors; // Set of var indices that "affect" minimum, or are affected by minDom's minimum.
    contributors.insert(1);
    std::vector<ConstrainedVariableId>::iterator it = m_variables.begin();
    unsigned int i = 2;
    for (it++, it++; it != m_variables.end(); it++, i++) {
      Domain& curDom = getCurrentDomain(*it);
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
    edouble minimum = minDom.getLowerBound();
    if (contributors.size() == 1) {
      // If there is only one other var that has a value in minDom,
      // it needs to be restricted to minDom to satisfy the constraint.
      i = *(contributors.begin());
      check_error(i > 0);
      Domain& curDom = getCurrentDomain(m_variables[i]);
      IntervalDomain restriction(minimum, minDom.getUpperBound());
      curDom.intersect(restriction);
      return; // No other contributors, so cannot be more to do.
    }
    while (!contributors.empty()) {
      i = *(contributors.begin());
      check_error(i > 0);
      contributors.erase(contributors.begin());
      Domain& curDom = getCurrentDomain(m_variables[i]);
      if (minimum < curDom.getLowerBound())
        continue;
      IntervalDomain restriction(minimum, curDom.getUpperBound());
      if (curDom.intersect(restriction) && curDom.isEmpty())
        return;
    }
  }

  EqualMaximumConstraint::EqualMaximumConstraint(const std::string& name,
                                                 const std::string& propagatorName,
                                                 const ConstraintEngineId constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(m_variables.size() > 1);
    for (unsigned int i = 0; i < m_variables.size(); i++)
      check_error(getCurrentDomain(m_variables[i]).isNumeric());
    // Should probably call Domain::canBeCompared() and check
    // minDelta() as well.
  }

  // If EqualMaxConstraint::handleExecute's contributors were a class
  // data member, then EqualMaxConstraint::canIgnore() could be quite
  // specific about events to ignore.  If the event(s) were also
  // available to handleExecute(), it could focus on just the changed
  // vars.

  void EqualMaximumConstraint::handleExecute() {
    Domain& maxDom = getCurrentDomain(m_variables[0]);
    Domain& firstDom = getCurrentDomain(m_variables[1]);
    edouble minSoFar = firstDom.getLowerBound();
    edouble maxSoFar = firstDom.getUpperBound();
    std::set<unsigned int> contributors; // Set of var indices that "affect" maximum, or are affected by maxDom's maximum.
    contributors.insert(1);
    std::vector<ConstrainedVariableId>::iterator it = m_variables.begin();
    unsigned int i = 2;
    for (it++, it++; it != m_variables.end(); it++, i++) {
      Domain& curDom = getCurrentDomain(*it);
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
    edouble maximum = maxDom.getUpperBound();
    if (contributors.size() == 1) {
      // If there is only one other var that has a value in maxDom,
      // it needs to be restricted to maxDom to satisfy the constraint.
      i = *(contributors.begin());
      check_error(i > 0);
      Domain& curDom = getCurrentDomain(m_variables[i]);
      IntervalDomain restriction(maxDom.getLowerBound(), maximum);
      curDom.intersect(restriction);
      return; // No other contributors, so cannot be more to do.
    }
    while (!contributors.empty()) {
      i = *(contributors.begin());
      check_error(i > 0);
      contributors.erase(contributors.begin());
      Domain& curDom = getCurrentDomain(m_variables[i]);
      if (maximum > curDom.getUpperBound())
        continue;
      IntervalDomain restriction(curDom.getLowerBound(), maximum);
      if (curDom.intersect(restriction) && curDom.isEmpty())
        return;
    }
  }


RotateScopeRightConstraint::RotateScopeRightConstraint(const std::string& name,
                                                       const std::string& propagatorName,
                                                       const ConstraintEngineId constraintEngine,
                                                       const std::vector<ConstrainedVariableId>& variables,
                                                       const std::string& otherName,
                                                       const int& rotateCount)
    : Constraint(name, propagatorName, constraintEngine, variables), m_otherConstraint()
  {
    check_error(static_cast<unsigned>(abs(rotateCount)) < m_variables.size());
    std::vector<ConstrainedVariableId> otherScope;
    otherScope.reserve(m_variables.size());

    unsigned long i;
    if (rotateCount > 0) {
      unsigned long realCount = static_cast<unsigned long>(rotateCount);
      // Rotate to right: last var becomes first, pushing others to the right.
      for (i = realCount; i > 0; i--)
        otherScope.push_back(m_variables[m_variables.size() - i]);
      for (i = 0; i < m_variables.size() - realCount; i++)
        otherScope.push_back(m_variables[i]);
    }
    else {
      // Rotate to left: first var becomes last, pushing others to the left.
      for (i = static_cast<unsigned>(abs(rotateCount)); i < m_variables.size(); i++)
        otherScope.push_back(m_variables[i]);
      for (i = 0; i < static_cast<unsigned>(abs(rotateCount)); i++)
        otherScope.push_back(m_variables[i]);
    }
    check_error(m_variables.size() == otherScope.size());
    m_otherConstraint = constraintEngine->createConstraint(otherName, otherScope);
  }

SwapTwoVarsConstraint::SwapTwoVarsConstraint(const std::string& name,
                                             const std::string& propagatorName,
                                             const ConstraintEngineId constraintEngine,
                                             const std::vector<ConstrainedVariableId>& variables,
                                             const std::string& otherName,
                                             int firstIndex, int secondIndex)
    : Constraint(name, propagatorName, constraintEngine, variables), m_otherConstraint()
  {
    check_error(static_cast<unsigned>(abs(firstIndex)) < m_variables.size());
    check_error(static_cast<unsigned>(abs(secondIndex)) < m_variables.size());
    check_error(firstIndex != secondIndex);
    unsigned long realFirstIndex = (firstIndex < 0 ?
                                    (m_variables.size() - static_cast<unsigned long>(std::abs(firstIndex))) :
                                    static_cast<unsigned long>(firstIndex));
    unsigned long realSecondIndex = (secondIndex < 0 ?
                                     (m_variables.size() - static_cast<unsigned long>(std::abs(secondIndex))) :
                                     static_cast<unsigned long>(secondIndex));
    check_error(realFirstIndex != realSecondIndex);
    std::vector<ConstrainedVariableId> otherScope(m_variables);
    otherScope[realFirstIndex] = m_variables[realSecondIndex];
    otherScope[realSecondIndex] = m_variables[realFirstIndex];
    m_otherConstraint = constraintEngine->createConstraint(otherName, otherScope);
  }


  LockConstraint::LockConstraint(const std::string& name,
				 const std::string& propagatorName,
				 const ConstraintEngineId constraintEngine,
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
    else {
       debugMsg("LockConstraint","current domain:" << m_currentDomain.toString()
    		                     << " is not a superset of lock domain :" << m_lockDomain.toString()
    		                     << " emptying current domain"
       );

       m_currentDomain.empty(); // Otherwise, the lock is enforced by forcing a relaxation
    }
  }

  // Enforces X >=0, Y<=0, X+Y==0
  NegateConstraint::NegateConstraint(const std::string& name,
				   const std::string& propagatorName,
				   const ConstraintEngineId constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == 2);
    check_error(!variables[0]->baseDomain().isEnumerated());
    check_error(!variables[1]->baseDomain().isEnumerated());
  }

  void NegateConstraint::handleExecute() {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));

    check_error(Domain::canBeCompared(domx, domy));
    check_error(!domx.isEmpty() && !domy.isEmpty());

    edouble xMin, xMax, yMin, yMax;
    domx.getBounds(xMin, xMax);
    domy.getBounds(yMin, yMax);

    // Prune immediately to enforce X >= 0 && Y <= 0.
    //xMin = (xMin >= 0 ? xMin : 0);
    //xMax = (xMax >= xMin ? xMax : xMin);
    //yMax = (yMax <= 0 ? yMax : 0);
    //yMin = (yMin <= yMax ? yMin : 0);

    if (domx.intersect(-yMax, -yMin) && domx.isEmpty())
      return;

    domy.intersect(-xMax, -xMin);
  }


  TestEQ::TestEQ(const std::string& name,
                           const std::string& propagatorName,
                           const ConstraintEngineId constraintEngine,
                           const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])),
      m_arg2(getCurrentDomain(variables[2])){
    check_error(variables.size() == ARG_COUNT);
  }

  void TestEQ::handleExecute(){

    debugMsg("TestEQ:handleExecute", "comparing " << m_arg1.toString() << " with " << m_arg2.toString());

    if(m_arg1.isSingleton() &&
       m_arg2.isSingleton() &&
       m_arg1.intersects(m_arg2)) // Exactly equal with no flexibility
       m_test.remove(0);
    else if(!m_arg1.intersects(m_arg2)) // No intersection so they cannot be equal
      m_test.remove(1);

    if(m_test.isSingleton()){
      if(m_test.getSingletonValue() == true){
        if(m_arg1.intersect(m_arg2) && m_arg1.isEmpty())
          return;

        m_arg2.intersect(m_arg1);
      }
    }
  }

  TestSingleton::TestSingleton(const std::string& name,
			   const std::string& propagatorName,
			   const ConstraintEngineId constraintEngine,
			   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])),
      m_modifiedVariables(makeScope(variables[0])) {
    check_error(variables.size() == ARG_COUNT);
  }

  void TestSingleton::handleExecute(){

    debugMsg("TestSingleton:handleExecute", "comparing " << m_arg1.toString() << " setting " << m_test.toString());

    if(m_arg1.isSingleton()) // it is a singleton
       m_test.remove(0); // set the test to be true
    //TODO: what about:
    //m_test is a singleton, m_arg1 is not.
    //what is test if m_arg1 is not a singleton.
  }

  const std::vector<ConstrainedVariableId>& TestSingleton::getModifiedVariables() const {
    return m_modifiedVariables;
  }

  TestSpecified::TestSpecified(const std::string& name,
			   const std::string& propagatorName,
			   const ConstraintEngineId constraintEngine,
			   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])){
    check_error(variables.size() == ARG_COUNT);
  }

  void TestSpecified::handleExecute(){

    debugMsg("TestSingleton:handleExecute", "comparing " << m_arg1.toString() << " setting " << m_test.toString());

    if(m_arg1.isSingleton() && getScope()[1]->isSpecified())// it is a singleton
       m_test.remove(0); // set the test to be true
    //TODO: what about:
    //m_test is a singleton, m_arg1 is not.
    //what is test if m_arg1 is not a singleton.
  }

  TestNEQ::TestNEQ(const std::string& name,
			   const std::string& propagatorName,
			   const ConstraintEngineId constraintEngine,
			   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])),
      m_arg2(getCurrentDomain(variables[2])){
    check_error(variables.size() == ARG_COUNT);
  }

  void TestNEQ::handleExecute(){

    debugMsg("TestNEQ:handleExecute", "comparing " << m_arg1.toString() << " with " << m_arg2.toString());

    if(m_arg1.isSingleton() &&
       m_arg2.isSingleton()) {
      if (m_arg1.intersects(m_arg2)) { // They are singletons and are equal, return false.
	m_test.remove(1);
      } else {
	m_test.remove(0);
      }
    }

    if(m_test.isSingleton()) {
      if(m_test.getSingletonValue() == true) { //Enforce inequality
	if(m_arg1.isSingleton()) {
	  m_arg2.remove(m_arg1.getSingletonValue());
	}
	if(m_arg2.isSingleton()) {
	  m_arg1.remove(m_arg2.getSingletonValue());
	}
      } else { //Enforce equality
	m_arg2.intersect(m_arg1);
	m_arg1.intersect(m_arg2);
      }
    }
  }


  TestOr::TestOr(const std::string& name,
		 const std::string& propagatorName,
		 const ConstraintEngineId constraintEngine,
		 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])),
      m_arg2(getCurrentDomain(variables[2])){
    check_error(variables.size() == ARG_COUNT);
  }

  void TestOr::handleExecute(){

    debugMsg("TestOr:handleExecute", "comparing " << m_arg1.toString() << " with " << m_arg2.toString());

    if(m_arg1.isSingleton() && m_arg2.isSingleton()) { //A and b are singltons, so set the value.
      if (m_arg1.getSingletonValue() == 0 && m_arg2.getSingletonValue() == 0) {
        m_test.remove(1);
      } else {
        m_test.remove(0);
      }
    }

    if(m_test.isSingleton()){ //Test value specified, so set up the others
      if (m_test.getSingletonValue() != 0) { //It's true, so a or b == true

	if (m_arg1.isSingleton()) {
	  if (m_arg1.getSingletonValue() == 0) { //a == false, so b must be true
	    m_arg2.remove(0);
	  }
	} else if(m_arg2.isSingleton()) {
	  if (m_arg2.getSingletonValue() == 0) { //b == false, so a must be true
	    m_arg1.remove(0);
	  }
	} //If none are singletons, nothing to be done: FIXME!

      } else { //It's false, so both a and b are false.
	m_arg1.remove(1);
	m_arg2.remove(1);
      }
    }
  }

  TestAnd::TestAnd(const std::string& name,
		   const std::string& propagatorName,
		   const ConstraintEngineId constraintEngine,
		   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])),
      m_arg2(getCurrentDomain(variables[2])){
    check_error(variables.size() == ARG_COUNT);
  }

  void TestAnd::handleExecute(){

    debugMsg("TestAnd:handleExecute", "comparing " << m_arg1.toString() << " with " << m_arg2.toString() << ", test " << m_test);

    if(m_arg1.isSingleton() && m_arg2.isSingleton()) { //A and b are singltons, so set the value.
      if (m_arg1.getSingletonValue() == 0 || m_arg2.getSingletonValue() == 0) { //a == false or b == false, so set to false
        m_test.intersect(make_int(0, 0));
      } else { //a == b == true, set to true
        m_test.intersect(make_int(1, 1));
      }
    }

    if(m_test.isSingleton()){ //Test value specified, so set up the others
      if (m_test.getSingletonValue() != 0) { //It's true, so a and b == true
        m_arg1.intersect(make_int(1, 1));
        m_arg2.intersect(make_int(1, 1));
      } else { //It's false, so one of a or b must be false
	if (m_arg1.isSingleton()) {
	  if (m_arg1.getSingletonValue() != 0) { //a == true, so b must be false
	    m_arg2.intersect(make_int(0, 0));
	  }
	}
	if(m_arg2.isSingleton()) {
	  if (m_arg2.getSingletonValue() != 0) { //b == true, so a must be false
	    m_arg1.intersect(make_int(0, 0));
	  }
	} //If none are singletons, nothing to be done: FIXME!
      }
    }

  }

  TestLessThan::TestLessThan(const std::string& name,
			     const std::string& propagatorName,
			     const ConstraintEngineId constraintEngine,
			     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])),
      m_arg2(getCurrentDomain(variables[2])){
    check_error(variables.size() == ARG_COUNT);
    checkError(m_arg1.isNumeric(), variables[1]);
    checkError(m_arg2.isNumeric(), variables[2]);
  }

  void TestLessThan::handleExecute(){
    if(m_arg1.getUpperBound() < m_arg2.getLowerBound())
      m_test.remove(0);
    else if(m_arg1.getLowerBound() >= m_arg2.getUpperBound())
      m_test.remove(1);

    // If true, apply the constraint (arg1 < arg2)
    if(m_test.isSingleton() && m_test.getSingletonValue() == 1){
      IntervalDomain& domx = static_cast<IntervalDomain&>(m_arg1);
      IntervalDomain& domy = static_cast<IntervalDomain&>(m_arg2);
      LessThanConstraint::propagate(domx, domy);
    }

    // If false, apply the converse (arg1 >= arg2);
    if(m_test.isSingleton() && m_test.getSingletonValue() == 0){
      IntervalDomain& domx = static_cast<IntervalDomain&>(m_arg1);
      IntervalDomain& domy = static_cast<IntervalDomain&>(m_arg2);
      LessThanEqualConstraint::propagate(domy, domx);
    }
  }

  TestLEQ::TestLEQ(const std::string& name,
			     const std::string& propagatorName,
			     const ConstraintEngineId constraintEngine,
			     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_test(getCurrentDomain(variables[0])),
      m_arg1(getCurrentDomain(variables[1])),
      m_arg2(getCurrentDomain(variables[2])){
    check_error(variables.size() == ARG_COUNT);
  }

  void TestLEQ::handleExecute(){
    debugMsg("TestLEQ:handleExecute", "BEFORE:" << toString());

    if(m_arg1.getUpperBound() <= m_arg2.getLowerBound())
	m_test.remove(0);
    else if(m_arg1.getLowerBound() > m_arg2.getUpperBound())
	m_test.remove(1);

    // If true, apply the constraint (arg1 <= arg2)
    if(m_test.isSingleton() && m_test.getSingletonValue() == 1){
      IntervalDomain& domx = static_cast<IntervalDomain&>(m_arg1);
      IntervalDomain& domy = static_cast<IntervalDomain&>(m_arg2);
      LessThanEqualConstraint::propagate(domx, domy);
    }

    // If false, apply the converse (arg1 > arg2);
    if(m_test.isSingleton() && m_test.getSingletonValue() == 0){
      IntervalDomain& domx = static_cast<IntervalDomain&>(m_arg1);
      IntervalDomain& domy = static_cast<IntervalDomain&>(m_arg2);
      LessThanConstraint::propagate(domy, domx);
    }

    debugMsg("TestLEQ:handleExecute", "AFTER:" << toString());
  }


  /**
   * WithinBounds Implementation
   */
  WithinBounds::WithinBounds(const std::string& name,
			     const std::string& propagatorName,
			     const ConstraintEngineId constraintEngine,
			     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_x(static_cast<IntervalDomain&>(getCurrentDomain(variables[0]))),
      m_y(static_cast<IntervalDomain&>(getCurrentDomain(variables[1]))),
      m_z(static_cast<IntervalDomain&>(getCurrentDomain(variables[2]))),
      m_leq(name, propagatorName, constraintEngine, makeScope(variables[1], variables[2])){
    check_error(variables.size() == ARG_COUNT);
  }

  void WithinBounds::handleExecute(){
    // Process x - let the composed constraint look after y <= z
    m_x.intersect(m_y.getLowerBound(), m_z.getUpperBound());
  }

  // void AbsoluteValueCT::checkArgTypes(const std::vector<DataTypeId>& argTypes) const
  // {
  //     if (argTypes.size() != 2) {
  //         std::ostringstream msg; msg << "Constraint AbsoluteValue takes 2 args, not " << argTypes.size();
  //         throw msg.str();
  //     }

  //     for (unsigned int i=0; i< argTypes.size(); i++) {
  //         if (!argTypes[i]->isNumeric()) {
  //             std::ostringstream msg;
  //             msg << "Parameter " << i << " for Constraint AbsoluteValue is not numeric : "
  //                 << argTypes[i]->getName().toString();
  //             throw msg.str();
  //         }
  //     }

  //     if (!argTypes[0]->isAssignableFrom(argTypes[1])) {
  //         std::ostringstream msg;
  //         msg << argTypes[0]->getName().toString() << " can't hold AbsoluteValue for : "
  //             << argTypes[1]->getName().toString();
  //         throw msg.str();
  //     }
  // }

  AbsoluteValue::AbsoluteValue(const std::string& name,
			       const std::string& propagatorName,
			       const ConstraintEngineId constraintEngine,
			       const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_x(static_cast<IntervalDomain&>(getCurrentDomain(variables[0]))),
      m_y(static_cast<IntervalDomain&>(getCurrentDomain(variables[1]))) {
    check_error(variables.size() == ARG_COUNT);
  }

  void AbsoluteValue::handleExecute() {
    edouble lb, ub;

    if(m_y.getLowerBound() >= 0) {
      lb = m_y.getLowerBound();
      ub = m_y.getUpperBound();
    }
    else {
      if(m_y.getUpperBound() >= 0)
	lb = 0.0;
      else
	lb = std::min(std::abs(m_y.getLowerBound()), std::abs(m_y.getUpperBound()));
      ub = std::max(std::abs(m_y.getLowerBound()), m_y.getUpperBound());
    }

    m_x.intersect(IntervalDomain(lb, ub));
    lb = m_x.getLowerBound();
    ub = m_x.getUpperBound();

    //for any absolute value domain [lb ub], there are two domains that could have
    //produced it: [lb ub] and [-ub lb].  If there is a non-empty intersection between y
    //and both domains, then we cannot determine a correct further restriction for y
    //except when a == 0 and [-ub ub] is a subset of y
    if(lb == 0 && m_y.isMember(-ub) && m_y.isMember(ub))
      m_y.intersect(IntervalDomain(-ub, ub));

    if((m_y.isMember(lb) || m_y.isMember(ub)) && (m_y.isMember(-lb) || m_y.isMember(-ub)))
      return;

    if(m_y.isMember(lb) || m_y.isMember(ub))
      m_y.intersect(IntervalDomain(lb, ub));
    else if(m_y.isMember(-lb) || m_y.isMember(-ub))
      m_y.intersect(IntervalDomain(-ub, -lb));
  }

  SquareOfDifferenceConstraint::SquareOfDifferenceConstraint(const std::string& name,
							     const std::string& propagatorName,
							     const ConstraintEngineId constraintEngine,
							     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == ARG_COUNT);
  }

  /**
     Propagate only forward.
     Moreover, the way it is used, either the points are completely
     specified (singletons), or they are INF
  */
  void SquareOfDifferenceConstraint::handleExecute() {
    Domain& domx = getCurrentDomain(m_variables[V1]);
    Domain& domy = getCurrentDomain(m_variables[V2]);
    Domain& doma = getCurrentDomain(m_variables[RES]);

    // domains should be closed
    if ( domx.isOpen() || domy.isOpen() ||
	 !domx.isSingleton() || !domy.isSingleton() )
      return;

    // get the boundaries
    edouble x, y;
    x = domx.getSingletonValue();
    y = domy.getSingletonValue();
    edouble square = (x-y)*(x-y);

    doma.intersect( IntervalDomain(square) );

  }

  DistanceFromSquaresConstraint::DistanceFromSquaresConstraint(const std::string& name,
					 const std::string& propagatorName,
					 const ConstraintEngineId constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error(variables.size() == ARG_COUNT);
  }

  /**
     Propagate only forward.
     Moreover, the way it is used, either the squares are completely
     specified (singletons), or they are INF
   */
  void DistanceFromSquaresConstraint::handleExecute() {
    Domain& domx = getCurrentDomain(m_variables[V1]);
    Domain& domy = getCurrentDomain(m_variables[V2]);
    Domain& doma = getCurrentDomain(m_variables[RES]);

    // domains should be closed
    if ( domx.isOpen() || domy.isOpen() ||
	 !domx.isSingleton() || !domy.isSingleton() )
      return;

    // get the boundaries
    edouble x, y;
    x = domx.getSingletonValue();
    y = domy.getSingletonValue();
    edouble distance = std::sqrt(x+y);

    doma.intersect( IntervalDomain(distance) );

    // cout << "DistanceFromSquares "<<x<<" "<<y<<" "<<distance<<endl;
  }

  CalcDistanceConstraint::CalcDistanceConstraint(const std::string& name,
						 const std::string& propagatorName,
						 const ConstraintEngineId constraintEngine,
						 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_distance(getCurrentDomain(variables[DISTANCE])),
      m_x1(getCurrentDomain(variables[X1])),
      m_y1(getCurrentDomain(variables[Y1])),
      m_x2(getCurrentDomain(variables[X2])),
      m_y2(getCurrentDomain(variables[Y2])){
    checkError(variables.size() == ARG_COUNT, variables.size() << " is the wrong number of arguments for " << name);
  }

  void CalcDistanceConstraint::handleExecute(){
    if(!m_x1.areBoundsFinite() ||
       !m_y1.areBoundsFinite()  ||
       !m_x2.areBoundsFinite() ||
       !m_y2.areBoundsFinite())
      return;

    debugMsg("CalcDistanceConstraint:handleExecute", "BEFORE:" << toString());

    // Compute bounds for dx
    NumericDomain dx;
    dx.insert(std::abs(m_x1.getLowerBound() - m_x2.getLowerBound()));
    dx.insert(std::abs(m_x1.getLowerBound() - m_x2.getUpperBound()));
    dx.insert(std::abs(m_x1.getUpperBound() - m_x2.getLowerBound()));
    dx.insert(std::abs(m_x1.getUpperBound() - m_x2.getUpperBound()));
    if(m_x1.intersects(m_x2))
      dx.insert(0.0);
    dx.close();

    // Compute bounds for dy
    NumericDomain dy;
    dy.insert(std::abs(m_y1.getLowerBound() - m_y2.getLowerBound()));
    dy.insert(std::abs(m_y1.getLowerBound() - m_y2.getUpperBound()));
    dy.insert(std::abs(m_y1.getUpperBound() - m_y2.getLowerBound()));
    dy.insert(std::abs(m_y1.getUpperBound() - m_y2.getUpperBound()));
    if(m_y1.intersects(m_y2))
      dy.insert(0.0);
    dy.close();

    m_distance.intersect(compute(dx.getLowerBound(), dy.getLowerBound()), compute(dx.getUpperBound(), dy.getUpperBound()));

    debugMsg("CalcDistanceConstraint:handleExecute", "AFTER:" << toString());
  }

  edouble CalcDistanceConstraint::compute(edouble x1, edouble y1, edouble x2, edouble y2){
    edouble result = std::sqrt(std::pow(x2-x1, 2)+ std::pow(y2-y1, 2));
    return result;
  }

  edouble CalcDistanceConstraint::compute(edouble a, edouble b){
    edouble result = std::sqrt(std::pow(a, 2)+ std::pow(b, 2));
    return result;
  }

  /**************************************************************************************/

  SineFunction::SineFunction(const std::string& name,
			     const std::string& propagatorName,
			     const ConstraintEngineId constraintEngine,
			     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_target(getCurrentDomain(variables[0])),
      m_source(getCurrentDomain(variables[1])){
    checkError(variables.size() == ARG_COUNT, variables.size() << " is the wrong number of arguments for " << name);
  }

  void SineFunction::handleExecute(){
    // Requires the angle to be defined on a right angled triangle
    m_source.intersect(-90, 90);

    static const edouble PIE = 3.141592;

    if(!m_source.isEmpty()){
      NumericDomain dom;
      dom.insert(std::sin(m_source.getLowerBound() * PIE / 180));
      dom.insert(std::sin(m_source.getUpperBound() * PIE / 180));
      m_target.intersect(dom.getLowerBound(), dom.getUpperBound());
    }
  }


  /**************************************************************************************/

  RandConstraint::RandConstraint(const std::string& name,
			     const std::string& propagatorName,
			     const ConstraintEngineId constraintEngine,
			     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables), m_rvalue(rand() % 32768) {}

  void RandConstraint::handleExecute() {
    getCurrentDomain(m_variables[0]).intersect(m_rvalue, m_rvalue);
  }

namespace {
//eint mod(eint a, eint b) { return a % b; }
eint mod(edouble a, edouble b) {return static_cast<eint::basis_type>(std::fmod(cast_basis(a), cast_basis(b)));}
}
  CREATE_FUNCTION_CONSTRAINT_TWO_ARG(Max, std::max, edouble);
  CREATE_FUNCTION_CONSTRAINT_TWO_ARG(Min, std::min, edouble);
  CREATE_FUNCTION_CONSTRAINT_ONE_ARG(Abs, std::abs, edouble);
  CREATE_FUNCTION_CONSTRAINT_TWO_ARG(Pow, std::pow, edouble);
  CREATE_FUNCTION_CONSTRAINT_ONE_ARG(Sqrt, std::sqrt, edouble);
  CREATE_FUNCTION_CONSTRAINT_TWO_ARG(Mod, mod, eint);



  CREATE_FUNCTION_CONSTRAINT_ONE_ARG(Floor, std::floor, edouble);
  CREATE_FUNCTION_CONSTRAINT_ONE_ARG(Ceil, std::ceil, edouble);

EqUnionConstraint::EqUnionConstraint(const std::string& name,
                                     const std::string& propagatorName,
                                     const ConstraintEngineId constraintEngine,
                                     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {}

void EqUnionConstraint::handleExecute() {
  if(getCurrentDomain(m_variables[0]).isEnumerated()) {
    handleExecuteEnumerated(dynamic_cast<EnumeratedDomain&>(getCurrentDomain(m_variables[0])),
                            m_variables.begin() + 1, m_variables.end());
  }
  else {
    handleExecuteInterval(dynamic_cast<IntervalDomain&>(getCurrentDomain(m_variables[0])),
                          m_variables.begin() + 1, m_variables.end());
  }
}

void EqUnionConstraint::handleExecuteInterval(IntervalDomain& dest,
                                              std::vector<ConstrainedVariableId>::const_iterator start,
                                              const std::vector<ConstrainedVariableId>::const_iterator end) {
  edouble minValue = std::numeric_limits<edouble>::max(),
      maxValue = std::numeric_limits<edouble>::min();
  
  for(std::vector<ConstrainedVariableId>::const_iterator it = start; it != end; ++it) {
    minValue = std::min(minValue, getCurrentDomain(*it).getLowerBound());
    maxValue = std::max(maxValue, getCurrentDomain(*it).getUpperBound());
  }
  minValue = std::max(minValue, dest.getLowerBound());
  maxValue = std::min(maxValue, dest.getUpperBound());

  dest.intersect(minValue, maxValue);
  for(std::vector<ConstrainedVariableId>::const_iterator it = start; it != end; ++it) {
    if(dest.isEmpty())
      getCurrentDomain(*it).empty();
    else
      getCurrentDomain(*it).intersect(dest);
  }
}

namespace {
struct InInterval {
  InInterval(const edouble v) : m_v(v) {}
  bool operator()(const std::pair<edouble, edouble>& i) {
    return i.first <= m_v && m_v <= i.second;
  }
 private:
  edouble m_v;
};
}
void EqUnionConstraint::handleExecuteEnumerated(EnumeratedDomain& dest,
                                                std::vector<ConstrainedVariableId>::const_iterator start,
                                                const std::vector<ConstrainedVariableId>::const_iterator end) {
  std::set<edouble> enumUnion;
  std::vector<std::pair<edouble, edouble> > intervalUnion;

  //collect enumerated and interval unions separately
  for(std::vector<ConstrainedVariableId>::const_iterator it = start; it != end; ++it) {
    Domain& temp = getCurrentDomain(*it);
    if(temp.isEnumerated()) {
      EnumeratedDomain& etemp = dynamic_cast<EnumeratedDomain&>(getCurrentDomain(*it));
      enumUnion.insert(etemp.getValues().begin(), etemp.getValues().end());
    }
    else {
      intervalUnion.push_back(std::make_pair(temp.getLowerBound(), temp.getUpperBound()));
    }
  }
  //NOTE: if it turns out that a lot of time is being spent in checking values against
  //the intervalUnion, it may be worth it to combine contiguous unions
  
  std::list<edouble> existingValues;
  dest.getValues(existingValues);
  for(std::list<edouble>::const_iterator it = existingValues.begin();
      it != existingValues.end(); ++it) {
    if((enumUnion.find(*it) == enumUnion.end()) &&
       (std::find_if(intervalUnion.begin(), intervalUnion.end(), InInterval(*it)) ==
          intervalUnion.end()))
      dest.remove(*it);
  }


  for(std::vector<ConstrainedVariableId>::const_iterator it = start; it != end; ++it) {
    if(dest.isEmpty())
      getCurrentDomain(*it).empty();
    else
      getCurrentDomain(*it).intersect(dest);
  }
}




} // end namespace EUROPA
