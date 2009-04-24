//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software
#include "Resource.hh"
#include "Debug.hh"
#include "Utils.hh"
#include "Domains.hh"
#include "ConstraintEngine.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "ResourceViolation.hh"
#include "ResourceFlaw.hh"

#include <vector>
#include <string>
#include <sstream>

/**
 * @file Resource.cc
 * @author Conor McGann
 * @brief Implements the core components and collaborations of Resource, Transaction, Instant and Violation.
 * @todo Reinstate incremental computation
 * @todo Consider changing initial capacity to be a bound.
 * @note Had to prefix call to cleanup in Utils with namespace qualifier for g++ 2.96. Bizarre!
 * @date 2005
 */

namespace EUROPA  {

  void Resource::updateInstantBounds(InstantId& inst, const double lowerMin,
                                  const double lowerMax, const double upperMin,
                                  const double upperMax) {
    inst->updateBounds(lowerMin, lowerMax, upperMin, upperMax);
  }

  std::string toString(const ResourceId& res){
    std::stringstream sstr;
    res->print(sstr);
    return sstr.str();
  }

  Resource::Resource(const PlanDatabaseId& planDatabase,
                     const LabelStr& type,
                     const LabelStr& name,
                     double initialCapacity,
                     double limitMin,
                     double limitMax,
                     double productionRateMax,
                     double productionMax,
                     double consumptionRateMax,
                     double consumptionMax) : Object(planDatabase, type, name) {
    init(initialCapacity,
         limitMin,
         limitMax,
         productionRateMax,
         productionMax,
         consumptionRateMax,
         consumptionMax);
  }

  Resource::Resource(const PlanDatabaseId& planDatabase,
                     const LabelStr& type,
                     const LabelStr& name,
                     bool open) : Object(planDatabase, type, name, open) {
    // call to init is deferred
  }

  Resource::Resource(const ObjectId parent,
                     const LabelStr& type,
                     const LabelStr& name,
                     bool open) : Object(parent, type, name, open) {
    // call to init is deferred
  }

  void Resource::init(double initialCapacity,
                      double limitMin,
                      double limitMax,
                      double productionRateMax,
                      double productionMax,
                      double consumptionRateMax,
                      double consumptionMax) {
    m_initialCapacity = initialCapacity;
    m_limitMin = limitMin;
    m_limitMax = limitMax;

    if(productionRateMax == PLUS_INFINITY)
      m_productionRateMax = productionMax;
    else
      m_productionRateMax = productionRateMax;

    m_productionMax = productionMax;

    if(consumptionRateMax == MINUS_INFINITY)
      m_consumptionRateMax = consumptionMax;
    else
      m_consumptionRateMax = consumptionRateMax;

    m_consumptionMax = consumptionMax;
    markDirty();
    check_error(isValid());
  }

  Resource::~Resource() {
    discard(false);
  }

  void Resource::handleDiscard(){
    resetProfile();
    Object::handleDiscard();
  }

  void Resource::add(const TokenId& token){
    Object::add(token);
    markDirty();
    debugMsg("Resource:add",getName().toString() << " added " << token->toString());
  }

  void Resource::remove(const TokenId& token){
    Object::remove(token);
    markDirty();
    debugMsg("Resource:remove",getName().toString() << " removed " << token->toString());
  }

  bool Resource::isDirty() const {return m_dirty;}

  void Resource::markDirty()
  {
    m_dirty = true;
    debugMsg("Resource:markDirty",getName().toString() << " marked Dirty");
  }

  bool Resource::insert(const TransactionId& tx) {
    check_error(isValid());
    check_error(tx.isValid());

    // Must ensure:
    // 1. All timepoints intersecting this Transaction time should be updated
    // 2. If either bound of the transaction time is not a timepoint then it should result in a time point insertion

    // Insert the start
    int startTime = tx->getEarliest();
    std::map<int, InstantId>::iterator it = m_instants.lower_bound(startTime);
    InstantId instant;

    // If we are at the end, the list is empty, so we have to insert
    if(it == m_instants.end())
      it = createInstant(startTime, it);

    // Now assign the instant accordingly in either case
    instant = it->second;

    if(instant->getTime() != startTime){
      it = createInstant(startTime, --it);
      instant = it->second;
    }

    // Now insert the transaction to the instant.
    instant->insert(tx);

    // iterate forward until u hit the end, or you hit a point with a time later than the latest time
    int endTime = tx->getLatest();
    bool endInserted = (instant->getTime() == endTime);
    ++it;
    while(it!=m_instants.end() && !endInserted){
      instant = it->second;
      check_error(instant.isValid());
      if(instant->getTime() > endTime){ // Indicate we are done and exit
        break;
      }
      else{
        instant->insert(tx);
        ++it;
      }

      endInserted = (instant->getTime() == endTime);
    }

    if(!endInserted){ // Insert a new instant
      it = createInstant(endTime, it);
      it->second->insert(tx);
    }

    check_error(isValid());

    return(true);
  }

  void Resource::cleanup(const TransactionId& tx) {
    check_error(tx.isValid());
    check_error(ALWAYS_FAILS); // Until we get into incremental algorithm

    // Obtain the starting instant
    std::map<int, InstantId>::iterator it = m_instants.lower_bound(tx->getEarliest());

    while(it != m_instants.end()){
      InstantId instant = it->second;
      if(instant->getTime() > tx->getLatest())
        break;
      instant->remove(tx);
      if (!hasStartOrEndTransaction(instant)){
        m_instants.erase(it++);
        delete (Instant*) instant;
      }
      else
        ++it;
    }

    check_error(isValid());
  }

  void Resource::getInstants(std::list<InstantId>& resultSet, int lowerBound, int upperBound) {
    check_error(resultSet.empty());
    check_error(resultSet.empty());
    check_error(lowerBound <= upperBound);

    // Propagate token-object relationshiop and any constraints
    getPlanDatabase()->getConstraintEngine()->propagate();

    std::map<int, InstantId>::const_iterator it = m_instants.upper_bound(lowerBound);
    while(it != m_instants.end()){
      InstantId instant = it->second;
      check_error(instant.isValid());
      check_error(lowerBound <= instant->getTime());

      if(instant->getTime() > upperBound)
        break;
      else {
        resultSet.push_back(instant);
        ++it;
      }
    }
  }

  const std::map<int, InstantId>& Resource::getInstants() const {return m_instants;}

  /** Recompute optimistically m_totalUsedConsumption and m_totalUsedProduction */
  void Resource::recomputeTotals() {
    // Reset
    m_totalUsedConsumption = 0;
    m_totalUsedProduction = 0;

    // Go over all transactions
    for ( std::set<TokenId>::iterator tx_it = m_tokens.begin();
          tx_it != m_tokens.end(); ) {
      TransactionId tx = *tx_it;
      ++tx_it;

      // Skip consideration of this transaction if it is not definitley associated with this resource
      if(!tx->getObject()->lastDomain().isSingleton())
	continue;

      checkError(tx->getObject()->lastDomain().getSingletonValue() == getId(),
		 "If a singleton, and we have synched correctly, then it must be assigned to this resource.");

      double min = tx->getMin();
      double max = tx->getMax();

      // Be optimistic: assume we use as little as possible
      // The minimum possible consumption
      if ( max<0 ) {
        m_totalUsedConsumption += max;
      }
      // The minimum possible production
      if ( min>0 ) {
        m_totalUsedProduction += min;
      }
    }

    // Check and create global violations
    if ( m_totalUsedConsumption < m_consumptionMax ) {
      addGlobalResourceViolation( ResourceViolation::ConsumptionSumExceeded );
    }
    if ( m_totalUsedProduction > m_productionMax ) {
      addGlobalResourceViolation( ResourceViolation::ProductionSumExceeded );
    }
  }

  void Resource::updateTransactionProfile() {
    static bool calledWithin = false; //MJI

    if(calledWithin) //MJI
      return; //MJI

    if(!m_dirty)
      return;

    // Clear prior data
    resetProfile();

    recomputeTotals();

    // Reset changed flag since updates have been calculated.
    m_dirty = false;

    // If have global violations, don't even bother with instants
    if ( !m_globalViolations.empty() ) return;

    rebuildProfile();

    if(m_instants.empty())
      return;

    // If have global violations, don't even bother with instants
    if ( !m_globalViolations.empty() ) return;

    check_error(isValid());

    computeTransactionProfile();

    debugMsg("Resource:updateTransactionProfile", EUROPA::toString(ResourceId(m_id)));

    check_error(isValid());
  }

  /**
   * @brief We should be running this incremenatlly if we track and the algorithm for doing
   * so is implemented in Resources under NewPlan. Main issue is tracking where you have to start from.
   */
  void Resource::computeTransactionProfile() {
    // Get the seed for completion since it is iteratively defined

    // Since we have upper and lower bounds now, we can support also uncertainty
    // in the initial value!
    double runningLowerMin = m_initialCapacity;
    double runningLowerMax = m_initialCapacity;
    double runningUpperMin = m_initialCapacity;
    double runningUpperMax = m_initialCapacity;

    std::map<int, InstantId>::const_iterator it = m_instants.begin();

    // Now we should be set up to iteratively compute the revised transaction profile
    while(it != m_instants.end()){
      InstantId instant = it->second;
      const TransactionSet& transactions = instant->getTransactions();
      TransactionSet::iterator tx_it = transactions.begin();
      while (tx_it != transactions.end()) {
        TransactionId tx = *tx_it;
        computeRunningTotals(instant, tx, runningLowerMin, runningLowerMax, runningUpperMin,
                             runningUpperMax);

        ++tx_it;
      } // of while over Transactions for this instant

      // Now update the instant with the results
      instant->updateBounds( runningLowerMin, runningLowerMax,
                             runningUpperMin, runningUpperMax );

      // Now we need to figure out of any constraints have been violated.
      // Allow this to be a point of extension by inheritance or with a strategy pattern.
      applyConstraints(instant);

      // Move on to the next instant
      ++it;
    }
  }

  void Resource::computeRunningTotals(const InstantId& instant,
				      const TransactionId& tx,
				      double& runningLowerMin,
				      double& runningLowerMax,
				      double& runningUpperMin,
				      double& runningUpperMax) {
    double min = tx->getMin();
    double max = tx->getMax();

    // if the transaction just started, add producer to upper bounds and
    // consumer to the lower bounds
    if ( tx->getEarliest() == instant->getTime() ) {
      if ( max>0 )
        runningUpperMax += max;
      if ( min>0 )
        runningUpperMin += min;
      if ( max<0 )
        runningLowerMax += max;
      if ( min<0 )
        runningLowerMin += min;
    }

    // if the transaction just ended, add producer to lower bounds and
    // consumer to the upper bounds
    if ( tx->getLatest() == instant->getTime() ) {
      if ( max<0 )
        runningUpperMax += max;
      if ( min<0 )
        runningUpperMin += min;
      if ( max>0 )
        runningLowerMax += max;
      if ( min>0 )
        runningLowerMin += min;
    }
  }

#ifdef OLD_TRANSACTION_PROFILE
  void Resource::computeTransactionProfile() {
    // CompletedMax(i) = CompletedMax(i-1) + Sum(new completions (max))
    // CompletedMin(i) = CompletedMin(i-1) + Sum(new completions (min))
    // Max(i) = Completed(i) + Sum(max of Producers)
    // Min(i) = Completed(i) + Sum(min of Consumers)

    // Get the seed for completion since it is iteratively defined
    double completedMax = m_initialCapacity;
    double completedMin = m_initialCapacity;
    double productionSum = 0;
    double consumptionSum = 0;

    std::map<int, InstantId>::const_iterator it = m_instants.lower_bound(m_horizonStart);
    check_error(it != m_instants.end()); // Should always get a hit

    // If we are not starting out at the beginning
    if(it != m_instants.begin()){
      // Back up to the previous position to obtain values
      it--;

      // Obtain values from an instant that is the predecessor
      if(it  != m_instants.begin()){
        InstantId instant = it->second;
        completedMax = instant->getCompletedMax();
        completedMin = instant->getCompletedMin();
        productionSum = instant->getProductionSum();
        consumptionSum = instant->getConsumptionSum();
      }

      // Now advance it forward again
      ++it;
    }

    // Now we should be set up to iteratively compute the revised transaction profile
    while(it != m_instants.end()){
      InstantId instant = it->second;
      double newCompletionsMin = 0;
      double newCompletionsMax = 0;
      double max = 0;
      double min = 0;
      double productionMin = 0;
      double consumptionMin = 0;

      const TransactionSet& transactions = instant->getTransactions();
      TransactionSet::iterator tx_it = transactions.begin();
      while (tx_it != transactions.end()) {
        TransactionId tx = *tx_it;
        if (tx->getLatest() == instant->getTime()) {
          // It has just completed
          newCompletionsMax += tx->getMax();
          newCompletionsMin += tx->getMin();

          // If it is strictly a producer then incorporate the minimum into
          // the running total for this instant
          if (tx->canProduce() && !tx->canConsume())
            productionMin += tx->getMin();
          else
            if (tx->canConsume() && !tx->canProduce())
              // If it is strictly a consumer, the max will be the least consumption
              consumptionMin += tx->getMax();
        } else {
          // Note that some transactions might be candidates for consumption and production
          if (tx->canProduce())
            max += tx->getMax();
          if (tx->canConsume())
            min += tx->getMin();
        }
        ++tx_it;
      }

      completedMax += newCompletionsMax;
      completedMin += newCompletionsMin;
      productionSum += productionMin;
      consumptionSum += consumptionMin;

      // Now update the instant with the results
      instant->updateBounds(completedMax,
                            completedMin,
                            max + completedMax,
                            min + completedMin,
                            productionMin,
                            consumptionMin,
                            productionSum,
                            consumptionSum);


      // Now we need to figure out of any constraints have been violated.
      // Allow this to be a point of extension by inheritance or with a strategy pattern.
      applyConstraints(instant);

      // Move on to the next instant
      ++it;
    }
  }
#endif // OLD_TRANSACTION_PROFILE

  void Resource::print(ostream& os) {
    updateTransactionProfile();

    //    os << "Time:[levelMin, levelMax, productionSum, consumptionSum] ";
    os << "Time:[lowerMin, lowerMax, upperMin, upperMax] ";
    for(std::map<int, InstantId>::const_iterator it = m_instants.begin(); it != m_instants.end(); ++it){
      InstantId current = it->second;
      check_error(current.isValid());
      current->print(os);
      os << " ";
    }

    os << std::endl;
  };


  std::string Resource::toString(){
    std::ostringstream sstr;
    print(sstr);
    return sstr.str();
  }

  bool Resource::isViolated() {
    std::list<ResourceViolationId> violations;
    getResourceViolations(violations);
    return(violations.size() > 0);
  }

  void Resource::getResourceViolations(std::list<ResourceViolationId>& results, int, int) {
    check_error(isValid());
    updateTransactionProfile();
    results.clear();

    // First, add global violations
    for ( std::set<ResourceViolationId>::iterator it = m_globalViolations.begin();
          it!=m_globalViolations.end(); ++it ) {
      results.push_back(*it);
    }

    // Start at the point of the first violation and aggregate from there
    std::map<int, InstantId>::const_iterator it = m_instants.begin();
    while(it != m_instants.end()){
      InstantId current = it->second;
      ++it;

      std::list<ResourceViolationId> violations = current->getResourceViolations();
      for (std::list<ResourceViolationId>::iterator it_1 = violations.begin(); it_1 != violations.end(); ++it_1)
        results.push_back(*it_1);
    }

    debugMsg("Resource:getResourceViolations", toString());
  }

  bool Resource::isFlawed() {
    std::list<ResourceFlawId> flaws;
    getResourceFlaws(flaws, 0, 0); // the last two ints are not being used for now
    return(flaws.size() > 0);
  }

  bool Resource::hasFlaws() {
    return isFlawed();
  }

  void Resource::getResourceFlaws(std::list<ResourceFlawId>& results, int, int) {
    check_error(isValid());

    // @todo some sort of check if the profile is fresh, so that
    // we do not recompute it like crazy
    updateTransactionProfile();
    results.clear();

    // Start at the point of the first violation and aggregate from there
    std::map<int, InstantId>::const_iterator it = m_instants.begin();
    while(it != m_instants.end()){
      InstantId current = it->second;
      ++it;

      std::list<ResourceFlawId> flaws = current->getResourceFlaws();
      for (std::list<ResourceFlawId>::iterator it_1 = flaws.begin(); it_1 != flaws.end(); ++it_1)
        results.push_back(*it_1);
    }
  }

  /**
   * This method returns TRUE iff there is a violation.
   * Flaws are computed and stored within the instant, but they do not
   * affect the return value.
   */
  bool Resource::applyConstraints(const InstantId& instant) {
    bool result = false;
    instant->reset();
    /*
    // Test if the minimum production at this instant exceeds the rate limit
    if (instant->getUpperMax() > m_productionRateMax){
      instant->addResourceViolation(ResourceViolation::ProductionRateExceeded);
      result = true;
    }

    // Test if the minimum consumption at this instant exceeds the rate limit
    if (instant->getConsumptionMin() < m_consumptionRateMax) {
      instant->addResourceViolation(ResourceViolation::ConsumptionRateExceeded);
      result = true;
    }
    */
    // Check optimistic bounds for violations
    if ( instant->getUpperMax() + m_productionMax - m_totalUsedProduction < m_limitMin ) {
      // definitely violating lower limit
      instant->addResourceViolation( ResourceViolation::LevelTooLow );
      result = true;
    }
    // Consumptions are all negative
    if ( instant->getLowerMin() + m_consumptionMax - m_totalUsedConsumption > m_limitMax ) {
      // definitely violating upper limit
      instant->addResourceViolation( ResourceViolation::LevelTooHigh );
      result = true;
    }

    //-----------------------------
    // Check optimistic bounds for flaws
    if ( instant->getLowerMax()  < m_limitMin ) {
      // definitely have a flaw on lower limit
      instant->addResourceFlaw( ResourceFlaw::LevelTooLow );
      result = true;
    }
    if ( instant->getUpperMin()  > m_limitMax ) {
      // definitely have a flaw on upper limit
      instant->addResourceFlaw( ResourceFlaw::LevelTooHigh );
      result = true;
    }

    return(result);
  }

  bool Resource::isValid() const {
    checkError(m_id.isValid(), m_id);
    checkError(m_limitMin <= m_limitMax, m_limitMin << " > " << m_limitMax);
    checkError(m_initialCapacity >= m_limitMin,
	       m_initialCapacity << " > " << m_limitMin);
    checkError(m_initialCapacity <= m_limitMax,
	       m_initialCapacity<< " > " << m_limitMax);
    checkError(m_productionRateMax <= m_productionMax,
	       m_productionRateMax << " > " << m_productionMax );
    checkError(m_productionRateMax >= 0,
	       m_productionRateMax << " > " << m_productionMax );
    checkError(m_consumptionRateMax >= m_consumptionMax,
	       m_consumptionRateMax<< " > " << m_consumptionMax);
    checkError(m_consumptionRateMax <= 0,
	       "Consumption rate is " << m_consumptionRateMax << " but must be negative.");

    for (std::map<int, InstantId>::const_iterator it = m_instants.begin(); it != m_instants.end(); ++it)
      check_error(it->second.isValid());

    return true;
  }

  bool Resource::hasStartOrEndTransaction(const InstantId& instant) const {
    check_error(instant.isValid());
    TransactionSet transactions = instant->getTransactions();
    for (TransactionSet::const_iterator it = transactions.begin(); it != transactions.end(); ++it) {
      TransactionId t = *it;
      if (t->getEarliest() == instant->getTime() || t->getLatest() == instant->getTime())
        return(true);
    }
    return(false);
  }

  /**
   * Get the set of transactions overlapping the given interval which are definitiely
   * assigned to the resource.
   */
  void Resource::getTransactions(std::set<TransactionId>& resultSet,
                                 int lowerBound, int upperBound, bool propagate) {
    check_error(resultSet.empty());
    check_error(lowerBound <= upperBound);

    // Propagate token-object relationshiop and any constraints
    if(propagate)
      getPlanDatabase()->getConstraintEngine()->propagate();

    for(std::set<TokenId>::const_iterator it = m_tokens.begin(); it != m_tokens.end(); ++it){
      TransactionId tx = *it;

      // Skip consideration of this transaction if it is not definitley associated with this resource
      if(!tx->getObject()->lastDomain().isSingleton())
	continue;

      checkError(tx->getObject()->lastDomain().getSingletonValue() == getId(),
		 "If a singleton, and we have synched correctly, then it must be assigned to this resource.");

      IntervalIntDomain bounds(lowerBound, upperBound);
      bounds.intersect(tx->getTime()->lastDomain());
      if(!bounds.isEmpty())
        resultSet.insert(tx);
    }
  }

  std::map<int, InstantId>::iterator Resource::createInstant(int time, const std::map<int, InstantId>::iterator& hint){
    check_error(m_instants.find(time) == m_instants.end());
    InstantId newInstant = (new Instant(time))->getId();

    std::map<int, InstantId>::iterator result =
      m_instants.insert(hint, std::pair<int, InstantId>(time, newInstant));

    // Now insert transactions from prior instant if they will span this one
    if(result == m_instants.begin())
      return result;

    --result;
    InstantId previous = result->second;
    check_error(previous.isValid());

    // If we were able to move backwards
    if(previous != newInstant){
      check_error(previous->getTime() < time);
      const TransactionSet& transactions = previous->getTransactions();
      for(TransactionSet::const_iterator it = transactions.begin(); it != transactions.end(); ++it){
        TransactionId tx = *it;
        if(tx->getLatest() > time)
          newInstant->insert(tx);
      }
    }

    ++result;
    return result;
  }

  void Resource::resetProfile(){
    //EUROPA::cleanup(m_instants);
    std::map<int, InstantId>::const_iterator it = m_instants.begin();
    while(it != m_instants.end()){
      InstantId item = (it++)->second;
      check_error(item.isValid());
      delete (Instant*) item;
    }
    m_instants.clear();

    EUROPA::cleanup(m_globalViolations);
  }

  void Resource::rebuildProfile(){
    // Go over all transactions
    for(std::set<TokenId>::const_iterator it = m_tokens.begin(); it!= m_tokens.end(); ++it){
      TransactionId tx = *it;

      // Skip consideration of this transaction if it is not definitley associated with this resource
      if(!tx->getObject()->lastDomain().isSingleton())
	continue;

      checkError(tx->getObject()->lastDomain().getSingletonValue() == getId(),
		 "If a singleton, and we have synched correctly, then it must be assigned to this resource.");

      // Create/modify instants
      check_error(tx.isValid());
      insert(tx);
    }
  }

  void Resource::addGlobalResourceViolation( ResourceViolation::Type type ) {
    m_globalViolations.insert( (new ResourceViolation(type))->getId() );
  }

  void Resource::setProductionMax( const double& value ) { m_productionMax = value; }
  void Resource::setProductionRateMax( const double& value ) { m_productionRateMax = value; }
  void Resource::setConsumptionMax( const double& value ) { m_consumptionMax = value; }
  void Resource::setConsumptionRateMax( const double& value ) { m_consumptionRateMax = value; }

  void Resource::getLevelAt( int timepoint, IntervalDomain& result ) {
    updateTransactionProfile();

    // Find the instant for the timepoint. If it does not exist, take the last one
    std::map<int, InstantId>::iterator current = m_instants.lower_bound(timepoint);

    // If there are no instants, then the level is a singleton for the initial capacity.
    if(m_instants.empty()){
      result.set(m_initialCapacity);
      return;
    }

    // If it is not a direct hit, and we are at the beginning, return the initial capacity also.
    if(current == m_instants.begin() && current->first > timepoint){
      result.set(m_initialCapacity);
      return;
    }

    // If we are at the end or we have not had a direct hit, move backwards
    if(current == m_instants.end() || current->first > timepoint)
      current--;

    checkError(current->second.isValid(), "Invalid instant retrieved. Indicates a synchronization bug.");

    InstantId instant = current->second;

    result = IntervalDomain(instant->getLowerMin(), instant->getUpperMax());

    debugMsg("Resource:getLevelAt",
	     "Returning " << result.toString() << " for timepoint "
	     << timepoint << " at instant " << instant->getTime());
  }

  void Resource::updateInitialState(const IntervalDomain& value ) {
    check_error( value.isSingleton(),
                 "In the current implementation the initial value of Resource should be a singleton" );
    m_initialCapacity = value.getSingletonValue();
  }

} // namespace
