#ifndef H_Solver
#define H_Solver

/**
 * @file Solver.hh
 * @author Conor McGann
 * @date April, 2005
 * @brief Defines the main interface for interaction with a solver - an agent that identifies and resolves flaws in a partial plan.
 * @ingroup Solvers
 */

#include "SolverDefs.hh"
#include "FlawManager.hh"
#include "SearchListener.hh"
#include "EntityIterator.hh"
#include "ConstraintEngineListener.hh"
#include "PlanDatabaseListener.hh"

namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Defines the main solver interface for identification and resolution of flaws on a plan database.
     *
     * A solver may or may not do planning i.e. goal decomposition. Most generally, it will process a set of flaws in a partial plan until
     * there are no more in scope.The Solver is a mediator between Flaw Managers and Decision Points. This solver provides a chronological backtracking search.
     *
     * @see FlawManager, DecisionPoint
     */
    class Solver {
    public:

      /**
       * @brief Constructor
       */
      Solver(const PlanDatabaseId& db, const TiXmlElement& configData);

      /**
       * @brief Required for safe deletion of subclasses
       */
      virtual ~Solver();

      /**
       * @brief Invocation to solve any flaws in scope for the current Partial Plan
       *
       * This method will NOT reset a prior search stack.
       * @param maxSteps The maximum number of additional steps permitted to resolve all flaws in THIS iteration.
       * @param maxDepth The maximum growth in stack size permitted to resolve all flaws in THIS iteration.
       * @return true if all flaws resolved within maxSteps and maxDepth, otherwise false.
       * @see reset, clear.
       */
      bool solve(unsigned int maxSteps = PLUS_INFINITY, unsigned int maxDepth = PLUS_INFINITY);

      /**
       * @brief Invocation for a single step of flaw resolution.
       *
       * Prior to executing this, the solver must not be backtracking. There may be side-effects as one or more
       * decisions (or even all decisions) may be backtracked by attempting a step.
       */
      void step();

      /**
       * @brief Retracts all decisions stored in the internal decision stack.
       * @note Will not force propagation.
       */
      void reset();

      /**
       * @brief Resets (undo and delete) a specific number of decisions, in reverse chronological order, 
       * in the internal decision stack
       * @param depth The number of decisions to reset.
       */
      void reset(unsigned int depth);

      /**
       * @brief Backjumps a specific number of decisions, in reverse chronological order.
       * @param stepCount the number of steps to undo. Will delete all but the last one.
       * @return false if there are more choices to make, otherwise true.
       * @see backtrack
       */
      bool backjump(unsigned int stepCount);

      /**
       * @brief Clears current decisions on the stack without any modifications to the plan.
       * 
       * Used if we want the solver to work on a set of flaws without coupling this work to prior search.
       */
      void clear();

      /**
       * @brief Standard Id accessor.
       */
      const SolverId& getId() const;

      /**
       * @brief Name accessor
       */
      const LabelStr& getName() const;

      /**
       * @brief The size of the search stack
       */
      unsigned int getDepth() const;

      /**
       * @brief The total number of search steps since the Solver was previously cleared.
       */
      unsigned int getStepCount() const;

      /**
       * @brief Tests if we have concluded there are no more flaws.
       */
      bool noMoreFlaws() const;

      /**
       * @brief Tests if the search space have been exhausted. 
       */
      bool isExhausted() const;

      /**
       * @brief tests if the search step and depth limits hane been exceeded
       */
      bool isTimedOut() const;

      /**
       * @brief Retrieve all decisions on the stack.
       */
      const DecisionStack& getDecisionStack() const;
      
      std::string getDecisionStackAsString() const;

      /**
       * @brief Register a listener for search actions and status.
       */
      void addListener(const SearchListenerId& sl);

      /**
       * @brief Unregister a listener for search actions and status.
       */
      void removeListener(const SearchListenerId& sl);

      /**
       * @brief Set the maximum number of steps to take when planning under PlanWorks control.
       */
      void setMaxSteps(const unsigned int steps);

      /**
       * @brief Set the maximum depth to search when planning under PlanWorks control.
       */
      void setMaxDepth(const unsigned int depth);

      /**
       * @brief Create an iterator over the set of flaws.
       */
      IteratorId createIterator();

      /**
       * @brief True if the given entity is in scope.
       * @see FlawManager::inScope
       */
      bool inScope(const EntityId& entity);

      /**
       * @brief Retrieve an active flaw handler for the given entity
       */
      FlawHandlerId getFlawHandler(const EntityId entity);

      bool isConstraintConsistent() const;

      std::string getLastExecutedDecision() const;

      std::multimap<Priority, std::string> getOpenDecisions() const; 

      std::string printOpenDecisions() const;      
      
      /**
       * @brief Access the context of this Solver.
       */
      ContextId getContext() const {return m_context;}

			/**
			 * @brief Verify that the state of the Solver is consistent with our expectaions.
			 */
			bool isValid() const;

    protected:

      /**
       * @brief Retrieves a new DecisionPoint from the set of Flaw Managers.
       *
       * @note Customize this method in a derived class to change the overall algorithm for selecting the next flaw to work on.
       */
      virtual void allocateNewDecisionPoint();

      /**
       * @brief Will backtrack from current failed state in the search to a point from which the search can resume.
       * @return true if search can resume. Otherwise false, indicating search is exhausted.
       */
      bool backtrack();

      /**
       * @brief Iterates over Flaw Managers to obtain a flaw that is forced i.e. a dead-end or a unit decision.
       * @return DecisionPointId::noId() if there is no such flaw, otherwise the decision point to take next.
       */
      DecisionPointId getZeroCommitmentDecision();
      
      bool doSolve(unsigned int maxSteps, unsigned int maxDepth);
      void doStep();

      static void cleanup(DecisionStack& decisionStack);

    private:

      /**
       * @brief Internal utility to cleanup all decision data
       * @see cleanup(DecisionStack& decisionStack)
       */
      void cleanupDecisions();

      void notifyAdded(const TokenId& token);

      void notifyRemoved(const TokenId& token);
      
			bool isDecided(const EntityId& entity);

			bool hasDecidedParameter(const TokenId& token);

      /**
       * @brief Used to enforce scope restrictions for common filters across all flaw managers
       */
      class MasterFilter: public FlawManager {
      public:
	MasterFilter(const TiXmlElement& configData): FlawManager(configData){}
	IteratorId createIterator() {return IteratorId::noId();}
      private:
	friend class Solver;
	void handleInitialize(){}

	/**
	 * @brief Over-ride standard behavior to do nothing
	 */
// 	void notifyRemoved(const ConstrainedVariableId& var){}
// 	void notifyRemoved(const ConstraintId& var) {}
// 	void notifyRemoved(const TokenId& tok) {FlawManager::notifyRemoved(tok);}
      };

      /* Data Members */
      SolverId m_id;
      LabelStr m_name;
      const PlanDatabaseId m_db;
      DecisionPointId m_activeDecision; /*!< Stores the decision point we are currently working on. 
					  It should not be in the decision stack. */
      unsigned int m_stepCountFloor; /*!< Stores previous step count where multiple solution iterations are tried. */
      unsigned int m_depthFloor; /*!< Stores previous depth where multiple solution iterations are tried.*/
      unsigned int m_stepCount; /*!< The aggregate number of steps over one or more calls to solve, 
				  without reseting or clearing. */
      bool m_noFlawsFound; /*!< True when we have no more flaws to work on. */
      bool m_exhausted; /*!< True when we if and only if we have failed to recover from a dead-end */
      bool m_timedOut;/*!< True of the depth or step limits are exceeded */
      unsigned int m_maxSteps; /*!< The maximum number of steps to take.  Used only for planner control.*/
      unsigned int m_maxDepth; /*!< The maximum depth to search.  Used only for planner control.*/
      MasterFilter m_masterFlawFilter; /*!< Used to handle shared filter data across contained flaw managers */
      ContextId m_context; /*<! Used to share data from the Solver on down.*/
      FlawManagers m_flawManagers; /*!< Sequence of flaw managers to include in scope */
      DecisionStack m_decisionStack; /*!< Stack of decisions made */
      std::string m_lastExecutedDecision; /*!< Kept for debugging and UI purposes */
      std::list<SearchListenerId> m_listeners; /*!< The set of listeners for the search */

      class FlawIterator : public Iterator {
      public:
	FlawIterator(const FlawManagers& flawManagers);
	~FlawIterator();
	bool done() const;
	const EntityId next();
	unsigned int visited() const {return m_visited;}
      protected:
      private:
	unsigned int m_visited;
	std::list<IteratorId> m_iterators;
	std::list<IteratorId>::iterator m_it;
      };
      

      /**
       * @brief Plugs manager into ConstraintEngine events to synchronize flaw candidates
       */
      class CeListener: public ConstraintEngineListener {
      public:
	CeListener(const ConstraintEngineId& ce, Solver& dm);

	void notifyRemoved(const ConstrainedVariableId& variable);
	void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);
	void notifyAdded(const ConstraintId& constraint);
	void notifyRemoved(const ConstraintId& constraint);

      private:
	Solver& m_solver;
      };

      void notifyRemoved(const ConstrainedVariableId& variable);
      void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);
      void notifyAdded(const ConstraintId& constraint);
      void notifyRemoved(const ConstraintId& constraint);

      friend class Solver::CeListener;
      Solver::CeListener m_ceListener; /*!< For Processing constraint engine events */

      class DbListener : public PlanDatabaseListener {
      public:
	DbListener(const PlanDatabaseId& db, Solver& dm);
	void notifyRemoved(const TokenId& token);
	void notifyAdded(const TokenId& token);
      private:
	Solver& m_solver;
      };
      
      friend class Solver::DbListener;
      Solver::DbListener m_dbListener;
    };
  }
} 
#endif
