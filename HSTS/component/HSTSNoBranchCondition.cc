#include "HSTSNoBranchCondition.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {

  HSTSNoBranchCondition::HSTSNoBranchCondition(const DecisionManagerId& dm) : Condition(dm) {
    check_error(m_id.isValid());
  }

  HSTSNoBranchCondition::~HSTSNoBranchCondition() { }

  void HSTSNoBranchCondition::initialize(const HSTSNoBranchId& noBranchSpec) {
    m_noBranches = noBranchSpec->getNoBranchSpec();
    if (!m_noBranches.empty()) {
      std::cout << " No Branch is NOT empty" << std::endl;
      notifyChanged();
    }
    else { std::cout << " No Branch is empty" << std::endl; }
    
  }

  bool HSTSNoBranchCondition::test(const EntityId& entity) {
    check_error(entity.isValid());
    if (!ConstrainedVariableId::convertable(entity)) return true;
    ConstrainedVariableId var(entity);

    std::cout << " NoBranches has " << (*m_noBranches.begin()).c_str() << std::endl;

    std::string varName = var->getName().c_str();
    if (!var->getParent().isNoId()) {
      std::string parentName = var->getParent()->getName().c_str();
      varName = parentName + "." + varName;
    }

    std::cout << " Testing variable (" << var->getKey() << ") " << varName;
    std::set<LabelStr>::iterator it = m_noBranches.find(varName);
    if (it != m_noBranches.end()) {
      std::cout << " matched NO_BRANCH" << std::endl;
      return false;
    }
    std::cout << " didn't match NO_BRANCH" << std::endl;
    return true;
  }

  void HSTSNoBranchCondition::print (std::ostream& os) { os << "NO_BRANCH_CONDITION"; }

}
