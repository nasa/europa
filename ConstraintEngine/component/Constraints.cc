#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "Domain.hh"
#include "Utils.hh"

namespace Prototype
{
  /**
   * Utility class that might get promoted later
   */
  class Infinity{
  public:
    static double plus(double n1, double n2, double defaultValue){
      if(abs((int)n1) == PLUS_INFINITY || abs((int)n2) == PLUS_INFINITY)
	return defaultValue;
      else
	return n1 + n2;
    }

    static double minus(double n1, double n2, double defaultValue){
      if(abs((int)n1) == PLUS_INFINITY || abs((int)n2) == PLUS_INFINITY)
	return defaultValue;
      else
	return n1 - n2;
    }
  };

  AddEqualConstraint::AddEqualConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);
    for(int i=0; i< ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  void AddEqualConstraint::handleExecute()
  {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domz = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

    /* Test preconditions for continued execution */
    if(domx.isDynamic() ||
       domy.isDynamic() ||
       domz.isDynamic())
      return;
 
    check_error(!domx.isEmpty() && !domy.isEmpty() && !domz.isEmpty());

    double xMin;
    double xMax;
    double yMin;
    double yMax;
    double zMin;
    double zMax;

    domx.getBounds(xMin, xMax);
    domy.getBounds(yMin, yMax);
    domz.getBounds(zMin, zMax);


    // Process Z
    double xMax_plus_yMax = Infinity::plus(xMax, yMax, zMax);
    if(zMax > xMax_plus_yMax)
      zMax = domz.translateNumber(xMax_plus_yMax, false);

    double xMin_plus_yMin = Infinity::plus(xMin, yMin, zMin);
    if(zMin < xMin_plus_yMin)
      zMin = domz.translateNumber(xMin_plus_yMin, true);

    if(domz.intersect(zMin,zMax) && domz.isEmpty())
      return;

    // Process X
    double zMax_minus_yMin = Infinity::minus(zMax, yMin, xMax);
    if(xMax > zMax_minus_yMin)
      xMax = domx.translateNumber(zMax_minus_yMin, false);

    double zMin_minus_yMax = Infinity::minus(zMin, yMax, xMin);
    if(xMin < zMin_minus_yMax)
      xMin = domx.translateNumber(zMin_minus_yMax, true);

    if(domx.intersect(xMin,xMax)  && domx.isEmpty())
      return;

    // Process Y
    double zMax_minus_xMin = Infinity::minus(zMax, xMin, yMax);
    if(yMax > zMax_minus_xMin)
      yMax = domy.translateNumber(zMax_minus_xMin, false);

    double zMin_minus_xMax = Infinity::minus(zMin, xMax, yMin);
    if(yMin < zMin_minus_xMax)
      yMin = domy.translateNumber(zMin_minus_xMax, true);

    if(domy.intersect(yMin,yMax) && domy.isEmpty())
      return;

    // Now, rounding issues from mixed numeric types can lead to the following inconsitency, not yet caught.
    // We handle it here however, by emptying the domain if the invariant post-condition is not satisfied. The
    // motivating case for this: A:Int[0,10] + B:Int[0,10] == C:Real[0.5, 0.5].
    if(!domz.isMember(Infinity::plus(yMax, xMin, zMin)) || !domz.isMember(Infinity::plus(yMin, xMax, zMin)))
      domz.empty();
  }

  EqualConstraint::EqualConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);

    // check the arguments - must both be enumerations or intervals
    check_error(getCurrentDomain(m_variables[X]).isEnumerated() == getCurrentDomain(m_variables[Y]).isEnumerated());
  }

  void EqualConstraint::handleExecute()
  {
    AbstractDomain& domx = getCurrentDomain(m_variables[X]);
    AbstractDomain& domy = getCurrentDomain(m_variables[Y]);

    // Discontinue if either domain is dynamic
    if(domx.isDynamic() || domy.isDynamic())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    domx.equate(domy);

    check_error(domx.isEmpty() || domy.isEmpty() || domx == domy);
  }

  AbstractDomain&  EqualConstraint::getCurrentDomain(const ConstrainedVariableId& var){
    return Constraint::getCurrentDomain(var);
  }

  SubsetOfConstraint::SubsetOfConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const ConstrainedVariableId& variable,
					 const AbstractDomain& superset)
					 
    : UnaryConstraint(name, propagatorName, constraintEngine, variable), 
    m_isDirty(true),
    m_currentDomain(getCurrentDomain(variable)),
    m_executionCount(0){
    check_error(superset.isDynamic() || !superset.isEmpty());
    check_error(getCurrentDomain(variable).getType() == superset.getType() ||
		(getCurrentDomain(variable).isEnumerated() && getCurrentDomain(variable).isEnumerated()));

    if(m_currentDomain.isEnumerated())
      m_superSetDomain = new EnumeratedDomain((const EnumeratedDomain&) superset);
    else if (superset.getType() == AbstractDomain::INT_INTERVAL)
      m_superSetDomain = new IntervalIntDomain((const IntervalIntDomain&) superset);
    else if (superset.getType() == AbstractDomain::REAL_INTERVAL)
      m_superSetDomain = new IntervalDomain((const IntervalDomain&) superset);
    else if (superset.getType() == AbstractDomain::BOOL)
      m_superSetDomain = new BoolDomain((const BoolDomain&) superset);
  }

  SubsetOfConstraint::~SubsetOfConstraint(){
    delete m_superSetDomain;
  }

  void SubsetOfConstraint::handleExecute(){
    if(m_currentDomain.isEnumerated())
      ((EnumeratedDomain&)m_currentDomain).intersect((const EnumeratedDomain&) *m_superSetDomain);
    else
      ((IntervalDomain&)m_currentDomain).intersect((const IntervalDomain&) *m_superSetDomain);

    m_isDirty  = false;
    m_executionCount++;
  }

  bool SubsetOfConstraint::canIgnore(const ConstrainedVariableId& variable, 
				     int argIndex, 
				     const DomainListener::ChangeType& changeType) {
    check_error(argIndex == 0);
    return(changeType != DomainListener::RELAXED);
  }

  int SubsetOfConstraint::executionCount() const {return m_executionCount;}

  const AbstractDomain& SubsetOfConstraint::getDomain() const {return *m_superSetDomain;}

  LessThanEqualConstraint::LessThanEqualConstraint(const LabelStr& name,
						   const LabelStr& propagatorName,
						   const ConstraintEngineId& constraintEngine,
						   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);
  }

  void LessThanEqualConstraint::handleExecute()
  {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));

    // Discontinue if either domain is dynamic
    if(domx.isDynamic() || domy.isDynamic())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    // Discontinue if any domain is enumerated but not a singleton
    if (domx.isEnumerated() && !domx.isSingleton())
      return;

    if (domy.isEnumerated() && !domy.isSingleton())
      return;

    if(domx.intersect(domx.getLowerBound(), domy.getUpperBound()) && domx.isEmpty())
      return;

    domy.intersect(domx.getLowerBound(), domy.getUpperBound());
  }

  bool LessThanEqualConstraint::canIgnore(const ConstrainedVariableId& variable, 
					  int argIndex, 
					  const DomainListener::ChangeType& changeType) {
    return(changeType == DomainListener::SET ||
	   changeType == DomainListener::SET_TO_SINGLETON ||
	   (argIndex == X && 
	    (changeType == DomainListener::UPPER_BOUND_DECREASED)) ||
	   (argIndex == Y && 
	    (changeType == DomainListener::LOWER_BOUND_INCREASED ||
	     changeType == DomainListener::RESTRICT_TO_SINGLETON)));
  }

  NotEqualConstraint::NotEqualConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);

    // check the arguments - must both be enumerations, since we don't implement splitting of domains
    check_error(getCurrentDomain(m_variables[X]).isEnumerated() && getCurrentDomain(m_variables[Y]).isEnumerated());
  }

  void NotEqualConstraint::handleExecute()
  {
    AbstractDomain& domx = getCurrentDomain(m_variables[X]);
    AbstractDomain& domy = getCurrentDomain(m_variables[Y]);

    // Discontinue if either domain is dynamic
    if(domx.isDynamic() || domy.isDynamic())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    if(domx.isSingleton() && domy.isMember(domx.getSingletonValue()))
      domy.remove(domx.getSingletonValue());
    else if (domy.isSingleton() && domx.isMember(domy.getSingletonValue()))
      domx.remove(domy.getSingletonValue());
  }

  bool NotEqualConstraint::canIgnore(const ConstrainedVariableId& variable, 
				     int argIndex, 
				     const DomainListener::ChangeType& changeType){
    // if it is a restriction, but not a singleton, then we can ignore it.
    if(changeType != DomainListener::RESET && changeType != DomainListener::RELAXED)
      return !getCurrentDomain(variable).isSingleton();
  }

  /*************************************************************
   * MultEqualConstraint
   *************************************************************/
  MultEqualConstraint::MultEqualConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);
    for(int i=0; i< ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  /*!< Simple local utility functions */
  double max(double a, double b){return (a > b ? a : b);}
  double min(double a, double b){return (a < b ? a : b);}

  /*!< Helper method to compute new bounds for both X and Y in X*Y==Z */
  bool updateMinAndMax(IntervalDomain& targetDomain, 
		       double denomMin,
		       double denomMax,
		       double numMin,
		       double numMax){
    double xMax = targetDomain.getUpperBound();
    double xMin = targetDomain.getLowerBound();
    double newMin = xMax;
    double newMax = xMin;

    if(denomMin == 0 || denomMax == 0){ // If the denominators are 0, we know the results are infinite
      if(numMax > 0)
	newMax = PLUS_INFINITY;
      if(numMin < 0)
	newMin = MINUS_INFINITY;
    }
    else { // Otherwise we must examine min and max of various permutations in order to handle signs correctly
      if(denomMin != 0){
	newMax = max(newMax, max(numMax / denomMin, numMin / denomMin));
	newMin = min(newMin, min(numMax / denomMin, numMin / denomMin));
      }

      if(denomMax != 0){
	newMax = max(newMax, max(numMax / denomMax, numMin/ denomMax));
	newMin = min(newMin, min(numMax / denomMax, numMin/ denomMax));
      }
    }

    if(xMax > newMax)
      xMax = targetDomain.translateNumber(newMax, false);
    if(xMin < newMin)
      xMin = targetDomain.translateNumber(newMin, true);

    if(targetDomain.intersect(xMin,xMax)  && targetDomain.isEmpty())
      return false;
    else
      return true;
  }

  void MultEqualConstraint::handleExecute()
  {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domz = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

    /* Test preconditions for continued execution */
    if(domx.isDynamic() ||
       domy.isDynamic() ||
       domz.isDynamic())
      return;
 

    check_error(!domx.isEmpty() && !domy.isEmpty() && !domz.isEmpty());

    double xMin;
    double xMax;
    double yMin;
    double yMax;
    double zMin;
    double zMax;

    domx.getBounds(xMin, xMax);
    domy.getBounds(yMin, yMax);
    domz.getBounds(zMin, zMax);

    // Process Z
    double max_z = max(xMax * yMax, max(xMin * yMin, max(xMin * yMax, xMax * yMin)));
    if(zMax > max_z)
      zMax = domz.translateNumber(max_z, false);

    double min_z = min(xMax * yMax, min(xMin * yMin, min(xMin * yMax, xMax * yMin)));
    if(zMin < min_z)
      zMin = domz.translateNumber(min_z, true);

    if(domz.intersect(zMin,zMax) && domz.isEmpty())
      return;

    // Process X
    if(!updateMinAndMax(domx, yMin, yMax, zMin, zMax))
      return;

    // Process Y
    updateMinAndMax(domy, xMin, xMax, zMin, zMax);
  }

  /*************************************************************
   * AddMultEqualConstraint: A + (B*C) == D
   *************************************************************/
  AddMultEqualConstraint::AddMultEqualConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_interimVariable(constraintEngine, IntervalDomain(), false, LabelStr("InternalConstraintVariable"), getId()),
      m_multEqualConstraint(LabelStr("Internal::multEqual"),
			    propagatorName,
			    constraintEngine,
			    makeScope(m_variables[B], m_variables[C], m_interimVariable.getId())),
      m_addEqualConstraint(LabelStr("Internal:addEqual"), 
			   propagatorName, 
			   constraintEngine, 
			   makeScope(m_interimVariable.getId(), m_variables[A], m_variables[D])){
    check_error(m_variables.size() == ARG_COUNT);
    for(int i=0; i< ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  // All the work is done by the composite constraints
  void AddMultEqualConstraint::handleExecute(){}

}
