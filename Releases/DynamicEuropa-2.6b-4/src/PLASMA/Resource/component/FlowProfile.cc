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

namespace EUROPA
{
    FlowProfileGraph::FlowProfileGraph( const TransactionId& source, const TransactionId& sink, bool lowerlevel ):
      m_lowerLevel( lowerlevel ),
      m_recalculate( false ),
      m_graph( 0 ),
      m_source( 0 ),
      m_sink( 0 )
    {
      m_graph = new Graph();
      m_source = m_graph->createNode( source );
      m_sink = m_graph->createNode( sink );
      m_maxflow = new MaximumFlowAlgorithm( m_graph, m_source, m_sink );
    }

    FlowProfileGraph::~FlowProfileGraph()
    {
      delete m_maxflow;
      m_maxflow = 0;

      delete m_graph;
      m_graph = 0;

      m_source = 0;
      m_sink = 0;
    }

    void FlowProfileGraph::enableAt( const TransactionId& t1, const TransactionId& t2 )
    {
      debugMsg("FlowProfileGraph:enableAt","Transaction "
	       << t1->time()->toString() << " and transaction "
	       << t2->time()->toString() << " lower level: "
	       << std::boolalpha << m_lowerLevel );


      if( 0 == m_graph->getNode( t1 ) )
	return;

      if( 0 == m_graph->getNode( t2 ) )
	return;

      m_recalculate = true;

      m_graph->createEdge( t1, t2, Edge::getMaxCapacity() );
      m_graph->createEdge( t2, t1, Edge::getMaxCapacity() );
    }

    void FlowProfileGraph::enableAtOrBefore( const TransactionId& t1, const TransactionId& t2 )
    {
      debugMsg("FlowProfileGraph:enableAtOrBefore","Transaction "
	       << t1->time()->toString() << " and transaction "
	       << t2->time()->toString() << " lower level: "
	       << std::boolalpha << m_lowerLevel );


      if( 0 == m_graph->getNode( t1 ) )
	return;

      if( 0 == m_graph->getNode( t2 ) )
	return;

      m_recalculate = true;

      m_graph->createEdge( t1, t2, 0 );
      m_graph->createEdge( t2, t1, Edge::getMaxCapacity() );
    }

    bool FlowProfileGraph::isEnabled(  const TransactionId& transaction ) const
    {
      Node* node = m_graph->getNode( transaction );

      return 0 == node ? false : node->isEnabled();
    }

    void FlowProfileGraph::enableTransaction( const TransactionId& t, const InstantId& i, TransactionId2InstantId& contributions )
    {
      debugMsg("FlowProfileGraph:enableTransaction","Transaction ("
	       << t->getId() << ") "
	       << t->time()->toString() << " lower level: "
	       << std::boolalpha << m_lowerLevel );

      TransactionId source = TransactionId::noId();
      TransactionId target = TransactionId::noId();

      edouble edgeCapacity = 0;

      if( ( m_lowerLevel && t->isConsumer() )
	  ||
	  (!m_lowerLevel && !t->isConsumer() ) )
	{
	  // connect to the source of the graph
	  source = m_source->getIdentity();
	  target = t;

	  edgeCapacity = t->quantity()->lastDomain().getUpperBound();
	}
      else
	{
	  // connect to the sink of the graph
	  source = t;
	  target = m_sink->getIdentity();

	  edgeCapacity = t->quantity()->lastDomain().getLowerBound();
	}

      if( 0 == edgeCapacity )
	{
	  debugMsg("FlowProfileGraph:enableTransaction","Transaction "
		   << t << " starts contributing at "
		   << i->getTime() << " lower level " << std::boolalpha << m_lowerLevel );

	  contributions[ t ] = i;

	  return;
	}

      check_error( TransactionId::noId() != source );
      check_error( TransactionId::noId() != target );

      m_recalculate = true;

      m_graph->createNode( t, true );
      m_graph->createEdge( source, target, edgeCapacity );
      m_graph->createEdge( target, source, 0 );
    }

    void FlowProfileGraph::removeTransaction( const TransactionId& id )
    {
      debugMsg("FlowProfileGraph:removeTransaction","Transaction ("
	       << id->getId() << ") lower level: "
	       << std::boolalpha << m_lowerLevel );

      m_recalculate = true;

      m_graph->removeNode( id );
    }

    void FlowProfileGraph::reset()
    {
      m_recalculate = true;

      m_graph->setDisabled();

      m_sink->setEnabled();
      m_source->setEnabled();
    }


    edouble FlowProfileGraph::getResidualFromSource()
    {
      edouble residual = 0.0;

      if( m_recalculate )
	{
	  m_maxflow->execute();

	  m_recalculate = false;
	}

      EdgeOutIterator ite( *m_source );

      for( ; ite.ok(); ++ite )
	{
	  Edge* edge = *ite;

	  residual += m_maxflow->getResidual( edge );
	}

      return residual;
    }

    void FlowProfileGraph::disable(  const TransactionId& id )
    {
      debugMsg("FlowProfileGraph:disable","Transaction ("
	       << id->getId() << ") lower level: "
	       << std::boolalpha << m_lowerLevel );

      Node* node = m_graph->getNode( id );

      check_error( 0 != node );
      check_error( node->isEnabled() );

      node->setDisabled();
    }

    void FlowProfileGraph::pushFlow( const TransactionId& id )
    {
      Node* node = m_graph->getNode( id );

      check_error( 0 != node );
      check_error( node->isEnabled() );

      if( !m_recalculate )
	{
	  debugMsg("FlowProfileGraph:pushFlow","Transaction ("
		   << id->getId() << ") lower level: "
		   << std::boolalpha << m_lowerLevel );

	  m_maxflow->pushFlowBack( node );
	}
      else
	{
	  debugMsg("FlowProfileGraph:pushFlow","Transaction ("
		   << id->getId() << ") lower level: "
		   << std::boolalpha << m_lowerLevel
		   << " skipping pushing flow back because a recalculation is required.");

	}
    }

    void FlowProfileGraph::restoreFlow()
    {
      m_maxflow->execute( false );
    }



    //-------------------------------

    FlowProfile::FlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector):
      Profile( db, flawDetector),
      m_lowerLevelGraph( 0 ),
      m_upperLevelGraph( 0 ),
      m_recalculateLowerLevel( false ),
      m_recalculateUpperLevel( false )
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

      m_dummySourceTransaction = ( new Transaction(dummy1->getId(), dummy2->getId(), false ) )->getId();
      m_dummySinkTransaction = ( new Transaction(dummy3->getId(), dummy4->getId(), false) )->getId();

      initializeGraphs();
    }

    FlowProfile::~FlowProfile()
    {
      delete m_lowerLevelGraph;
      m_lowerLevelGraph = 0;

      delete m_upperLevelGraph;
      m_upperLevelGraph = 0;

      delete (Transaction*) m_dummySinkTransaction;
      delete (Transaction*) m_dummySourceTransaction;
    }

    bool FlowProfile::getEarliestLowerLevelInstant( const TransactionId& t, InstantId& i )
    {
      check_error( t.isValid());

      TransactionId2InstantId::iterator ite = m_lowerLevelContribution.find( t );

      if( m_lowerLevelContribution.end() == ite )
	return false;

      i = (*ite).second;

      return true;
    }

    bool FlowProfile::getEarliestUpperLevelInstant( const TransactionId& t, InstantId& i )
    {
      check_error( t.isValid());

      TransactionId2InstantId::iterator ite = m_upperLevelContribution.find( t );

      if( m_upperLevelContribution.end() == ite )
	return false;

      i = (*ite).second;

      return true;
    }

    void FlowProfile::initializeGraphs()
    {
      delete m_lowerLevelGraph;

      m_lowerLevelGraph = new FlowProfileGraph( m_dummySourceTransaction, m_dummySinkTransaction, true );

      delete m_upperLevelGraph;

      m_upperLevelGraph = new FlowProfileGraph( m_dummySourceTransaction, m_dummySinkTransaction, false );
    }


    void FlowProfile::initRecompute( InstantId inst )
    {
      check_error(inst.isValid());

      debugMsg("FlowProfile:initRecompute","Instant (" << inst->getId() << ") at time "
	       << inst->getTime() );

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

    void FlowProfile::postHandleRecompute()
    {
      m_recalculateLowerLevel = false;
      m_recalculateUpperLevel = false;
    }

    void FlowProfile::recomputeLevels( InstantId prev, InstantId inst )
    {
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
	  const TransactionId& transaction1 = (*iter);

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
	      if( transaction1->time()->lastDomain().getLowerBound() == inst->getTime() )
		{
		  enableTransaction( transaction1, inst );

		  std::set<TransactionId>::const_iterator secondIter = transactions.begin();

		  for( ; secondIter != end; ++secondIter )
		    {
		      const TransactionId& transaction2 = (*secondIter);

		      if( transaction1 != transaction2  )
			{
			  if( transaction2->time()->lastDomain().getUpperBound() != inst->getTime() )
			    {
			      enableTransaction( transaction2, inst );

			      debugMsg("FlowProfile:recomputeLevels","Determining ordering of pending transaction ("
				       << transaction1->getId() << ") "
				       << transaction1->time()->toString() << " and pending transaction ("
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
		}
	    }
	}


      edouble lowerLevel = inst->getLowerLevel();

      if( m_recalculateLowerLevel )
	lowerLevel = m_lowerClosedLevel - m_lowerLevelGraph->getResidualFromSource();

      edouble upperLevel = inst->getUpperLevel();

      if( m_recalculateUpperLevel )
	upperLevel = m_upperClosedLevel + m_upperLevelGraph->getResidualFromSource();

      debugMsg("FlowProfile::recomputeLevels","Computed levels for instance at time "
	       << inst->getTime() << "["
	       << lowerLevel << ","
	       << upperLevel << "]");

      inst->update( lowerLevel, lowerLevel, upperLevel, upperLevel,
		    0, 0, 0, 0,
		    0, 0, 0, 0,
		    0, 0, 0, 0 );
    }

    FlowProfile::Order FlowProfile::getOrdering( const TransactionId t1, const TransactionId t2 )
    {
      // in case constraint added and already constrained to be before or after we no longer have to
      // recalculate

      check_error(t1.isValid());
      check_error(t2.isValid());

      TransactionIdTransactionIdPair p12 = std::make_pair( t1, t2 );

      TransactionIdTransactionIdPair2Order::const_iterator ite = m_orderedAt.find( p12 );

      if( ite != m_orderedAt.end() )
	{
	  return (*ite).second;
	}

      ite = m_orderings.find( p12 );

      if( ite != m_orderings.end() )
	{
	  return (*ite).second;
	}

      TransactionIdTransactionIdPair p21 = std::make_pair( t2, t1 );

      ite = m_orderings.find( p21 );

      if( ite != m_orderings.end() )
	{
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

    void FlowProfile::handleTransactionAdded(const TransactionId t)
    {
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
	  startRecalculation = std::min( m_recomputeInterval->getStartTime(), (eint) t->time()->lastDomain().getLowerBound() );
	}
      else
	{
	  startRecalculation = (eint) t->time()->lastDomain().getLowerBound();
	}

      // startRecalculation = MINUS_INFINITY;
      endRecalculation = PLUS_INFINITY;

      if( ProfileIteratorId::noId() != m_recomputeInterval )
	delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

      m_previousTimeBounds[ t ] = std::make_pair( (eint) t->time()->lastDomain().getLowerBound() , (eint) t->time()->lastDomain().getUpperBound()  );

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
 	  startRecalculation = std::min( m_recomputeInterval->getStartTime(), (eint) t->time()->lastDomain().getLowerBound() );
 	}
      else
	{
 	  startRecalculation = (eint) t->time()->lastDomain().getLowerBound();
 	}

      endRecalculation = PLUS_INFINITY;

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

      m_previousTimeBounds.erase( t );
    }

    void FlowProfile::handleTransactionTimeChanged(const TransactionId t, const DomainListener::ChangeType& type)
    {
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
	  if( ProfileIteratorId::noId() != m_recomputeInterval )
	    {
	      startRecalculation = std::min( m_recomputeInterval->getStartTime(), (eint) t->time()->lastDomain().getLowerBound() );
	      endRecalculation = std::max( m_recomputeInterval->getEndTime(), (eint) t->time()->lastDomain().getUpperBound() );
	    }
	  else
	    {
	      startRecalculation = (eint) t->time()->lastDomain().getLowerBound();
	      endRecalculation = (eint) t->time()->lastDomain().getUpperBound();
	    }
	}
      break;
      default:
	break;
      };

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

      debugMsg("FlowProfile:handleTransactionTimeChanged","TransactionId (" << t->getId() << ") change " << type );
    }

    void FlowProfile::handleTransactionQuantityChanged(const TransactionId t, const DomainListener::ChangeType& type)
    {
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
      default:
	break;
      };

      eint startRecalculation = PLUS_INFINITY;
      eint endRecalculation = MINUS_INFINITY;

      if( ProfileIteratorId::noId() != m_recomputeInterval )
	{
	  startRecalculation = std::min( m_recomputeInterval->getStartTime(), (eint) t->time()->lastDomain().getLowerBound() );
	  endRecalculation = std::max( m_recomputeInterval->getEndTime(), (eint) t->time()->lastDomain().getUpperBound() );
	}
      else
	{
	  startRecalculation = (eint) t->time()->lastDomain().getLowerBound();
	  endRecalculation = (eint) t->time()->lastDomain().getUpperBound();
	}

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

      debugMsg("FlowProfile:handleTransactionQuantityChanged","TransactionId (" << t->getId() << ") change " << type << " to " << t->quantity()->toString() );
    }

    void FlowProfile::handleTemporalConstraintAdded( const TransactionId predecessor, const int preArgIndex,
						     const TransactionId successor, const int sucArgIndex)
    {
      debugMsg("FlowProfile:handleTemporalConstraintAdded","TransactionId1 (" << predecessor->getId() << ") before TransactionId2 (" << successor->getId() << ")");

      check_error(predecessor.isValid());
      check_error(successor.isValid());

      m_orderings.clear();

      eint startRecalculation = PLUS_INFINITY;
      eint endRecalculation = MINUS_INFINITY;

      if( ProfileIteratorId::noId() != m_recomputeInterval )
	{
	  startRecalculation = std::min( m_recomputeInterval->getStartTime(), std::min( (eint) predecessor->time()->lastDomain().getLowerBound(), (eint) successor->time()->lastDomain().getLowerBound() ) );
	  endRecalculation = std::max( m_recomputeInterval->getEndTime(), std::max( (eint) predecessor->time()->lastDomain().getUpperBound(), (eint) successor->time()->lastDomain().getUpperBound() ) );
	}
      else
	{
	  startRecalculation = std::min( (eint) predecessor->time()->lastDomain().getLowerBound(), (eint) successor->time()->lastDomain().getLowerBound() );
	  endRecalculation = std::max( (eint) predecessor->time()->lastDomain().getUpperBound(), (eint) successor->time()->lastDomain().getUpperBound() );
	}

      if(m_recomputeInterval.isValid())
        delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;

    }

    void FlowProfile::handleTemporalConstraintRemoved( const TransactionId predecessor, const int preArgIndex,
						       const TransactionId successor, const int sucArgIndex)
    {
      debugMsg("FlowProfile:handleTemporalConstraintRemoved","TransactionId1 (" << predecessor->getId() << ") before TransactionId2 (" << successor->getId() << ")");

      check_error(predecessor.isValid());
      check_error(successor.isValid());

      m_orderings.clear();
      m_orderedAt.clear();

      eint startRecalculation = PLUS_INFINITY;
      eint endRecalculation = MINUS_INFINITY;

      if( ProfileIteratorId::noId() != m_recomputeInterval )
	{
	  eint start = m_recomputeInterval->getStartTime();
	  eint end = m_recomputeInterval->getEndTime();

	  startRecalculation = std::min( start, std::min( (eint) predecessor->time()->lastDomain().getLowerBound(), (eint) successor->time()->lastDomain().getLowerBound() ) );
	  endRecalculation = std::max( end, std::max( (eint) predecessor->time()->lastDomain().getUpperBound(), (eint) successor->time()->lastDomain().getUpperBound() ) );
	}
      else
	{
	  startRecalculation = std::min( (eint) predecessor->time()->lastDomain().getLowerBound(), (eint) successor->time()->lastDomain().getLowerBound() );
	  endRecalculation = std::max( (eint) predecessor->time()->lastDomain().getUpperBound(), (eint) successor->time()->lastDomain().getUpperBound() );
	}


      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator( getId(), startRecalculation, endRecalculation ))->getId();

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;
    }
}
