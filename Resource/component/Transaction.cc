#include "Transaction.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "IntervalDomain.hh"
#include "Constraint.hh"
#include "ConstraintLibrary.hh"
#include <vector>


namespace Prototype {

  Transaction::Transaction(const PlanDatabaseId& planDatabase,
			   const LabelStr& predicateName,
			   const IntervalIntDomain& timeBaseDomain,
			   double min, 
			   double max,
			   bool closed) 
    : EventToken(planDatabase, 
		 predicateName,
		 false,
		 timeBaseDomain,
		 Token::noObject(),
		 false)
    
  {
    //create the usage variable
    m_usage = (new TokenVariable<IntervalDomain>(m_id,
						 m_allVariables.size(),
						 m_planDatabase->getConstraintEngine(), 
						 IntervalDomain(),
						 true,
						 LabelStr("Usage")))->getId();
    m_allVariables.push_back(m_usage);

    m_usage->specify(IntervalDomain(min, max));
   
    if(closed)
      close();
  }
 
  Transaction::Transaction(const PlanDatabaseId& planDatabase,
			   const LabelStr& predicateName,
			   bool rejectable,
			   const IntervalIntDomain& timeBaseDomain,
			   const LabelStr& objectName,
			   bool closed)
    : EventToken(planDatabase, 
		 predicateName, 
		 false,
		 timeBaseDomain,
		 objectName,
		 false) {
    // No assignment of variable here. Use close method
  }

  Transaction::Transaction(const TokenId& parent,
			   const LabelStr& predicateName,
			   const IntervalIntDomain& timeBaseDomain,
			   const LabelStr& objectName,
			   bool closed)
    : EventToken(parent, 
		 predicateName,
		 timeBaseDomain,
		 objectName,
		 closed){
    // No assignment of variable here. Use close method
  }
  
  void Transaction::close() {
    EventToken::close();

    check_error(m_usage.isValid());

    // add the resource constraint which will act as a messenger to changes and inform the ResourcePropagator.
    std::vector<ConstrainedVariableId> temp;
    temp.push_back(getObject());
    temp.push_back(getTime());
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

    // Now activate the transaction, since resource transactions should not be merged.
    // Note that we do not use 'activate' from the super class since that can lead to a cycle
    // when a transaction is created via subgoaling.
    activateInternal();
  }

  ResourceId Transaction::getResource() const {
    if(getObject()->lastDomain().isSingleton())
      return getObject()->lastDomain().getSingletonValue();
    else
      return ResourceId::noId();
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
    check_error(m_id.isValid());

    // Do simple limit checking
    check_error(getMin() >= -LARGEST_VALUE);
    check_error(getMax() <= LARGEST_VALUE);
    check_error(getEarliest() >= -LATEST_TIME);
    check_error(getLatest() <= LATEST_TIME);
    return true;
  }

  void Transaction::setEarliest(int earliest)
  {
    int t_earliest = getEarliest();
    int t_latest = getLatest();

    check_error(earliest <= t_latest );
    bool isRelaxed = (t_earliest > earliest);
    getTime()->reset();
    getTime()->specify(IntervalIntDomain(earliest, t_latest));
    if(getResource() != ResourceId::noId()){
      if(isRelaxed)
	getResource()->notifyTimeRelaxed(m_id);
      else
	getResource()->notifyTimeRestricted(m_id);
    }
  }

  void Transaction::setLatest(int latest)
  {
    int t_earliest = getEarliest();
    int t_latest = getLatest();

    check_error(latest >= t_earliest);
    bool isRelaxed = (t_latest < latest);
    getTime()->reset();
    getTime()->specify(IntervalIntDomain(t_earliest, latest));
    if(getResource() != ResourceId::noId()){
      if(isRelaxed)
	getResource()->notifyTimeRelaxed(m_id);
      else
	getResource()->notifyTimeRestricted(m_id);
    }
  }

  double Transaction::getMin() const {
    return m_usage->lastDomain().getLowerBound();
  }

  double Transaction::getMax() const {
    return m_usage->lastDomain().getUpperBound();
  }


  void Transaction::setMin(double min)
  {
    check_error(min <= getMax());
    double max = getMax();
    if (min < getMin())
      m_usage->reset();
    m_usage->specify(IntervalDomain(min, max));
    if(getResource() != ResourceId::noId())
      getResource()->notifyQuantityChanged(m_id);
  }
  void Transaction::setMax(double max)
  {
    check_error(max >= getMin());
    double min = getMin();
    if (max > getMax())
      m_usage->reset();
    m_usage->specify(IntervalDomain(min, max));
    if(getResource() != ResourceId::noId())
      getResource()->notifyQuantityChanged(m_id);
  }
  void Transaction::print(ostream& os)
  {
    os << "RESOURCE: " << (getResource() == ResourceId::noId() ? "UNASSIGNED" : getResource()->getName().toString())
       << "[" << getMin() << ", " << getMax() << ", " << getEarliest() << ", " << getLatest() << "]";
  }
} //namespace prototype
