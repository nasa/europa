#ifndef _H_HSTSPlanIdReader
#define _H_HSTSPlanIdReader

#include "CBPlanner.hh"
#include "HSTSNoBranch.hh"
#include "LabelStr.hh"

namespace PLASMA {

  class HSTSPlanIdReader {
  public:
    HSTSPlanIdReader(HSTSNoBranchId& noBranchSpec);
    virtual ~HSTSPlanIdReader();

    void read(const std::string& configFile);
  private:
    HSTSNoBranchId m_noBranchSpec;
  };
}
#endif
