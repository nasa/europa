#ifndef _H_HSTSNoBranch
#define _H_HSTSNoBranch

#include "CBPlanner.hh"
#include "LabelStr.hh"

namespace PLASMA {

  class HSTSNoBranch {
  public:
    HSTSNoBranch(const SchemaId& schema);
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
