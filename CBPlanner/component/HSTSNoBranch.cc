#include "HSTSNoBranch.hh"
#include "Schema.hh"

namespace PLASMA {

  HSTSNoBranch::HSTSNoBranch(const SchemaId& schema) : m_schema(schema) {}

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
