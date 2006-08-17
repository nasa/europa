#include "AverInterp.hh"
#include "Assertion.hh"
#include "AssertionExecutor.hh"
#include "Test.hh"
#include "Utils.hh"
#include <algorithm>

namespace EUROPA {

  std::map<const std::string, AssertionExecutorId> AverInterp::s_executors =
  std::map<const std::string, AssertionExecutorId>();
  
  std::list<AssertionId> AverInterp::s_assertions = std::list<AssertionId>();
  std::list<TestId> AverInterp::s_tests = std::list<TestId>();
  std::list<TestId> AverInterp::s_rootTests = std::list<TestId>();
  TiXmlDocument* AverInterp::s_doc = (TiXmlDocument*) NULL;

  void AverInterp::init(const std::string& filePath, 
                        const SOLVERS::SolverId& dm = SOLVERS::SolverId::noId(),
                        const ConstraintEngineId& ce = ConstraintEngineId::noId(),
                        const PlanDatabaseId& db = PlanDatabaseId::noId(),
                        const RulesEngineId& re = RulesEngineId::noId()) {
    EventAggregator::instance(dm, ce, db, re);
    AverHelper::setDb(db);
    TiXmlElement* root = commonInit(filePath);
    s_rootTests.push_back(buildTest(root));
    delete s_doc;
  }

  void AverInterp::init(const std::string& filePath,
                        const std::list<std::string>& tests,
                        const SOLVERS::SolverId& dm = SOLVERS::SolverId::noId(),
                        const ConstraintEngineId& ce = ConstraintEngineId::noId(),
                        const PlanDatabaseId& db = PlanDatabaseId::noId(),
                        const RulesEngineId& re = RulesEngineId::noId()) {
    EventAggregator::instance(dm, ce, db, re);
    AverHelper::setDb(db);
    TiXmlElement* root = commonInit(filePath);
    std::list<std::string> tempTests;
  

    for(std::list<std::string>::const_iterator it = tests.begin(); it != tests.end(); ++it) {
           tempTests.push_back(*it);
    }

    tempTests.sort();
    removeAllElse(root, tempTests);
    s_rootTests.push_back(buildTest(root));
    delete s_doc;
  }

  TiXmlElement* AverInterp::commonInit(const std::string& filePath) {
    s_doc = new TiXmlDocument(filePath.c_str());
    check_error(s_doc, "Failed to instantiate document.", AverErr::XmlError());
    if(!s_doc->LoadFile())
      check_error(s_doc->LoadFile(), s_doc->ErrorDesc(), AverErr::XmlError());
    check_error(std::string("Test") == std::string(s_doc->RootElement()->Value()), 
                "Root element not a Test", AverErr::XmlError());
    return s_doc->RootElement();
  }

  void AverInterp::removeAllElse(TiXmlElement* root, std::list<std::string>& tests) {
    if(root == NULL)
      return;
    if(find(tests.begin(), tests.end(), std::string(root->Value())) != tests.end()) {
      tests.remove(std::string(root->Value()));
      root->Parent()->RemoveChild(root);
      return;
    }
    removeAllElse(root->NextSiblingElement(), tests);
    removeAllElse(root->FirstChildElement(), tests);
  }

  TestId AverInterp::buildTest(const TiXmlElement* test) {
    check_error(test != NULL, "Attempted to build null test.", AverErr::XmlError());
    check_error(std::string("Test") == std::string(test->Value()), 
                "Attempted to build test from non-test.", AverErr::FlowError());
    TestId testId = (new Test(std::string(test->Attribute("name"))))->getId();
    for(TiXmlElement* child = test->FirstChildElement();
        child != NULL; child = child->NextSiblingElement()) {
      if(std::string("Test") == std::string(child->Value()))
        testId->add(buildTest(child));
      else if(std::string("At") == std::string(child->Value()))
        testId->add(buildAssertion(child));
    }
    s_tests.push_back(testId);
    return testId;
  }

  AssertionId AverInterp::buildAssertion(const TiXmlElement* assn) {
    check_error(assn != NULL, "Attempted to build null assertion.", AverErr::XmlError());
    check_error(std::string("At") == std::string(assn->Value()), 
                "Attempted to build assertion from non-assertion.",
                AverErr::FlowError());
    check_error(assn->Attribute("file") != NULL, AverErr::XmlError());
    check_error(assn->Attribute("lineText") != NULL, AverErr::XmlError());
    check_error(assn->Attribute("cond") != NULL, AverErr::XmlError());
  
    int lineNo = 0;
    check_error(assn->QueryIntAttribute("lineNo", &lineNo) == TIXML_SUCCESS,
                AverErr::XmlError());

    std::string file(assn->Attribute("file"));
    std::string lineText(assn->Attribute("lineText"));
    std::string cond(assn->Attribute("cond"));
    assn->QueryIntAttribute("lineNo", &lineNo);
    
    check_error(cond != "", AverErr::XmlError());
    
    TiXmlElement* step = assn->FirstChildElement();
    check_error(step != NULL, AverErr::XmlError());

    TiXmlElement* stmt = step->NextSiblingElement();
    check_error(stmt != NULL, AverErr::XmlError());

    AssertionExecutorId exec = getExecutor(cond, step);
    AssertionId assnId = (new Assertion(stmt, file, lineNo, lineText))->getId();
    exec->add(assnId);
    s_assertions.push_back(assnId);
    return assnId;
  }

  AssertionExecutorId AverInterp::getExecutor(const std::string& cond,
                                              const TiXmlElement* step) {
    check_error(std::string("step") == std::string(step->Value()),
                "Attempted to get executor for non-step.", AverErr::XmlError());
    std::map<const std::string,AssertionExecutorId>::const_iterator elem;
    if((elem = s_executors.find(cond)) != s_executors.end())
      return (*elem).second;
    AssertionExecutorId exec = (new AssertionExecutor(step))->getId();
    s_executors[cond] = exec;
    return exec;
  }

  void AverInterp::terminate() {
    for(std::list<TestId>::const_iterator it = s_rootTests.begin();
        it != s_rootTests.end(); ++it) {
      TestId test = *it;
      test->dumpResults(std::cout);
    }
    s_rootTests.clear();
    cleanup(s_assertions);
    cleanup(s_tests);
    cleanup<const std::string, AssertionExecutor >(s_executors);
    EventAggregator::remove();
  }
}
