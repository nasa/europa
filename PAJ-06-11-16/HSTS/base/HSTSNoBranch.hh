#ifndef _H_HSTSNoBranch
#define _H_HSTSNoBranch

#include "LabelStr.hh"
#include "Id.hh"
#include "PlanDatabaseDefs.hh"
#include <set>

namespace EUROPA {

  class HSTSNoBranch;
  typedef Id<HSTSNoBranch> HSTSNoBranchId;

  class HSTSNoBranch {
  public:
    HSTSNoBranch();
    virtual ~HSTSNoBranch();

    void addNoBranch(const LabelStr& pred, const int index);
    void addNoBranch(const LabelStr& variable);
    const std::set<LabelStr>& getNoBranchSpec() const;

  private:
    SchemaId m_schema;
    std::set<LabelStr> m_noBranches;
  };
}
#endif
