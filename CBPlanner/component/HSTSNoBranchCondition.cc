#include "HSTSNoBranchCondition.hh"
#include "ConstrainedVariable.hh"

namespace PLASMA {

  HSTSNoBranchCondition::HSTSNoBranchCondition(const DecisionManagerId& dm) : Condition(dm) {
    check_error(m_id.isValid());
  }

  HSTSNoBranchCondition::~HSTSNoBranchCondition() { }

  void HSTSNoBranchCondition::initialize(const HSTSNoBranch& noBranchSpec) {
    m_noBranches = noBranchSpec.getNoBranchSpec();
    notifyChanged();
  }

  bool HSTSNoBranchCondition::test(const EntityId& entity) {
    check_error(entity.isValid());
    if (!ConstrainedVariableId::convertable(entity)) return true;
    ConstrainedVariableId var(entity);
    std::set<LabelStr>::iterator it = m_noBranches.find(var->getName());
    if (it != m_noBranches.end())
      return false;
    return true;
  }

  void HSTSNoBranchCondition::print (std::ostream& os) { os << "NO_BRANCH_CONDITION"; }

}
