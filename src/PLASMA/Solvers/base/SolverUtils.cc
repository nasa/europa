#include "SolverUtils.hh"

namespace EUROPA {
  namespace SOLVERS {
    DEFINE_GLOBAL_CONST(std::string, WILD_CARD, "*");
    DEFINE_GLOBAL_CONST(std::string, DELIMITER, ".");
    DEFINE_GLOBAL_CONST(long, ZERO_COMMITMENT_SCORE, 1);
    DEFINE_GLOBAL_CONST(long, WORST_SCORE, cast_int(PLUS_INFINITY)+1);
  }
}
