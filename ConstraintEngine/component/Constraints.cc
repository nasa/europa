#include "Constraints.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "IntervalRealDomain.hh"
#include "LabelSet.hh"

namespace Prototype
{
  AddEqualConstraint::AddEqualConstraint(const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint("AddEqual", constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);
    for(int i=0; i< ARG_COUNT; i++)
      check_error(!getCurrentDomain(m_variables[i]).isEnumerated());
  }

  void AddEqualConstraint::handleExecute()
  {
    IntervalDomain& domx = dynamic_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
    IntervalDomain& domy = dynamic_cast<IntervalDomain&>(getCurrentDomain(m_variables[Y]));
    IntervalDomain& domz = dynamic_cast<IntervalDomain&>(getCurrentDomain(m_variables[Z]));

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

  EqualConstraint::EqualConstraint(const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : Constraint("Equal", constraintEngine, variables){
    check_error(variables.size() == ARG_COUNT);

    // type check the arguments - only work with enumerations for now
    check_error(getCurrentDomain(m_variables[X]).getType() == getCurrentDomain(m_variables[Y]).getType());
  }

  void EqualConstraint::handleExecute()
  {
    AbstractDomain& domx = getCurrentDomain(m_variables[X]);
    AbstractDomain& domy = getCurrentDomain(m_variables[Y]);

    // Discontinue if either domain is dynamic
    if(domx.isDynamic() || domy.isDynamic())
      return;

    check_error(!domx.isEmpty() && !domy.isEmpty());

    // Discontinue if they are not both finite or infinite
    if(domx.isFinite() != domy.isFinite()){
      domx.empty();
      return;
    }

    // By construction, we know the arguments are of the same type. So special case
    // accordingly
    if(domx.getType() == AbstractDomain::LABEL_SET){
      LabelSet& dx = dynamic_cast<LabelSet&>(domx);
      LabelSet& dy = dynamic_cast<LabelSet&>(domy);
      dx.equate(dy);
    }
    else {
      IntervalDomain& dx = dynamic_cast<IntervalDomain&>(domx);
      IntervalDomain& dy = dynamic_cast<IntervalDomain&>(domy);
      if(dx.intersect(dy.getLowerBound(), dy.getUpperBound()) && dx.isEmpty())
	return;

      dy.intersect(dx.getLowerBound(), dx.getUpperBound());
    }
  }


  void EqualConstraint::handleExecute(const ConstrainedVariableId& variable, 
				      int argIndex, 
				      const DomainListener::ChangeType& changeType){
    handleExecute();
  }

  AbstractDomain&  EqualConstraint::getCurrentDomain(const ConstrainedVariableId& var){
    return Constraint::getCurrentDomain(var);
  }


  SubsetOfConstraint::SubsetOfConstraint(const ConstraintEngineId& constraintEngine,
					 const ConstrainedVariableId& variable,
					 const AbstractDomain& superset)
    : Constraint("SubsetOf", constraintEngine, variable), 
    m_isDirty(true),
    m_currentDomain(getCurrentDomain(variable)),
    m_executionCount(0){
    check_error(superset.isDynamic() || !superset.isEmpty());
    check_error(getCurrentDomain(variable).getType() == superset.getType());

    switch(m_currentDomain.getType()){
    case AbstractDomain::LABEL_SET:
      m_superSetDomain = new LabelSet((const LabelSet&) superset);
      break;
    case AbstractDomain::INT_INTERVAL:
      m_superSetDomain = new IntervalIntDomain((const IntervalIntDomain&) superset);
      break;
    case AbstractDomain::REAL_INTERVAL:
      m_superSetDomain = new IntervalRealDomain((const IntervalRealDomain&) superset);
      break;
    default:
      check_error(ALWAYS_FAILS);
    }
  }

  SubsetOfConstraint::~SubsetOfConstraint(){
    delete m_superSetDomain;
  }

  void SubsetOfConstraint::handleExecute(){
    switch(m_currentDomain.getType()){
    case AbstractDomain::LABEL_SET:
      ((LabelSet&)m_currentDomain).intersect((const LabelSet&) *m_superSetDomain);
      break;
    case AbstractDomain::INT_INTERVAL:
      ((IntervalIntDomain&)m_currentDomain).intersect((const IntervalIntDomain&)*m_superSetDomain);
      break;
    case AbstractDomain::REAL_INTERVAL:
      ((IntervalIntDomain&)m_currentDomain).intersect((const IntervalIntDomain&)*m_superSetDomain);
      break;
    default:
      check_error(ALWAYS_FAILS);
    }

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
				     const DomainListener::ChangeType& changeType) const {
    check_error(argIndex == 0);
    return(changeType != DomainListener::RELAXED);
  }

  int SubsetOfConstraint::executionCount() const {return m_executionCount;}
}
