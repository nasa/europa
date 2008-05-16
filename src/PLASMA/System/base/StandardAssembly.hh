#ifndef _H_StandardAssembly
#define _H_StandardAssembly

/**
 * @file StandardAssembly.hh
 * @author Conor McGann
 */

#include "EuropaEngineBase.hh"
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
  class StandardAssembly : public EuropaEngineBase {
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

  protected:
    StandardAssembly();
  };
}
#endif
