#include "Transaction.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "../PlanDatabase/TokenVariable.hh"
#include "../PlanDatabase/PlanDatabase.hh"
#include "../ConstraintEngine/IntervalDomain.hh"
#include "../ConstraintEngine/Constraint.hh"
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include <vector>

namespace Prototype {

  Transaction::Transaction(const PlanDatabaseId& planDatabase,
			   const LabelStr& predicateName,
			   const IntervalIntDomain& timeBaseDomain,
			   double min, 
			   double max) 
    : EventToken(planDatabase, 
		 predicateName,
		 false,
		 timeBaseDomain,
		 Token::noObject(),
		 false)
    
  {
    m_min = min;
    m_max = max;


    // for now set m_resource to getObject.  Later remove m_resource as redundant and replace it with refs to getObject.
    //m_resource = getObject();    


    //create the usage variable
    m_usage = (new TokenVariable<IntervalDomain>(m_id,
						 m_allVariables.size(),
						 m_planDatabase->getConstraintEngine(), 
						 IntervalDomain(),
						 true,
						 LabelStr("Usage")))->getId();
    m_allVariables.push_back(m_usage);

    m_usage->specify(IntervalDomain(m_min, m_max));

    // add the resource constraint which will act as a messenger to changes and inform the ResourcePropagator.
    std::vector<ConstrainedVariableId> temp;
    temp.push_back(getTime());
    temp.push_back(getObject());
    temp.push_back(m_usage);
    ConstraintId usageRelation = 
      ConstraintLibrary::createConstraint(LabelStr("ResourceRelation"), m_planDatabase->getConstraintEngine(), temp);
    m_standardConstraints.insert(usageRelation);

    // add the resource constraint which will act as a messenger to changes and inform the ResourcePropagator.
    std::vector<ConstrainedVariableId> temp1;
    temp1.push_back(getObject());
    temp1.push_back(getTime());
    temp1.push_back(m_usage);
    ConstraintId horizonRelation = 
      ConstraintLibrary::createConstraint(LabelStr("HorizonRelation"), m_planDatabase->getConstraintEngine(), temp1);
    m_standardConstraints.insert(horizonRelation);

    close();

    // All transactions as immediately active
    activate();

  }
    

  int Transaction::getEarliest() const 
  {
    return((int) getTime()->lastDomain().getLowerBound());
  }

  int Transaction::getLatest() const 
  {
    return((int) getTime()->lastDomain().getUpperBound());
  }

  bool Transaction::isValid() const
  {
    check_error(m_min >= -LARGEST_VALUE);
    check_error(m_max <= LARGEST_VALUE);
    check_error(getEarliest() >= -LATEST_TIME);
    check_error(getLatest() <= LATEST_TIME);
    check_error(m_min <= m_max);
    check_error(getEarliest() <= getLatest());
    check_error(m_id.isValid());
    return true;
  }

  void Transaction::notifyInserted(ResourceId& resource)
  {
    check_error(resource.isValid());
    check_error(m_resource == ResourceId::noId());
    m_resource = resource;
  }

  void Transaction::notifyRemoved(ResourceId& resource)
  {
    check_error(m_resource == resource);
    m_resource = ResourceId::noId();
  }

  void Transaction::notifyDeleted(ResourceId& resource)
  {
    check_error(m_resource == resource);
    m_resource = ResourceId::noId();
  }

  void Transaction::setEarliest(int earliest)
  {
    int t_earliest = getEarliest();
    int t_latest = getLatest();

    check_error(earliest <= t_latest );
    bool isRelaxed = (t_earliest > earliest);
    getTime()->reset();
    getTime()->specify(IntervalIntDomain(earliest, t_latest));
    if(m_resource != ResourceId::noId()){
      if(isRelaxed)
	m_resource->notifyTimeRelaxed(m_id);
      else
	m_resource->notifyTimeRestricted(m_id);
    }
    noteChanged();
  }
  void Transaction::setLatest(int latest)
  {
    int t_earliest = getEarliest();
    int t_latest = getLatest();

    check_error(latest >= t_earliest);
    bool isRelaxed = (t_latest < latest);
    getTime()->reset();
    getTime()->specify(IntervalIntDomain(t_earliest, latest));
    if(m_resource != ResourceId::noId()){
      if(isRelaxed)
	m_resource->notifyTimeRelaxed(m_id);
      else
	m_resource->notifyTimeRestricted(m_id);
    }
    noteChanged();
  }
  void Transaction::setMin(double min)
  {
    check_error(min <= m_max);
    m_min = min;
    m_usage->reset();
    m_usage->specify(IntervalDomain(m_min, m_max));
    if(m_resource != ResourceId::noId())
      m_resource->notifyQuantityChanged(m_id);
    noteChanged();
  }
  void Transaction::setMax(double max)
  {
    check_error(max >= m_min);
    m_max = max;
    m_usage->reset();
    std::cout << m_usage->lastDomain().getUpperBound() << std::endl;
    m_usage->specify(IntervalDomain(m_min, m_max));
    std::cout << m_usage->lastDomain().getUpperBound() << std::endl;
    if(m_resource != ResourceId::noId())
      m_resource->notifyQuantityChanged(m_id);
    noteChanged();
  }
  void Transaction::print(ostream& os)
  {
    os << "RESOURCE: " << (m_resource == ResourceId::noId() ? "UNASSIGNED" : m_resource->getName().toString())
       << "[" << m_min << ", " << m_max << ", " << getEarliest() << ", " << getLatest() << "]";
  }

  bool Transaction::checkAndClearChange()
  {
    bool retVal = m_changed;
    m_changed = false;
    return(retVal);
  }

  void Transaction::noteChanged()
  {
    check_error(isValid());
    m_changed = true;
  }

} //namespace prototype
