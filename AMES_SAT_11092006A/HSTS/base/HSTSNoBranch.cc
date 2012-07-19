#include "HSTSNoBranch.hh"
#include "Schema.hh"

namespace EUROPA {

  HSTSNoBranch::HSTSNoBranch() { 
    m_schema = Schema::instance(); // for convenience
  }

  HSTSNoBranch::~HSTSNoBranch() {}

  void HSTSNoBranch::addNoBranch(const LabelStr& pred, const int index) {
    const LabelStr varName(m_schema->getNameFromIndex(pred, index));
    const std::string vnstr(varName.c_str());
    const std::string pstr(pred.c_str());
    m_noBranches.insert(pstr+Schema::getDelimiter()+vnstr);
  }

  void HSTSNoBranch::addNoBranch(const LabelStr& varName) {
    m_noBranches.insert(varName);
  }

  const std::set<LabelStr>& HSTSNoBranch::getNoBranchSpec() const { return m_noBranches; }

}
