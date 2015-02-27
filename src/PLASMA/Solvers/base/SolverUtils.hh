#ifndef H_PlannerUtils
#define H_PlannerUtils

#include "ConstrainedVariable.hh"
#include "RuleInstance.hh"
#include "Token.hh"
#include "Object.hh"
#include "Error.hh"
#include "SolverDefs.hh"

namespace EUROPA {
namespace SOLVERS {

DECLARE_GLOBAL_CONST(std::string, WILD_CARD);
DECLARE_GLOBAL_CONST(std::string, DELIMITER);
DECLARE_GLOBAL_CONST(long, ZERO_COMMITMENT_SCORE);
DECLARE_GLOBAL_CONST(long, WORST_SCORE);

}
}
#endif
