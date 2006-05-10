#ifndef FLOW_PROFILE_HEADER__
#define FLOW_PROFILE_HEADER__

/**
 * @file SAVH_Edge.hh
 * @author David Rijsman
 * @brief Defines the public interface for a maximum flow algorithm
 * @date April 2006
 * @ingroup Resource
 */

#include "DomainListener.hh"
#include "SAVH_Profile.hh"
#include "SAVH_ResourceDefs.hh"
#include "TemporalPropagator.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
    class Graph;
    class Node;

    class FlowProfile:
      public Profile
    {
    public:
      FlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector, const double initLevelLb = 0, const double initLevelUb = 0 );
      virtual ~FlowProfile();
    protected:
    private:
      void enableTransaction( const TransactionId t );

      void handleOrderedAt( const TransactionId t1, const TransactionId t2 );
      void handleOrderedAtOrBefore( const TransactionId t1, const TransactionId t2 );
      void handleTransactionAdded( const TransactionId t);
      void handleTransactionRemoved( const TransactionId t);
      void handleTransactionTimeChanged( const TransactionId t, const DomainListener::ChangeType& type );
      void handleTransactionQuantityChanged( const TransactionId t, const DomainListener::ChangeType& type );
      void handleTransactionsOrdered( const TransactionId t1, const TransactionId t2 );

      void initRecompute(InstantId inst);
      void initRecompute();
      bool isConstrainedToBeforeOrAt( const TransactionId t1, const TransactionId t2 );
      bool isConstrainedToAt( const TransactionId t1, const TransactionId t2 );

      void recomputeLevels(InstantId inst);
      void resetEdgeWeights( const TransactionId t );

      
      // temp untill the base class has it
      PlanDatabaseId m_planDatabase;
     
      SAVH::TransactionId m_dummySourceTransaction;
      SAVH::TransactionId m_dummySinkTransaction;

      SAVH::Graph* m_lowerLevelGraph;
      SAVH::Node* m_lowerLevelSource;
      SAVH::Node* m_lowerLevelSink;
      
      SAVH::Graph* m_upperLevelGraph;
      SAVH::Node* m_upperLevelSource;
      SAVH::Node* m_upperLevelSink;

      double m_lowerClosedLevel;
      double m_upperClosedLevel;
    };
  }
}

#endif //FLOW_PROFILE_HEADER__
