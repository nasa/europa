#ifndef _H_Test
#define _H_Test

/**
 * @file   Test.hh
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @brief A class for managing Tests.
 * @date   Thu Jan 13 11:05:56 2005
 * @ingroup Aver
 */

#include "AverDefs.hh"

#include <iostream>
#include <string>
#include <list>

namespace EUROPA {

  /**
   *@class Test
   *@brief A class for managing tests and assertions and formatting error output.
   *
   *An Aver test file is organized into named Tests which contain other Tests and Assertions.  The Test and Assertion classes reflect that 
   *structure in the interest of timely destruction and useful error output.
   */
  class Test {
  public:
    /**
     *@brief Constructor.
     *@param name the name of this Test.
     */
    Test(const std::string& name);

    /**
     *@brief Destructor.
     */
    ~Test();

    /**
     *@brief Adds a sub-test to this Test.
     *@param test the test to add.
     */
    void add(const TestId& test) {m_tests.push_back(test);}

    /**
     *@brief Adds an Assertion to this Test.
     *@param assn the assertion to add.
     */
    void add(const AssertionId& assn) {m_assertions.push_back(assn);}

    /**
     *@brief Output the test results to an output stream.  Calls dumpResults on all sub-Tests and Assertions.
     *@param os the output stream.
     */
    void dumpResults(std::ostream& os) const;

    /**
     *@brief Gets the Id representing this test.
     *@return the Id representing this test.
     */
    TestId getId() const {return m_id;}
  protected:
  private:

    std::string m_name; /*!< The name of this Test */
    std::list<TestId> m_tests; /*!< The list of all sub-Tests. */
    std::list<AssertionId> m_assertions; /*!< The list of all Assertions in this test. */
    TestId m_id; /*!< The Id for this test.*/
  };
}

#endif
