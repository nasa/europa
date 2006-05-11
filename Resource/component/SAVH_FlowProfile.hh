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

    class FlowProfileGraph
    {
    public:
      FlowProfileGraph( const SAVH::TransactionId& source, const SAVH::TransactionId& sink, bool lowerLevel );
      ~FlowProfileGraph();
      
      void enableAt( const SAVH::TransactionId& t1, const SAVH::TransactionId& t2 );
      void enableAtOrBefore( const SAVH::TransactionId& t1, const SAVH::TransactionId& t2 );
      void enableTransaction( const SAVH::TransactionId& transaction );

      double getResidualFromSource();

      bool isLowerLevel() const { return m_lowerLevel; }

      void removeTransaction( const SAVH::TransactionId& id );
      void reset();
    private:
      bool m_lowerLevel;
      bool m_recalculate;
      SAVH::Graph* m_graph;
      SAVH::Node* m_source;
      SAVH::Node* m_sink;
    };

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

      void recomputeLevels(InstantId prev, InstantId inst);
      void resetEdgeWeights( const TransactionId t );

     
      SAVH::TransactionId m_dummySourceTransaction;
      SAVH::TransactionId m_dummySinkTransaction;

      SAVH::FlowProfileGraph* m_lowerLevelGraph;
      SAVH::FlowProfileGraph* m_upperLevelGraph;

      double m_lowerClosedLevel;
      double m_upperClosedLevel;
    };
  }
}

#endif //FLOW_PROFILE_HEADER__
