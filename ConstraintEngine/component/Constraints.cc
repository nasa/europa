#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "Domain.hh"

namespace Prototype
{
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
    if(zMax > xMax + yMax)
      zMax = domz.translateNumber(xMax + yMax, false);
    if(zMin < xMin + yMin)
      zMin = domz.translateNumber(xMin + yMin, true);
    if(domz.intersect(zMin,zMax) && domz.isEmpty())
      return;

    // Process X
    if(xMax > zMax - yMin)
      xMax = domx.translateNumber(zMax - yMin, false);
    if(xMin < zMin - yMax)
      xMin = domx.translateNumber(zMin - yMax, true);
    if(domx.intersect(xMin,xMax)  && domx.isEmpty())
      return;

    // Process Y
    if(yMax > zMax - xMin)
      yMax = domy.translateNumber(zMax - xMin, false);
    if(yMin < zMin - xMax)
      yMin = domy.translateNumber(zMin - xMax, true);
    domy.intersect(yMin,yMax);
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
    // Check the arguments - no enumerations supported
    check_error(getCurrentDomain(m_variables[X]).isInterval());
    check_error(getCurrentDomain(m_variables[Y]).isInterval());
  }

  void LessThanEqualConstraint::handleExecute()
  {
    IntervalDomain& domx = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));

    // Discontinue if either domain is dynamic
    if(domx.isDynamic() || domy.isDynamic())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    // Discontinue if they are not both finite or infinite
    if(domx.isFinite() != domy.isFinite()){
      domx.empty();
      return;
    }

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
    if(zMax > xMax * yMax)
      zMax = domz.translateNumber(xMax * yMax, false);
    if(zMin < xMin * yMin)
      zMin = domz.translateNumber(xMin * yMin, true);
    if(domz.intersect(zMin,zMax) && domz.isEmpty())
      return;

    // Process X
    if(yMin != 0 && xMax > zMax / yMin)
      xMax = domx.translateNumber(zMax / yMin, false);
    if(yMax != 0 && xMin < zMin / yMax)
      xMin = domx.translateNumber(zMin / yMax, true);
    if(domx.intersect(xMin,xMax)  && domx.isEmpty())
      return;

    // Process Y
    if(xMin != 0 && yMax > zMax / xMin)
      yMax = domy.translateNumber(zMax / xMin, false);
    if(xMax != 0 && yMin < zMin / xMax)
      yMin = domy.translateNumber(zMin / xMax, true);
    domy.intersect(yMin,yMax);
  }
}
