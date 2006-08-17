#include "Assertion.hh"
#include "AssertionExecutor.hh"
#include "AverHelper.hh"
#include "EnumeratedDomain.hh"

namespace EUROPA {

  //  AssertionExecutor::AssertionExecutor(const TiXmlElement* step) : m_id(this) {
  AssertionExecutor::AssertionExecutor(const TiXmlElement* step) {
    check_error(step->Attribute("qualifier") != NULL, AverErr::XmlError());
    check_error(step->Attribute("operator") != NULL, AverErr::XmlError());
    m_qualifier = step->Attribute("qualifier");
    m_operator = step->Attribute("operator");
    m_stepDom = AverHelper::evaluateDomain(step->FirstChildElement(), false);
    m_executed = false;
  }
  
  AssertionExecutor::~AssertionExecutor() {
    delete m_stepDom;
    //m_id.remove();
  }

  void AssertionExecutor::notifyStep(const SOLVERS::DecisionPointId& dec) {
    AggregateListener::notifyStep(dec);
    if(checkExecute()) {
      for(std::list<AssertionId>::iterator it = m_assertions.begin();
          it != m_assertions.end(); ++it) {
        AssertionId assn = *it;
        assn->execute();
      }
      m_executed = true;
    }
  }
  
  bool AssertionExecutor::checkExecute() {
    if((m_qualifier == "any" || m_qualifier == "first" || m_qualifier == "last") && m_executed)
      return false;
    EnumeratedDomain currStepDom((double) currentStep(), true,
                                 EnumeratedDomain::getDefaultTypeName().c_str());
    return AverHelper::evaluateBoolean(m_operator, currStepDom, *(*m_stepDom));
  }
}
