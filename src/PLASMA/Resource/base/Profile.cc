#include "Profile.hh"
#include "Transaction.hh"
#include "Instant.hh"
#include "FVDetector.hh"
#include "ProfilePropagator.hh"
#include "ConstrainedVariable.hh"
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "Debug.hh"

#include <algorithm>
#include <functional>

namespace EUROPA {

    Profile::Profile(const PlanDatabaseId db, const FVDetectorId flawDetector, const edouble initLevelLb, const edouble initLevelUb)
      : m_id(this), m_changeCount(0), m_needsRecompute(false), m_initLevelLb(initLevelLb), m_initLevelUb(initLevelUb),
        m_planDatabase(db), m_detector(flawDetector) {
      m_removalListener = (new ConstraintRemovalListener(db->getConstraintEngine(), m_id))->getId();
    }

    Profile::~Profile() {
      debugMsg("Profile:~Profile", "In profile destructor for " << getId());
      delete (ConstraintEngineListener*) m_removalListener;
      debugMsg("Profile:~Profile", "Cleaning up instants...");
      for(std::map<eint, InstantId>::iterator it = m_instants.begin(); it != m_instants.end(); ++it)
        delete (Instant*) it->second;
      debugMsg("Profile:~Profile", "Cleaning up variable listeners...");
      for(std::multimap<TransactionId, ConstraintId>::iterator it = m_variableListeners.begin();
          it != m_variableListeners.end(); ++it)
        it->second->discard();
      debugMsg("Profile:~Profile", "Cleaning up constraint addition listeners...");
      for(std::map<TransactionId, ConstrainedVariableListenerId>::iterator it = m_otherListeners.begin();
      it != m_otherListeners.end(); ++it)
    	  delete (ConstraintAdditionListener*) (it->second);

      if(m_recomputeInterval.isValid()) {
        debugMsg("Profile:~Profile", "Deleting profile iterator " << m_recomputeInterval->getId() );
        delete (ProfileIterator*) m_recomputeInterval;
      }
      m_id.remove();
    }

    void Profile::addTransaction(const TransactionId t) {
      checkError(m_transactions.find(t) == m_transactions.end(), "Attempted to insert a transaction twice!");
      checkError(m_variableListeners.find(t) == m_variableListeners.end(), "Already have time and/or quantity listeners for this transaction.");
      checkError(m_otherListeners.find(t) == m_otherListeners.end(), "Already a constraint addition listener for this transaction.");
      debugMsg("Profile:addTransaction", "Adding " << (t->isConsumer() ? "consumer " : "producer ") << "transaction " << t << " for time " <<
               t->time()->toString() << " with quantity " << t->quantity()->toString());

      eint startTime = (eint) t->time()->lastDomain().getLowerBound();
      eint endTime = (eint) t->time()->lastDomain().getUpperBound();

      //if instants for the start and end time don't already exist, add them
      addInstantsForBounds(t);

      //add the transaction to the set of instants
      ProfileIterator profIt(m_id, startTime, endTime);
      while(!profIt.done()) {
        debugMsg("Profile:addTransaction", "Adding transaction " << t << " to instant for time " << profIt.getInstant()->getTime());
        profIt.getInstant()->addTransaction(t);
        profIt.next();
      }


      //add listener
      m_variableListeners.insert(std::pair<TransactionId, ConstraintId>(t, (new VariableListener(m_planDatabase->getConstraintEngine(), m_id, t, makeScope(t->time())))->getId()));
      m_variableListeners.insert(std::pair<TransactionId, ConstraintId>(t, (new VariableListener(m_planDatabase->getConstraintEngine(), m_id, t, makeScope(t->quantity()), true))->getId()));
      m_otherListeners.insert(std::pair<TransactionId, ConstrainedVariableListenerId>(t, (new ConstraintAdditionListener(t->time(), t, m_id))->getId()));

      m_transactions.insert(t);
      m_transactionsByTime.insert(std::make_pair(t->time(), t));
      m_changeCount++;
      m_needsRecompute = true;
      handleTransactionAdded(t);
    }

    void Profile::handleTransactionAdded(const TransactionId e) {
      //by default, create an iterator over all time
      if(m_recomputeInterval.isValid())	{
        debugMsg("Profile:addTransaction", "Deleting profile iterator " << m_recomputeInterval->getId() );
        delete (ProfileIterator*) m_recomputeInterval;
      }
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    bool Profile::containsChange(const InstantId instant) {
      return (instant->containsStartOrEnd());
    }

    void Profile::removeTransaction(const TransactionId t) {
      if(m_transactions.find(t) == m_transactions.end())
        return;
//       checkError(m_transactions.find(t) != m_transactions.end(), "Attempted to remove a transaction that isn't present!");

      debugMsg("Profile:removeTransaction", "Removing transaction " << t << " for time " << t->time()->toString() << " with quantity " << t->quantity()->toString());

      eint startTime = (eint) t->time()->lastDomain().getLowerBound();
      eint endTime = (eint) t->time()->lastDomain().getUpperBound();
      ProfileIterator profIt(m_id, startTime, endTime);
      std::vector<eint> emptyInstants;
      //remove the transaction from its instants
      while(!profIt.done()) {
        debugMsg("Profile:removeTransaction", "Removing transaction " << t << " from instant for time " << profIt.getInstant()->getTime());
        profIt.getInstant()->removeTransaction(t);
        //if the instant contains no transactions or doesn't represent the start or end of a transaction, it is empty and should be deleted.
	if(profIt.getInstant()->getTransactions().empty() || !containsChange(profIt.getInstant())) {
          debugMsg("Profile:removeTransaction", "Instant for time " << profIt.getInstant()->getTime() << " is empty.");
          emptyInstants.push_back(profIt.getInstant()->getTime());
        }
        profIt.next();
      }

      //remove any constraints on the transaction from the profile.
      std::vector<std::pair<ConstraintId, int> > removals;
      for(ConstraintSet::const_iterator it = m_temporalConstraints.begin(); it != m_temporalConstraints.end(); ++it) {
        if((*it)->isVariableOf(t->time())) {
          removals.push_back(std::make_pair(*it, 
                                            (int) std::distance((*it)->getScope().begin(), 
                                                                std::find((*it)->getScope().begin(), (*it)->getScope().end(),
                                                                          t->time()))));
        }
      }
      for(std::vector<std::pair<ConstraintId, int> >::const_iterator it = removals.begin(); it != removals.end(); ++it)
        handleConstraintMessage(it->first, t->time(), it->second, false);

      m_transactions.erase(t);
      m_transactionsByTime.erase(t->time());

      //remove the listeners
      checkError(m_variableListeners.find(t) != m_variableListeners.end(),
                 "Bizarre.  No listeners for transaction.");
      for(std::multimap<TransactionId, ConstraintId>::iterator it = m_variableListeners.find(t);
          it != m_variableListeners.end() && it->first == t; ++it) {
        debugMsg("Profile:removeTransaction", "Discarding " << it->second->toString());
        it->second->discard();
      }

      m_variableListeners.erase(t);
      handleTransactionVariableDeletion(t);

      for(std::vector<eint>::const_iterator it = emptyInstants.begin(); it != emptyInstants.end(); ++it) {
        std::map<eint, InstantId>::iterator instIt = m_instants.find(*it);
        //this can't be an error because the discard above constitues a relaxation of the variable, which will get handled in-situ
        //and may remove instants in the emptyInstants vector.
        //checkError(instIt != m_instants.end(), "Computed empty instant at " << *it << " but there is no such instant in the profile.");
        if(instIt == m_instants.end())
          continue;
        InstantId inst = instIt->second;
        debugMsg("Profile:removeTransaction", "Removing instant at time " << inst->getTime());
        condDebugMsg(inst->getTransactions().empty(), "Profile:removeTransaction", "because it has no transactions.");
        condDebugMsg(!containsChange(inst), "Profile:removeTransaction", "because it does not mark a change.");
        m_instants.erase(*it);
        m_detector->notifyDeleted(inst);
	inst->discard();;
      }
      m_changeCount++;
      m_needsRecompute = true;
      handleTransactionRemoved(t);
    }

    void Profile::handleTransactionRemoved(const TransactionId e) {
      //by default, create an iterator over all time
      if(m_recomputeInterval.isValid())
        {
          debugMsg("Profile:handleTransactionRemoved", "Deleting profile iterator " << m_recomputeInterval->getId() );
          delete (ProfileIterator*) m_recomputeInterval;
        }
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    void Profile::transactionTimeChanged(const TransactionId e, const DomainListener::ChangeType& change) {
      checkError(e.isValid(), "Invalid transaction.");
      checkError(m_transactions.find(e) != m_transactions.end(), "Unknown transaction " << e->time()->toString() << " " << e->quantity()->toString());

      eint startTime = (eint) e->time()->lastDomain().getLowerBound();
      eint endTime = (eint) e->time()->lastDomain().getUpperBound();
      switch(change) {
      case DomainListener::UPPER_BOUND_DECREASED:
      case DomainListener::LOWER_BOUND_INCREASED:
      case DomainListener::BOUNDS_RESTRICTED:
      case DomainListener::RESTRICT_TO_SINGLETON:
      case DomainListener::SET_TO_SINGLETON: {
        debugMsg("Profile:handleTimeChanged", "Handling restriction of transaction " << e << " at time " << e->time()->toString() << " with quantity " << e->quantity()->toString());
        eint first, last;
        std::map<eint, InstantId>::iterator it;
        for(it = m_instants.begin(); it != m_instants.end(); ++it)
          if(it->second->getTransactions().find(e) != it->second->getTransactions().end())
            break;
        checkError(it != m_instants.end(), "No instant containing this transaction.");
        first = it->second->getTime();
        it = m_instants.upper_bound(first);
        if(it != m_instants.end())
          for(; it != m_instants.end(); ++it)
            if(it->second->getTransactions().find(e) == it->second->getTransactions().end())
              break;
        --it;
        last = it->second->getTime();
        debugMsg("Profile:handleTimeChanged", "Possibly removing transaction " << e << " from Instants in range [" << first << " " << last << "]");
        ProfileIterator profIt(m_id, first, last);
        std::vector<eint> emptyInstants;
        while(!profIt.done()) {
          InstantId inst = profIt.getInstant();
          if(!e->time()->lastDomain().isMember(inst->getTime())) {
            debugMsg("Profile:handleTimeChanged", "Removing transaction " << e << " from Instant at time " << inst->getTime());
            inst->removeTransaction(e);
            if(inst->getTransactions().empty() || !containsChange(inst))
              emptyInstants.push_back(inst->getTime());
          }
          else
            inst->updateTransaction(e);
          profIt.next();
        }

        //add instants for start and end and add overlapping transactions to them
        addInstantsForBounds(e);

        for(std::vector<eint>::iterator it = emptyInstants.begin(); it != emptyInstants.end(); ++it) {
          std::map<eint, InstantId>::iterator instIt = m_instants.find(*it);
          checkError(instIt != m_instants.end(), "Computed empty instant at time " << *it << " but no such instant exists.");
          InstantId inst = instIt->second;
          debugMsg("Profile:handleTimeChanged", "Removing instant at time " << inst->getTime());
          condDebugMsg(inst->getTransactions().empty(), "Profile:handleTimeChanged", "because it has no transactions.");
          condDebugMsg(!containsChange(inst), "Profile:handleTimeChanged", "because it does not mark a change.");
          m_instants.erase(*it);
          m_detector->notifyDeleted(inst);
          delete (Instant*) inst;
        }
      }
        break;
      case DomainListener::RESET:
      case DomainListener::RELAXED: {
        debugMsg("Profile:handleTimeChanged", "Handling relaxation of transaction " << e << " at time " << e->time()->toString() << " with quantity " << e->quantity()->toString());
        ProfileIterator it(m_id, startTime, endTime);
        addInstantsForBounds(e);
        std::vector<eint> emptyInstants;
        while(!it.done()) {
          InstantId inst = it.getInstant();
          if(!containsChange(inst))
            emptyInstants.push_back(inst->getTime());
          else if(inst->getTransactions().find(e) == inst->getTransactions().end())
            inst->addTransaction(e);
          else
            inst->updateTransaction(e);
          it.next();
        }
        for(std::vector<eint>::iterator it = emptyInstants.begin(); it != emptyInstants.end(); ++it) {
          std::map<eint, InstantId>::iterator instIt = m_instants.find(*it);
          checkError(instIt != m_instants.end(), "Computed empty instant at time " << *it << " but no such instant exists.");
          InstantId inst = instIt->second;
          debugMsg("Profile:handleTimeChanged", "Removing instant at time " << inst->getTime());
          condDebugMsg(inst->getTransactions().empty(), "Profile:handleTimeChanged", "because it has no transactions.");
          condDebugMsg(!containsChange(inst), "Profile:handleTimeChanged", "because it does not mark a change.");
          m_instants.erase(*it);
          m_detector->notifyDeleted(inst);
          delete (Instant*) inst;
        }
      }
        break;
      default:
        break;
      };
      m_changeCount++;
      m_needsRecompute = true;
      handleTransactionTimeChanged(e, change);
    }

    void Profile::handleTransactionTimeChanged(const TransactionId e, const DomainListener::ChangeType& change) {
      //by default, create an iterator over all time
      if(m_recomputeInterval.isValid())	{
        debugMsg("Profile:handleTimeChanged", "Deleting profile iterator " << m_recomputeInterval->getId() );
        delete (ProfileIterator*) m_recomputeInterval;
      }
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    void Profile::transactionQuantityChanged(const TransactionId e, const DomainListener::ChangeType& change) {
      m_changeCount++;
      m_needsRecompute = true;
      handleTransactionQuantityChanged(e, change);
    }

    void Profile::handleTransactionQuantityChanged(const TransactionId e, const DomainListener::ChangeType& change) {
      //by default, create an iterator over all time
      if(m_recomputeInterval.isValid()) {
        debugMsg("Profile:transactionQuantityChanged", "Deleting profile iterator " << m_recomputeInterval->getId() );
        delete (ProfileIterator*) m_recomputeInterval;
      }
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

  bool Profile::checkMessageConsistency() {
    for(ConstraintSet::const_iterator it = m_temporalConstraints.begin(); it != m_temporalConstraints.end(); ++it) {
      ConstraintId constr(*it);
      checkError(constr.isValid(), "Id " << constr << " is invalid.");
      checkError(constr->getScope().size() == 2 || constr->getScope().size() == 3,
                 "Unexpected scope size in " << constr->toLongString());
//       checkError(m_transactionsByTime.find(constr->getScope()[0]) != m_transactionsByTime.end(),
//                  "First argument to temporal constraint isn't a known time-point: " << std::endl << constr->toLongString());
//       checkError(constr->getScope().size() == 3 ||
//                  m_transactionsByTime.find(constr->getScope()[1]) != m_transactionsByTime.end(),
//                  "Second argument to temporal constraint isn't a known time-point: " << std::endl << constr->toLongString());
//       checkError(constr->getScope().size() == 2 ||
//                  m_transactionsByTime.find(constr->getScope()[2]) != m_transactionsByTime.end(),
//                  "Third argument to temporal constraint isn't a known time-point: " << std::endl << constr->toLongString());
    }
    return true;
  }

    void Profile::handleConstraintMessage(const ConstraintId c, const ConstrainedVariableId var, int argIndex, bool addition) {
      check_error(c->getScope().size() == 2 || c->getScope().size() == 3);
      //do nothing if this variable is the distance argument in a temporalDistance constraint
      if(c->getScope().size() == 3 && argIndex == 1)
        return;
      checkError((c->getScope().size() == 2 && (argIndex == 0 || argIndex == 1)) ||
                 (c->getScope().size() == 3 && (argIndex == 0 || argIndex == 2)), "Invalid argument index: " << argIndex);
      checkError(checkMessageConsistency(), "m_constraintsForNotification is inconsistent.");

      debugMsg("Profile:handleConstraintMessage", 
               "Received a " << (addition ? "addition" : "removal") << " message on " << std::endl << c->toLongString());
      //hoisted this common code out of the ifs.  could be put back in for efficiency's sake.
      int otherIndex = -1;
      if(c->getScope().size() == 2) {
        if(argIndex == 0)
          otherIndex = 1;
        else
          otherIndex = 0;
      }
      else if(c->getScope().size() == 3) {
        if(argIndex == 0)
          otherIndex = 2;
        else
            otherIndex = 0;
      }
      
      std::map<ConstrainedVariableId, TransactionId>::iterator transIt = m_transactionsByTime.find(c->getScope()[otherIndex]);
      if(transIt == m_transactionsByTime.end()) {
        debugMsg("Profile:handleConstraintMessage", 
                 "Variable at index " << otherIndex << " doesn't have a transaction in the profile.");
        return;
      }
      TransactionId trans1 = transIt->second;

      transIt = m_transactionsByTime.find(c->getScope()[argIndex]);
      if(transIt == m_transactionsByTime.end()) {
        debugMsg("Profile:handleConstraintMessage", 
                 "Variable at index " << argIndex << " doesn't have a transaction in the profile.");
        return;
      }
      TransactionId trans2 = transIt->second;


      if(addition) {
        if(m_temporalConstraints.find(c) != m_temporalConstraints.end()) {
          debugMsg("Profile:handleConstraintMessage",
                   "Already added constraint " << std::endl << c->toLongString());
          return;
        }
        m_temporalConstraints.insert(c);
        m_changeCount++;
        m_needsRecompute = true;
        if(otherIndex < argIndex)
          handleTemporalConstraintAdded(trans1, otherIndex, trans2, argIndex);
        else
          handleTemporalConstraintAdded(trans2, argIndex, trans1, otherIndex);
      }
      else {
        if(m_temporalConstraints.find(c) == m_temporalConstraints.end()) {
          debugMsg("Profile:handleConstraintMessage",
                   "Already removed constraint " << std::endl << c->toLongString());
          return;
        }
        if(m_temporalConstraints.erase(c) > 0) {
          m_changeCount++;
          m_needsRecompute = true;
          if(otherIndex < argIndex)
            handleTemporalConstraintRemoved(trans1, otherIndex, trans2, argIndex);
          else
            handleTemporalConstraintRemoved(trans2, argIndex, trans1, otherIndex);
        }
      }

      if(m_needsRecompute)
        ((ProfilePropagator*)m_planDatabase->getConstraintEngine()->getPropagatorByName(VariableListener::PROPAGATOR_NAME()))->setUpdateRequired(true);
    }

    void Profile::temporalConstraintAdded(const ConstraintId c, const ConstrainedVariableId var, int argIndex) {
      handleConstraintMessage(c, var, argIndex, true);
    }

    void Profile::handleTemporalConstraintAdded(const TransactionId predecessor, const int preArgIndex,
                                                const TransactionId successor, const int sucArgIndex){
      if(m_recomputeInterval.isValid())
        delete (ProfileIterator*) m_recomputeInterval;
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    void Profile::temporalConstraintRemoved(const ConstraintId c, const ConstrainedVariableId var, int argIndex) {
      handleConstraintMessage(c, var, argIndex, false);
    }

    void Profile::handleTemporalConstraintRemoved(const TransactionId predecessor, const int preArgIndex,
                                                  const TransactionId successor, const int sucArgIndex){
      if(m_recomputeInterval.isValid())
        delete (ProfileIterator*) m_recomputeInterval;
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    /**
      * @brief Remove
      */
     void Profile::handleTransactionVariableDeletion(const TransactionId& t){
         std::map<TransactionId, ConstrainedVariableListenerId>::iterator listIt = m_otherListeners.find(t);
         checkError(listIt != m_otherListeners.end(),
                    "Attempted to remove variable listener for transaction at time " << t->time()->toString() << " with quantity " << t->quantity()->toString() << ".");
         delete (ConstraintAdditionListener*) listIt->second;
         m_otherListeners.erase(t);
     }

    void Profile::getLevel(const eint time, IntervalDomain& dest) {
      if(needsRecompute())
        handleRecompute();
      std::map<eint, InstantId>::iterator it = getGreatestInstant(time);
      IntervalDomain result;

      if(it == m_instants.end())
        result.intersect(m_initLevelLb, m_initLevelUb);
      else {
        InstantId inst = getGreatestInstant(time)->second;
        result.intersect(inst->getLowerLevel(), inst->getUpperLevel());
      }
      dest = result;
    }

    //i should really re-name these.
    std::map<eint, InstantId>::iterator Profile::getGreatestInstant(const eint time) {
      debugMsg("Profile:getGreatestInstant", "Greatest Instant not greater than " << time);

      if(m_instants.empty())
        return m_instants.end();

      std::map<eint, InstantId>::iterator retval = m_instants.lower_bound(time);

      //checkError(retval != m_instants.end(), "No instant with time not greater than " << time);
      if(retval == m_instants.end() || retval->second->getTime() > time)
        --retval;
      checkError(retval != m_instants.end(), "No instant with time not greater than " << time);

      //if we're still greater than the given time, return end
      if(retval->second->getTime() > time)
        return m_instants.end();

      debugMsg("Profile:getGreatestInstant", "Got instant at time " << retval->second->getTime());
      return retval;
    }
    
    std::map<eint, InstantId>::iterator Profile::getLeastInstant(const eint time) {
      debugMsg("Profile:getLeastInstant", "Least Instant not less than " << time);
      if(m_instants.empty())
        return m_instants.end();

      std::map<eint, InstantId>::iterator retval = m_instants.lower_bound(time);
      
      if(retval == m_instants.end())
        --retval;
      //checkError(retval != m_instants.end(), "No instant with time not less than " << time);
      debugMsg("Profile:getLeastInstant", "Got instant at time " << retval->second->getTime());
      return retval;
    }

    bool Profile::isValid() {return true;}

    void Profile::recompute() {
      if(needsRecompute())
        handleRecompute();
    }

    void Profile::handleRecompute() {
      checkError(m_recomputeInterval.isValid(),
                 "Attempted to recompute levels over an invalid interval.");
      condDebugMsg(m_recomputeInterval->done(), "Profile:recompute", "No instants over which to recompute.");
      debugMsg("Profile:handleRecompute","Invoked");
      debugMsg("Profile:recompute:prePrint", std::endl << toString());
      if(!m_recomputeInterval->done()) {
        //bool consistant = m_constraintEngine->propagate();
        //checkError(consistant, "Attempted to recompute a profile with an inconsistent constraint network.");

        InstantId prev = InstantId::noId();

	bool violation = false;

        //if there is no preceding instant, do a clean init
        if(m_recomputeInterval->getInstant()->getTime() == m_instants.begin()->first) {
          initRecompute();
          m_detector->initialize();
        }
        else {
          initRecompute(m_recomputeInterval->getInstant());
          m_detector->initialize(m_recomputeInterval->getInstant());

	  violation = m_detector->detect(m_recomputeInterval->getInstant());

          prev = m_recomputeInterval->getInstant();
          m_recomputeInterval->next();
        }

        while(!m_recomputeInterval->done()
	      &&
	      !violation ) {
          InstantId inst = m_recomputeInterval->getInstant();
          debugMsg("Profile:recompute", "Recomputing levels at instant " << inst->getTime());
          check_error(inst.isValid());
          recomputeLevels( prev, inst);
          prev = inst;
          //stop detecting flaws and violations if the detector says so.
          violation = m_detector->detect(inst);
          m_recomputeInterval->next();
        }
      }
      debugMsg("Profile:recompute:postPrint", std::endl << toString());
      debugMsg("Profile:handleRecompute", "Deleting profile iterator " << m_recomputeInterval->getId() );
      delete (ProfileIterator*) m_recomputeInterval;
      m_recomputeInterval = ProfileIteratorId::noId();
      m_needsRecompute = false;
      postHandleRecompute();
    }

    void Profile::addInstantsForBounds(const TransactionId t) {
      eint first = (eint) t->time()->lastDomain().getLowerBound();
      eint last =  (eint) t->time()->lastDomain().getUpperBound();

      {
        std::map<eint, InstantId>::iterator ite = m_instants.find( first );
	
        if( ite == m_instants.end() ) {
          addInstant(first);
        }
        else {
          InstantId& inst = (*ite).second;
          inst->updateTransaction( t );
        }
      }

      {
        std::map<eint, InstantId>::iterator ite = m_instants.find( last );
        if( ite == m_instants.end() ) {
          addInstant(last);
        }
        else {
          InstantId& inst = (*ite).second;
          inst->updateTransaction( t );
        }
      }
    }

    void Profile::addInstant(const eint time) {
      checkError(m_instants.find(time) == m_instants.end(), "Attempted to add a redundant instant for time " << time);
      debugMsg("Profile:addInstant", "Adding instant for time " << time);
      InstantId inst = (new Instant(time, m_id))->getId();
      m_instants.insert(std::pair<eint, InstantId>(time, inst));

      for(std::set<TransactionId>::const_iterator it = m_transactions.begin(); it != m_transactions.end(); ++it) {
        TransactionId trans = *it;
        check_error(trans.isValid());
        check_error(trans->time().isValid());
        if(trans->time()->lastDomain().isMember(time)) {
          debugMsg("Profile:addInstant", "Adding transaction " << trans << " spanning " << trans->time()->toString() << " to instant at time " << time);
          inst->addTransaction(trans);
        }
      }
    }

    void Profile::removeInstant(const eint time) {
      std::map<eint, InstantId>::iterator pit = m_instants.find(time);
      check_error(pit != m_instants.end());
      if (!containsChange(pit->second)) {
	InstantId inst = pit->second;
	debugMsg("Profile:removeInstant",
		 "Removing instant at time " << inst->getTime()
		 << " because it does not mark a change.");
	m_instants.erase(pit);
	m_detector->notifyDeleted(inst);
	delete (Instant*) inst;
      }
    }

    void Profile::getTransactionsToOrder(const InstantId& inst, std::vector<TransactionId>& results) {
      check_error(inst.isValid());
      check_error(results.empty());
      results.insert(results.end(), inst->getTransactions().begin(), inst->getTransactions().end());
    }

    Profile::VariableListener::VariableListener(const ConstraintEngineId& constraintEngine,
                                                const ProfileId profile,
                                                const TransactionId trans,
                                                const std::vector<ConstrainedVariableId>& scope, const bool isQuantity)
      : Constraint(CONSTRAINT_NAME(), PROPAGATOR_NAME(), constraintEngine, scope), m_profile(profile), m_trans(trans), m_isQuantity(isQuantity) {
      check_error(profile.isValid());
      check_error(trans.isValid());
      debugMsg("Profile:VariableListener", "Created listener for profile " << m_profile << " and transaction " << m_trans << std::endl << toString());
    }

    bool Profile::VariableListener::canIgnore(const ConstrainedVariableId& variable,
                                              int argIndex,
                                              const DomainListener::ChangeType& changeType) {
      check_error(variable.isValid(), toString());
      check_error(m_profile.isValid(), toString());
      check_error(m_trans.isValid(), toString());
      if(m_isQuantity) {
        debugMsg("Profile:VariableListener", "Notifying profile " << m_profile << " of change to quantity variable " << variable->toString());
        m_profile->transactionQuantityChanged(m_trans, changeType);
      }
      else {
        debugMsg("Profile:VariableListener", "Notifying profile " << m_profile << " of change to time variable " << variable->toString());
        m_profile->transactionTimeChanged(m_trans, changeType);
      }
      return false;
    }

    Profile::ConstraintAdditionListener::ConstraintAdditionListener(const ConstrainedVariableId& var, TransactionId tid, ProfileId profile)
      : ConstrainedVariableListener(var), m_profile(profile), m_tid(tid) {}

    Profile::ConstraintAdditionListener::~ConstraintAdditionListener() {
    }

    void Profile::ConstraintAdditionListener::notifyDiscard() {
    	m_profile->handleTransactionVariableDeletion(m_tid);
    }

    void Profile::ConstraintAdditionListener::notifyConstraintAdded(const ConstraintId& constr, int argIndex) {
      static const LabelStr temporal("Temporal");
      if(constr->getPropagator()->getName() == temporal) {
        debugMsg("Profile:ConstraintAdditionListener",
                 getId() << " Notifying profile " << m_profile << " of addition of constraint " << constr->toString() <<
                 " to variable " << m_var->toLongString() << " at index " << argIndex);
        m_profile->temporalConstraintAdded(constr, m_var, argIndex);
      }
    }

    void Profile::ConstraintAdditionListener::notifyConstraintRemoved(const ConstraintId& constr, int argIndex) {
      static const LabelStr temporal("Temporal");
      if(constr->getPropagator()->getName() == temporal) {
        debugMsg("Profile:ConstraintAdditionListener",
                 getId() << " Notifying profile " << m_profile << " of removal of constraint " << constr->toString() <<
                 " from variable " << m_var->toLongString() << " at index " << argIndex);
        m_profile->temporalConstraintRemoved(constr, m_var, argIndex);
      }
    }

    std::string Profile::toString() const {
      std::stringstream sstr;
      sstr << "Profile " << m_id << std::endl;
      for(std::map<eint, InstantId>::const_iterator it = m_instants.begin(); it != m_instants.end(); ++it)
        sstr << it->second->toString() << std::endl;
      return sstr.str();
    }

    ProfileIterator::ProfileIterator(const ProfileId prof, const eint startTime, const eint endTime)
      : m_id(this), m_profile(prof), m_changeCount(prof->m_changeCount) {
      //if(m_profile->m_needsRecompute)
      //m_profile->handleRecompute();
      debugMsg("ProfileIterator:ProfileIterator", "Creating iterator over interval [" << startTime << " " << endTime << "] with change count " <<
               m_changeCount);
      m_realEnd = m_profile->m_instants.end();
      m_start = m_profile->getLeastInstant(startTime);
      m_startTime = (m_start == m_realEnd ? MINUS_INFINITY : m_start->first);
      m_end = m_profile->getGreatestInstant(endTime);
      m_endTime = (m_end == m_realEnd ? PLUS_INFINITY : m_end->first);
      if(m_end != m_realEnd)
        ++m_end;
      debugMsg("ProfileIterator:ProfileIterator", "Actual interval [" << (m_start == m_realEnd ? (2 * MINUS_INFINITY) : m_start->second->getTime()) << " " <<
               (m_end == m_realEnd ? (2 * PLUS_INFINITY) : m_end->second->getTime()) << ")");
    }

    bool ProfileIterator::isStale() const {
      debugMsg("ProfileIterator:isStale", "Checking " << m_changeCount << " ?= " << m_profile->m_changeCount);
      return m_changeCount != m_profile->m_changeCount;
    }

    bool ProfileIterator::done() const {
      //checkError(!isStale(), "Stale profile iterator.");
      debugMsg("ProfileIterator:done", "Checking to see if " << (m_start == m_realEnd ? (2 * MINUS_INFINITY) : m_start->second->getTime()) <<
               " is less than " << (m_end == m_realEnd ? (2 * PLUS_INFINITY) : m_end->second->getTime()));
      return m_start == m_end;
    }

    eint ProfileIterator::getStartTime() const {
      return m_startTime;
    }

    eint ProfileIterator::getTime() const {
      checkError(!isStale(), "Stale profile iterator.");
      checkError(!done(), "Attempted to get time of a done iterator.");
      return m_start->second->getTime();
    }

    eint ProfileIterator::getEndTime() const {
      return m_endTime;
    }

    edouble ProfileIterator::getLowerBound() const {
      checkError(!isStale(), "Stale profile iterator.");
      checkError(!done(), "Attempted to get bound of a done iterator.");
      m_profile->recompute();
      return m_start->second->getLowerLevel();
    }

    edouble ProfileIterator::getUpperBound() const {
      checkError(!isStale(), "Stale profile iterator.");
      checkError(!done(), "Attempted to get bound of a done iterator.");
      m_profile->recompute();
      return m_start->second->getUpperLevel();
    }

    InstantId ProfileIterator::getInstant() const {
      checkError(!isStale(), "Stale profile iterator.");
      checkError(!done(), "Attempted to get Instant of a done iterator.");
      return m_start->second;
    }

    bool ProfileIterator::next() {
      checkError(!isStale(), "Stale profile iterator.");
      checkError(!done(), "Attempted to iterate off the end.");

      debugMsg("ProfileIterator:next", "Iterating from " << m_start->second->getTime() << " ...");
      ++m_start;
      debugMsg("ProfileIterator:next", "... to " << (m_start == m_realEnd ? (2 * PLUS_INFINITY) : m_start->second->getTime()));
      return !done();
    }

  bool Profile::hasConstraint(const ConstraintId& constr) const {
    return m_temporalConstraints.find(constr) != m_temporalConstraints.end();
  }

  Profile::ConstraintRemovalListener::ConstraintRemovalListener(const ConstraintEngineId& ce, ProfileId profile)
    : ConstraintEngineListener(ce), m_profile(profile) {}
  
  void Profile::ConstraintRemovalListener::notifyRemoved(const ConstraintId& constr) {
    if(m_profile->hasConstraint(constr)) {
      int index = 0;
      for(std::vector<ConstrainedVariableId>::const_iterator it = constr->getScope().begin(); it != constr->getScope().end();
          ++it, ++index)
        m_profile->handleConstraintMessage(constr, *it, index, false);
    }
  }
}
