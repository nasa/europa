#ifndef _H_HSTSNoBranchCondition
#define _H_HSTSNoBranchCondition

#include "Condition.hh"
#include "HSTSNoBranch.hh"

namespace EUROPA {

  class HSTSNoBranchCondition : public Condition {
  public:
    HSTSNoBranchCondition(const DecisionManagerId& dm);
    virtual ~HSTSNoBranchCondition();

    void initialize(const HSTSNoBranchId& noBranchSpec);

    bool test(const EntityId& entity);

    void print (std::ostream& os = std::cout);

  private:
    std::set<LabelStr> m_noBranches;
  };

}
#endif
