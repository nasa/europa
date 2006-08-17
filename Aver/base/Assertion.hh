#ifndef _H_Assertion
#define _H_Assertion

/**
 * @file   Assertion.hh
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @brief Defines the Assertion class--the most basic unit of a Test.
 * @date   Wed Jan 12 14:34:02 2005
 * @ingroup Aver
 */

#include "AverDefs.hh"
#include "AverHelper.hh"
#include "tinyxml.h"
#include <string>

namespace EUROPA {


  /**
   *@class Assertion
   *@brief An individual Assertion.
   *
   * Each boolean statement in an Aver test file is compiled to a set of Aver instructions which represent the queries and comparisons
   * required for the assertion.  These instructions are held in an Assertion.  When executed by an AssertionExecutor, the compiled 
   * instructions are interpreted by the AverHelper class, and in the case of failure, failure actions are taken by the Assertion.
   * At any given time, an Assertion can be incomplete (not executed) or it can have failed (the boolean assertion was
   * violated) or passed.  An Assertion that is incomplete when AverInterp::terminate() is called is considered to have failed.
   */
  class Assertion {
  public:
    enum FailMode {
      FAIL_FAST = 0, /**< If the assertion fails, the program should terminate immediately with an error. */
      FAIL_WARN, /**< If the assertion fails, the program should immediately print a warning. */
      FAIL_WAIT /**< If the assertion fails, the program should continue executing.  All failures are printed at termination. */
    };
    enum Status {
      INCOMPLETE = 0, /**< The assertion hasn't executed yet. */
      PASSED, /**< The assertion succeeded. */
      FAILED /**< The assertion failed. */
    };

    /**
     *@brief Constructor.
     *@param assn the XML representing the assertion
     *@param file the name of the file the assertion was in.  Used as part of an error report.
     *@param lineNo the line on which the assertion was defined.  Used as part of an error report.
     *@param lineText the text of the assertion as it appears on lineNo in file.  Used as part of an error report.
     */
    Assertion(TiXmlElement* assn, const std::string& file, const int lineNo, 
              const std::string& lineText);

    /**
     *@brief Destructor.
     */
    ~Assertion();

    /**
     *@brief Returns the Id for this Assertion.
     *@return the Id for this Assertion
     */
    AssertionId getId() const {return m_id;}

    /**
     *@brief Returns the Status of this Assertion.
     *@return the Status of this Assertion.
     */
    Status status(){return m_status;}

    /**
     *@brief Print a status report on the given stream.
     *@param os the output stream.  Defaults to std::cerr.
     */
    void dumpResults(std::ostream& os = std::cerr);

    /**
     *@brief Set the current failure mode to FAIL_FAST (abort immediately on failure).
     */
    static void failFast() {s_failureMode = FAIL_FAST;}

    /**
     *@brief Set the current failure mode to FAIL_WARN (warn immediately on failure).
     */
    static void failWarn() {s_failureMode = FAIL_WARN;}

    /**
     *@brief Set the current failure mode to FAIL_WAIT (store failure messages and print at end).
     */
    static void failWait() {s_failureMode = FAIL_WAIT;}
  protected:
    friend class AssertionExecutor;

    /**
     *@brief Execute this assertion.
     */
    void execute();
  private:

    /**
     * @brief Perform the appropriate failure action.
     */
    void fail();

    /**
     * @brief Get the error report as a string.
     * @return the error report.
     */
    std::string failureString();
    
    AssertionId m_id; /*!< The Id for this Assertion. */
    std::string m_file; /*!< The path to the file in which this Assertion was defined. */
    int m_lineNo; /*!< The line on which this Assertion appears */
    std::string m_lineText; /*!< The text of the Assertion. */
    AverExecutionPathId m_exec; /*!< The compiled instructions of the Assertion */
    Status m_status; /*!< The status of the Assertion */
    std::list<std::string> m_errors; /*!< The list of all errors that have occurred.  Used only in FAIL_WAIT mode. */

    static FailMode s_failureMode; /*!< The action to be performed on Assertion failure. */
  };
}

#endif
