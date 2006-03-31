#include "SAVH_TimetableProfile.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {
  namespace SAVH {
    TimetableProfile::TimetableProfile(const ConstraintEngineId ce, const FVDetectorId flawDetector) 
      : Profile(ce, flawDetector), m_lowerLevelMin(0), m_lowerLevelMax(0), m_upperLevelMin(0), m_upperLevelMax(0),
	m_minCumulativeConsumption(0), m_maxCumulativeConsumption(0), m_minCumulativeProduction(0), m_maxCumulativeProduction(0) {}

    void TimetableProfile::initRecompute(InstantId inst) {
      checkError(m_recomputeInterval.isValid(), "Attempted to initialize recomputation without a valid starting point!");
      m_lowerLevelMin = inst->getLowerLevel();
      m_lowerLevelMax = inst->getLowerLevelMax();
      m_upperLevelMin = inst->getUpperLevelMin();
      m_upperLevelMax = inst->getUpperLevel();
      m_minCumulativeConsumption = inst->getMinCumulativeConsumption();
      m_maxCumulativeConsumption = inst->getMaxCumulativeConsumption();
      m_minCumulativeProduction = inst->getMinCumulativeProduction();
      m_maxCumulativeProduction = inst->getMaxCumulativeProduction();
    }

    void TimetableProfile::initRecompute() {
      m_lowerLevelMin = 0;
      m_lowerLevelMax = 0;
      m_upperLevelMin = 0;
      m_upperLevelMax = 0;
      m_minCumulativeConsumption = 0;
      m_maxCumulativeConsumption = 0;
      m_minCumulativeProduction = 0;
      m_maxCumulativeProduction = 0;
    }

    void TimetableProfile::recomputeLevels(InstantId inst) {
      check_error(inst.isValid());
      
      double maxInstantProduction(0), minInstantProduction(0), maxInstantConsumption(0), minInstantConsumption(0);
      
      const std::set<TransactionId>& transactions(inst->getTransactions());
      for(std::set<TransactionId>::const_iterator it = transactions.begin(); it != transactions.end(); ++it) {
	TransactionId trans = *it;
	double lb, ub;
	trans->quantity()->lastDomain().getBounds(lb, ub);
	bool isConsumer = trans->isConsumer();

	//the minInstant values are 0 unless there is a transaction that cannot happen before or after this instant, so we have to add those
	if(trans->time()->lastDomain().isSingleton()) {
	  if(isConsumer)
	    minInstantConsumption += lb;
	  else
	    minInstantProduction += lb;
	}

	//of course, in the upper case we consume and produce the most possible at this instant
	if(isConsumer)
	  maxInstantConsumption += ub;
	else
	  maxInstantProduction += ub;
	
	//if the transaction just started, add producer to upper bounds and consumer to lower bounds
	if(trans->time()->lastDomain().getLowerBound() == inst->getTime()) {
	  if(isConsumer) {
	    m_lowerLevelMin -= ub;
	    m_lowerLevelMax -= lb;
	    m_maxCumulativeConsumption += ub;
	  }
	  else {
	    m_upperLevelMin += lb;
	    m_upperLevelMax += ub;
	    m_maxCumulativeProduction += ub;
	  }
	}
	//if the transaction just ended, add producer to lower bounds and consumer to upper bounds
	if(trans->time()->lastDomain().getUpperBound() == inst->getTime()) {
	  if(isConsumer) {
	    m_upperLevelMax -= lb;
	    m_upperLevelMin -= ub;
	    if(!trans->time()->lastDomain().isSingleton())
	      m_maxCumulativeConsumption += ub;
	    m_minCumulativeConsumption += lb;
	  }
	  else {
	    m_lowerLevelMin += lb;
	    m_lowerLevelMax += ub;
	    if(!trans->time()->lastDomain().isSingleton())
	      m_maxCumulativeProduction += ub;
	    m_minCumulativeProduction += lb;
	  }
	}
      }
      inst->update(m_lowerLevelMin, m_lowerLevelMax, m_upperLevelMin, m_upperLevelMax,
		   minInstantConsumption, maxInstantConsumption, minInstantProduction, maxInstantProduction,
		   m_minCumulativeConsumption, m_maxCumulativeConsumption, m_minCumulativeProduction, m_maxCumulativeProduction);
    }
    
    //for the moment, these always recompute over the entire interval.
    void TimetableProfile::handleTransactionAdded(const TransactionId t) {
      Profile::handleTransactionAdded(t);
      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    void TimetableProfile::handleTransactionRemoved(const TransactionId t) {
      Profile::handleTransactionRemoved(t);
      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }
    
    void TimetableProfile::handleTransactionTimeChanged(const TransactionId t, const DomainListener::ChangeType& type) {
      Profile::handleTransactionTimeChanged(t, type);
	if(m_recomputeInterval.isValid())
	  delete (ProfileIterator*) m_recomputeInterval;
	m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    void TimetableProfile::handleTransactionQuantityChanged(const TransactionId t, const DomainListener::ChangeType& type) {
      Profile::handleTransactionQuantityChanged(t, type);
      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    //do nothing, since we don't take those types of changes into account
    void TimetableProfile::handleTransactionsOrdered(const TransactionId t1, const TransactionId t2) {
    }
  }
}
