#include "Assertion.hh"
#include <sstream>

namespace EUROPA {

  Assertion::FailMode Assertion::s_failureMode = Assertion::FAIL_FAST;

  Assertion::Assertion(TiXmlElement* assn, const std::string& file, const int lineNo,
                       const std::string& lineText) : m_id(this), m_file(file), 
                                                      m_lineNo(lineNo), m_lineText(lineText),
                                                      m_status(INCOMPLETE) {
    m_exec = AverHelper::buildExecutionPath(assn);
  }

  Assertion::~Assertion() {
    m_id.remove();
    delete (AverExecutionPath*) m_exec;
    
  }

  void Assertion::execute() {
    if(!AverHelper::execute(m_exec)) {
      m_status = FAILED;
      fail();
      return;
    }
    m_status = PASSED;
  }
  
  void Assertion::fail() {
    if(m_status != FAILED)
      return;
    std::string error = failureString();
    if(s_failureMode == FAIL_FAST || s_failureMode == FAIL_WARN) {
      std::cerr << error << std::endl;
      if(s_failureMode == FAIL_FAST)
        exit(EXIT_FAILURE);
    }
    else
      m_errors.push_back(error);
  }

  std::string Assertion::failureString() {
    std::stringstream out;
    out << "Assertion FAILED at " << m_file << ", line " << m_lineNo << ".\n";
    out << "Assertion: " << m_lineText << "\n";
    return out.str();
  }

  void Assertion::dumpResults(std::ostream& os) {
    os << "'" << m_lineText << "' at " << m_file << ", " << m_lineNo << ": ";
    if(m_status == FAILED || !m_errors.empty()) {
      int numFailures = (m_errors.empty() ? 1 : m_errors.size());
      os << "FAILED (" << numFailures << ")";
    }
    else if(m_status == PASSED)
      os << "PASSED";
    else
      os << "INCOMPLETE";
    os << std::endl;
  }
}
