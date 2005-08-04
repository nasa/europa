#include "HSTSNoBranchCondition.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"
#include "AbstractDomain.hh"

namespace EUROPA {

  HSTSNoBranchCondition::HSTSNoBranchCondition(const DecisionManagerId& dm) : Condition(dm) {
    check_error(m_id.isValid());
  }

  HSTSNoBranchCondition::~HSTSNoBranchCondition() { }

  void HSTSNoBranchCondition::initialize(const HSTSNoBranchId& noBranchSpec) {
    m_noBranches = noBranchSpec->getNoBranchSpec();
    if (!m_noBranches.empty()) {
      notifyChanged();
    }
  }

  bool HSTSNoBranchCondition::test(const EntityId& entity) {
    check_error(entity.isValid());
    if (!ConstrainedVariableId::convertable(entity)) return true;
    ConstrainedVariableId var(entity);
    std::string varName = var->getName().c_str();
    if (!var->getParent().isNoId()) {
      std::string parentName = var->getParent()->getName().c_str();
      varName = parentName + "." + varName;
    }

    debugMsg("HSTSNoBranchCondition"," Testing variable (" << var->getKey() << ") " << varName);
    std::set<LabelStr>::iterator it = m_noBranches.find(varName);
    if (it != m_noBranches.end()) {
      if(var->lastDomain().isSingleton()) //changing NO_BRANCH semantic to allow decisions on unit vars
        return true;
      debugMsg("HSTSNoBranchCondition"," matched NO_BRANCH");
      return false;
    }
    debugMsg("HSTSNoBranchCondition"," didn't match NO_BRANCH");
    return true;
  }

  void HSTSNoBranchCondition::print (std::ostream& os) { os << "NO_BRANCH_CONDITION"; }

}
