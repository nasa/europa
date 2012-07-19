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
#include "IncrementalFlowProfile.hh"
#include "Graph.hh"
#include "Instant.hh"
#include "MaxFlow.hh"
#include "Node.hh"
#include "Transaction.hh"
#include "Profile.hh"
#include "TemporalAdvisor.hh"
#include "Utils.hh"
#include "Variable.hh"

namespace EUROPA
{
    IncrementalFlowProfile::IncrementalFlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector, const double initLevelLb, const double initLevelUb ):
      FlowProfile( db, flawDetector, initLevelLb, initLevelUb )
    {
      debugMsg("IncrementalFlowProfile:IncrementalFlowProfile","Initial level [" << initLevelLb << "," << initLevelUb << "]");
    }

    IncrementalFlowProfile::~IncrementalFlowProfile()
    {
    }

    void IncrementalFlowProfile::initRecompute( InstantId inst )
    {
      debugMsg("IncrementalFlowProfile::initRecompute","For instant (" << inst->getId() << ")");

      std::map<int, InstantId>::iterator it = getGreatestInstant( inst->getTime() - 1 );

      if( m_instants.end() != it  )
        {
          InstantId previous = it->second;

          debugMsg("IncrementalFlowProfile::initRecompute","For instant ("
                   << inst << ") getting initial levels from instant ("
                   << previous << ")");
          // initial level
          m_lowerClosedLevel = previous->getLowerLevel();

          // initial level
          m_upperClosedLevel = previous->getUpperLevel();
        }
      else
        {
          // initial level
          m_lowerClosedLevel = m_initLevelLb;

          // initial level
          m_upperClosedLevel = m_initLevelUb;
        }

      initializeGraphs();

      std::set<TransactionId> enabledLower;
      std::set<TransactionId> enabledUpper;

      // we got to put all the transactions which are pending at this instant but are not yet contributing
      // to the levels back into the maximum flow!
      const std::set<TransactionId>& transactions = inst->getTransactions();

      {
        std::set<TransactionId>::const_iterator iter = transactions.begin();
        std::set<TransactionId>::const_iterator end = transactions.end();

        for( ; iter != end; ++iter )
          {
            const TransactionId& transaction = (*iter);

	    InstantId startLowerLevel;
            bool isContributingToLowerLevel = getEarliestLowerLevelInstant( transaction, startLowerLevel );
	      //TransactionId2InstantId::const_iterator llc_iter = m_lowerLevelContribution.find( transaction );

	    InstantId startUpperLevel;
	    bool isContributingToUpperLevel = getEarliestUpperLevelInstant( transaction, startUpperLevel );
            //TransactionId2InstantId::const_iterator ulc_iter = m_upperLevelContribution.find( transaction );

            if( ( isContributingToLowerLevel
                  &&
                  startLowerLevel->getTime() >= inst->getTime() )
                ||
                !isContributingToLowerLevel )
              {
                m_lowerLevelGraph->enableTransaction( transaction, inst, m_lowerLevelContribution );
                enabledLower.insert( transaction );
              }

            if( ( isContributingToUpperLevel
                  &&
                  startUpperLevel->getTime() >= inst->getTime() )
                ||
                !isContributingToUpperLevel )
              {
                m_upperLevelGraph->enableTransaction( transaction, inst, m_upperLevelContribution );
                enabledUpper.insert( transaction );
              }
          }
      }

      {
        std::set<TransactionId>::const_iterator iter = enabledLower.begin();
        std::set<TransactionId>::const_iterator end = enabledLower.end();

        for( ; iter != end; ++iter )
          {
            const TransactionId& transaction1 = (*iter);

            std::set<TransactionId>::const_iterator iter2 = iter;

            for( ; iter2 != end; ++iter2 )
              {
                const TransactionId& transaction2 = (*iter2);

                if( transaction1 != transaction2 )
                  {
                    Order order = getOrdering( transaction1, transaction2 );

                    if( STRICTLY_AT == order )
                      {
                        m_lowerLevelGraph->enableAt( transaction1, transaction2 );
                      }
                    else if( BEFORE_OR_AT == order )
                      {
                        m_lowerLevelGraph->enableAtOrBefore( transaction1, transaction2 );
                      }
                    else if( AFTER_OR_AT == order  )
                      {
                        m_lowerLevelGraph->enableAtOrBefore( transaction2, transaction1 );
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
      }

      {
        std::set<TransactionId>::const_iterator iter = enabledUpper.begin();
        std::set<TransactionId>::const_iterator end = enabledUpper.end();

        for( ; iter != end; ++iter )
          {
            const TransactionId& transaction1 = (*iter);

            std::set<TransactionId>::const_iterator iter2 = iter;

            for( ; iter2 != end; ++iter2 )
              {
                const TransactionId& transaction2 = (*iter2);

                if( transaction1 != transaction2 )
                  {
                    Order order = getOrdering( transaction1, transaction2 );

                    if( STRICTLY_AT == order )
                      {
                        m_upperLevelGraph->enableAt( transaction1, transaction2 );
                      }
                    else if( BEFORE_OR_AT == order )
                      {
                        m_upperLevelGraph->enableAtOrBefore( transaction1, transaction2 );
                      }
                    else if( AFTER_OR_AT == order  )
                      {
                        m_upperLevelGraph->enableAtOrBefore( transaction2, transaction1 );
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
      }

      recomputeLevels( inst, m_lowerClosedLevel, m_upperClosedLevel );
    }

    void IncrementalFlowProfile::initRecompute()
    {
      checkError(m_recomputeInterval.isValid(), "Attempted to initialize recomputation without a valid starting point!");

      debugMsg("IncrementalFlowProfile::initRecompute","");

      initializeGraphs();

//       if( m_recalculateLowerLevel )
//         {
//           m_lowerLevelContribution.clear();
//         }

//       if( m_recalculateUpperLevel )
//         {
//           m_upperLevelContribution.clear();
//         }

      // initial level
      m_lowerClosedLevel = m_initLevelLb;

      // initial level
      m_upperClosedLevel = m_initLevelUb;
    }

    bool IncrementalFlowProfile::enableOrderings( const InstantId& inst  )
    {
      debugMsg("IncrementalFlowProfile:enableOrderings","Instant Id (" << inst->getId() << ")");

      bool returnValue = false;

      const std::set<TransactionId>& startingTransactions = inst->getStartingTransactions();

      std::set<TransactionId>::const_iterator ite = startingTransactions.begin();
      std::set<TransactionId>::const_iterator end = startingTransactions.end();

      for( ; ite != end; ++ite )
        {
          const TransactionId& transaction1 = (*ite);

          if( !transaction1->time()->lastDomain().isSingleton() )
            {
              enableTransaction( transaction1, inst );

              returnValue = true;

              const std::set<TransactionId>& transactions = inst->getTransactions();

              std::set<TransactionId>::const_iterator iter = transactions.begin();
              std::set<TransactionId>::const_iterator end = transactions.end();

              for( ; iter != end; ++iter )
                {
                  const TransactionId& transaction2 = (*iter);

                  bool ordering12Calculated = false;
                  Order ordering12 = UNKNOWN;

                  if( m_recalculateLowerLevel )
                    {
                      if( transaction1 != transaction2
                          &&
                          !transaction2->time()->lastDomain().isSingleton()
                          &&
                          transaction2->time()->lastDomain().getUpperBound() != inst->getTime()
                          &&
                          (transaction2->time()->lastDomain().getLowerBound() == inst->getTime() || m_lowerLevelGraph->isEnabled( transaction2 ) ) )
                        {
                          ordering12 = getOrdering( transaction1, transaction2 );
                          ordering12Calculated = true;

                          if( STRICTLY_AT == ordering12 )
                            {
                              m_lowerLevelGraph->enableAt( transaction1, transaction2 );
                            }
                          else if( BEFORE_OR_AT == ordering12 )
                            {
                              m_lowerLevelGraph->enableAtOrBefore( transaction1, transaction2 );
                            }
                          else if( AFTER_OR_AT == ordering12  )
                            {
                              m_lowerLevelGraph->enableAtOrBefore( transaction2, transaction1 );
                            }
                          else
                            {
                              debugMsg("IncrementalFlowProfile:enableOrderings","Transaction ("
                                       << transaction1->getId() << ") and Transaction ("
                                       << transaction2->getId() << ") not constrained");
                            }
                        }
                    }

                  if( m_recalculateUpperLevel )
                    {
                      if( transaction1 != transaction2
                          &&
                          !transaction2->time()->lastDomain().isSingleton()
                          &&
                          transaction2->time()->lastDomain().getUpperBound() != inst->getTime()
                          &&
                          ( transaction2->time()->lastDomain().getLowerBound() == inst->getTime() || m_upperLevelGraph->isEnabled( transaction2 ) ) )
                        {
                          if( !ordering12Calculated )
                            ordering12 = getOrdering( transaction1, transaction2 );

                          if( STRICTLY_AT == ordering12 )
                            {
                              m_upperLevelGraph->enableAt( transaction1, transaction2 );
                            }
                          else if( BEFORE_OR_AT == ordering12 )
                            {
                              m_upperLevelGraph->enableAtOrBefore( transaction1, transaction2 );
                            }
                          else if( AFTER_OR_AT == ordering12  )
                            {
                              m_upperLevelGraph->enableAtOrBefore( transaction2, transaction1 );
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
            }
        }

      return returnValue;
    }

    void IncrementalFlowProfile::recomputeLevels( InstantId prev, InstantId inst )
    {
      check_error( prev.isValid() || InstantId::noId() == prev );
      check_error( inst.isValid() );

      debugMsg("IncrementalFlowProfile::recomputeLevels","Previous instant ("
               << prev << ") and current instant ("
               << inst << ")");

      double lowerLevel = prev == InstantId::noId() ? m_lowerClosedLevel : prev->getLowerLevel();
      double upperLevel = prev == InstantId::noId() ? m_upperClosedLevel : prev->getUpperLevel();

      recomputeLevels( inst, lowerLevel, upperLevel );
    }

    void IncrementalFlowProfile::recomputeLevels( InstantId inst, double lowerLevel, double upperLevel )
    {
      //        static int counter = 0;
      //        counter++;

      //        debugMsg("Performance::recomputeLevels", "Invocation counter = " << counter );

      debugMsg("IncrementalFlowProfile::recomputeLevels","Instant ("
               << inst->getId() << ") at time "
               << inst->getTime() << " start levels ["
               << lowerLevel << ","
               << upperLevel << "]");

      bool expansion = enableOrderings( inst );

      {
        if( expansion )
          {
            if( m_recalculateLowerLevel )
              {
                double delta = m_lowerLevelGraph->disableReachableResidualGraph( m_lowerLevelContribution, inst );

                debugMsg("IncrementalFlowProfile::recomputeLevels","Expansion leads to delta lower level of "
                         << delta );

                lowerLevel += delta;
              }

            if( m_recalculateUpperLevel )
              {
                double delta = m_upperLevelGraph->disableReachableResidualGraph( m_upperLevelContribution, inst );

                debugMsg("IncrementalFlowProfile::recomputeLevels","Expansion leads to delta upper level of "
                         << delta );

                upperLevel += delta;
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
                             << ended->getId() << ") "
                             << ended->time()->toString() << " "
                             << ended->quantity()->toString() );

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
		    debugMsg("IncrementalFlowProfile:recomputeLevels","Transaction "
			     << ended << " starts contributing at "
			     << inst->getTime() << " lower level true");

                    m_lowerLevelContribution[ ended ] = inst;

                    if( ended->isConsumer() )
                      {
                        lowerLevel -= ended->quantity()->lastDomain().getUpperBound();

			debugMsg("IncrementalFlowProfile::recomputeLevels","Transaction ("
                                 << ended->getId() << ") decreases lower level by "
                                 << ended->quantity()->lastDomain().getUpperBound() << " (new level "
                                 << lowerLevel << ")");
                      }
                    else
                      {
                        lowerLevel += ended->quantity()->lastDomain().getLowerBound();


                        debugMsg("IncrementalFlowProfile::recomputeLevels","Transaction ("
                                 << ended->getId() << ") increases lower level by "
                                 << ended->quantity()->lastDomain().getLowerBound() << " (new level "
                                 << lowerLevel << ")");
                      }
                  }
              }

            if( m_recalculateUpperLevel )
              {
                bool enteredClosedSet = false;

                if( m_upperLevelGraph->isEnabled( ended ) )
                  {
                    debugMsg("IncrementalFlowProfile::recomputeLevels","Contracting from upper graph transaction ("
                             << ended->getId() << ") "
                             << ended->time()->toString() << " "
                             << ended->quantity()->toString() );

                    enteredClosedSet = true;
                    contraction = true;
                    m_upperLevelGraph->pushFlow( ended );
                    m_upperLevelGraph->disable( ended );
                  }
                else if( ended->time()->lastDomain().isSingleton() )
                  {
                    debugMsg("IncrementalFlowProfile::recomputeLevels","Transaction ("
                             << ended->getId() << ") straight from open to closed set");

                    enteredClosedSet = true;
                  }

                if( enteredClosedSet )
                  {
		    debugMsg("IncrementalFlowProfile:recomputeLevels","Transaction "
			     << ended << " starts contributing at "
			     << inst->getTime() << " lower level false");

                    m_upperLevelContribution[ ended ] = inst;

                    if( ended->isConsumer() )
                      {
                        upperLevel -= ended->quantity()->lastDomain().getLowerBound();

                        debugMsg("IncrementalFlowProfile::recomputeLevels","Transaction ("
                                 << ended->getId() << ") decreases upper level by "
                                 << ended->quantity()->lastDomain().getLowerBound() << " (new level "
                                 << upperLevel << ")");
                      }
                    else
                      {
                        upperLevel += ended->quantity()->lastDomain().getUpperBound();

                        debugMsg("IncrementalFlowProfile::recomputeLevels","Transaction ("
                                 << ended->getId() << ") increases upper level by "
                                 << ended->quantity()->lastDomain().getUpperBound() << " (new level "
                                 << upperLevel << ")");		      }
                  }
              }
          }

        if( contraction )
          {
            if( m_recalculateLowerLevel )
              {
                m_lowerLevelGraph->restoreFlow();

                double delta = m_lowerLevelGraph->disableReachableResidualGraph( m_lowerLevelContribution, inst );

                debugMsg("IncrementalFlowProfile::recomputeLevels","Contraction leads to delta lower level of "
                         << delta );

                lowerLevel += delta;
              }

            if( m_recalculateUpperLevel )
              {
                m_upperLevelGraph->restoreFlow();

                double delta = m_upperLevelGraph->disableReachableResidualGraph( m_upperLevelContribution, inst );

                debugMsg("IncrementalFlowProfile::recomputeLevels","Contraction leads to delta upper level of "
                         << delta );

                upperLevel += delta;
              }
          }
      }


      debugMsg("IncrementalFlowProfile::recomputeLevels","Computed levels for instance at time "
               << inst->getTime() << "["
               << lowerLevel << ","
               << upperLevel << "]");

      debugMsg("IncrementalFlowProfile::calculatedLevels","Computed levels for instance at time "
               << inst->getTime() << "["
               << lowerLevel << ","
               << upperLevel << "]");

      inst->update( lowerLevel, lowerLevel, upperLevel, upperLevel,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0 );          }

}
