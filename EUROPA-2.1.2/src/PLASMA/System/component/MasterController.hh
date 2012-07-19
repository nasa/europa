#ifndef H_MasterController
#define H_MasterController

/**
 * @author Michael Iatauro, Conor McGann
 * @brief A common base class providing PlanWorks integration support.
 */

#include "PlannerControlIntf.hh"
#include "SolverDefs.hh"
#include "RulesEngineDefs.hh"
#include "EuropaEngine.hh"
#include <string>
#include <list>

namespace EUROPA {

  class MasterController;
  
  class MasterControllerFactory
  {
  public:
      virtual ~MasterControllerFactory() {}
      virtual MasterController* createInstance() = 0;
  };
  
  /**
   * @brief
   */
  class MasterController : public EuropaEngine
  {
  public:

    enum Status { IN_PROGRESS=0,
		  TIMEOUT_REACHED,
		  PLAN_FOUND,
		  SEARCH_EXHAUSTED,
		  INITIALLY_INCONSISTENT
    };

    static MasterControllerFactory* s_factory;
    
    /**
     * @brief Implement this function to link to specific concrete class.
     */
    static MasterController* createInstance();

    virtual ~MasterController();
    
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

    /**
     * @brief Complete planning, writing out the last step only
     */
    Status complete();

    /**
     * @brief Termination
     */
    static void terminate();

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
     * @brief Utility for path extraction
     */
    static std::string extractPath(const char* configPath);

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
    SOLVERS::PlanWriter::PartialPlanWriter* m_ppw;

    static MasterController* s_instance;
  };
}
#endif
