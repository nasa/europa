#ifndef _H_HSTSAssembly
#define _H_HSTSAssembly

/**
 * @file   HSTSAssembly.hh
 * @author Tania Bedrax-Weiss <tania@email.arc.nasa.gov>
 * @date   Mon Dec 27 17:00:11 2004
 * @brief  
 * @ingroup HSTS
 */

#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"
#include "CBPlanner.hh"

namespace EUROPA {

  /**
   * @brief Provides a simple facade over standard components. 
   * Require a database with exactly 1 instance of PlannerConfig
   *
   * The standard assembly includes all major EUROPA components for the PlanDatabase:
   * @li ConstraintEngine
   * @li PlanDatabase
   * @li RulesEngine
   * @li Resources
   * @li TemporalNetwork
   * @li Chronological backtracking planner
   */
  class HSTSAssembly {
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
    HSTSAssembly(const SchemaId& schema);

    /**
     * @brief Deallocate all data associated with this instance.
     */
    virtual ~HSTSAssembly();

    /**
     * @brief Play Transactions stored in the given file
     */
    bool playTransactions(const char* txSource);

    /**
     * @brief Writes the current state of the plan database to the
     * given stream
     */
    void write(std::ostream& os) const;

    /**
     * @brief Invoke the planner. Calls playTransactions(txSource).
     * @param txSource The source from which we get the initial state
     * @return The result of planning
     * @see CBPlanner::Status
     */
    CBPlanner::Status plan(const char* txSource, const char* heurSource, const char* pidSource);

  const unsigned int getTotalNodesSearched() const;
  const unsigned int getDepthReached() const;

  protected:
    /**
     * @brief Provides accessor for static variable indicating if the static members,
     * primarily related to various factories have been initialized e.g.: Rules, Constraints, 
     * Token Types, Object Types and domain types.
     */
    static bool& isInitialized();

    ConstraintEngineId m_constraintEngine; /*!< A Constraint Engine for propagation of relations */
    PlanDatabaseId m_planDatabase; /*!< A PlanDatabase as central state representation */
    RulesEngineId m_rulesEngine; /*!< A Rules Engine to enforce model rules. */
  private:
    unsigned int m_totalNodes;
    unsigned int m_finalDepth;
  };

  inline std::ostream& operator<<(std::ostream& outStream, const HSTSAssembly& plan) {
    plan.write(outStream);
    return(outStream);
  }

} /* namespace EUROPA */
#endif
