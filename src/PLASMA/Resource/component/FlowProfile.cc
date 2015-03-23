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

#include "ConstrainedVariable.hh"
#include "ConstraintEngine.hh"
#include "Debug.hh"
#include "Domains.hh"
#include "FlowProfile.hh"
#include "FlowProfileGraph.hh"
#include "Graph.hh"
#include "Instant.hh"
#include "Node.hh"
#include "Transaction.hh"
#include "Profile.hh"
#include "TemporalAdvisor.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Utils.hh"
#include "Variable.hh"

namespace EUROPA {
    //-------------------------------

FlowProfile::FlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector):
    Profile( db, flawDetector),
    m_previousTimeBounds(),
    m_dummySourceTransaction(),
    m_dummySinkTransaction(),
    m_lowerLevelGraph(NULL),
    m_upperLevelGraph(NULL),
    m_lowerClosedLevel(0),
    m_upperClosedLevel(0),
    m_recalculateLowerLevel( false ),
    m_recalculateUpperLevel( false ),
    m_orderings(),
    m_orderedAt(),
    m_lowerLevelContribution(),
    m_upperLevelContribution()
{
  m_recomputeInterval = (new ProfileIterator(getId()))->getId();

  // every node in the maximum flow graph is identified by the id of the associated Transaction
  // we make a dummy transaction for the source and sink nodes of the graphs
  Variable<IntervalIntDomain> * dummy1 =
      new Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0));
  Variable<IntervalIntDomain> * dummy2 =
      new Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0));
  Variable<IntervalIntDomain> * dummy3 =
      new Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0));
  Variable<IntervalIntDomain> * dummy4 =
      new Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0));

  m_dummySourceTransaction = 
      (new Transaction(dummy1->getId(), dummy2->getId(), false, EntityId::noId()))->getId();
  m_dummySinkTransaction = 
      (new Transaction(dummy3->getId(), dummy4->getId(), false, EntityId::noId()))->getId();

  initializeGraphs<FlowProfileGraphImpl>();
}

    FlowProfile::~FlowProfile()
    {
      delete m_lowerLevelGraph;
      m_lowerLevelGraph = 0;

      delete m_upperLevelGraph;
      m_upperLevelGraph = 0;

      delete static_cast<Transaction*>(m_dummySinkTransaction);
      delete static_cast<Transaction*>(m_dummySourceTransaction);
    }

bool FlowProfile::getEarliestLowerLevelInstant(const TransactionId t, InstantId& i) {
  check_error( t.isValid());

  if(t->isConsumer()) {
    i = getInstant(t->time()->lastDomain().getLowerBound());
    return true;
  }
  else {
    std::map<eint, InstantId>::const_iterator end = 
        m_instants.upper_bound(t->time()->lastDomain().getUpperBound());
    for(std::map<eint, InstantId>::const_iterator start = 
            m_instants.lower_bound(t->time()->lastDomain().getLowerBound());
        start != end; ++start) {
      InstantId inst = start->second;
      for(std::set<TransactionId>::const_iterator it = 
              inst->getTransactions().begin();
          it != inst->getTransactions().end(); ++it) {
        if((*it)->isConsumer()) {
          switch(getOrdering(t, *it)) {
            case BEFORE_OR_AT:
            case STRICTLY_AT:
              i = inst;
              return true;
            case AFTER_OR_AT:
            case NOT_ORDERED:
            case UNKNOWN:
              break;
          }
        }
      }
    }
    i = getInstant(t->time()->lastDomain().getUpperBound());
    return true;
  }
}

bool FlowProfile::getEarliestUpperLevelInstant(const TransactionId t, InstantId& i) {
  check_error( t.isValid());

  if(!t->isConsumer()) {
    i = getInstant(t->time()->lastDomain().getLowerBound());
    return true;
  }
  else {
    std::map<eint, InstantId>::const_iterator end = 
        m_instants.upper_bound(t->time()->lastDomain().getUpperBound());
    for(std::map<eint, InstantId>::const_iterator start = 
            m_instants.lower_bound(t->time()->lastDomain().getLowerBound());
        start != end; ++start) {
      InstantId inst = start->second;
      for(std::set<TransactionId>::const_iterator it = 
              inst->getTransactions().begin();
          it != inst->getTransactions().end(); ++it) {
        if(!(*it)->isConsumer()) {
          switch(getOrdering(t, *it)) {
            case BEFORE_OR_AT:
            case STRICTLY_AT:
              i = inst;
              return true;
            case AFTER_OR_AT:
            case NOT_ORDERED:
            case UNKNOWN:
              break;
          }
        }
      }
    }
    i = getInstant(t->time()->lastDomain().getUpperBound());
    return true;
  }
}


void FlowProfile::initRecompute(InstantId inst) {
  check_error(inst.isValid());
  
  debugMsg("FlowProfile:initRecompute","Instant (" << inst->getId() << ") at time "
           << inst->getTime() );
  m_lowerClosedLevel = getInitCapacityLb();
  m_upperClosedLevel = getInitCapacityUb();
  
  for(std::set<TransactionId>::const_iterator it = m_transactions.begin(); 
      it != m_transactions.end(); ++it) {
    if((*it)->time()->lastDomain().getUpperBound() < inst->getTime()) {
      m_upperClosedLevel = m_upperClosedLevel + 
          ((*it)->isConsumer() ? 
           -((*it)->quantity()->lastDomain().getLowerBound()) :
           (*it)->quantity()->lastDomain().getUpperBound());
      
      m_lowerClosedLevel = m_lowerClosedLevel + 
          ((*it)->isConsumer() ? 
           -((*it)->quantity()->lastDomain().getUpperBound()) :
           (*it)->quantity()->lastDomain().getLowerBound());
    }
  }
}

    void FlowProfile::initRecompute()
    {
    	checkError(m_recomputeInterval.isValid(), "Attempted to initialize recomputation without a valid starting point!");

    	debugMsg("FlowProfile:initRecompute","");

    	if( m_recalculateLowerLevel )
    		m_lowerLevelGraph->reset();

    	if( m_recalculateUpperLevel )
    		m_upperLevelGraph->reset();

    	// initial level
    	m_lowerClosedLevel = getInitCapacityLb();
    	m_upperClosedLevel = getInitCapacityUb();
    }

    void FlowProfile::postHandleRecompute(const eint& endTime, const std::pair<edouble,edouble>& endDiff)
    {
    	Profile::postHandleRecompute(endTime,endDiff);
    	m_recalculateLowerLevel = false;
    	m_recalculateUpperLevel = false;
    }

void FlowProfile::recomputeLevels(InstantId prev, InstantId inst) {
  check_error( prev.isValid() || InstantId::noId() == prev );
  check_error( inst.isValid() );

  debugMsg("FlowProfile:recomputeLevels","Computing instant ("
           << inst->getId() << ") at time "
           << inst->getTime() << " closed levels ["
           << m_lowerClosedLevel << ","
           << m_upperClosedLevel << "]");

  const std::set<TransactionId>& transactions = inst->getTransactions();

  std::set<TransactionId>::const_iterator iter = transactions.begin();
  std::set<TransactionId>::const_iterator end = transactions.end();

  for( ; iter != end; ++iter )
  {
    const TransactionId transaction1 = (*iter);

    // inst->getTransactions returns all transaction overlapping inst->getTime
    // right inclusive
    if( transaction1->time()->lastDomain().getUpperBound() == inst->getTime() )
    {
      debugMsg("FlowProfile::recomputeLevels","Transaction ("
               << transaction1->getId() << ") "
               << transaction1->time()->toString() << " "
               << transaction1->quantity()->toString() << " enters closed set.");

      if( m_recalculateLowerLevel )
        m_lowerLevelGraph->removeTransaction( transaction1 );

      if( m_recalculateUpperLevel )
        m_upperLevelGraph->removeTransaction( transaction1 );

      // if upperbound equals the instant time the transaction enters the closed set
      if( transaction1->isConsumer() )
      {
        if( m_recalculateUpperLevel )
          m_upperClosedLevel -= transaction1->quantity()->lastDomain().getLowerBound();

        if( m_recalculateLowerLevel )
          m_lowerClosedLevel -= transaction1->quantity()->lastDomain().getUpperBound();
      }
      else
      {
        if( m_recalculateUpperLevel )
          m_upperClosedLevel += transaction1->quantity()->lastDomain().getUpperBound();

        if( m_recalculateLowerLevel )
          m_lowerClosedLevel += transaction1->quantity()->lastDomain().getLowerBound();
      }
    }
    else
    {
      // if( transaction1->time()->lastDomain().getLowerBound() == inst->getTime() )
      // {
        enableTransaction( transaction1, inst );

        std::set<TransactionId>::const_iterator secondIter = transactions.begin();

        for( ; secondIter != end; ++secondIter )
        {
          const TransactionId transaction2 = (*secondIter);

          if( transaction1 != transaction2  )
          {
            if( transaction2->time()->lastDomain().getUpperBound() != inst->getTime() )
            {
              enableTransaction( transaction2, inst );

              debugMsg("FlowProfile:recomputeLevels",
                       "Determining ordering of pending transaction ("
                       << transaction1->getId() << ") "
                       << transaction1->time()->toString() << 
                       " and pending transaction ("
                       << transaction2->getId() << ") "
                       << transaction2->time()->toString() );

              Order order = getOrdering( transaction1, transaction2 );

              if( STRICTLY_AT == order )
              {
                handleOrderedAt( transaction1, transaction2 );
              }
              else if( BEFORE_OR_AT == order )
              {
                handleOrderedAtOrBefore( transaction1, transaction2 );
              }
              else
              {
                debugMsg("FlowProfile::recomputeLevels","Transaction ("
                         << transaction1->getId() << ") and Transaction ("
                         << transaction2->getId() << ") not constrained");
              }
            }
          }
        }
      // }
    }
  }


  edouble lowerLevel = inst->getLowerLevel();

  if( m_recalculateLowerLevel )
    lowerLevel = m_lowerClosedLevel - m_lowerLevelGraph->getResidualFromSource(m_orderedAt, m_orderings);

  edouble upperLevel = inst->getUpperLevel();

  if( m_recalculateUpperLevel )
    upperLevel = m_upperClosedLevel + m_upperLevelGraph->getResidualFromSource(m_orderedAt, m_orderings);

  debugMsg("FlowProfile::recomputeLevels","Computed levels for instance at time "
           << inst->getTime() << "["
           << lowerLevel << ","
           << upperLevel << "]");

  inst->update( lowerLevel, lowerLevel, upperLevel, upperLevel,
                0, 0, 0, 0,
                0, 0, 0, 0,
                0, 0, 0, 0 );
}

Order FlowProfile::getOrdering( const TransactionId t1, const TransactionId t2 )
    {
      // in case constraint added and already constrained to be before or after we no longer have to
      // recalculate

      check_error(t1.isValid());
      check_error(t2.isValid());

      TransactionIdTransactionIdPair p12 = std::make_pair( t1, t2 );

      TransactionIdTransactionIdPair2Order::const_iterator ite = m_orderedAt.find( p12 );

      if( ite != m_orderedAt.end() )
	{
          debugMsg("FlowProfile:getOrdering", "Cached " << t1 << " = " << t2);
	  return (*ite).second;
	}

      ite = m_orderings.find( p12 );

      if( ite != m_orderings.end() )
	{
          debugMsg("FlowProfile:getOrdering",
                   "Cached " << t1 << " " << ite->second << " " << t2);
	  return (*ite).second;
	}

      TransactionIdTransactionIdPair p21 = std::make_pair( t2, t1 );

      ite = m_orderings.find( p21 );

      if( ite != m_orderings.end() )
	{
          debugMsg("FlowProfile:getOrdering",
                   "Cached " << t2 << " " << ite->second << " " << t1 << ", reversing");
	  if( (*ite).second == BEFORE_OR_AT )
	    return AFTER_OR_AT;

	  if( (*ite).second == AFTER_OR_AT )
	    return BEFORE_OR_AT;

	  return (*ite).second;
	}


      Order returnValue = NOT_ORDERED;

      // if the two transistion belong to a start and end variable of the same
      // token we know the ordering without having to ask for the temporal
      // distance
      if( t1->time()->parent() == t2->time()->parent()
	  &&
	  TokenId::convertable( t1->time()->parent() ) )
	{
	  TokenId token( t1->time()->parent() );

	  if( token->start() == t1->time() )
	    {
	      returnValue = BEFORE_OR_AT;
	    }
	  else
	    {
	      returnValue = AFTER_OR_AT;
	    }
	}
      else
	{
	  const IntervalIntDomain distance = m_planDatabase->getTemporalAdvisor()->getTemporalDistanceDomain( t1->time(), t2->time(), true );
          debugMsg("FlowProfile:getOrdering", "Got a distance of " << distance.toString());
	  if( distance.getLowerBound() == 0 && distance.getUpperBound() == 0 )
	    {
	      returnValue = STRICTLY_AT;
	    }
	  else if( distance.getLowerBound() >= 0 )
	    {
	      returnValue = BEFORE_OR_AT;
	    }
	  else if( distance.getUpperBound() <= 0 )
	    {
	      returnValue = AFTER_OR_AT;
	    }
	}

      if( returnValue == STRICTLY_AT )
	{
	  m_orderedAt[ std::make_pair( t1, t2 ) ] = returnValue;
	}
      else
	{
	  m_orderings[ std::make_pair( t1, t2 ) ] = returnValue;
	}

      return returnValue;
    }

    void FlowProfile::handleOrderedAt( const TransactionId t1, const TransactionId t2 )
    {
      check_error(t1.isValid());
      check_error(t2.isValid());

      debugMsg("FlowProfile:handleOrderedAt","TransactionId ("
	       << t1->getId() << ") at TransactionId ("
	       << t2->getId() << ")");

      if( m_recalculateLowerLevel )
	m_lowerLevelGraph->enableAt( t1, t2 );

      if( m_recalculateUpperLevel )
	m_upperLevelGraph->enableAt( t1, t2 );
    }

    void FlowProfile::handleOrderedAtOrBefore( const TransactionId t1, const TransactionId t2 )
    {
      check_error(t1.isValid());
      check_error(t2.isValid());

      debugMsg("FlowProfile:handleOrderedAtOrBefore","TransactionId ("
	       << t1->getId() << ") at or before TransactionId ("
	       << t2->getId() << ")");

      if( m_recalculateLowerLevel )
	m_lowerLevelGraph->enableAtOrBefore( t1, t2 );

      if( m_recalculateUpperLevel )
	m_upperLevelGraph->enableAtOrBefore( t1, t2 );
    }

void FlowProfile::handleTransactionAdded(const TransactionId t) {
  check_error( t.isValid() );

  debugMsg("FlowProfile:handleTransactionAdded","TransactionId ("
           << t->getId() << ") time "
           << t->time()->lastDomain() << " quantity "
           << t->quantity()->lastDomain() << " consumer: "
           << std::boolalpha << t->isConsumer() );

  m_recalculateLowerLevel = true;
  m_recalculateUpperLevel = true;

  eint startRecalculation = PLUS_INFINITY;
  eint endRecalculation = MINUS_INFINITY;

  if( ProfileIteratorId::noId() != m_recomputeInterval )
  {
    startRecalculation = std::min( m_recomputeInterval->getStartTime(),
                                   static_cast<eint>(t->time()->lastDomain().getLowerBound()));
  }
  else
  {
    startRecalculation = static_cast<eint>(t->time()->lastDomain().getLowerBound());
  }

  // startRecalculation = MINUS_INFINITY;
  endRecalculation = PLUS_INFINITY;

  if( ProfileIteratorId::noId() != m_recomputeInterval )
    delete static_cast<ProfileIterator*>(m_recomputeInterval);

  m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

  m_previousTimeBounds[ t ] =
      std::make_pair(static_cast<eint>(t->time()->lastDomain().getLowerBound()),
                     static_cast<eint>(t->time()->lastDomain().getUpperBound()));

  debugMsg("FlowProfile:handleTransactionAdded","Set interval to [" << startRecalculation << "," << endRecalculation << "]");
}

void FlowProfile::enableTransaction( const TransactionId t, const InstantId inst )
    {
      debugMsg("FlowProfile:enableTransaction","TransactionId (" << t->getId() << ")");

      m_lowerLevelGraph->enableTransaction( t, inst, m_lowerLevelContribution );
      m_upperLevelGraph->enableTransaction( t, inst, m_upperLevelContribution );
    }

void FlowProfile::handleTransactionRemoved( const TransactionId t ) {
  check_error(t.isValid());

  debugMsg("FlowProfile:handleTransactionRemoved","TransactionId (" << t->getId() << ")");

  m_recalculateLowerLevel = true;
  m_recalculateUpperLevel = true;

  m_lowerLevelGraph->removeTransaction( t );
  m_upperLevelGraph->removeTransaction( t );

  m_lowerLevelContribution.erase( t );
  m_upperLevelContribution.erase( t );

  eint startRecalculation = PLUS_INFINITY;
  eint endRecalculation = MINUS_INFINITY;

  if( ProfileIteratorId::noId() != m_recomputeInterval )
  {
    startRecalculation = std::min( m_recomputeInterval->getStartTime(),
                                   static_cast<eint>(t->time()->lastDomain().getLowerBound()));
  }
  else
  {
    startRecalculation = static_cast<eint>(t->time()->lastDomain().getLowerBound());
  }

  endRecalculation = PLUS_INFINITY;

  if(m_recomputeInterval.isValid())
    delete static_cast<ProfileIterator*>(m_recomputeInterval);

  m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

  m_previousTimeBounds.erase( t );
}

void FlowProfile::handleTransactionTimeChanged(const TransactionId t,
                                               const DomainListener::ChangeType& type) {
  check_error(t.isValid());

  m_recalculateLowerLevel = true;
  m_recalculateUpperLevel = true;

  eint startRecalculation = PLUS_INFINITY;
  eint endRecalculation = MINUS_INFINITY;

  switch( type) {
    case DomainListener::UPPER_BOUND_DECREASED:
    case DomainListener::RESTRICT_TO_SINGLETON:
    case DomainListener::SET_TO_SINGLETON:
    case DomainListener::LOWER_BOUND_INCREASED:
    case DomainListener::BOUNDS_RESTRICTED:
      {
        TransactionId2IntIntPair::const_iterator ite = m_previousTimeBounds.find( t );

        // we should have the previous value!
        check_error( ite != m_previousTimeBounds.end() );

        eint previousStart =  (*ite).second.first;
        eint previousEnd =  (*ite).second.second;

        if( ProfileIteratorId::noId() != m_recomputeInterval )
        {
          startRecalculation = std::min( m_recomputeInterval->getStartTime(), previousStart );
          endRecalculation = std::max( m_recomputeInterval->getEndTime(), previousEnd );
        }
        else
        {
          startRecalculation = previousStart;
          endRecalculation = previousEnd;
        }
      }
      break;
    case DomainListener::RESET:
    case DomainListener::RELAXED:
      {
        if( ProfileIteratorId::noId() != m_recomputeInterval ) {
          startRecalculation = std::min(m_recomputeInterval->getStartTime(),
                                        static_cast<eint>(t->time()->lastDomain().getLowerBound()));
          endRecalculation = std::max(m_recomputeInterval->getEndTime(),
                                      static_cast<eint>(t->time()->lastDomain().getUpperBound()));
        }
        else {
          startRecalculation = static_cast<eint>(t->time()->lastDomain().getLowerBound());
          endRecalculation = static_cast<eint>(t->time()->lastDomain().getUpperBound());
        }
      }
      break;
    case DomainListener::REFTIME_CHANGED:
    case DomainListener::VALUE_REMOVED:
    case DomainListener::CLOSED:
    case DomainListener::OPENED:
    case DomainListener::EMPTIED:
    default:
      break;
  };

  if(m_recomputeInterval.isValid())
    delete static_cast<ProfileIterator*>(m_recomputeInterval);

  m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

  debugMsg("FlowProfile:handleTransactionTimeChanged","TransactionId (" << t->getId() << ") change " << type );
}

void FlowProfile::handleTransactionQuantityChanged(const TransactionId t,
                                                   const DomainListener::ChangeType& type) {
  check_error(t.isValid());

  switch( type) {
    case DomainListener::UPPER_BOUND_DECREASED:
      {
        if( t->isConsumer() )
        {
          m_recalculateLowerLevel = true;
        }
        else
        {
          m_recalculateUpperLevel = true;
        }

      }
      break;
    case DomainListener::RESET:
    case DomainListener::RELAXED:
    case DomainListener::RESTRICT_TO_SINGLETON:
    case DomainListener::SET_TO_SINGLETON:
      {
        m_recalculateLowerLevel = true;
        m_recalculateUpperLevel = true;
      }
      break;
    case DomainListener::LOWER_BOUND_INCREASED:
      {
        if( t->isConsumer() )
        {
          m_recalculateUpperLevel = true;
        }
        else
        {
          m_recalculateLowerLevel = true;
        }

      }
      break;
    case DomainListener::BOUNDS_RESTRICTED:
      {
        m_recalculateLowerLevel = true;
        m_recalculateUpperLevel = true;

      }
      break;
    case DomainListener::VALUE_REMOVED:
    case DomainListener::CLOSED:
    case DomainListener::REFTIME_CHANGED:
    case DomainListener::OPENED:
    case DomainListener::EMPTIED:
    default:
      break;
  };

  eint startRecalculation = PLUS_INFINITY;
  eint endRecalculation = MINUS_INFINITY;

  if( ProfileIteratorId::noId() != m_recomputeInterval )
  {
    startRecalculation = std::min(m_recomputeInterval->getStartTime(),
                                  static_cast<eint>(t->time()->lastDomain().getLowerBound()));
    endRecalculation = std::max(m_recomputeInterval->getEndTime(),
                                static_cast<eint>(t->time()->lastDomain().getUpperBound()));
  }
  else
  {
    startRecalculation = static_cast<eint>(t->time()->lastDomain().getLowerBound());
    endRecalculation = static_cast<eint>(t->time()->lastDomain().getUpperBound());
  }

  if(m_recomputeInterval.isValid())
    delete static_cast<ProfileIterator*>(m_recomputeInterval);

  m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

  debugMsg("FlowProfile:handleTransactionQuantityChanged","TransactionId (" << t->getId() << ") change " << type << " to " << t->quantity()->toString() );
}

void FlowProfile::handleTemporalConstraintAdded(const TransactionId predecessor,
                                                const unsigned int ,
                                                const TransactionId successor,
                                                const unsigned int ) {
      debugMsg("FlowProfile:handleTemporalConstraintAdded","TransactionId1 (" << predecessor->getId() << ") before TransactionId2 (" << successor->getId() << ")");

      check_error(predecessor.isValid());
      check_error(successor.isValid());

      m_orderings.clear();

      eint startRecalculation = PLUS_INFINITY;
      eint endRecalculation = MINUS_INFINITY;

      if( ProfileIteratorId::noId() != m_recomputeInterval ) {
        startRecalculation =
            std::min( m_recomputeInterval->getStartTime(),
                      std::min(static_cast<eint>(predecessor->time()->lastDomain().getLowerBound()),
                               static_cast<eint>(successor->time()->lastDomain().getLowerBound())));
        endRecalculation =
            std::max(m_recomputeInterval->getEndTime(),
                     std::max(static_cast<eint>(predecessor->time()->lastDomain().getUpperBound()),
                              static_cast<eint>(successor->time()->lastDomain().getUpperBound())));
      }
      else {
        startRecalculation =
            std::min(static_cast<eint>(predecessor->time()->lastDomain().getLowerBound()),
                     static_cast<eint>(successor->time()->lastDomain().getLowerBound()));
        endRecalculation =
            std::max(static_cast<eint>(predecessor->time()->lastDomain().getUpperBound()),
                     static_cast<eint>(successor->time()->lastDomain().getUpperBound()));
      }

      if(m_recomputeInterval.isValid())
        delete static_cast<ProfileIterator*>(m_recomputeInterval);

      m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;

    }

void FlowProfile::handleTemporalConstraintRemoved(const TransactionId predecessor, 
                                                  const unsigned int ,
                                                  const TransactionId successor,
                                                  const unsigned int ) {
  debugMsg("FlowProfile:handleTemporalConstraintRemoved",
           "TransactionId1 (" << predecessor->getId() << 
           ") before TransactionId2 (" << successor->getId() << ")");

  check_error(predecessor.isValid());
  check_error(successor.isValid());

  m_orderings.clear();
  m_orderedAt.clear();

  eint startRecalculation = PLUS_INFINITY;
  eint endRecalculation = MINUS_INFINITY;

  if( ProfileIteratorId::noId() != m_recomputeInterval ) {
    eint start = m_recomputeInterval->getStartTime();
    eint end = m_recomputeInterval->getEndTime();

    startRecalculation = 
        std::min(start, 
                 std::min(static_cast<eint>(predecessor->time()->lastDomain().getLowerBound()),
                          static_cast<eint>(successor->time()->lastDomain().getLowerBound())));
    endRecalculation = 
        std::max(end,
                 std::max(static_cast<eint>(predecessor->time()->lastDomain().getUpperBound()),
                          static_cast<eint>(successor->time()->lastDomain().getUpperBound())));
  }
  else {
    startRecalculation = 
        std::min(static_cast<eint>(predecessor->time()->lastDomain().getLowerBound()),
                 static_cast<eint>(successor->time()->lastDomain().getLowerBound()));
    endRecalculation = 
        std::max(static_cast<eint>(predecessor->time()->lastDomain().getUpperBound()),
                 static_cast<eint>(successor->time()->lastDomain().getUpperBound()));
  }


  if(m_recomputeInterval.isValid())
    delete static_cast<ProfileIterator*>(m_recomputeInterval);

  m_recomputeInterval = 
      (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

  m_recalculateLowerLevel = true;
  m_recalculateUpperLevel = true;
}
}
