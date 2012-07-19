#ifndef H_Heuristic
#define H_Heuristic

/**
 * @brief Declares the HeuristicsEngine
 * @author Conor McGann
 */

#include "HSTSDefs.hh"
#include "XMLUtils.hh"
#include "Entity.hh"

#include <vector>

namespace EUROPA {

  /**
   * @brief Stores the heuristic rule triggered off a Token or variable
   */
  class Heuristic: public Entity {
  public:
    /**
     * @brief Constructor for cases with no master relation
     */
    Heuristic(const HeuristicsEngineId& heuristicsEngine,
	      const LabelStr& predicate, 
	      const Priority& priority,
	      bool mustBeOrphan = false,
	      const std::vector< GuardEntry >& guards = noGuards());
    /**
     * @brief Constructor for cases qualified with a master relation
     */
    Heuristic(const HeuristicsEngineId& heuristicsEngine,
	      const LabelStr& predicate, 
	      const Priority& priority,
	      const std::vector< GuardEntry >& guards,
	      const LabelStr& masterPredicate,
	      const MasterRelation& masterRelation,
	      const std::vector< GuardEntry >& masterGuards = noGuards());

    virtual ~Heuristic();

    const HeuristicId& getId() const;

    const LabelStr& getPredicate() const;

    const MasterRelation& getMasterRelation() const;

    /**
     * @brief Get the prority
     */
    const Priority& getPriority() const;

    /**
     * @brief Retrieves a weight based on the number of criteria satisfied.
     */
    double getWeight() const;

    /**
     * @brief True if there are any guards posted on this heuristic.
     */
    bool hasGuards() const;

    /**
     * @brief Evaluates dynamic matching - i.e. guards against variables
     */
    bool test(const std::vector<ConstrainedVariableId>& scope) const;

    /**
     * @brief Test if the given token can be matched against this heuristic.
     * @see canMatch(const MasterRelation& a, const MasterRelation& b)
     */
    virtual bool canMatch(const TokenId& token) const;

    /**
     * @brief Allocate an instance for active evaluation. Pure virtau factor function.
     * @param token The token to monitor
     * @param heuristicsEngine To pass to the instance for call backs
     * @return An instance.
     */
    virtual HeuristicInstanceId createInstance(const TokenId& token) const = 0;

    /**
     * @brief Helper method to make the scope from guard data
     */
    void makeConstraintScope(const TokenId& token, std::vector<ConstrainedVariableId>& scope) const;

    /**
     * @brief Useful utility for eye-balling what the heuristic actually is.
     */
    virtual std::string toString() const;

    /**
     * @brief Handly default vector for no guards
     */
    static const std::vector< GuardEntry >& noGuards();

    static const MasterRelation DONT_CARE; /*!< No restriction - orphan or not. */
    static const MasterRelation NONE; /*!< Must be an orphan */
    static const MasterRelation BEFORE; /*!< Precise match */
    static const MasterRelation AFTER; /*!< Precise match */
    static const MasterRelation ANY; /*!< Precise match */
    static const MasterRelation OTHER; /*!< A specific relation other than BEFORE, AFTER */
    static const MasterRelation MEETS;
    static const MasterRelation MET_BY;

    /**
     * @brief Helper method to test if one relation can be matched to another. 
     * 
     * These mappings are not symmetric in most cases. In terms of specificity, 
     * relationA >= relationB.
     *
     * Examples:
     * @li canMatch(meets, meets) TRUE
     * @li canMatch(any, before) FALSE
     * @li canMatch(before, any) TRUE
     *
     * @see canMatch(const TokenId& token);
     */
    static bool canMatch(const MasterRelation& a, const MasterRelation& b);

    /**
     * @brief Helper method to do value conversions for object domains. This one does the work.
     */
    static double Heuristic::convertValueIfNecessary(const PlanDatabaseId& db,
						     const ConstrainedVariableId& guardVar,
						     const double& testValue);

    static const unsigned int WEIGHT_BASE = 100000; /*!< Used to weight active instances. 
						      Establishes upper limit on priority values.*/
  protected:

    /**
     * @brief Helper method to compute the weight based on the guard count and the priority.
     * 
     * Will help select active heuristics based on the specificity of the guard criteria and the
     * best priority after that. Specificity strictly dominates.
     * @return A positive number based on specificity and priority for picking the right heuristic
     */
    double computeWeight(unsigned int guardCount, const Priority& priority) const;

    void commonInit();

    bool matches(const ConstrainedVariableId& guardVar, const double& testValue) const;

    HeuristicId m_id;
    const HeuristicsEngineId m_heuristicsEngine;
    const LabelStr m_predicate;
    const Priority m_priority;
    double m_weight;
    const std::vector< GuardEntry > m_guards; /*!< <index, value> tuples */
    const LabelStr m_masterPredicate; /*!< Master predicate name */
    const MasterRelation m_masterRelation; /*!< Relation from master predicate */
    const std::vector< GuardEntry > m_masterGuards; /*!< <index, value> tuples */
  };

  /**
   * @brief Extends base class with data specific to variable choice ordering and pruning
   */
  class VariableHeuristic: public Heuristic {
  public:

    /**
     * @brief Indicates the type of value ordering model.
     */
    enum DomainOrder {
      VGENERATOR = 0, /*!< Some unknown kind of value generation method ? */
      ASCENDING, /*!< Ascending order, no explicit enumeration or pruning */
      DESCENDING, /*!< Descending order, no explict enumeration or pruning */
      ENUMERATION /*!< Explicitly enumerated sequence of values. */
    };


    /**
     * @brief Convenience constructor
     */
    VariableHeuristic(const HeuristicsEngineId& heuristicsEngine,
		      const LabelStr& predicate, 
		      const LabelStr& variableTarget,
		      const Priority& priority,
		      bool mustBeOrphan = false,
		      const std::vector< GuardEntry >& guards = noGuards());
    /**
     * @brief Constructor for cases with no master relation
     */
    VariableHeuristic(const HeuristicsEngineId& heuristicsEngine,
		      const LabelStr& predicate, 
		      const LabelStr& variableTarget,
		      const Priority& priority,
		      const std::list<double>& values,
		      const DomainOrder& domainOrder,
		      bool mustBeOrphan ,
		      const std::vector< GuardEntry >& guards);
    /**
     * @brief Constructor for cases qualified with a master relation
     */
    VariableHeuristic(const HeuristicsEngineId& heuristicsEngine,
		      const LabelStr& predicate, 
		      const LabelStr& variableTarget,
		      const Priority& priority,
		      const std::list<double>& values,
		      const DomainOrder& domainOrder,
		      const std::vector< GuardEntry >& guards,
		      const LabelStr& masterPredicate,
		      const MasterRelation& masterRelation,
		      const std::vector< GuardEntry >& masterGuards);

    HeuristicInstanceId createInstance(const TokenId& token) const;

    const std::list<double>& getValues() const;

    const DomainOrder& getDomainOrder() const;

    std::string toString() const;

    static const std::vector<std::string>& domainOrderStrings();

    static const std::list<double>& noValues();

    /**
     * @brief Utility to impose ordering and possible pruning on an enumerated domain based on its heuristic
     * order
     * @param var The variable whose values are being ordered
     * @param values A specific sequence of values that may be used, in which case order will be ignored
     * @param order An order indication in the event that values are not provided.
     * @param orderedChoices The input sequence of remaining values and the output. It may be pruned.
     */
    static void orderChoices(const PlanDatabaseId& db,
			     const ConstrainedVariableId& var,
			     const std::list<double>& values, 
			     const DomainOrder& domainOrder,
			     std::list<double>& orderedChoices);

  private:
    void commonInit();

    static std::string toString(const ConstrainedVariableId& var, const std::list<double>& choices);

    //VariableHeuristicId m_id;
    const LabelStr m_variableTarget;
    std::list<double> m_values;
    const DomainOrder m_domainOrder;
  };

  /**
   * @brief Specializtion to add token target information
   */
  class TokenHeuristic: public Heuristic {
  public:

    enum CandidateOrder {
      TGENERATOR = 0,
      NEAR,
      FAR,
      EARLY,
      LATE,
      MAX_FLEXIBLE,
      MIN_FLEXIBLE,
      LEAST_SPECIFIED,
      MOST_SPECIFIED,
      NONE,
      UNKNOWN /**<Do not use except to verify later assignment. */
    };


    /**
     * @brief Constructor for handy use
     */
    TokenHeuristic(const HeuristicsEngineId& heuristicsEngine,
		   const LabelStr& predicate, 
		   const Priority& priority,
		   bool mustBeOrphan = false,
		   const std::vector< GuardEntry >& guards = Heuristic::noGuards());
    /**
     * @brief Constructor for cases with no master relation
     */
    TokenHeuristic(const HeuristicsEngineId& heuristicsEngine,
		   const LabelStr& predicate, 
		   const Priority& priority,
		   const std::vector<LabelStr>& states,
		   const std::vector<CandidateOrder>& orders,
		   bool mustBeOrphan,
		   const std::vector< GuardEntry >& guards = Heuristic::noGuards());

    /**
     * @brief Constructor for cases qualified with a master relation
     */
    TokenHeuristic(const HeuristicsEngineId& heuristicsEngine,
		   const LabelStr& predicate,
		   const Priority& priority,
		   const std::vector<LabelStr>& states,
		   const std::vector<CandidateOrder>& orders,
		   const std::vector< GuardEntry >& guards,
		   const LabelStr& masterPredicate,
		   const MasterRelation& masterRelation,
		   const std::vector< GuardEntry >& masterGuards = Heuristic::noGuards());


    virtual HeuristicInstanceId createInstance(const TokenId& token) const;

    const std::vector<LabelStr>& getStates() const;

    const std::vector<CandidateOrder>& getOrders() const;

    std::string toString() const;

    static const std::vector<std::string>& candidateOrderStrings();

    static const std::vector< LabelStr >& noStates();

    static const std::vector< TokenHeuristic::CandidateOrder >& noOrders();

    /**
     * @brief Helper method to order choices with respect to a given reference token. Applies for merge and activate.
     * @param tokensToOrder the sequence of choice tokens to re-order.
     * @param referenceToken the reference token from which to compute values.
     */
    static void orderTokens(std::vector<TokenId>& choicesToOrder, 
			    const TokenId& referenceToken,
			    const TokenHeuristic::CandidateOrder& order);

    /**
     * @brief Helper method to order choices with respect to a given reference token. Applies for merge and activate.
     * @param tokensToOrder the sequence of choice tokens to re-order.
     * @param referenceToken the reference token from which to compute values.
     */
    static void orderTokens(std::vector< OrderingChoice >& choicesToOrder,
			    const TokenId& referenceToken,
			    const TokenHeuristic::CandidateOrder& order);

    /**
     * @brief Helper method to compute the distance between the given tokens base don abs of their midpoints.
     */
    static int absoluteDistance(const TokenId& a, const TokenId& b);

    /**
     * @brief Helper method to compute the midpoint time of a token based on its max temporal extent
     */
    static int midpoint(const TokenId& token);

  private:
    void commonInit();

    static std::string toString(const std::vector<TokenId>& choices);
    static std::string toString(const std::vector<OrderingChoice>& choices);

    //TokenHeuristicId m_id;
    std::vector<LabelStr> m_states;
    std::vector<CandidateOrder> m_orders;
  };

  /**
   * @brief Extends token heuristic to handle allocation of multiple
   * targets and matching with master and slave. 
   */
  class DefaultCompatibilityHeuristic: public Heuristic {
  public:

    /**
     * @brief Constructor with more restrictive use. No relationships specified.
     * @brief heuristicsEngine
     * @param masterPredicate The master predicate to match against
     * @param priority Priority
     * @param masterGuards Master Guards
     */
    DefaultCompatibilityHeuristic(const HeuristicsEngineId& heuristicsEngine,
				  const LabelStr& masterPredicate, 
				  const Priority& priority,
				  const std::vector< GuardEntry >& masterGuards = Heuristic::noGuards());

    /**
     * @brief Handles the allocation cases where the token is the master and the slave. 
     *
     * If the token is not matched by the master predicate, but is a slave of the given master, then
     * the target will be set to the token. If the token is question is matched against the master
     * then the target set will include the token and all its variables.
     */
    HeuristicInstanceId createInstance(const TokenId& token) const;


    /**
     * @brief Test if the given token can be matched against this heuristic
     *
     * Over-rides the default behaviour to match against a token whose master is a match or which is itself a match
     */
    virtual bool canMatch(const TokenId& token) const;

    std::string toString() const;
  };
}

#endif
