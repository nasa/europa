/**
 * @file ConstraintTesting.cc
 * @author Will Edgington
 * @date December 2004
 * @brief Functions and a class to assist with testing constraint classes.
 */

#include "ConstraintTesting.hh"

#include <list>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cppunit/extensions/HelperMacros.h>

#include "Error.hh"
#include "tinyxml.h"
#include "Domains.hh"
#include "ConstraintType.hh"
#include "Variable.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"
#include "DataTypes.hh"

namespace EUROPA {


  std::map<std::string, SymbolDomain*>& ConstraintTestCase::symbolDomainsMap() {
    static std::map<std::string, SymbolDomain*> sl_map;
    return sl_map;
  }

  std::string ConstraintTestCase::toString() const
  {
      std::ostringstream os;

      os << m_fileName << " " << m_case << " " << m_constraintName << "(";

      std::list<Domain*>::const_iterator it = m_domains.begin();
      for(int i=0;it != m_domains.end();++it) {
          if (i > 0)
              os << ",";
          os << (*it)->toString().c_str();
          i++;
      }
      os << ")" << std::endl;

      return os.str();
  }

  /**
   * @brief Convert c string into EUROPA floating point number
   * @param a String which will be converted
   * @note roughtly the same as the standard atoi(char*), but with EUROPA's +/-inf
   */
  static edouble atoef(const char * a)
  {
    if(strcmp(a,"-inf") == 0)
      return MINUS_INFINITY;
    else if(strcmp(a,"+inf") == 0)
      return PLUS_INFINITY;
    else
      return atof(a);
  }

  /**
   * @brief Create a new concrete *Domain from data read from the stream.
   * @param element XML element containing domain.
   * @note Incomplete, but should allow tests to pass.
   */
  static Domain* readSet(ConstraintEngineId ce, const TiXmlElement & element) {
      const char * tagname = element.Value();

      if(strcmp(tagname, "BoolDomain") == 0) {
          BoolDomain * dom = new BoolDomain();
          dom->empty();
          for (const TiXmlElement * child = element.FirstChildElement() ;
          child != NULL ; child = child->NextSiblingElement()) {
              check_error(strcmp(child->Value(), "element") == 0);

              const char * value = child->Attribute("value");
              if(strcmp(value,"true") == 0) dom->insert(true);
              else if(strcmp(value,"false") == 0) dom->insert(false);
              else check_error(false);
          }
          return dom;
      }
      else if(strcmp(tagname, "NumericDomain") == 0) {
          NumericDomain * dom = new NumericDomain();
          dom->empty();
          for (const TiXmlElement * child = element.FirstChildElement() ;
          child != NULL ; child = child->NextSiblingElement()) {
              check_error(strcmp(child->Value(), "element") == 0);

              const char * value = child->Attribute("value");

              dom->insert(atoef(value));
          }
          dom->close();
          return dom;
      }
      else if(strcmp(tagname, "SymbolDomain") == 0) {
          const char * type = element.Attribute("type");
          DataTypeId dt;

          if (ce->getCESchema()->isDataType(type))
              dt = ce->getCESchema()->getDataType(type);
          else {
              dt = (new RestrictedDT(type,SymbolDT::instance(),SymbolDomain()))->getId();
              ce->getCESchema()->registerDataType(dt);
          }

          SymbolDomain * dom = new SymbolDomain((edouble)LabelStr("foo"),dt);
          dom->empty(); dom->open();

          SymbolDomain * base = ConstraintTestCase::symbolDomainsMap()[std::string(type)];
          if(!base) {
              base = new SymbolDomain((edouble)LabelStr("foo"),dt->getId());
              base->empty(); base->open();
              ConstraintTestCase::symbolDomainsMap()[std::string(type)] = base;
          }

          for (const TiXmlElement * child = element.FirstChildElement() ;
          child != NULL ; child = child->NextSiblingElement()) {
              check_error(strcmp(child->Value(), "element") == 0);

              const char * value = child->Attribute("value");

              dom->insert(LabelStr(value));
              base->insert(LabelStr(value));
          }
          dom->close();
          return dom;
      }
      else {
          check_error(false);
      }
      return NULL;
  }

  /**
   * @brief Create a new IntervalDomain from data read from the stream.
   * @note Incomplete, but should allow tests to pass.
   */
  static Domain* readInterval(const TiXmlElement & element) {
    const char * tagname = element.Value();

    const char * lba = element.Attribute("lb");
    const char * uba = element.Attribute("ub");

    if(strcmp(tagname, "IntervalDomain") == 0) {
      if(lba == NULL && uba == NULL)
	{
	  IntervalDomain * toRet = new IntervalDomain();
	  toRet->empty();
	  return toRet;
	}
      check_error(lba != NULL && uba != NULL);

      return new IntervalDomain(atoef(lba), atoef(uba));
    }
    else if(strcmp(tagname, "IntervalIntDomain") == 0) {
      if(lba == NULL && uba == NULL)
	{
	  IntervalIntDomain * toRet = new IntervalIntDomain();
	  toRet->empty();
	  return toRet;
	}
      check_error(lba != NULL && uba != NULL);

      //return new IntervalIntDomain(cast_int(atoef(lba)), cast_int(atoef(uba)));
      return new IntervalIntDomain(cast_int(atoef(lba)), cast_int(atoef(uba)));
    }
    else
      check_error(false);
    return NULL;
  }

  /**
   * @brief Read domains from an element's children, delegates to readSet or readInterval
   * @note Incomplete, but should allow tests to pass.
   */
  static void readDomains(ConstraintEngineId ce, const TiXmlElement & element, std::list<Domain*> & domains) {
    if(&element == NULL) return;
    check_error_variable(const char * name = element.Value());
    checkError(strcmp(name,"Inputs") == 0 || strcmp(name,"Outputs") == 0,
               "unexpected element type \"" << name << "\"");

    for (const TiXmlElement * child_el = element.FirstChildElement() ;
         child_el; child_el = child_el->NextSiblingElement()) {
      const char * cname = child_el->Value();
      Domain * dom = NULL;
      if(strcmp(cname,"BoolDomain") == 0 || strcmp(cname,"NumericDomain") == 0 || strcmp(cname,"SymbolDomain") == 0)
        dom = readSet(ce, *child_el);
      else if(strcmp(cname,"IntervalDomain") == 0 || strcmp(cname,"IntervalIntDomain") == 0)
        dom = readInterval(*child_el);
      else
        checkError(false,"Parent \"" << element.Value() << "\" shouldn't contain child \"" << cname << "\"");

      if(dom != NULL)
        domains.push_back(dom);
    }
  }

  bool readTestCases(ConstraintEngineId ce, std::string file, std::list<ConstraintTestCase>& testCases) {
    TiXmlDocument doc(file.c_str());

    debugMsg("ConstraintTesting:readTestCases", "Test cases loading from " << file);
    if(!doc.LoadFile())
      return false;

    for (TiXmlElement * constraint = doc.RootElement()->FirstChildElement("Constraint") ;
	 constraint; constraint = constraint->NextSiblingElement("Constraint")) {
      const char * constraintName = constraint->Attribute("name");
      const char * testcase = constraint->Attribute("test");
      check_error(constraintName != NULL);

      TiXmlElement * inputs = constraint->FirstChildElement("Inputs");
      TiXmlElement * outputs = constraint->FirstChildElement("Outputs");

      std::list<Domain*> domains, inputDoms, outputDoms;

      readDomains(ce,*inputs, inputDoms);
      readDomains(ce,*outputs, outputDoms);


      // Simple checks that this test case is OK.
      CPPUNIT_ASSERT(inputDoms.size() == outputDoms.size());

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
      testCases.push_back(ConstraintTestCase(constraintName, file,testcase, domains));
    }

    debugMsg("ConstraintTesting:readTestCases", "Test cases loaded from " << file);
    return true;
  }

  bool executeTestCases(const ConstraintEngineId& engine,
			std::list<ConstraintTestCase>& testCases) {
    // Run each test, in the same order they appear in the list,
    //   keeping a count of failed test cases.
    unsigned int problemCount = 0;
    std::set<std::string> warned; /**< List of unregistered constraints seen so far. */

    for ( ; !testCases.empty(); testCases.pop_front()) {
      debugMsg("ConstraintTesting:executeTestCases","Executing " << testCases.front().toString());
      // Warn about unregistered constraint names and otherwise ignore tests using them.
      if (!engine->getCESchema()->isConstraintType(LabelStr(testCases.front().m_constraintName), false)) {
        if (warned.find(testCases.front().m_constraintName) == warned.end()) {
          std::cout << "\n    Warning: "
                    << testCases.front().m_fileName << ':' << testCases.front().m_case
                    << ": constraint " << testCases.front().m_constraintName
                    << " is unregistered; skipping tests of it.\n";
          warned.insert(testCases.front().m_constraintName);
        }
        continue;
      }

      std::list<Domain*> testDomains(testCases.front().m_domains);
      // Each input domain must have a matching output domain.
      CPPUNIT_ASSERT(testDomains.size() % 2 == 0);

      // Build the scope and the list of expected output domains.
      std::vector<ConstrainedVariableId> scope;
      std::list<Domain*> outputDoms;
      ConstrainedVariableId cVarId;
      while (!testDomains.empty()) {
        Domain *domPtr = testDomains.front();
        CPPUNIT_ASSERT(domPtr != 0 && (domPtr->isOpen() || !domPtr->isEmpty()));
        testDomains.pop_front();

	LabelStr typeName = domPtr->getTypeName();
	cVarId = engine->createVariable(typeName.c_str(), *domPtr);

        delete domPtr;
        scope.push_back(cVarId);
        domPtr = testDomains.front();
        checkError(domPtr != NULL, "All domains must be defined (possibly empty) for ConstraintTesting");
        outputDoms.push_back(domPtr);
        testDomains.pop_front();
      }
      CPPUNIT_ASSERT(scope.size() == outputDoms.size());

      // Create and execute the constraint.

      std::stringstream scopeStr;
      for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it)
	scopeStr << " " << (*it)->toString();

      ConstraintId constraint = engine->createConstraint(LabelStr(testCases.front().m_constraintName), scope);

      debugMsg("ConstraintTesting:executeTestCases", "Created constraint " << constraint->toString() <<
	       " with scope " << scopeStr.str() << " for test "
	       << testCases.front().m_case);

      // Compare derived domains with outputDoms.
      std::vector<ConstrainedVariableId>::iterator scopeIter = scope.begin();
      unsigned int i = 1;
      bool problem = false;
      for ( ; scopeIter != scope.end() && !outputDoms.empty(); scopeIter++, i++) {
        Domain *domPtr = outputDoms.front();
        outputDoms.pop_front();
        if (domPtr->isEmpty()) {
          if (!(*scopeIter)->derivedDomain().isEmpty()) {
            if (!problem)
              std::cerr << testCases.front().m_fileName << ':' << testCases.front().m_case
                        << ": unexpected result propagating " << testCases.front().m_constraintName;
            std::cerr << ";\n  argument " << i << " is " << (*scopeIter)->derivedDomain()
                      << "\n     rather than empty";
            problem = true;
          }
        } else
          if ((*scopeIter)->derivedDomain().isEmpty()) {
            if (!domPtr->isEmpty()) {
              if (!problem)
                std::cerr << testCases.front().m_fileName << ':' << testCases.front().m_case
                          << ": unexpected result propagating " << testCases.front().m_constraintName;
              std::cerr << ";\n  argument " << i << " is empty"
                        << "\n    rather than " << *domPtr;
              problem = true;
            }
          } else
            if ((*scopeIter)->derivedDomain() != *domPtr) {
              if (!problem)
                std::cerr << testCases.front().m_fileName << ':' << testCases.front().m_case
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
