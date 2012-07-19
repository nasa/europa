#ifndef _H_MasterMustBeInserted
#define _H_MasterMustBeInserted

/**
 * @autor Conor McGann
 * @brief Provides declaration for a filter to exclude open conditions if there is
 * a master that is not yet constrained.
 */
#include "Condition.hh"


namespace EUROPA {

  /**
   * @brief Simple condition to filter open condition flaws based on the state of the master. The
   * master must be slotted
   */
  class MasterMustBeInserted : public Condition {
  public:
    MasterMustBeInserted(const DecisionManagerId& dm);
    virtual ~MasterMustBeInserted();
   
    /**
     * @brief Used to invoke evaluation inside decision manager.
     */
    bool test(const EntityId& entity);

    /**
     * @brief Implements actual test. Can be called independently of a decision manager.
     */
    static bool executeTest(const TokenId& token);

    /**
     * @brief Implements actual test. Can be called independently of a decision manager.
     */
    static bool executeTest(const ConstrainedVariableId& var);

    inline void print (std::ostream& os) { os << "MASTER_MUST_BE_INSERTED_CONDITION"; }
  };

} /* namespace Europa */
#endif

