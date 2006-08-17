#ifndef _H_OpenDecisionManager
#define _H_OpenDecisionManager

/**
 * @author Conor McGann
 * @note Derived mostly from the Solver module. Less factored for customization of different elements
 * independently and less support for efficient search for next decision.
 */


#include "CBPlannerDefs.hh"
#include "PlanDatabaseListener.hh"
#include "ConstraintEngineListener.hh"
#include "HSTSDefs.hh"
#include "EntityIterator.hh"

namespace EUROPA {


  /**
   * @brief A ConditionSet is defined to impose a key based ordering so that it will be repeatable and
   * the order of evaluation can be controlled based on order of allocation.
   */
  typedef std::set<ConditionId, EntityComparator<ConditionId> > ConditionSet;

  class OpenDecisionManager : public Entity {
  public:

    /**
     * @brief Constructor. Will auto allocate the heuristics engine.
     */
    OpenDecisionManager(const PlanDatabaseId& db);

    /**
     * @brief Constructor. Takes a heuristics engine externally managed.
     */
    OpenDecisionManager(const PlanDatabaseId& db, const HeuristicsEngineId& he);

    virtual ~OpenDecisionManager();

    const OpenDecisionManagerId& getId() const;

    virtual DecisionPointId getNextDecision();

    /**
     * @brief Will populate the set of flaw candidates for the given flaw set. Will
     * only execute the update if needed. This is required for now since conditions cannot be allocated
     * to an OpenDecisionManager at the construction time of the latter
     */
    void initializeIfNeeded();

    /**
     * @brief Retreive the total number of decisions that are in scope
     */
    unsigned int getNumberOfDecisions() const;

    /**
     * @brief Used to obtain a Zero commitment decision. Tried first.
     */
    virtual DecisionPointId getZeroCommitmentDecision();

    /**
     * @brief Test if the give variable is a compat guard.
     */
    bool isCompatGuard(const ConstrainedVariableId& var) const;

    /**
     * @brief Test if the given variable is a unit
     */
    bool isUnitDecision(const ConstrainedVariableId& var) const;

    /**
     * @brief Test if the given variable is a variable decision
     */
    bool isVariableDecision(const ConstrainedVariableId& var) const;


    /**
     * @brief Test if the given token is a token decision
     */
    bool isTokenDecision(const TokenId& token) const;

    /**
     * @brief Test if the given token is a threat
     */
    bool isObjectDecision(const TokenId& token) const;

    /**
     * @brief Print open decisions to a stream
     * @see printOpenDecisions()
     */
    void printOpenDecisions(std::ostream& os);

    /**
     * @brief Print open decisions to a string and return the string. Useful in debug statements.
     */
    std::string printOpenDecisions() const;

    /**
     * @brief Will delegate to sub-type specific initialization code which may be over-ridden.
     * @todo should be private but used in HSTS for testing
     */
    void initializeChoices(const DecisionPointId& dec);

    /**
     * @brief Will delecate to sub-type specific factory methods.
     */
    DecisionPointId createDecisionPoint(const EntityId& flaw);

    /**
     * @brief Custmization point to prefere higher keys
     */
    virtual bool preferHigherKeys() const;

    void print(std::ostream& os = std::cout);

    /* Condition handling */
    void attach(const ConditionId& cond);
    void detach(const ConditionId& cond);
    std::vector<ConditionId> getConditions() const;
    ConditionId getCondition(const int pos) const;

    /**
     * @brief Utility class to factor out choice counting and key ordering enforcement
     */
    class Evaluator {
    public:
      virtual ~Evaluator(){}
      virtual unsigned int countChoices(const EntityId& candidate) const = 0;
      bool preferHigherKeys() const {return m_preferHigherKeys;}

    protected:
      Evaluator(bool preferHigherKeys):m_preferHigherKeys(preferHigherKeys){}

    private:
      const bool m_preferHigherKeys;
    };

  protected:

    /**
     * @brief Retrieve a decision that beats the given current best priority.
     * @param bestp The current best priority. May update this if we find a better decision.
     * @return A noId if no better option, otherwise a good decision point that needs initilization.
     */
    virtual DecisionPointId getBestTokenDecision(Priority& bestp, const TokenSet& candidates);

    /**
     * @brief Retrieve a decision that beats the given current best priority.
     * @param bestp The current best priority. May update this if we find a better decision.
     * @param candidates The collection of compat or non-compat non singleton decisions
     * @return A noId if no better option, otherwise a good decision point that needs initilization.
     */
    virtual DecisionPointId getBestVariableDecision(Priority& bestp,
						    const std::vector<ConstrainedVariableId>& candidates);


    /* DecisionPoint allocation and initialization methods. These are factored out in the Solver framework to
       be plugins.*/
    virtual void initializeTokenChoices(const TokenDecisionPointId& tdp);
    virtual void initializeVariableChoices(const ConstrainedVariableDecisionPointId& vdp);
    virtual void initializeObjectChoices(const ObjectDecisionPointId& odp);
    virtual ObjectDecisionPointId createObjectDecisionPoint(const TokenId& token);
    virtual TokenDecisionPointId createTokenDecisionPoint(const TokenId& token);
    virtual ConstrainedVariableDecisionPointId createConstrainedVariableDecisionPoint(const ConstrainedVariableId& variable);

    /* Immutable accessors to the flaw data */
    const ConstrainedVariableSet& getUnitVariableFlawCandidates() const;
    const ConstrainedVariableSet& getVariableFlawCandidates() const;
    const TokenSet& getTokenFlawCandidates() const;

    /**
     * Helper method to swap one condition with another, and discard old one.
     */
    static void updateDecisionPoints(DecisionPointId& candidate, DecisionPointId& best);

    /**
     * Helper method to evaluate entity w.r.t static conditions only. Encapslates condition access.
     */
    virtual bool passesStaticConditions(const EntityId& entity) const;

    /**
     * Helper method to evaluate entity w.r.t dynamic conditions only. Encapslates condition access.
     */
    virtual bool passesDynamicConditions(const EntityId& entity) const;

    /**
     * Helper method to establish the base set of token choices. No Pruning.
     */
    void initializeTokenChoicesInternal(const TokenDecisionPointId& tdp);

    /**
     * Helper method to establish the base set of object choices. No Pruning.
     */
    void initializeObjectChoicesInternal(const ObjectDecisionPointId& odp);

    const PlanDatabaseId m_db;

  private:

    static bool passesConditions(const ConditionSet& conditions, const EntityId& entity);

    /**
     * @brief Helper method to populate candiate flaw set from scratch. Will clear currentvalues
     */
    void populateFlawCandidates();

    /**
     * @brief Plugs into Plan Database events on the Plan
     * Database to synchronize flaw candidates.
     */
    class DbListener: public PlanDatabaseListener {
    public:
      DbListener(const PlanDatabaseId& db,
		 OpenDecisionManager& dm);

    private:
      void notifyAdded(const TokenId& token);
      void notifyRemoved(const TokenId& token);
      void notifyActivated(const TokenId& token);
      void notifyDeactivated(const TokenId& token);
      void notifyMerged(const TokenId& token);
      void notifySplit(const TokenId& token);
      void notifyRejected(const TokenId& token);
      void notifyReinstated(const TokenId& token);

      OpenDecisionManager& m_odm;
    };

    friend class OpenDecisionManager::DbListener;

    void addFlaw(const TokenId& token);
    void removeFlaw(const TokenId& token);

    /* TOKEN FLAW HANDLING */
    TokenSet m_tokenFlawCandidates; /*!< The set of candidate token flaws */

    /**
     * @brief Plugs manager into ConstraintEngine events to
     * synchronize flaw candidates
     * @note We do not process notifyAdded as the variable is not built yet. This means we
     * are not handling the case of a variable being created with an open domain. This is fine
     * for token variables since they are triggered elsewhere, but it means that we do not ever support decisions
     * on global variables with dynamic domains.
     */
    class CeListener: public ConstraintEngineListener {
    public:
      CeListener(const ConstraintEngineId& ce, 
		 OpenDecisionManager& odm);

      void notifyRemoved(const ConstrainedVariableId& variable);
      void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);
      void notifyAdded(const ConstraintId& constraint);
      void notifyRemoved(const ConstraintId& constraint);

    private:
      OpenDecisionManager& m_odm;
    };

    friend class OpenDecisionManager::CeListener;

    void updateFlaw(const ConstrainedVariableId& var);
    void removeFlaw(const ConstrainedVariableId& var);
    bool variableCachesAreValid() const;
    void addGuard(const ConstrainedVariableId& var);
    void removeGuard(const ConstrainedVariableId& var);
    void handleConstraintAddition(const ConstraintId& constraint);
    void handleConstraintRemoval(const ConstraintId& constraint);

    /**
     * @brief Utility to test if the given variable is part of a token that is merged, rejected or inactive.
     */
    static bool variableOfNonActiveToken(const ConstrainedVariableId& var);

    /**
     * @brief Utility to test of the given variable is decideable.
     */
    static bool isDecidable(const ConstrainedVariableId& var);

    /**
     * @brief Utility to test if the given variable is a temporal variable.
     */
    static bool isTemporalVariable(const EntityId& entity);

    /**
     * @brief Utility to test if the variable is buffered in either variable flaw candidate buffer
     */
    bool isBuffered(const ConstrainedVariableId& var) const;

    /**
     * @brief Helper method for doing the right thing for flaw calclation
     */
    bool isFlaw(const EntityId& candidate) const;


    /**
     * @brief Main candidate selection method for a given collection
     * @param p The current best priority
     * @param it An iterator to iterate through the candidates
     * @param evlaluator An evaluation function to count the choices for the candidate
     */
    DecisionPointId getBestCandidate(Priority& p, 
				     Iterator& it, 
				     const Evaluator& evaluator);

    OpenDecisionManagerId m_id;
    OpenDecisionManager::DbListener* m_dbListener; /**< to listen to events that may affect decisions. */
    OpenDecisionManager::CeListener* m_ceListener; /**< to listen to events that may affect decisions. */
    ConditionSet m_staticConditions;
    ConditionSet m_dynamicConditions;

    /* VARIABLE FLAW HANDLING */
    ConstrainedVariableSet m_unitVariableFlawCandidates; /*!< All singleton variables flaw candidates */
    ConstrainedVariableSet m_variableFlawCandidates; /*!< All variables that have passed the static filter */
    std::map<ConstrainedVariableId, unsigned int> m_guardCache; /*!< Cache of variables that are
								  guarded. Includes reference counts. */

    bool m_initialized; /*!< Tracks if it has been initialized with flaws. */

    HeuristicsEngineId m_he; /*!< Used to compute priorities */ 
    bool m_createdHE; /*!< To determine if we need to deallocate in destructor */
  };

}

#endif
