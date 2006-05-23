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
#include "SAVH_IncrementalFlowProfile.hh"
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
    IncrementalFlowProfile::IncrementalFlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector, const double initLevelLb, const double initLevelUb ):
      FlowProfile( db, flawDetector, initLevelLb, initLevelUb )
    {
    }
     
    IncrementalFlowProfile::~IncrementalFlowProfile()
    {
    }
 
    void IncrementalFlowProfile::initRecompute() 
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

    void IncrementalFlowProfile::enableOrderings( const TransactionId& transaction1, const InstantId& inst  )
    {
      debugMsg("IncrementalFlowProfile:enableOrderings","TransactionId (" << transaction1->getId() << ")");

      const std::set<TransactionId>& transactions = inst->getTransactions();

      std::set<TransactionId>::const_iterator iter = transactions.begin();
      std::set<TransactionId>::const_iterator end = transactions.end();

      for( ; iter != end; ++iter )
	{
	  const TransactionId& transaction2 = (*iter);
		      
	  if( transaction1 != transaction2 
	      && 
	      !transaction2->time()->lastDomain().isSingleton()
	      &&
	      ( ( m_recalculateLowerLevel 
		  && ( transaction2->time()->lastDomain().getLowerBound() == inst->getTime() || m_lowerLevelGraph->isEnabled( transaction2 ) ) )
		||
		( m_recalculateUpperLevel 
		  && ( transaction2->time()->lastDomain().getLowerBound() == inst->getTime() || m_upperLevelGraph->isEnabled( transaction2 ) ) ) ) ) 
	    {
	      debugMsg("IncrementalFlowProfile:enableOrderings","Transaction (" 
		       << transaction1->getId() << ") "
		       << transaction1->time()->toString() << " and Transaction ("
		       << transaction2->getId() << ") "
		       << transaction2->time()->toString() );
		  
	      if( isConstrainedToAt( transaction1, transaction2 ) ) 
		{
		  if( m_recalculateLowerLevel )
		    m_lowerLevelGraph->enableAt( transaction1, transaction2 );
		      
		  if( m_recalculateUpperLevel )
		    m_upperLevelGraph->enableAt( transaction1, transaction2 );
		}
	      else if( isConstrainedToBeforeOrAt( transaction1, transaction2 ) )  
		{
		  if( m_recalculateLowerLevel )
		    m_lowerLevelGraph->enableAtOrBefore( transaction1, transaction2 );
		      
		  if( m_recalculateUpperLevel )
		    m_upperLevelGraph->enableAtOrBefore( transaction1, transaction2 );
		}
	      else 
		{
		  debugMsg("IncrementalFlowProfile:enableOrderings","Transaction (" 
			   << transaction1->getId() << ") and Transaction ("
			   << transaction2->getId() << ") not constrained");
		}
	    }
	}
    }

    void IncrementalFlowProfile::recomputeLevels( InstantId prev, InstantId inst ) 
    {
      check_error( prev.isValid() || InstantId::noId() == prev );
      check_error( inst.isValid() );

      debugMsg("IncrementalFlowProfile::recomputeLevels","Instant (" 
	       << inst->getId() << ") at time "
	       << inst->getTime() );

      double lowerLevel = prev == InstantId::noId() ? m_initLevelLb : prev->getLowerLevel();
      double upperLevel = prev == InstantId::noId() ? m_initLevelUb : prev->getUpperLevel();

      const std::set<TransactionId>& startingTransactions = inst->getStartingTransactions();

      bool expansion = false;
      
      {
	std::set<TransactionId>::const_iterator ite = startingTransactions.begin();
	std::set<TransactionId>::const_iterator end = startingTransactions.end();
	
	for( ; ite != end; ++ite )
	  {
	    const TransactionId& started = (*ite);
	    
	    check_error( started->time()->lastDomain().getLowerBound() == inst->getTime() );

	    // if the transaction is a singleton it goes straight into the closed set
	    if( !started->time()->lastDomain().isSingleton() )
	      {
		debugMsg("IncrementalFlowProfile::recomputeLevels","Expanding graphs with transaction ("
			 << started->getId() << ")");

		expansion = true;
		
		enableTransaction( started );
		
		enableOrderings( started, inst );
	      }
	  }
	
	if( expansion )
	  {
	    if( m_recalculateLowerLevel )
	      {
		lowerLevel += m_lowerLevelGraph->disableReachableResidualGraph();
	      }
	    
	    if( m_recalculateUpperLevel )
	      {
		upperLevel += m_upperLevelGraph->disableReachableResidualGraph();
	      }
	  }
      }

      const std::set<TransactionId>& endingTransactions = inst->getEndingTransactions();

      bool contraction = false;

      {
	std::set<TransactionId>::const_iterator ite = endingTransactions.begin();
	std::set<TransactionId>::const_iterator end = endingTransactions.end();
	
	for( ; ite != end; ++ite )
	  {
	    const TransactionId& ended = (*ite);

	    if( m_recalculateLowerLevel )
	      {
		bool enteredClosedSet = false;

		// if it is still enabled it is not yet contributing to the level
		if( m_lowerLevelGraph->isEnabled( ended ) )
		  {
		    debugMsg("IncrementalFlowProfile::recomputeLevels","Contracting from lower graph transaction ("
			     << ended->getId() << ")");

		    enteredClosedSet = true;
		    contraction = true;
		    m_lowerLevelGraph->pushFlow( ended );
		    m_lowerLevelGraph->disable( ended );
		  }
		else if( ended->time()->lastDomain().isSingleton() )
		  {
		    debugMsg("IncrementalFlowProfile::recomputeLevels","Transaction ("
			     << ended->getId() << ") straight from open to closed set");

		    enteredClosedSet = true;
		  }

		if( enteredClosedSet )
		  {
		    if( ended->isConsumer() )
		      {
			lowerLevel -= ended->quantity()->lastDomain().getUpperBound();
		      }
		    else
		      {
			lowerLevel += ended->quantity()->lastDomain().getLowerBound();
		      }		  
		  }
	      }

	    if( m_recalculateUpperLevel )
	      {
		bool enteredClosedSet = false;

		if( m_upperLevelGraph->isEnabled( ended ) )
		  {
		    debugMsg("IncrementalFlowProfile::recomputeLevels","Contracting from upper graph transaction ("
			     << ended->getId() << ")");

		    enteredClosedSet = true;
		    contraction = true;
		    m_upperLevelGraph->pushFlow( ended );
		    m_upperLevelGraph->disable( ended );
		  }
		else if( ended->time()->lastDomain().isSingleton() )
		  {
		    enteredClosedSet = true;
		  }

		if( enteredClosedSet )
		  {
		    debugMsg("IncrementalFlowProfile::recomputeLevels","Transaction ("
			     << ended->getId() << ") straight from open to closed set");

		    if( ended->isConsumer() )
		      {
			upperLevel -= ended->quantity()->lastDomain().getLowerBound();
		      }
		    else
		      {
			upperLevel += ended->quantity()->lastDomain().getUpperBound();
		      }
		  }
	      }
	  }

	if( contraction )
	  {
	    if( m_recalculateLowerLevel )
	      {
		m_lowerLevelGraph->restoreFlow();
		lowerLevel += m_lowerClosedLevel - m_lowerLevelGraph->getResidualFromSource();
	      }

	    if( m_recalculateUpperLevel )
	      {
		m_upperLevelGraph->restoreFlow();
		upperLevel += m_upperClosedLevel + m_upperLevelGraph->getResidualFromSource();
	      }
	  }
      }
  

      debugMsg("IncrementalFlowProfile::recomputeLevels","Computed levels for instance at time "
	       << inst->getTime() << "["
	       << lowerLevel << "," 
	       << upperLevel << "]");

      inst->update( lowerLevel, lowerLevel, upperLevel, upperLevel, 
		    0, 0, 0, 0, 
		    0, 0, 0, 0, 
		    0, 0, 0, 0 );      
    }
  }
}
