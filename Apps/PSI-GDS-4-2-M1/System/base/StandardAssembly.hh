#ifndef _H_StandardAssembly
#define _H_StandardAssembly

/**
 * @file StandardAssembly.hh
 * @author Conor McGann
 */

#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"

namespace EUROPA {

  /**
   * @brief Provides a simple facade over standard components.
   *
   * The standard assembly includes all major EUROPA components for the PlanDatabase:
   * @li ConstraintEngine
   * @li PlanDatabase
   * @li RulesEngine
   * @li Resources
   * @li TemporalNetwork
   */
  class StandardAssembly {
  public:

    /**
     * @brief Sets up the necessary constraint factories. The assembly must be explicitly
     * initialized before use.
     */
    static void initialize();

    /**
     * @brief Allows explicit clean-up of static members created during initialization.
     * @see initialize
     */
    static void terminate();

    /**
     * @brief Constructor
     * @param schema The model for type enforcement.
     */
    StandardAssembly(const SchemaId& schema);

    /**
     * @brief Deallocate all data associated with this instance.
     */
    virtual ~StandardAssembly();

    /**
     * @brief Play Transactions stored in the given file
     */
    bool playTransactions(const char* txSource, bool interp = false);

    /**
     * @brief Writes the current state of the plan database to the
     * given stream
     */
    void write(std::ostream& os) const;

    /**
     * @brief Writes the current state of the plan database to a string and returns the string
     */
    std::string toString() const;

    /**
     * The following set of methods are used by the PlannerControl
     * interface for PlanWorks.
     */
    const PlanDatabaseId& getPlanDatabase() const { return m_planDatabase; }

    const ConstraintEngineId& getConstraintEngine() const {return m_constraintEngine;}

    const RulesEngineId& getRulesEngine() const {return m_rulesEngine;}

  protected:
    StandardAssembly();
    /**
     * @brief Provides accessor for static variable indicating if the static members,
     * primarily related to various factories have been initialized e.g.: Rules, Constraints, 
     * Token Types, Object Types and domain types.
     */
    static bool& isInitialized();

    ConstraintEngineId m_constraintEngine; /*!< A Constraint Engine for propagation of relations */
    PlanDatabaseId m_planDatabase; /*!< A PlanDatabase as central state representation */
    RulesEngineId m_rulesEngine; /*!< A Rules Engine to enforce model rules. */
    DbClientTransactionPlayer* m_transactionPlayer;
  };
}
#endif
