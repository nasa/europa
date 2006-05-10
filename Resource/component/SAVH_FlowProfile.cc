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

    FlowProfile::FlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector, const double initLevelLb, const double initLevelUb): 
      Profile( db, flawDetector, initLevelLb, initLevelUb),
      m_planDatabase( db ),
      m_lowerLevelGraph( 0 ),
      m_lowerLevelSource( 0 ),
      m_lowerLevelSink( 0 ),
      m_upperLevelGraph( 0 ),
      m_upperLevelSource( 0 ),
      m_upperLevelSink( 0 ) 
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

      m_lowerLevelGraph = new SAVH::Graph();
      m_lowerLevelSource = m_lowerLevelGraph->createNode( m_dummySourceTransaction );
      m_lowerLevelSink = m_lowerLevelGraph->createNode( m_dummySinkTransaction );

      m_upperLevelGraph = new SAVH::Graph();
      m_upperLevelSource = m_upperLevelGraph->createNode( m_dummySourceTransaction );
      m_upperLevelSink = m_upperLevelGraph->createNode( m_dummySinkTransaction );
    }

    FlowProfile::~FlowProfile() {

      delete m_lowerLevelGraph;
      m_lowerLevelGraph = 0;

      // nodes are deleted by the graph
      m_lowerLevelSource  = 0;
      m_lowerLevelSink = 0;

      delete m_upperLevelGraph;
      m_upperLevelGraph = 0;

      // nodes are deleted by the graph
      m_upperLevelSource = 0;
      m_upperLevelSink = 0;

      delete (SAVH::Transaction*) m_dummySinkTransaction;
      delete (SAVH::Transaction*) m_dummySourceTransaction;
    }

    void FlowProfile::initRecompute(InstantId inst) {
      check_error(inst.isValid());

      debugMsg("FlowProfile:initRecompute","Instant (" << inst->getId() << ")");
    }

    void FlowProfile::initRecompute() {
      checkError(m_recomputeInterval.isValid(), "Attempted to initialize recomputation without a valid starting point!");
      
      debugMsg("FlowProfile:initRecompute","");
      
      // initial level
      m_lowerClosedLevel = m_initLevelLb;

      // initial level
      m_upperClosedLevel = m_initLevelUb;
    }

    void FlowProfile::recomputeLevels(InstantId inst) {
      check_error(inst.isValid());

      debugMsg("FlowProfile:recomputeLevels","Instant (" 
	       << inst->getId() << ") at time "
	       << inst->getTime() );

      m_lowerLevelGraph->setDisabled();
      
      m_lowerLevelSink->setEnabled();
      m_lowerLevelSource->setEnabled();
      
      m_upperLevelGraph->setDisabled();
      
      m_upperLevelSink->setEnabled();
      m_upperLevelSource->setEnabled();

      const std::set<TransactionId>& transactions = inst->getTransactions();

      std::set<TransactionId>::const_iterator iter = transactions.begin();
      std::set<TransactionId>::const_iterator end = transactions.end();

      for( ; iter != end; ++iter ){
	const TransactionId& transaction1 = (*iter);
	
	if( transaction1->time()->lastDomain().getUpperBound() != inst->getTime() )  {
	  enableTransaction( transaction1 );
	  
	  check_error( transaction1.isValid() );
	  
	  std::set<TransactionId>::const_iterator secondIter = transactions.begin();
	  
	  for( ; secondIter != end; ++secondIter ) {
	    const TransactionId& transaction2 = (*secondIter);
	    
	    if( transaction2->time()->lastDomain().getUpperBound() != inst->getTime() )
	      {
		check_error( transaction2.isValid() );
		
		if( transaction1 != transaction2 ) {
		  debugMsg("FlowProfile:recomputeLevels","Transaction (" 
			   << transaction1->getId() << ") "
			   << transaction1->time()->toString() << " and Transaction ("
			   << transaction2->getId() << ") "
			   << transaction2->time()->toString() );
		  
		  if( isConstrainedToAt( transaction1, transaction2 ) ) {
		    handleOrderedAt( transaction1, transaction2 );
		  }
		  else if( isConstrainedToBeforeOrAt( transaction1, transaction2 ) )  {
		    handleOrderedAtOrBefore( transaction1, transaction2 );
		  }
		  else {
		    debugMsg("FlowProfile::recomputeLevels","Transaction (" 
			     << transaction1->getId() << ") and Transaction ("
			     << transaction2->getId() << ") not constrained");
		  }
		}
	      }
	  }
	}
	else {
	  check_error( transaction1->time()->lastDomain().getUpperBound() == inst->getTime() );
	  
	  if( transaction1->isConsumer() ) {
	    m_upperClosedLevel -= transaction1->quantity()->lastDomain().getLowerBound();
	    m_lowerClosedLevel -= transaction1->quantity()->lastDomain().getUpperBound();
	  }
	  else {
	    m_upperClosedLevel += transaction1->quantity()->lastDomain().getUpperBound();
	    m_lowerClosedLevel += transaction1->quantity()->lastDomain().getLowerBound();
	  }
	}
      }

      double lowerboundIncrement = m_lowerClosedLevel;

      {
	debugMsg("FlowProfile::recomputeLevels","Computing lower level for instance at time "
		 << inst->getTime() << " closed level is "
		 << m_lowerClosedLevel );

	MaximumFlowAlgorithm maxflow( m_lowerLevelGraph, m_lowerLevelSource, m_lowerLevelSink );
	
	maxflow.execute();

	EdgeOutIterator ite( *m_lowerLevelSource );

	for( ; ite.ok(); ++ite )
	  {
	    Edge* edge = *ite;
	    
	    lowerboundIncrement -= maxflow.getResidual( edge );
	  }
      }

      double upperboundIncrement = m_upperClosedLevel;

      {
	debugMsg("FlowProfile::recomputeLevels","Computing upper level for instance at time "
		 << inst->getTime() << " base level is "
		 << m_upperClosedLevel );

	MaximumFlowAlgorithm maxflow( m_upperLevelGraph, m_upperLevelSource, m_upperLevelSink );
	
	maxflow.execute();


	EdgeOutIterator ite( *m_upperLevelSource );

	for( ; ite.ok(); ++ite )
	  {
	    Edge* edge = *ite;
	    
	    upperboundIncrement += maxflow.getResidual( edge );
	  }
      }

	debugMsg("FlowProfile::recomputeLevels","Computed levels for instance at time "
		 << inst->getTime() << "["
		 << lowerboundIncrement << "," 
		 << upperboundIncrement << "]");

      inst->update( lowerboundIncrement, lowerboundIncrement, upperboundIncrement, upperboundIncrement, 
		    0, 0, 0, 0, 
		    0, 0, 0, 0, 
		    0, 0, 0, 0 );
    }

    bool FlowProfile::isConstrainedToBeforeOrAt( const TransactionId t1, const TransactionId t2 ) {
      check_error(t1.isValid());
      check_error(t2.isValid());

      const IntervalIntDomain distance = m_planDatabase->getTemporalAdvisor()->getTemporalDistanceDomain( t1->time(), t2->time(), true );
      
      return distance.getLowerBound() >= 0;
    }

    bool FlowProfile::isConstrainedToAt( const TransactionId t1, const TransactionId t2 ) {

      const IntervalIntDomain distance = m_planDatabase->getTemporalAdvisor()->getTemporalDistanceDomain( t1->time(), t2->time(), true );

      return distance.getLowerBound() == 0 && distance.getUpperBound() == 0;
    }

    void FlowProfile::handleOrderedAt( const TransactionId t1, const TransactionId t2 ) {
      check_error(t1.isValid());
      check_error(t2.isValid());
      
      debugMsg("FlowProfile:handleOrderedAt","TransactionId (" 
	       << t1->getId() << ") at TransactionId (" 
	       << t2->getId() << ")");

      check_error( 0 != m_lowerLevelGraph->getNode( t1 ) );
      check_error( 0 != m_lowerLevelGraph->getNode( t2 ) );

      m_lowerLevelGraph->createEdge( t1, t2, Edge::getMaxCapacity() );
      m_lowerLevelGraph->createEdge( t2, t1, Edge::getMaxCapacity() );

      check_error( 0 != m_upperLevelGraph->getNode( t1 ) );
      check_error( 0 != m_upperLevelGraph->getNode( t2 ) );

      m_upperLevelGraph->createEdge( t1, t2, Edge::getMaxCapacity() );
      m_upperLevelGraph->createEdge( t2, t1, Edge::getMaxCapacity() );
    }

    void FlowProfile::handleOrderedAtOrBefore( const TransactionId t1, const TransactionId t2 ) {
      check_error(t1.isValid());
      check_error(t2.isValid());
      
      debugMsg("FlowProfile:handleOrderedAtOrBefore","TransactionId (" 
	       << t1->getId() << ") at or before TransactionId (" 
	       << t2->getId() << ")");

      check_error( 0 != m_lowerLevelGraph->getNode( t1 ) );
      check_error( 0 != m_lowerLevelGraph->getNode( t2 ) );

      m_lowerLevelGraph->createEdge( t1, t2, 0 );
      m_lowerLevelGraph->createEdge( t2, t1, Edge::getMaxCapacity() );

      check_error( 0 != m_upperLevelGraph->getNode( t1 ) );
      check_error( 0 != m_upperLevelGraph->getNode( t2 ) );

      m_upperLevelGraph->createEdge( t1, t2, 0 );
      m_upperLevelGraph->createEdge( t2, t1, Edge::getMaxCapacity() );
    }

    void FlowProfile::handleTransactionAdded(const TransactionId t) {
      check_error(t.isValid());

      debugMsg("FlowProfile:handleTransactionAdded","TransactionId (" << t->getId() << ") " << t->time() );

      enableTransaction( t );

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    void FlowProfile::enableTransaction( const TransactionId t )
    {
      debugMsg("FlowProfile:enableTransaction","TransactionId (" << t->getId() << ")");

      SAVH::Node* lowerLevelNode = m_lowerLevelGraph->createNode( t, true );
      SAVH::Node* upperLevelNode = m_upperLevelGraph->createNode( t, true );

      if( t->isConsumer() ) 
	{
	  m_lowerLevelGraph->createEdge( m_lowerLevelSource->getIdentity(), lowerLevelNode->getIdentity(), t->quantity()->lastDomain().getUpperBound() ); 
	  m_lowerLevelGraph->createEdge( lowerLevelNode->getIdentity(), m_lowerLevelSource->getIdentity(), 0 ); 
	  
	  m_upperLevelGraph->createEdge( upperLevelNode->getIdentity(), m_upperLevelSink->getIdentity(), t->quantity()->lastDomain().getUpperBound() );
	  m_upperLevelGraph->createEdge( m_upperLevelSink->getIdentity(), upperLevelNode->getIdentity(), 0 );
	}
      else 
	{
	  m_lowerLevelGraph->createEdge( lowerLevelNode->getIdentity(), m_lowerLevelSink->getIdentity(), t->quantity()->lastDomain().getUpperBound() ); 
	  m_lowerLevelGraph->createEdge( m_lowerLevelSink->getIdentity(), lowerLevelNode->getIdentity(), 0 ); 
	  
	  m_upperLevelGraph->createEdge( m_upperLevelSource->getIdentity(), upperLevelNode->getIdentity(), t->quantity()->lastDomain().getUpperBound() );
	  m_upperLevelGraph->createEdge( upperLevelNode->getIdentity(), m_upperLevelSource->getIdentity(), 0 );
	}
    }
    
    void FlowProfile::resetEdgeWeights( const TransactionId t ) 
    {
      check_error(t.isValid());
      checkError( 0 != m_lowerLevelGraph->getNode( t ), "resetEdgeWeights, no node in lower level graph for transaction (" 
		       << t->getId() << ")");
      checkError( 0 != m_upperLevelGraph->getNode( t ), "resetEdgeWeights, no node in upper level graph for transaction (" 
		       << t->getId() << ")");

      debugMsg("FlowProfile:resetEdgeWeights","TransactionId (" << t->getId() << ")");

      if( t->isConsumer() ) 
	{
	  {
	    Edge* edge = m_lowerLevelGraph->getEdge( m_lowerLevelSource, m_lowerLevelGraph->getNode( t ) );

	    checkError(0 != edge, "resetEdgeWeights, no edges in lower level graph for transaction (" 
		       << t->getId() << ")");

	    edge->setCapacity( t->quantity()->lastDomain().getUpperBound() );
	  }

	  {
	    Edge* edge = m_upperLevelGraph->getEdge( m_upperLevelGraph->getNode( t ), m_upperLevelSink );

	    checkError(0 != edge, "resetEdgeWeights, no edges in upper level graph for transaction (" 
		       << t->getId() << ")");

	    edge->setCapacity( t->quantity()->lastDomain().getUpperBound() );
	  }
	}
      else
	{
	  {

	    Edge* edge = m_lowerLevelGraph->getEdge( m_lowerLevelGraph->getNode( t ), m_lowerLevelSink );

	    checkError(0 != edge, "resetEdgeWeights, no edges in lower level graph for transaction (" 
		       << t->getId() << ")");

	    edge->setCapacity( t->quantity()->lastDomain().getUpperBound() );
	  }

	  {
	    Edge* edge = m_upperLevelGraph->getEdge( m_upperLevelSource, m_upperLevelGraph->getNode( t ) );

	    checkError(0 != edge, "resetEdgeWeights, no edges in upper level graph for transaction (" 
		       << t->getId() << ")");

	    edge->setCapacity( t->quantity()->lastDomain().getUpperBound() );
	  }
	}
    }

    void FlowProfile::handleTransactionRemoved( const TransactionId t ) {
      check_error(t.isValid());

      debugMsg("FlowProfile:handleTransactionRemoved","TransactionId (" << t->getId() << ")");
      
      checkError( 0 != m_lowerLevelGraph->getNode( t ),
		  "Transaction (" 
		  << t->getId() << ") removed which is not in the lower level graph!");

      m_lowerLevelGraph->removeNode( t );

      checkError( 0 != m_upperLevelGraph->getNode( t ),
		  "Transaction (" 
		  << t->getId() << ") removed which is not in the upper level graph!");

      m_upperLevelGraph->removeNode( t );
      
      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;

      // this needs to be 'smarter'
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();
    }

    void FlowProfile::handleTransactionTimeChanged(const TransactionId t, const DomainListener::ChangeType& type)  {
      check_error(t.isValid());

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator(getId()))->getId();

      debugMsg("FlowProfile:handleTransactionTimeChanged","TransactionId (" << t->getId() << ") change " << type );
    }

    void FlowProfile::handleTransactionQuantityChanged(const TransactionId t, const DomainListener::ChangeType& type)
    {
      check_error(t.isValid());

      switch( type) {
      case DomainListener::UPPER_BOUND_DECREASED: 
      case DomainListener::RESET:
      case DomainListener::RELAXED: 
      case DomainListener::RESTRICT_TO_SINGLETON:
      case DomainListener::SET_TO_SINGLETON:
	{
	  resetEdgeWeights( t );
	}
      break;
      case DomainListener::LOWER_BOUND_INCREASED:
      case DomainListener::BOUNDS_RESTRICTED:
      default:
	break;
      };

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;

      m_recomputeInterval = (new ProfileIterator(getId()))->getId();

      debugMsg("FlowProfile:handleTransactionQuantityChanged","TransactionId (" << t->getId() << ") change " << type );
    }

    void FlowProfile::handleTransactionsOrdered(const TransactionId t1, const TransactionId t2)
    {
      check_error(t1.isValid());
      check_error(t2.isValid());

      if(m_recomputeInterval.isValid())
	delete (ProfileIterator*) m_recomputeInterval;
      m_recomputeInterval = (new ProfileIterator(getId()))->getId();

      debugMsg("FlowProfile:handleTransactionsOrdered","TransactionId1 (" << t1->getId() << ") before TransactionId2 (" << t2->getId() << ")");

    }
  }
}
