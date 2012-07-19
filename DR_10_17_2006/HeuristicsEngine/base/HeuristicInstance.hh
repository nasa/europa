#ifndef H_HeuristicInstance
#define H_HeuristicInstance

#include "HSTSDefs.hh"
#include "Constraint.hh"
#include "ConstraintEngine.hh"
#include "Entity.hh"

#include <vector>

/**
 * @brief Declares the HeuristicsEngine
 * @author Conor McGann
 */

namespace EUROPA {


  class HeuristicInstance: public Entity {
  public:

    /**
     * @brief Constructor for the single target case
     */
    HeuristicInstance(const TokenId& token,
		      int target,
		      const HeuristicId& heuristic, 
		      const HeuristicsEngineId& he);

    /**
     * @brief Constructor for the multi-target case
     */
    HeuristicInstance(const TokenId& token, 
		      const std::vector<int>& targets,
		      const TokenId& guardSource,
		      const HeuristicId& heuristic, 
		      const HeuristicsEngineId& he);

    ~HeuristicInstance();

    const HeuristicInstanceId& getId() const;

    /**
     * @brief Accessor for the token on which the heuristic applies.
     */
    const TokenId& getToken() const;

    /**
     * @brief Get the target keys to which the rule will apply a priority.
     */
    const std::vector<int>& getTargets() const;

    /**
     * @brief Test if the instance has been executed i.e. all its guards satisfied.
     */
    bool isExecuted() const;

    /**
     * @brief Get the prority
     * @note must be fired
     */
    const Priority& getPriority() const;

    /**
     * @brief Evaluate the number of criteria satisfied. Basically a weight.
     */
    double getWeight() const;

    /**
     * @brief Accessor to original heuristic
     */
    const HeuristicId& getHeuristic() const;

    /**
     * @brief Stream contents to a string
     */
    std::string toString() const;

    void resetListener();

    class VariableListener: public Constraint {
    public:
      /**
       * @brief Standard constraint constructor must be provided to facilitate
       * creation of a copy during merging.
       */
      VariableListener(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine, 
		       const std::vector<ConstrainedVariableId>& variables);

      /**
       * @brief Specilized constructor also provided to create from the Heuristics Engine
       */
      VariableListener(const ConstraintEngineId& ce,
		       const HeuristicInstanceId& instance,
		       const std::vector<ConstrainedVariableId>& scope);

      virtual ~VariableListener();

      /**
       * @brief Standard constraint name
       */
      static const LabelStr& CONSTRAINT_NAME(){
	static const LabelStr sl_const("HeuristicVariableListener");
	return sl_const;
      }

      /**
       * @brief Standard constraint name
       */
      static const LabelStr& PROPAGATOR_NAME(){
	static const LabelStr sl_const("Default");
	return sl_const;
      }

      void resetInstance();

    private:
      void handleDiscard();

      void handleExecute();

      HeuristicInstanceId m_instance;
    };


  private:
    friend class HeuristicsEngine;
    friend class Heuristic;
 
    /**
     * @brief Tests if the instance criteria are satisfied.
     */
    bool test() const;

    /**
     * @brief Execute the rule
     */
    void execute();

    /**
     * @brief Undo the rule
     */
    void undo();

    /**
     * @brief Helper method to stream targets to a string.
     */
    static std::string HeuristicInstance::targetsToString(const std::vector<int>& targets);

    /**
     * @brief Helper method to validate target keys.
     */
    static bool HeuristicInstance::validTargets(const std::vector<int>& targets);

    /**
     * @brief Help method to constructors
     */
    void commonInit();

    HeuristicInstanceId m_id;
    const TokenId m_token;
    std::vector<int> m_targets;
    const TokenId m_guardSource;
    const HeuristicId m_heuristic;
    const HeuristicsEngineId m_he;
    bool m_isExecuted;
    ConstraintId m_variableListener;
  };
}

#endif
