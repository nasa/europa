#ifndef _H_HSTSPlanIdReader
#define _H_HSTSPlanIdReader

#include "CBPlanner.hh"
#include "HSTSNoBranch.hh"
#include "LabelStr.hh"

namespace PLASMA {

  class HSTSPlanIdReader {
  public:
    HSTSPlanIdReader(HSTSNoBranch& noBranchSpec);
    void read(const std::string& configFile);
    virtual ~HSTSPlanIdReader();

  private:
    HSTSNoBranch m_noBranchSpec;
  };
}
#endif
