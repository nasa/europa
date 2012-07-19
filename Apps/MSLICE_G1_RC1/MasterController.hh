#ifndef H_MasterController
#define H_MasterController

/**
 * @author Michael Iatauro, Conor McGann
 * @brief A common base class providing PlanWorks integration support.
 */

#include "PlannerControlIntf.hh"
#include "SolverDefs.hh"
#include "RulesEngineDefs.hh"
#include <string>
#include <list>

namespace EUROPA {
  /**
   * @brief
   */
  class MasterController {
  public:

    enum Status { IN_PROGRESS=0,
		  TIMEOUT_REACHED,
		  PLAN_FOUND,
		  SEARCH_EXHAUSTED,
		  INITIALLY_INCONSISTENT
    };

    /**
     * @brief Implement this function to link to specific concrete class.
     */
    static MasterController* createInstance();

    /**
     * @brief Loads the model.
     */
    void loadModel(const char* libPath);

    /**
     * @brief Loads the initial state
     */
    int loadInitialState(const char* configPath, 
			 const char* initialStatePath,
			 const char* destination,
			 const char** sourcePaths,
			 const int numPaths);

    /**
     * @brief Singleton accessor
     */
    static MasterController* instance();

    virtual ~MasterController();

    /**
     * @brief Complete planning, writing out the last step only
     */
    Status complete();

    /**
     * @brief Termination
     */
    static void terminate();

    const PlanDatabaseId& getPlanDatabase() const;

    Status getStatus();

    /**
     * @brief Access output destination
     */
    const std::string getDestination() const;

    /**
     * @brief Accessor for steps taken.
     */
    unsigned int getStepCount() const;

    /**
     * @brief Advances the controller a single step in the solution process.
     */
    void next();

    /**
     * @brief Write the current partial plan
     */
    void write();

    /**
     * @brief Write summary data for the current partial plan
     */
    void writeStatistics();

    /**
     * @brief Utility to print and flush a message
     */
    static void logMsg(std::string msg);

    /**
     * @brief Utility to write the database to a string
     */
    static std::string toString(const PlanDatabaseId& planDatabase);

  protected:

    MasterController();

    /**
     * @brief Called after model is loaded
     */
    virtual void handleRegistration();

    /**
     * @brief
     */
    virtual void configureDatabase();

    /**
     * @brief Utility for path extraction
     */
    static std::string extractPath(const char* configPath);

  private:

    /**
     * @brief Unloads the model
     */
    void unloadModel();

    /**
     * @brief Called after the initial transactions are loaded into the database.
     */
    virtual void configureSolvers(const char* configPath) = 0;

    /**
     * @brief Advance 1 step in the solution process
     */
    virtual Status handleNext() = 0;

    unsigned int m_stepCount;
    Status m_status;
    std::string m_destPath;
    void* m_libHandle;
    std::ofstream* m_debugStream;

    ConstraintEngineId m_constraintEngine; /*!< A Constraint Engine for propagation of relations */
    PlanDatabaseId m_planDatabase; /*!< A PlanDatabase as central state representation */
    RulesEngineId m_rulesEngine; /*!< A Rules Engine to enforce model rules. */
    SOLVERS::PlanWriter::PartialPlanWriter* m_ppw;

    static MasterController* s_instance;
  };
}
#endif
