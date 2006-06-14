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
#include "IntervalIntDomain.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_Graph.hh"
#include "SAVH_Instant.hh"
#include "SAVH_MaxFlow.hh"
#include "SAVH_Node.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_Profile.hh"
#include "TemporalAdvisor.hh"
#include "Utils.hh"
#include "Variable.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
    FlowProfileGraph::FlowProfileGraph( const SAVH::TransactionId& source, const SAVH::TransactionId& sink, bool lowerlevel ):
      m_lowerLevel( lowerlevel ),
      m_recalculate( false ),
      m_graph( 0 ),
      m_source( 0 ),
      m_sink( 0 )
    {
      m_graph = new SAVH::Graph();
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

    void FlowProfileGraph::enableAt( const SAVH::TransactionId& t1, const SAVH::TransactionId& t2 ) 
    {
      debugMsg("FlowProfileGraph:enableAt","Transaction (" 
	       << t1->getId() << ") and transaction (" 
	       << t2->getId() << ") lower level: " 
	       << std::boolalpha << m_lowerLevel );

      if( 0 == m_graph->getNode( t1 ) )
	return;

      if( 0 == m_graph->getNode( t2 ) )
	return;


      m_recalculate = true;

      m_graph->createEdge( t1, t2, Edge::getMaxCapacity() );
      m_graph->createEdge( t2, t1, Edge::getMaxCapacity() );
    }

    void FlowProfileGraph::enableAtOrBefore( const SAVH::TransactionId& t1, const SAVH::TransactionId& t2 ) 
    {
      debugMsg("FlowProfileGraph:enableAtOrBefore","Transaction (" 
	       << t1->getId() << ") and transaction (" 
	       << t2->getId() << ") lower level: " 
	       << std::boolalpha << m_lowerLevel );

      if( 0 == m_graph->getNode( t1 ) )
	return;

      if( 0 == m_graph->getNode( t2 ) )
	return;

      m_recalculate = true;

      m_graph->createEdge( t1, t2, 0 );
      m_graph->createEdge( t2, t1, Edge::getMaxCapacity() );
    }

    bool FlowProfileGraph::isEnabled(  const SAVH::TransactionId& transaction ) const 
    { 
      Node* node = m_graph->getNode( transaction ); 
    
      return 0 == node ? false : node->isEnabled(); 
    }

    void FlowProfileGraph::enableTransaction( const SAVH::TransactionId& t )
    {
      debugMsg("FlowProfileGraph:enableTransaction","Transaction (" 
	       << t->getId() << ") lower level: " 
	       << std::boolalpha << m_lowerLevel );
      
      SAVH::TransactionId source = SAVH::TransactionId::noId();
      SAVH::TransactionId target = SAVH::TransactionId::noId();

      double edgeCapacity = 0;

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
	return;
      
      check_error( SAVH::TransactionId::noId() != source );
      check_error( SAVH::TransactionId::noId() != target );

      m_recalculate = true;

      m_graph->createNode( t, true );
      m_graph->createEdge( source, target, edgeCapacity );
      m_graph->createEdge( target, source, 0 );
    }

    void FlowProfileGraph::removeTransaction( const SAVH::TransactionId& id )
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

    double FlowProfileGraph::getResidualFromSource()
    {
      double residual = 0.0;

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

    double FlowProfileGraph::disableReachableResidualGraph()
    {
      debugMsg("FlowProfileGraph:disableReachableResidualGraph","Lower level: "
	       << std::boolalpha << m_lowerLevel );

      double residual = 0.0;

      if( m_recalculate )
	{
	  m_maxflow->execute();
	  
	  Node2Bool visited;

	  visited[ m_source ] = true;

	  visitNeighbors( m_source, residual, visited );
	}

      return residual;
    }

    void FlowProfileGraph::visitNeighbors( const Node* node, double& residual, Node2Bool& visited )
    {
      EdgeOutIterator ite( *node );
      
      for( ; ite.ok(); ++ite )
	{
	  Edge* edge = *ite;
	  
	  Node* target = edge->getTarget();
	  
	  if( false == visited[ target ] )
	    {
	      if( 0 != m_maxflow->getResidual( edge ) )
		{
		  visited[ target ] = true;
		  
		  if( target != m_source && target != m_sink )
		    {
		      debugMsg("FlowProfileGraph:visitNeighbors","Disabling node with transaction ("
			       << target->getIdentity()->getId() << ") lower level " << std::boolalpha << m_lowerLevel );

		      target->setDisabled();

		      const TransactionId& t = target->getIdentity();

		      int sign = t->isConsumer() ? -1 : +1;

		      if( ( m_lowerLevel && t->isConsumer() )
			  ||
			  (!m_lowerLevel && !t->isConsumer() ) )
			{
			  debugMsg("FlowProfileGraph:visitNeighbors","Adding "
				   << sign * t->quantity()->lastDomain().getUpperBound() << " to the level.");

			  residual += sign * t->quantity()->lastDomain().getUpperBound();
			}
		      else
			{
			  debugMsg("FlowProfileGraph:visitNeighbors","Adding "
				   << sign* t->quantity()->lastDomain().getLowerBound() << " to the level.");

			  residual += sign * t->quantity()->lastDomain().getLowerBound();
			}
		      
		      visitNeighbors( target, residual, visited );
		    }
		}
	    }
	}
    }

    void FlowProfileGraph::disable(  const SAVH::TransactionId& id )
    {
      debugMsg("FlowProfileGraph:disable","Transaction (" 
	       << id->getId() << ") lower level: " 
	       << std::boolalpha << m_lowerLevel );

      Node* node = m_graph->getNode( id );

      check_error( 0 != node );
      check_error( node->isEnabled() );
      
      node->setDisabled();
    }

    void FlowProfileGraph::pushFlow( const SAVH::TransactionId& id )
    {
      Node* node = m_graph->getNode( id );

      check_error( 0 != node );
      check_error( node->isEnabled() );

      m_maxflow->pushFlowBack( node );
    }

    void FlowProfileGraph::restoreFlow()
    {
      m_maxflow->execute( false );
    }



    //-------------------------------

    FlowProfile::FlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector, const double initLevelLb, const double initLevelUb): 
      Profile( db, flawDetector, initLevelLb, initLevelUb),
      m_lowerLevelGraph( 0 ),
      m_upperLevelGraph( 0 ),
      m_recalculateLowerLevel( false ),
      m_recalculateUpperLevel( false ),
      m_startRecalculation( PLUS_INFINITY ),
      m_endRecalculation( MINUS_INFINITY )
    {
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();

      // every node in the maximum flow graph is identified by the id of the associated Transaction
      // we make a dummy transaction for the source nodes of the graphs
      m_dummySourceTransaction = ( new SAVH::Transaction( Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0) ).getId(),
							  Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0) ).getId(),
							  false ) )->getId();

      // every node in the maximum flow graph is identified by the id of the associated Transaction
      // we make a dummy transaction for the sink nodes of the graphs
      m_dummySinkTransaction = ( new SAVH::Transaction(  Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0) ).getId(),
							 Variable<IntervalIntDomain>( db->getConstraintEngine(), IntervalIntDomain(0, 0) ).getId(),
							 false ) )->getId();

      m_lowerLevelGraph = new FlowProfileGraph( m_dummySourceTransaction, m_dummySinkTransaction, true );
      m_upperLevelGraph = new FlowProfileGraph( m_dummySourceTransaction, m_dummySinkTransaction, false );
    }

    FlowProfile::~FlowProfile() 
    {
      delete m_lowerLevelGraph;
      m_lowerLevelGraph = 0;

      delete m_upperLevelGraph;
      m_upperLevelGraph = 0;

      delete (SAVH::Transaction*) m_dummySinkTransaction;
      delete (SAVH::Transaction*) m_dummySourceTransaction;
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
      m_lowerClosedLevel = m_initLevelLb;

      // initial level
      m_upperClosedLevel = m_initLevelUb;
    }

    void FlowProfile::postHandleRecompute()
    {
      m_recalculateLowerLevel = false;
      m_recalculateUpperLevel = false;

      m_startRecalculation = PLUS_INFINITY;
      m_endRecalculation = MINUS_INFINITY; 
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
		  enableTransaction( transaction1 );
  
		  std::set<TransactionId>::const_iterator secondIter = transactions.begin();
		  
		  for( ; secondIter != end; ++secondIter ) 
		    {
		      const TransactionId& transaction2 = (*secondIter);
		      
		      if( transaction1 != transaction2  ) 
			{
			  if( transaction2->time()->lastDomain().getUpperBound() != inst->getTime() )
			    {
			      enableTransaction( transaction2 );

			      debugMsg("FlowProfile:recomputeLevels","Determining ordering of pending transaction (" 
				       << transaction1->getId() << ") "
				       << transaction1->time()->toString() << " and pending transaction ("
				       << transaction2->getId() << ") "
				       << transaction2->time()->toString() );
			      
			      if( isConstrainedToAt( transaction1, transaction2 ) ) 
				{
				  handleOrderedAt( transaction1, transaction2 );
				}
			      else if( isConstrainedToBeforeOrAt( transaction1, transaction2 ) )  
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


      double lowerLevel = inst->getLowerLevel();

      if( m_recalculateLowerLevel )
	lowerLevel = m_lowerClosedLevel - m_lowerLevelGraph->getResidualFromSource();
	
      double upperLevel = inst->getUpperLevel();

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

    bool FlowProfile::isConstrainedToBeforeOrAt( const TransactionId t1, const TransactionId t2 ) 
    {
      check_error(t1.isValid());
      check_error(t2.isValid());

      const IntervalIntDomain distance = m_planDatabase->getTemporalAdvisor()->getTemporalDistanceDomain( t1->time(), t2->time(), true );
      
      return distance.getLowerBound() >= 0;
    }

    bool FlowProfile::isConstrainedToAt( const TransactionId t1, const TransactionId t2 ) 
    {

      const IntervalIntDomain distance = m_planDatabase->getTemporalAdvisor()->getTemporalDistanceDomain( t1->time(), t2->time(), true );

      return distance.getLowerBound() == 0 && distance.getUpperBound() == 0;
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
      check_error(t.isValid());

      debugMsg("FlowProfile:handleTransactionAdded","TransactionId (" 
	       << t->getId() << ") time " 
	       << t->time()->lastDomain() << " quantity " 
	       << t->quantity()->lastDomain() << " consumer: "
	       << std::boolalpha << t->isConsumer() );

      //enableTransaction( t );

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;

      m_startRecalculation = MINUS_INFINITY; //std::min( m_startRecalculation, (int) t->time()->lastDomain().getLowerBound() );
      m_endRecalculation = PLUS_INFINITY;

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      
      m_recomputeInterval = (new ProfileIterator( getId(), m_startRecalculation, m_endRecalculation ))->getId();
    }

    void FlowProfile::enableTransaction( const TransactionId t )
    {
      debugMsg("FlowProfile:enableTransaction","TransactionId (" << t->getId() << ")");
      
      m_lowerLevelGraph->enableTransaction( t );
      m_upperLevelGraph->enableTransaction( t );
    }
    
    void FlowProfile::handleTransactionRemoved( const TransactionId t ) {
      check_error(t.isValid());

      debugMsg("FlowProfile:handleTransactionRemoved","TransactionId (" << t->getId() << ")");

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;
      
      m_lowerLevelGraph->removeTransaction( t );
      m_upperLevelGraph->removeTransaction( t );
      
      // done if recompute interval has no transactions left if this transaction is removed
      m_startRecalculation = MINUS_INFINITY; //std::min( m_startRecalculation, (int) t->time()->lastDomain().getLowerBound() );
      m_endRecalculation = PLUS_INFINITY;
      
      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      
      m_recomputeInterval = (new ProfileIterator( getId(), m_startRecalculation, m_endRecalculation ))->getId();
    }

    void FlowProfile::handleTransactionTimeChanged(const TransactionId t, const DomainListener::ChangeType& type)  
    {
      check_error(t.isValid());

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;

      // TODO: using the base domain to determine the start of the interval to recalculate is too 
      // conservative, we can do better only if we knew the value before the change
      m_startRecalculation = MINUS_INFINITY; //std::min( m_startRecalculation, (int) t->time()->baseDomain().getLowerBound() );
      m_endRecalculation = PLUS_INFINITY;

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      
      m_recomputeInterval = (new ProfileIterator( getId(), m_startRecalculation, m_endRecalculation ))->getId();

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

      m_startRecalculation = MINUS_INFINITY; //std::min( m_startRecalculation, (int) t->time()->lastDomain().getLowerBound() );
      m_endRecalculation = PLUS_INFINITY; //std::max( m_endRecalculation, (int) t->time()->lastDomain().getUpperBound() );

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      
      m_recomputeInterval = (new ProfileIterator( getId(), m_startRecalculation, m_endRecalculation ))->getId();

      debugMsg("FlowProfile:handleTransactionQuantityChanged","TransactionId (" << t->getId() << ") change " << type );
    }

    void FlowProfile::handleTemporalConstraintAdded( const TransactionId predecessor, const int preArgIndex,
						     const TransactionId successor, const int sucArgIndex)
    {
      debugMsg("FlowProfile:handleTemporalConstraintAdded","TransactionId1 (" << predecessor->getId() << ") before TransactionId2 (" << successor->getId() << ")");

      check_error(predecessor.isValid());
      check_error(successor.isValid());

      m_startRecalculation = MINUS_INFINITY; 
      m_endRecalculation = PLUS_INFINITY; 

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      
      m_recomputeInterval = (new ProfileIterator( getId(), m_startRecalculation, m_endRecalculation ))->getId();

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;

    }
      
    void FlowProfile::handleTemporalConstraintRemoved( const TransactionId predecessor, const int preArgIndex,
						       const TransactionId successor, const int sucArgIndex)
    {
      debugMsg("FlowProfile:handleTemporalConstraintRemoved","TransactionId1 (" << predecessor->getId() << ") before TransactionId2 (" << successor->getId() << ")");

      check_error(predecessor.isValid());
      check_error(successor.isValid());

      m_startRecalculation = MINUS_INFINITY; 
      m_endRecalculation = PLUS_INFINITY; 

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      
      m_recomputeInterval = (new ProfileIterator( getId(), m_startRecalculation, m_endRecalculation ))->getId();

      m_recalculateLowerLevel = true;
      m_recalculateUpperLevel = true;
    }

  }
}
