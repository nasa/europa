/**
 * @file ConstraintTesting.cc
 * @author Will Edgington
 * @date December 2004
 * @brief Functions and a class to assist with testing constraint classes.
 */

#include "ConstraintTesting.hh"

#include <list>
#include <set>
#include <string>
#include <iostream>
#include <fstream>

#include "Error.hh"
#include "AbstractDomain.hh"
#include "BoolDomain.hh"
#include "IntervalIntDomain.hh"
#include "IntervalDomain.hh"
#include "NumericDomain.hh"
#include "ConstraintLibrary.hh"
#include "Variable.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"

namespace EUROPA {

  /**
   * @brief Create a new concrete *Domain from data read from the stream.
   * @param in Stream to read from.
   * @note Incomplete, but should allow tests to pass.
   */
  static AbstractDomain* readSet(std::istream& in) {
    char ch;
    AbstractDomain *dom = 0;
    AbstractDomain::DomainType type = AbstractDomain::REAL_ENUMERATION;
    bool negative = false;
    double value;
    std::list<double> values;
    std::string member;
    std::list<std::string> members;
    for (in.get(ch); ch != '}' && in.good(); ) {
      switch (ch) {
      case ' ':
        negative = false;
        in.get(ch);
        continue;
      case '-':
        assertTrue(members.empty() && type == AbstractDomain::REAL_ENUMERATION);
        negative = true;
        in.get(ch);
        continue;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        assertTrue(members.empty() && type == AbstractDomain::REAL_ENUMERATION);
        if (negative)
          member = "-";
        member += ch;
        for (in.get(ch); ch != '}' && ch != ' ' && in.good(); in.get(ch))
          member += ch;
        assertTrue(in.good());
        value = atof(member.c_str());
        values.push_back(value);
        member = "";
        continue;
      default:
        if (negative)
          member = "-";
        member += ch;
        for (in.get(ch); ch != '}' && ch != ' ' && in.good(); in.get(ch))
          member += ch;
        assertTrue(in.good());
        if (member == "-Infinity" || member == "-Inf" || member == "-INF") {
          assertTrue(type == AbstractDomain::REAL_ENUMERATION);
          values.push_back(MINUS_INFINITY);
        } else
          if (member == "Infinity" || member == "Inf" || member == "INF") {
            assertTrue(type == AbstractDomain::REAL_ENUMERATION);
            values.push_back(PLUS_INFINITY);
          } else
            if (member == "false" || member == "False" || member == "FALSE") {
              members.push_back("false");
              // Allow "false" - but not "False"! - to be a member of a user defined type.
              // Need to know expected type of this arg of constraint to do better.
              if (type == AbstractDomain::REAL_ENUMERATION)
                type = AbstractDomain::BOOL;
            } else
              if (member == "true" || member == "True" || member == "TRUE") {
                members.push_back("true");
                // Allow "true" - but not "True"! - to be a member of a user defined type.
                // Need to know expected type of this arg of constraint to do better.
                if (type == AbstractDomain::REAL_ENUMERATION)
                  type = AbstractDomain::BOOL;
              } else {
                members.push_back(member);
                type = AbstractDomain::USER_DEFINED;
              }
        member = "";
        break;
      }
    }
    assertTrue(in.good() && ch == '}');
    assertTrue(values.empty() || members.empty());
    switch (type) {
    case AbstractDomain::REAL_ENUMERATION:
      dom = new NumericDomain(values);
      break;
    case AbstractDomain::BOOL:
      dom = new BoolDomain;
      dom->empty();
      for (std::list<std::string>::iterator it = members.begin();
           it != members.end(); it++)
        if (*it == "false")
          dom->insert(false);
        else {
          assertTrue(*it == "true", "Only 'false' and 'true' are supported for boolean values");
          dom->insert(true);
        }
      break;
    case AbstractDomain::USER_DEFINED:
      // Cannot support members without knowing how to map them to doubles.
      // For now, caller will have to skip this test.
      return(0);
      break;
    default: // Unsupported or unimplemented type.
      assertTrue(false);
      break;
    }
    assertTrue(dom != 0);
    return(dom);
  }

  /**
   * @brief Create a new IntervalDomain from data read from the stream.
   * @note Incomplete, but should allow tests to pass.
   */
  static AbstractDomain* readInterval(std::istream& in) {
    char ch;
    AbstractDomain *dom;
    bool negative = false;
    double endPoints[2];
    unsigned int which = 0; // 0 for no bounds; 1 for lower bound; 2 for upper bound.
    for (in.get(ch); ch != ']' && in.good(); ) {
      switch (ch) {
      case ' ': case '+':
        negative = false;
        in.get(ch);
        continue;
      case '-':
        negative = true;
        in.get(ch);
        continue;
      case 'I': case 'i': // Infinity or a variant thereof.
        which++;
        assertTrue(which < 3);
        if (negative)
          endPoints[which - 1] = MINUS_INFINITY;
        else
          endPoints[which - 1] = PLUS_INFINITY;
        for (in.get(ch); ch != ' ' && ch != ']' && in.good(); in.get(ch))
          ;
        assertTrue(in.good());
        continue;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        {
          which++;
          assertTrue(which < 3);
          std::string number;
          if (negative)
            number = "-";
          number += ch;
          for (in.get(ch); ch != ']' && ch != ' ' && in.good(); in.get(ch))
            number += ch;
          assertTrue(in.good());
          endPoints[which - 1] = atof(number.c_str());
        }
        continue;
      default:
        // Unrecognized input.
        assertTrue(false);
        break;
      }
    }
    assertTrue(in.good() && ch == ']' && which < 3);
    // Presume always IntervalDomain (rather than IntervalIntDomain) for now.
    // To know which will probably require a DomainType argument to this function.
    if (which == 0) {
      dom = new IntervalDomain();
      dom->empty();
    } else
      if (which == 1)
        dom = new IntervalDomain(endPoints[0]);
      else
        dom = new IntervalDomain(endPoints[0], endPoints[1]);
    assertTrue(dom != 0);
    return(dom);
  }

  bool EUROPA::readTestCases(std::string file, std::list<ConstraintTestCase>& testCases) {
    std::ifstream tCS(file.c_str()); /**< testCaseStream. */
    if (!tCS.is_open() || !tCS.good())
      return(false);
    unsigned line = 1; /**< Line within file. */
    std::string constraintName; /**< Name of a constraint, from each line of file. */
    char buf[20]; /**< For single "words" of input. */
    unsigned int cnt; /**< For test number. */
    char ch; /**< For braces, brackets, and other miscellany. */
    AbstractDomain *domain = 0;
    while (tCS.good() && !tCS.eof()) {
      tCS.width(5);
      tCS >> buf;
      if (tCS.eof())
        break;
      assertTrue(strcmp(buf, "test") == 0 && !tCS.eof() && tCS.good());
      tCS >> cnt;

      // This requirement that cnt == line is probably overkill and should be dropped.
      // If nothing else, it makes it harder to insert tests in the middle of files.
      assert(cnt == line && !tCS.eof() && tCS.good());

      constraintName = "";
      tCS.get(ch);
      assertTrue(ch == ' ' && !tCS.eof() && tCS.good());
      for (tCS.get(ch); ch != ' ' && tCS.good(); tCS.get(ch))
        constraintName += ch;
      assertTrue(constraintName.size() > 0 && !tCS.eof() && tCS.good());
      tCS.width(7);
      tCS >> buf;
      assertTrue(strcmp(buf, "inputs") == 0 && !tCS.eof() && tCS.good());
      tCS.get(ch);
      assertTrue(ch == ' ' && !tCS.eof() && tCS.good());
      // Build input domains until 'o'utputs is seen, then output domains until end of line.
      // Details depend on NewPlan/Libraries/Domain.cc::Domain::print() or similar, but
      // this is meant to be fairly flexible so that new tests can be written by hand.
      std::list<AbstractDomain*> domains, inputDoms, outputDoms;

      // Until Europa2 label sets are supported by readSet(),
      // we may have to skip some tests:
      bool skipThisTest = false;

      bool readingInputDoms = true;
      for (tCS.get(ch); ch != '\n' && tCS.good(); tCS.get(ch)) {
        if (skipThisTest)
          continue;
        switch (ch) {
        case ' ': // Blank between fields; ignore it.
          break;
        case '{': // Singleton, enumeration, or boolean but could be IntervalDomain, IntervalIntDomain, or BoolDomain.
          domain = readSet(tCS);
          // This if is temporary, until readSet is fully implemented.
          // Presently, it cannot support Europa2 label sets.
          if (domain == 0) {
            skipThisTest = true;
            continue;
          }
          assertTrue(domain != 0 && !tCS.eof() && tCS.good());
          if (readingInputDoms)
            inputDoms.push_back(domain);
          else
            outputDoms.push_back(domain);
          break;
        case '[': // Interval, real or integer, but could use 'Infinity' and variations.
          domain = readInterval(tCS);
          assertTrue(domain != 0 && !tCS.eof() && tCS.good());
          if (readingInputDoms)
            inputDoms.push_back(domain);
          else
            outputDoms.push_back(domain);
          break;
        case 'o':
          tCS.width(7);
          tCS >> buf;
          assertTrue(strcmp(buf, "utputs") == 0 && !tCS.eof() && tCS.good());
          readingInputDoms = false;
          break;
        default:
          assertTrue(false);
          break;
        } // switch (ch): '{', '[', or 'o'
      } // for tCS.get(ch); ch != '\n' && tCS.good(); tCS.get(ch)

      if (skipThisTest) {
        line++; // To preserve comparison to cnt in assertion.
        continue;
      }

      // Simple checks that this test case is OK.
      assertTrue(inputDoms.size() == outputDoms.size() && !tCS.eof() && tCS.good());

      // OK, done with a line, each line being a test, so
      // interleave the input and output domains to make
      // things easier in caller.
      domains.clear();
      while (!inputDoms.empty() && !outputDoms.empty()) {
        domains.push_back(inputDoms.front());
        inputDoms.pop_front();
        domains.push_back(outputDoms.front());
        outputDoms.pop_front();
      }
      // ... and add this test to the list:
      testCases.push_back(ConstraintTestCase(constraintName, file, line++, domains));
    } // while tCS.good() && !tCS.eof()
    return(tCS.eof());
  }

  bool EUROPA::executeTestCases(const ConstraintEngineId& engine,
                                std::list<ConstraintTestCase>& testCases) {
    // Run each test, in the same order they appear in the list,
    //   keeping a count of failed test cases.
    unsigned int problemCount = 0;
    std::set<std::string> warned; /**< List of unregistered constraints seen so far. */
    for ( ; !testCases.empty(); testCases.pop_front()) {
      // Warn about unregistered constraint names and otherwise ignore tests using them.
      if (!ConstraintLibrary::isRegistered(LabelStr(testCases.front().m_constraintName), false)) {
        if (warned.find(testCases.front().m_constraintName) == warned.end()) {
          std::cerr << testCases.front().m_fileName << ':' << testCases.front().m_line
                    << ": constraint " << testCases.front().m_constraintName
                    << " is unregistered; skipping tests of it.\n";
          warned.insert(testCases.front().m_constraintName);
        }
        continue;
      }

      std::list<AbstractDomain*> testDomains(testCases.front().m_domains);
      // Each input domain must have a matching output domain.
      assertTrue(testDomains.size() % 2 == 0);

      // Build the scope and the list of expected output domains.
      std::vector<ConstrainedVariableId> scope;
      std::list<AbstractDomain*> outputDoms;
      ConstrainedVariableId cVarId;
      while (!testDomains.empty()) {
        AbstractDomain *domPtr = testDomains.front();
        assertTrue(domPtr != 0 && (domPtr->isOpen() || !domPtr->isEmpty()));
        testDomains.pop_front();
        AbstractDomain::DomainType domType = domPtr->getType();

        // This is ugly and precludes support for USER_DEFINED domains
        // since the corresponding C++ class is unknown. --wedgingt 2004 Mar 10
        switch (domType) {
        case AbstractDomain::INT_INTERVAL:
          {
            Variable<IntervalIntDomain> *var = new Variable<IntervalIntDomain>(engine, IntervalIntDomain());
            assertTrue(var != 0);
            var->specify(*domPtr);
            cVarId = var->getId();
          }
          break;
        case AbstractDomain::REAL_INTERVAL:
          {
            Variable<IntervalDomain> *var = new Variable<IntervalDomain>(engine, IntervalDomain());
            assertTrue(var != 0);
            var->specify(*domPtr);
            cVarId = var->getId();
          }
          break;
        case AbstractDomain::REAL_ENUMERATION:
          {
            std::list<double> values;
            domPtr->getValues(values);
            Variable<NumericDomain> *var = new Variable<NumericDomain>(engine, NumericDomain(values));
            assertTrue(var != 0);
            var->specify(*domPtr);
            cVarId = var->getId();
          }
          break;
        case AbstractDomain::BOOL:
          {
            Variable<BoolDomain> *var = new Variable<BoolDomain>(engine, BoolDomain());
            assertTrue(var != 0);
            var->specify(*domPtr);
            cVarId = var->getId();
          }
          break;
        default:
          assertTrue(false);
          break;
        }

        delete domPtr;
        scope.push_back(cVarId);
        domPtr = testDomains.front();
        assertTrue(domPtr != 0);
        outputDoms.push_back(domPtr);
        testDomains.pop_front();
      }
      assertTrue(scope.size() == outputDoms.size());

      // Create and execute the constraint.
      ConstraintId constraint = ConstraintLibrary::createConstraint(LabelStr(testCases.front().m_constraintName), engine, scope);
      assertTrue(engine->pending());
      engine->propagate();
      assertFalse(engine->pending());

      // Compare derived domains with outputDoms.
      std::vector<ConstrainedVariableId>::iterator scopeIter = scope.begin();
      unsigned int i = 1;
      bool problem = false;
      for ( ; scopeIter != scope.end() && !outputDoms.empty(); scopeIter++, i++) {
        AbstractDomain *domPtr = outputDoms.front();
        outputDoms.pop_front();
        if (domPtr->isEmpty()) {
          if (!(*scopeIter)->derivedDomain().isEmpty()) {
            if (!problem)
              std::cerr << testCases.front().m_fileName << ':' << testCases.front().m_line
                        << ": unexpected result propagating " << testCases.front().m_constraintName;
            std::cerr << ";\n  argument " << i << " is " << (*scopeIter)->derivedDomain()
                      << "\n     rather than empty";
            problem = true;
          }
        } else
          if ((*scopeIter)->derivedDomain().isEmpty()) {
            if (!domPtr->isEmpty()) {
              if (!problem)
                std::cerr << testCases.front().m_fileName << ':' << testCases.front().m_line
                          << ": unexpected result propagating " << testCases.front().m_constraintName;
              std::cerr << ";\n  argument " << i << " is empty"
                        << "\n    rather than " << *domPtr;
              problem = true;
            }
          } else
            if ((*scopeIter)->derivedDomain() != *domPtr) {
              if (!problem)
                std::cerr << testCases.front().m_fileName << ':' << testCases.front().m_line
                          << ": unexpected result propagating " << testCases.front().m_constraintName;
              std::cerr << ";\n  argument " << i << " is " << (*scopeIter)->derivedDomain()
                        << "\n    rather than " << *domPtr;
              problem = true;
            }
        delete domPtr;
      } // for ( ; scopeIter != scope.end() && ...

      // Finish complaining if a compare failed.
      if (problem) {
        std::cerr << '\n';
        ++problemCount;
      }

      // Print that the test succeeded, count the successes for this constraint function, or whatever.
      delete (Constraint*) constraint;
      while (!scope.empty()) {
        cVarId = scope.back();
        scope.pop_back();
        delete (ConstrainedVariable*) cVarId;
      }
    }
    if (problemCount > 0) {
      std::cerr << __FILE__ << ':' << __LINE__ << ": testArbitraryConstraints() failed "
                << problemCount << " test cases" << std::endl;
      throw Error::GeneralUnknownError();
    }
    return(true);
  }

}; /* namespace EUROPA */
