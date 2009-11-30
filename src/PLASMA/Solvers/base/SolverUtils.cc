#include "SolverUtils.hh"

namespace EUROPA {
  namespace SOLVERS {
    DEFINE_GLOBAL_CONST(LabelStr, WILD_CARD, "*");
    DEFINE_GLOBAL_CONST(LabelStr, DELIMITER, ".");
    DEFINE_GLOBAL_CONST(unsigned int, ZERO_COMMITMENT_SCORE, 1);
    DEFINE_GLOBAL_CONST(unsigned int, WORST_SCORE, cast_int(PLUS_INFINITY)+1);
  }
}
