#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "LabelSet.hh"

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

    bool xIsInfinite = domx.getBounds(xMin, xMax);
    bool yIsInfinite = domy.getBounds(yMin, yMax);
    bool zIsInfinite = domz.getBounds(zMin, zMax);

    /* Bomb out if 2 or more domains are infinite */
    if((xIsInfinite && (yIsInfinite || zIsInfinite)) ||
       (yIsInfinite && zIsInfinite))
      return;

    if (!(xIsInfinite || yIsInfinite)) {
        if (zMin < xMin + yMin)
            zMin = xMin + yMin;
        if (zMax > xMax + yMax)
            zMax = xMax + yMax;
    }

    if(domz.intersect(zMin,zMax) && domz.isEmpty())
      return;
    
    if (!(zIsInfinite || yIsInfinite)) {
        if (xMax > zMax - yMin)
            xMax = zMax - yMin;
        if (xMin < zMin - yMax)
            xMin = zMin - yMax;
    }

    if(domx.intersect(xMin,xMax)  && domx.isEmpty())
      return;
    
    if (!(xIsInfinite || zIsInfinite)) {
        if (yMax > zMax - xMin)
            yMax = zMax - xMin;
        if (yMin < zMin - xMax)
            yMin = zMin - xMax;
    }

    domy.intersect(yMin,yMax);
  }


  void AddEqualConstraint::handleExecute(const ConstrainedVariableId& variable, 
					 int argIndex, 
					 const DomainListener::ChangeType& changeType){
    handleExecute();
  }

  EqualConstraint::EqualConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables), m_lastNotified(0){
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

  void EqualConstraint::handleExecute(const ConstrainedVariableId& variable, 
				      int argIndex, 
				      const DomainListener::ChangeType& changeType){
    handleExecute();
  }


  bool EqualConstraint::canIgnore(const ConstrainedVariableId& variable, 
				     int argIndex, 
				  const DomainListener::ChangeType& changeType) {

    if(m_lastNotified != m_constraintEngine->cycleCount()){
      m_lastNotified = m_constraintEngine->cycleCount();
      return false;
    }
    else
      return true;
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

  void SubsetOfConstraint::handleExecute(const ConstrainedVariableId& variable, 
					 int argIndex, 
					 const DomainListener::ChangeType& changeType){
    check_error(argIndex == 0);
    handleExecute();
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

  void LessThanEqualConstraint::handleExecute(const ConstrainedVariableId& variable, 
					      int argIndex, 
					      const DomainListener::ChangeType& changeType){
    handleExecute();
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
}
