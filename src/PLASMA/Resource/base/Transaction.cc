#include "Transaction.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "IntervalDomain.hh"
#include "Constraint.hh"
#include "ConstraintFactory.hh"
#include <vector>


namespace EUROPA {

  Transaction::Transaction(const PlanDatabaseId& planDatabase,
			   const LabelStr& predicateName,
			   const IntervalIntDomain& timeBaseDomain,
			   double min, 
			   double max,
			   bool closed) 
    : EventToken(planDatabase, 
		 predicateName,
		 false,
		 false,
		 timeBaseDomain,
		 Token::noObject(),
		 false)
    
  {
    commonInit(closed);
    m_usage->restrictBaseDomain(IntervalDomain(min, max));
  }

  Transaction::Transaction(const PlanDatabaseId& planDatabase,
			   const LabelStr& predicateName,
			   bool rejectable,
			   bool isFact,
			   const IntervalIntDomain& timeBaseDomain,
			   const LabelStr& objectName,
			   bool closed)
    : EventToken(planDatabase, 
		 predicateName, 
		 false,
		 isFact,
		 timeBaseDomain,
		 objectName,
		 false) {
    commonInit(closed);   
  }

  Transaction::Transaction(const TokenId& parent,
			   const LabelStr& relation,
			   const LabelStr& predicateName,
			   const IntervalIntDomain& timeBaseDomain,
			   const LabelStr& objectName,
			   bool closed)
    : EventToken(parent, 
		 relation,
		 predicateName,
		 timeBaseDomain,
		 objectName,
		 closed){
    commonInit(closed); 
  }

  void Transaction::commonInit(bool closed){
    //create the usage variable
    m_usage = (new TokenVariable<IntervalDomain>(m_id,
						 m_allVariables.size(),
						 m_planDatabase->getConstraintEngine(), 
						 IntervalDomain(),
						 false, // TODO: fixme
						 true,
						 LabelStr("quantity")))->getId();

    m_allVariables.push_back(m_usage);

    // add the resource constraint which will act as a messenger to changes and inform the ResourcePropagator.
    ConstraintId constraint = 
        m_planDatabase->getConstraintEngine()->createConstraint("ResourceTransactionRelation", 
					  makeScope(getObject(), getTime(), m_usage));
    m_standardConstraints.insert(constraint);
   
    if(closed)
      close();
  }

  void Transaction::close() {

    EventToken::close();

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

  eint Transaction::getEarliest() const 
  {
    return((eint) getTime()->lastDomain().getLowerBound());
  }

  eint Transaction::getLatest() const 
  {
    return((eint) getTime()->lastDomain().getUpperBound());
  }

  bool Transaction::isValid() const
  {
    check_error(m_id.isValid());

    // Do simple limit checking
    check_error(getMin() >= MINUS_INFINITY);
    check_error(getMax() <= PLUS_INFINITY);
    check_error(getEarliest() >= MINUS_INFINITY);
    check_error(getLatest() <= PLUS_INFINITY);
    return true;
  }

  void Transaction::setEarliest(int earliest)
  {
    int t_latest = getLatest();
    getTime()->restrictBaseDomain(IntervalIntDomain(earliest, t_latest));
  }

  void Transaction::setLatest(int latest)
  {
    int t_earliest = getEarliest();
    getTime()->restrictBaseDomain(IntervalIntDomain(t_earliest, latest));
  }

  double Transaction::getMin() const {
    return m_usage->lastDomain().getLowerBound();
  }

  double Transaction::getMax() const {
    return m_usage->lastDomain().getUpperBound();
  }

  void Transaction::setMin(double min)
  {
    double max = getMax();
    m_usage->restrictBaseDomain(IntervalDomain(min, max));
  }

  void Transaction::setMax(double max)
  {
    double min = getMin();

    m_usage->restrictBaseDomain(IntervalDomain(min, max));
  }

  void Transaction::print(ostream& os)
  {
    os << "RESOURCE: " << (getResource() == ResourceId::noId() ? "UNASSIGNED" : getResource()->getName().toString())
       << "[" << getMin() << ", " << getMax() << ", " << getEarliest() << ", " << getLatest() << "]";
  }
} //namespace EUROPA
