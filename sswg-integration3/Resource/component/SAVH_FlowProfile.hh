#ifndef FLOW_PROFILE_HEADER__
#define FLOW_PROFILE_HEADER__

/**
 * @file SAVH_Profile.hh
 * @author David Rijsman
 * @brief Defines the public interface for a maximum flow algorithm
 * @date April 2006
 * @ingroup Resource
 */

#include "DomainListener.hh"
#include "SAVH_MaxFlow.hh"
#include "SAVH_Profile.hh"
#include "SAVH_ResourceDefs.hh"
#include "SAVH_Types.hh"
#include "TemporalPropagator.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
    class Graph;
    class MaximumFlowAlgorithm;
    class Node;

    /**
     * @brief Graph structure to determine the subset of pending transactions at time T 
     * which has the largest contribution to an envelope (see FlowProfile)
     */
    class FlowProfileGraph
    {
    public:
      /**
       * @brief Creates a directed graph with a \a source and a \sink intended to calculate the 
       * lower level envelope in case \a lowerLevel is true otherwise intended to calculate the 
       * upper level envelope.
       */
      FlowProfileGraph( const SAVH::TransactionId& source, const SAVH::TransactionId& sink, bool lowerLevel );
      /**
       * @brief Destructor
       */
      ~FlowProfileGraph();
      /**
       * @brief Creates bi-directional edge between \a t1 and \a t2 with infinite capacity
       * as a result of a concurrent constraint between the two transactions
       */
      void enableAt( const SAVH::TransactionId& t1, const SAVH::TransactionId& t2 );
      /**
       * @brief Creates directed edge between \a t1 and \a t2 with infinite capacity
       * as a result of a before or at constraint between the two transactions (reverse
       * capacity set to zero)
       */
      void enableAtOrBefore( const SAVH::TransactionId& t1, const SAVH::TransactionId& t2 );
      /**
       * @brief Creates a node in the network and creates an edge:
       *
       * \verbatim
       *                               |            lower level               |            upper level 
       *   ---------------------------------------------------------------------------------------------------------
       *   transaction is consumer     |  <source, transaction> w = q.upper   |  <transaction, sink> w = q.lower
       *   ---------------------------------------------------------------------------------------------------------
       *   transaction is producer     |  <transaction, sink > w = q.lower    |  <source, transaction> w = q.upper        
       *   ---------------------------------------------------------------------------------------------------------
       * \endverbatim
       */
      void enableTransaction( const TransactionId& transaction, const InstantId& inst, TransactionId2InstantId& contributions );
      /**
       * @brief Returns true if \a transaction is enabled in the invoking 
       * instance
       */
      bool isEnabled(  const TransactionId& transaction ) const;
      /**
       * @brief Disables \a transaction, if enabled, for the invoking instance
       */
      void disable( const TransactionId& transaction ) ;
      /**
       * @brief Will push any flow wich flows through the node corresponding with \a transaction
       * back to the source of the edge the flow originates from. 
       * 
       * When we move from one instant to another we retract all the transactions whose end time
       * is equal to the new instant time and the transaction is still in the network. This method
       * will push any flow going through this node back to where it is coming from after which we 
       * try again to push the flow to the sink.

       * Will error out if no node corresponding to \a transaction is in the network or if the node
       * is not enabled.
       *
       * If a recalculation of the maximum flow is required this method will do nothing.
       * \todo verify if this is really required or perhaps we should error out?
       */
      void pushFlow( const SAVH::TransactionId& transaction );
      /**
       * @brief Returns the cummulative residual capacity originating from the source.
       *
       * Iterates over all outgoing edges from the source and sums the residual capicity of each
       * edge. Might trigger a maximum flow (re) calculation if required.
       */
      double getResidualFromSource();
      /**
       * @brief Disables every node reachable from the source in the residual network. Returns the sum 
       * of the contribution of each disabled node. The contribution is determined as following:
       * \verbatim
       *                               |  lower level |  upper level 
       *   ---------------------------------------------------------------------------------------------------------
       *   transaction is consumer     |    q.upper   |   q.lower 
       *   ---------------------------------------------------------------------------------------------------------
       *   transaction is producer     |    q.lower   |   q.upper
       *   ---------------------------------------------------------------------------------------------------------
       * \endverbatim
       *
       * where q is the quantity variable associated with the transactions associated with the node. The parameter \a
       * contributions, which maps a TransactionId to a InstantId is maps every transaction associated with a disabled
       * node to \a instant.
       */
      inline double disableReachableResidualGraph( TransactionId2InstantId& contributions, const InstantId& instant  );
      /**
       * @brief Returns true if the invoking instance calculates the lower level, otherwise returns false which indicates
       * the invoking instance is calculating the upper level.
       */
      bool isLowerLevel() const { return m_lowerLevel; }
      /**
       * @brief Removes transaction \a id from the network. 
       */
      void removeTransaction( const SAVH::TransactionId& id );
      /**
       * @brief Resets the invoking instance.
       *
       * Resetting entails disabling all the nodes, implying disabling all the edges, except for the 
       * source and the sink.
       * 
       */
      void reset();
      /**
       * @brief Restore flow invokes the maximum flow algorithm without resetting the existing distances 
       * and existing flows on the nodes.
       *
       * Restore flows is done after extracting all the nodes that expire (go from pending to closed set
       * at an instant) from the network by pushing flow back. 
       */
      void restoreFlow();
    private:
      /**
       * @brief Helper function for disableReachableResidualGraph
       */
      inline void visitNeighbors( const Node* node, double& residual, Node2Bool& visited, TransactionId2InstantId& contributions, const InstantId& instant  );
      /*!
       * @brief Boolean indicating if the instance is intended to calculate the lower level
       */
      bool m_lowerLevel;
      /*!
       * @brief Boolean indicating if the maximum flow solution needs to be recalculated
       */
      bool m_recalculate;
      /*!
       * @brief
       */
      SAVH::MaximumFlowAlgorithm* m_maxflow;
      /*!
       * @brief Bi directional graph datastructure 
       */
      SAVH::Graph* m_graph;
      /*!
       * @brief Source for the maximum flow problem
       */
      SAVH::Node* m_source;
      /*!
       * @brief Sink for the maximum flow problem
       */
      SAVH::Node* m_sink;
    };

    double FlowProfileGraph::disableReachableResidualGraph( TransactionId2InstantId& contributions, const InstantId& instant )
    {
      debugMsg("FlowProfileGraph:disableReachableResidualGraph","Lower level: "
	       << std::boolalpha << m_lowerLevel );

      double residual = 0.0;

      if( m_recalculate )
	{
	  debugMsg("FlowProfileGraph:disableReachableResidualGraph","Lower level: "
		   << std::boolalpha << m_lowerLevel << ", recalculate invoked.");

	  m_maxflow->execute();
	  
	  Node2Bool visited;

	  visited[ m_source ] = true;

	  visitNeighbors( m_source, residual, visited, contributions, instant );
	}

      return residual;
    }

    void FlowProfileGraph::visitNeighbors( const Node* node, double& residual, Node2Bool& visited, TransactionId2InstantId& contributions, const InstantId& instant  )
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

		      debugMsg("FlowProfileGraph::visitNeighbors","Transaction " 
			       << t << " starts contributing at " 
			       << instant->getTime() << " lower level " << std::boolalpha << m_lowerLevel ); 

		      contributions[ t ] = instant;

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
		      
		      visitNeighbors( target, residual, visited, contributions, instant );
		    }
		}
	    }
	}
    }



    /**
     * @brief Calculates the lower and upper level envelope of a resource.
     *
     * Given a set of transactions on a resource and temperol relations between
     * the transactions this algorithm calculates the tightest bounds possible.
     * The algorithm calculates the envelopes at any possible time the envelope
     * can change (at each instant).
     *
     * At each instant T the algorith determines the sets of transactions which 
     * have to contribute to the envelope level, closed set, the set which can 
     * contribute to the envelopes, the pending set, and the set of transaction
     * which can not contribute to the envelopes, the open set.
     *
     * The closed set are all transactions whose time upperbound is lower or 
     * equal to T. The pending set are the transaction whose lower time bound
     * is lower or equal to T and the upper time bound is greater than T. The 
     * open set of transactions are transaction whose lower time bound is greater
     * than T.
     *
     * To calculate the upper level envelope L(T) at time T we do the following:
     * -1- Initialize L(T) = 0
     * -2- Take the sum of the upper bound of the quantity of the production 
     *     transactaction in the closed set and add to L(T)
     * -3- Take the sum of the lower bound of the quantity of the consumer 
     *     transactaction in the closed set and add to L(T)
     * -4- Of all the transactions in the pending set at time T determine the 
     *     the set whose sum of production and consumptions is larger than any
     *     other set without violating any of the temporal relations (if two 
     *     transactions need to be executed at the same time they always need 
     *     to be in the same set). Add the sum of this set to L(T)
     *
     * To determine the set in step -4- we solve a maximum flow algorithm on 
     * the pending set. See FlowProfileGraph on how we construct a maximum 
     * flow graph out of the pending set.
     *
     * See 'N Muscettola. Computing the Envelope for Stepwise-Constant
     * Resource Allocations. CP 2002, LNCS 2470, pp 139-154, 2002'
     */
    class FlowProfile:
      public Profile
    {
    public:
      /**
       * @brief Constructor
       */
      FlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector, const double initLevelLb = 0, const double initLevelUb = 0 );
      /**
       * @brief Destructor
       */
      virtual ~FlowProfile();
      /**
       * @brief Retrieves the first (earliest) instant the transaction \t starts contributing to 
       * lower level. Will return true in case a instant has been associated with a transaction 
       * and will in that case store the instant in parameter \i. Returns false if no instant 
       * has been associated with transaction t.
       */
      bool getEarliestLowerLevelInstant( const TransactionId& t, InstantId& i );
      /**
       * @brief Retrieves the first (earliest) instant the transaction \t starts contributing to 
       * upper level. Will return true in case a instant has been associated with a transaction 
       * and will in that case store the instant in parameter \i. Returns false if no instant 
       * has been associated with transaction t.
       */
      bool getEarliestUpperLevelInstant( const TransactionId& t, InstantId& i );
    protected:
      /**
       * @brief Indicates the ordering between two time variables associated with a transaction
       */
      enum Order {
	AFTER_OR_AT = 0, /*!< Indicates one transaction is strictly after or at the same time with another transaction. */ 
	BEFORE_OR_AT,/*!< Indicates one transaction is strictly before or at the same time with another transaction. */ 
	NOT_ORDERED,/*!< Indicates one transaction is not ordered with another transaction. */ 
	STRICTLY_AT,/*!< Indicates one transaction is strictly at the same time with another transaction. */ 
	UNKNOWN 
      };
      /**
       * @brief Deletes pre-existing FlowProfileGraphs for the lower and upper level and allocates new ones.
       */
      void initializeGraphs();
      /**
       * @brief 
       */
      void postHandleRecompute();
      /**
       * @brief Enables a transaction t. A transaction is enabled a time T to calculate the 
       * envelopes for instant at time T if the lower bound of the time equals T (this is assuming
       * envelopes are calculated from earliest to latest instant). Another way of formulating this 
       * is that a transaction is enabled at the time it moves from the open set to the pending set
       * or straight to the closed set if the time is a singleton.
       */
      void enableTransaction( const TransactionId t, const InstantId i );
      /**
       * @brief Helper method for subclasses to respond to a temporal constraint being added between two transactions.
       * @param e The transaction whose timepoint has been constrained.
       * @param argIndex The index of the timepoint in the constraint.
       */ 
      void handleTemporalConstraintAdded(const TransactionId predecessor, const int preArgIndex,
					 const TransactionId successor, const int sucArgIndex);
      
      /**
       * @brief Helper method for subclasses to respond to a temporal constraint being removed between two transactions.
       * @param e The transaction whose timepoint has been removed from the constraint.
       * @param argIndex The index of the timepoint in the constraint.
       */
      void handleTemporalConstraintRemoved(const TransactionId predecessor, const int preArgIndex,
					   const TransactionId successor, const int sucArgIndex);
      /**
       * @brief Updates the maximum flow graphs in case transactions t1 and t2 are now strictly ordered. 
       */
      void handleOrderedAt( const TransactionId t1, const TransactionId t2 );
      /**
       * @brief Updates the maximum flow graphs in case transactions t1 and t2 are now weakly ordered. 
       */
      void handleOrderedAtOrBefore( const TransactionId t1, const TransactionId t2 );
      /**
       * @brief 
       */
      void handleTransactionAdded( const TransactionId t);
      /**
       * @brief 
       * @return
       */
      void handleTransactionRemoved( const TransactionId t);
      /**
       * @brief 
       * @return
       */
      void handleTransactionTimeChanged( const TransactionId t, const DomainListener::ChangeType& type );
      /**
       * @brief 
       * @return
       */
      void handleTransactionQuantityChanged( const TransactionId t, const DomainListener::ChangeType& type );
      /**
       * @brief 
       * @return
       */
      void initRecompute(InstantId inst);
      /**
       * @brief 
       * @return
       */
      void initRecompute();
      /**
       * @brief 
       * @return
       */
      Order getOrdering( const TransactionId t1, const TransactionId t2 );
      /**
       * @brief 
       * @return
       */
      void recomputeLevels(InstantId prev, InstantId inst);

      typedef std::pair< int, int > IntIntPair;
      typedef hash_map< TransactionId, IntIntPair, TransactionIdHash > TransactionId2IntIntPair;

      TransactionId2IntIntPair m_previousTimeBounds;

      SAVH::TransactionId m_dummySourceTransaction;
      SAVH::TransactionId m_dummySinkTransaction;

      SAVH::FlowProfileGraph* m_lowerLevelGraph;
      SAVH::FlowProfileGraph* m_upperLevelGraph;

      double m_lowerClosedLevel;
      double m_upperClosedLevel;

      bool m_recalculateLowerLevel;
      bool m_recalculateUpperLevel;    

      typedef std::pair<TransactionId,TransactionId> TransactionIdTransactionIdPair;
      typedef std::map< TransactionIdTransactionIdPair, Order > TransactionIdTransactionIdPair2Order;

      TransactionIdTransactionIdPair2Order m_orderings;
      TransactionIdTransactionIdPair2Order m_orderedAt;

      TransactionId2InstantId m_lowerLevelContribution;
      TransactionId2InstantId m_upperLevelContribution;
    };
  }
}

#endif //FLOW_PROFILE_HEADER__
