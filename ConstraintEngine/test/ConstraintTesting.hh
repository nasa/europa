/**
 * @file ConstraintTesting.hh
 * @author Will Edgington
 * @date December 2004
 * @brief Functions and a class to assist with testing constraint classes.
 */

#include <list>
#include <string>
#include <iostream>
#include <fstream>

#include "AbstractDomain.hh"
#include "ConstraintEngine.hh"

namespace EUROPA {

  /**
   * @brief Describes one constraint function test case for testArbitraryConstraints().
   * @see testArbitraryConstraints
   */
  class ConstraintTestCase {
  public:

    /**
     * @brief Primary constructor, requiring all of the info.
     */
    ConstraintTestCase(std::string cN, std::string fN, unsigned int l,
                       std::list<AbstractDomain*> doms)
      : m_constraintName(cN), m_fileName(fN), m_line(l),
        m_domains(doms) {
    }

    // Default copy constructor should be fine.

    const std::string m_constraintName; /**< Equal, AddEqual, etc. */
    const std::string m_fileName; /**< File containing the "source" of the test case.  Printed when test fails. */
    const unsigned int m_line; /**< Line within file of test case.  Printed when test fails. */
    const std::list<AbstractDomain*> m_domains; /**< Input and (expected) output domains, interleaved.
                                                 * That is, first is first input domain, second is first output domain,
                                                 * third is second input domain, fourth is second output domain, etc.
                                                 */
  };

  /**
   * @brief Read constraint test cases from the given file, adding them to the
   * list passed in.
   * @return True if the file contained valid descriptions of test
   * cases; false only if file cannot be opened; throw an error if the
   * file can be opened but there's bad or unexpected data in the file.
   */
  bool readTestCases(std::string file, std::list<ConstraintTestCase>& testCases);

  /**
   * @brief Execute the tests described by the test cases.
   */
  bool executeTestCases(const ConstraintEngineId& engine, std::list<ConstraintTestCase>& testCases);

}; /* namespace EUROPA */
