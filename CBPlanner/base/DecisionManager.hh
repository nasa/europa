#ifndef _H_DecisionManager
#define _H_DecisionManager


#include "CBPlannerDefs.hh"
#include "Condition.hh"
#include "DecisionPoint.hh"
#include "DecisionManagerListener.hh"
#include "PlanDatabase.hh"
#include <vector>
#include <set>
#include <string>
namespace EUROPA {

  class DecisionManager : public Entity {
  public:

    DecisionManager(const PlanDatabaseId& db, const OpenDecisionManagerId& odm);
    ~DecisionManager();

    const DecisionManagerId& getId() const;
    const PlanDatabaseId& getPlanDatabase() const;
    const OpenDecisionManagerId& getOpenDecisionManager() const;

    /**
     * @brief Retrieve the number of decisions available. Should
     * only be used for testing
     * @return A number of open decisions available
     */
    unsigned int getNumberOfDecisions() const;

    bool isVariableDecision(const ConstrainedVariableId& var) const;
    bool isTokenDecision(const TokenId& token) const;
    bool isObjectDecision(const TokenId& token) const;

    /**
     * @brief Retrieve all decisions on the stack.
     */
    const DecisionStack& getClosedDecisions() const;

    /**
     * @brief Reset the search state data without rolling back any decisions
     * @see retract
     */
    void reset();

    /**
     * @brief Reset the search state data and roll back all commitments.
     * @see reset
     */
    void retract();

    /**
     * @brief Access the current decision.
     */
    const DecisionPointId& getCurrentDecision() const;

    bool assignDecision();
    bool retractDecision(unsigned int& retractCount);
    bool isRetracting() const;
    bool hasDecisionToRetract() const;
    void plannerSearchFinished() const;
    void plannerTimeoutReached() const;

    // condition related methods
    std::vector<ConditionId> getConditions() const;
    ConditionId getCondition(const int pos) const;

    void add(const DecisionManagerListenerId& listener);
    void remove(const DecisionManagerListenerId& listener);

    void print(std::ostream& os = std::cout);
    void printOpenDecisions(std::ostream& os = std::cout);
    void printClosedDecisions(std::ostream& os = std::cout);

  private:
    friend class DecisionManagerListener;
    friend class Condition;

    /**
     * @brief Sets the current decision from the last decision on the stack
     */
    void popLastDecision();

    // condition methods
    void attach(const ConditionId& cond);
    void detach(const ConditionId& cond);

    // Miscellaneous helper methods
    bool isValid();
    bool propagate();

    DecisionManagerId m_id;
    PlanDatabaseId m_db;
    OpenDecisionManagerId m_odm;

    DecisionPointId m_curDec; /*!< Stores the decision point we are currently working on. 
				It should not be in the decision stack.*/
    bool m_retracting; /*!< True when we are in backtrack mode, i.e. trying to roll-back decisions */
    DecisionStack m_closedDecisions; /*!< The stack of closed decision */
    std::set<DecisionManagerListenerId> m_listeners; /**< listen to DM's events */
  };

  std::string makeStr(const std::list<DecisionPointId>& decs);
}

#endif
