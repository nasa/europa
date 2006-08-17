#ifndef H_HeuristicsEngine
#define H_HeuristicsEngine

#include "HSTSDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabaseListener.hh"
#include "Heuristic.hh"

#include <map>

/**
 * @brief Declares the HeuristicsEngine
 * @author Conor McGann
 * @note Very similar pattern to the RulesEngine module
 * @see Heuristic, HeuristicInstance
 */

namespace EUROPA {

  /**
   * @brief An ordered list of weighted, valid, heuristic instances. All should be fired.
   */
  typedef std::multimap<double, HeuristicInstanceId> HeuristicEntry;

  /**
   * @brief Mediator to co-ordinate incremental computation of heuristic priorities and choices.
   */
  class HeuristicsEngine {

  public:
    HeuristicsEngine(const PlanDatabaseId& planDatabase);

    ~HeuristicsEngine();

    const HeuristicsEngineId& getId() const;

    const PlanDatabaseId& getPlanDatabase() const;

    /**
     * @brief Test if the engine has been initiaized w.r.t. instances.
     */
    bool isInitialized() const;

    /**
     * @brief Use to initialize the engine before use. Will populate for existing 
     * tokens in the database.
     */
    void initialize();


    /**
     * @brief Called to add a new heuristic. No heuristic instances should be present.
     */
    void add(const HeuristicId& heuristic, const LabelStr& predicate = ANY_PREDICATE());

    /**
     * @brief Called when a heuristic is removed. No heuristic instances should be present.
     */
    void remove(const HeuristicId& heuristic);

    /**
     * @brief Get all registered heuristic rules
     */
    const std::multimap<double, HeuristicId>& getHeuristics() const;

    /**
     * @brief Get all allocated heuristic instances. May or may not be fired.
     */
    const std::multimap<int, HeuristicInstanceId>& getHeuristicInstances() const;

    /* QUERIES AND OPERATIONS FOR FLAW AND CHOICE ORDERING
       Constraint Engine Should be propagated  and constistent */

    const Priority& bestCasePriority() const;

    const Priority& worstCasePriority() const;

    bool betterThan(const Priority p1, const Priority p2) const;

    Priority getPriority(const EntityId& entity) const;

    Priority getDefaultVariablePriority() const;

    Priority getDefaultTokenPriority() const;

    const bool& preferNewerEntities() const;

    const VariableHeuristic::DomainOrder& getDefaultDomainOrder() const;

    void orderChoices(const ConstrainedVariableId& var, std::list<double>& choicesToOrder) const;

    void orderChoices(const TokenId& token, std::vector<LabelStr>& states, std::vector<TokenId>& choicesToOrder) const;

    void orderChoices(const TokenId& token, std::vector< OrderingChoice >& choicesToOrder ) const;

    /* OLD INTERFACES FOR INTEGRATION WITH EXISTING CODE W.R.T. DEFAULTS */

    void setDefaultCreationPreference(bool preferNewer);

    void setDefaultPriorityPreference(bool preferLowPriority);

    void setDefaultPriorityForToken(const Priority p, 
				    const LabelStr& pred, 
				    const std::vector< GuardEntry >& domainSpec);

    void setDefaultPriorityForToken(const Priority p);

    void setDefaultPriorityForConstrainedVariable(const Priority p);

    void setDefaultPreferenceForToken(const std::vector<LabelStr>& states, 
					 const std::vector<TokenHeuristic::CandidateOrder>& orders);

    void setDefaultPreferenceForConstrainedVariable(const VariableHeuristic::DomainOrder order);

  private:

    /**
     * @brief A callback from the HeuristicInstance when ALL its conditions are satisfied.
     */
    void notifyExecuted(const HeuristicInstanceId &instance);

    /**
     * @brief A callback from the HeuristicInstance when ANY of its conditions are not satisfied.
     */
    void notifyUndone(const HeuristicInstanceId &instance);

    /**
     * @brief Accessor for the constrant engine. Required for instance allocation.
     */
    ConstraintEngineId getConstraintEngine() const;

    /**
     * @brief Invoked when the given token is initially added to the Plan Database. This is the
     * trigger point to commence computation of heuristics.
     */
    void notifyAdded(const TokenId& token);

    /**
     * @brief Tail-recrusive procedure to go up the ancestor chain to get all relevant heuristics.
     */
    void handleAddition(const TokenId& token, const LabelStr& predicate);

    /**
     * @brief Invoked when the given tken is removd from the database.
     */
    void notifyRemoved(const TokenId& token);

    /**
     * @brief Plugs into Plan Database events track token activity.
     */
    class DbListener: public PlanDatabaseListener {
    public:
      DbListener(const PlanDatabaseId& db, const HeuristicsEngineId& he);

    private:
      void notifyAdded(const TokenId& token);
      void notifyRemoved(const TokenId& token);

      const HeuristicsEngineId m_he;
    };

    HeuristicInstanceId getActiveHeuristic(const EntityId& target) const;

    /**
     * @brief Helper method to get the order corresponding to the given state. Will fail if the state is not present.
     */
    static TokenHeuristic::CandidateOrder getOrderForState(const LabelStr& state,
							   const std::vector<LabelStr>& states,
							   const std::vector<TokenHeuristic::CandidateOrder>& orders);

    /**
     * @Brief Helper methd for deallocation safely
     */
    static void killIt(const HeuristicInstanceId& heuristicInstance);

    /**
     * @brief Utility to test if a HeuristicInstance exists in a given multi-map.
     */ 
    bool alreadyAllocated(const TokenId& token, const HeuristicId& heuristic) const;

    /**
     * @brief Validate the integrity of internal data structures
     */
    bool isValid() const;

    /**
     * @brief Utility to add a heuristic to the fired heuristics for a given target
     * @see notifyExecuted
     */
    void addHeuristicForTarget(int targetKey, const HeuristicInstanceId& instance);

    /**
     * @brief Utility to add a heuristic to the fired heuristics for a given target
     * @see notifyUndone
     */
    void removeHeuristicForTarget(int targetKey, const HeuristicInstanceId& instance);

    /* Friends are part of the collaboration pattern */
    friend class DbListener;
    friend class HeuristicInstance;
    friend class Heuristic;

    HeuristicsEngineId m_id;
    const PlanDatabaseId m_planDb;
    bool m_deleted; /*!< True of we are in the destructor. Used to turn off buffer updates on removal of Heuristics */
    bool m_initialized; /*!< True if we have initialized the engine with tokens in the database */
    PlanDatabaseListenerId m_planDbListener; /*!< Connection to the Plan Database to listen to token events */
    std::multimap<double, HeuristicId> m_heuristicsByPredicateKey; /*!< All heuristic by predicate (encoded) */
    std::multimap<int, HeuristicInstanceId> m_heuristicInstancesByToken; /*!< All active instances by token key */
    std::map<int, HeuristicEntry > m_firedHeuristics; /*!< Those that apply - by entity */
    bool m_preferLowPriority;
    bool m_preferNewOverOld;

    /* Default Data */
    Priority m_defaultTokenPriority;
    Priority m_defaultVariablePriority;
    VariableHeuristic::DomainOrder m_defaultDomainOrder;
    std::vector<LabelStr> m_defaultStates; 
    std::vector<TokenHeuristic::CandidateOrder> m_defaultOrders;

    DECLARE_STATIC_CLASS_CONST(LabelStr, ANY_PREDICATE, "ANY_PREDICATE");
    DECLARE_STATIC_CLASS_CONST(double, CONST_MIN_PRIORITY, MINUS_INFINITY - 1);
    DECLARE_STATIC_CLASS_CONST(double, CONST_MAX_PRIORITY, PLUS_INFINITY + 1);
  };

}

#endif
