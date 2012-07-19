#ifndef H_PlannerUtils
#define H_PlannerUtils

#include "LabelStr.hh"
#include "ConstrainedVariable.hh"
#include "RuleInstance.hh"
#include "Token.hh"
#include "Object.hh"
#include "Error.hh"
#include "SolverDefs.hh"

namespace EUROPA {
  namespace SOLVERS {

    DECLARE_GLOBAL_CONST(LabelStr, WILD_CARD);
    DECLARE_GLOBAL_CONST(LabelStr, DELIMITER);
    DECLARE_GLOBAL_CONST(unsigned int, ZERO_COMMITMENT_SCORE);
    DECLARE_GLOBAL_CONST(unsigned int, WORST_SCORE);

  }
}
#endif
