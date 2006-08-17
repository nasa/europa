
#ifndef _H_AverInterp
#define _H_AverInterp

/**
 * @file   AverInterp.hh
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @brief Defines the interface to the Aver interpreter
 * @date   Wed Jan 12 13:56:13 2005
 * @ingroup Aver
 */

#include "AverDefs.hh"
#include "tinyxml.h"
#include <list>
#include <map>
#include <string>
#include "EventAggregator.hh"
#include "AggregateListener.hh"
#include "SolverDefs.hh"

namespace EUROPA {

  /**
   * @class AverInterp
   * @brief Programmatic interface to the Aver interpreter.
   *
   * Interpretation of the Aver language is hidden from the user behind this fairly simple class.  There is never really an AverInterp 
   * instance.  Instead, it statically manages the instances of Tests and Assertions and calls the routines that compile Aver XML into
   * commands for the Aver machine.
   */
  class AverInterp {
  public:
    
    /**
     *@brief Initialization function.  Should be called before planning begins.  Most of the arguments are passed to 
     * EventAggregator::instance().
     *@param filePath the fully qualified path the the Aver XML file.
     *@param dm the decision manager (can be noId)
     *@param ce the constraint engine (can be noId)
     *@param db the plan database (must not be noId)
     *@param re the rules engine
     */
    static void init(const std::string& filePath, 
                     const SOLVERS::SolverId& dm,
                     const ConstraintEngineId& ce, 
                     const PlanDatabaseId& db,
                     const RulesEngineId& re);

    /**
     *@brief Initialization function.  Should be called before planning begins.  Most of the arguments are passed to 
     * EventAggregator::instance().
     *@param tests a list of the names of the tests to execute.  This function should only be used if a proper subset of the tests in the 
     * Aver XML file are to be executed.
     *@param filePath the fully qualified path the the Aver XML file.
     *@param dm the decision manager (can be noId)
     *@param ce the constraint engine (can be noId)
     *@param db the plan database (must not be noId)
     *@param re the rules engine (can be noId)
     */
    static void init(const std::string& filePath, 
                     const std::list<std::string>& tests, 
                     const SOLVERS::SolverId& dm,
                     const ConstraintEngineId& ce, 
                     const PlanDatabaseId& db, 
                     const RulesEngineId& re);

    /**
     *@brief Termination function.  Should be called after planning completes.  Will dump the test results to stderr.
     */
    static void terminate();
  protected:
  private:
    
    /**
     *@brief Performs the initialization common to both init functions.
     *@param filePath the fully-qualified path to the Aver XML file.
     *@return a pointer to the TinyXml structure representing the file.
     */
    static TiXmlElement* commonInit(const std::string& filePath);

    /**
     *@brief Removes all but the specified tests from the XML structure.
     *@param root a pointer to the root XML element.
     *@param tests the list of tests to be compiled and executed.
     */
    static void removeAllElse(TiXmlElement* root, std::list<std::string>& tests);

    /**
     *@brief Constructs a Test from the given XML element.
     *@param test a pointer to the Test XML element.
     *@return an Id representing the Test object.
     */
    static TestId buildTest(const TiXmlElement* test);
    
    /**
     *@brief Constructs an Assertion from the given XML element.
     *@param assn a pointer to the Assertion XML element.
     *@return an Id representing the Assertion element.
     */
    static AssertionId buildAssertion(const TiXmlElement* assn);

    /**
     *@brief Constructs a new or returns an existing AssertionExecutor for the given step condition.
     *@param cond the condition under which a set of Assertions should be executed.
     *@param step the XML representation of the condition
     *@return an Id representing an AssertionExecutor whose conditions are executed under the given condition.
     */
    static AssertionExecutorId getExecutor(const std::string& cond,
                                           const TiXmlElement* step);

    /**
     *@brief Constructor.  No implementation.
     */
    AverInterp(){}

    /**
     *@brief Destructor.  No implementation.
     */
    ~AverInterp(){}

    static std::map<const std::string, AssertionExecutorId> s_executors; /*!< A map from conditions to AssertionExecutors. */
    static std::list<AssertionId> s_assertions; /*!< The list of all Assertions. */
    static std::list<TestId> s_tests; /*!< The list of all Tests. */
    static std::list<TestId> s_rootTests; /*!< The list of all root Tests. */
    static TiXmlDocument* s_doc; /*!< Temporary pointer to the XML document.  Discarded after initialization. */
  };
}

#endif
