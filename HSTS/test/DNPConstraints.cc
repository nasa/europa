/**
 * @file DNPConstraints.cc
 * @author Will Edgington
 * @date December 2004
 * @brief Implementation of the constraint functions specific to the DNP domain.
 */

#include "DNPConstraints.hh"

using namespace PLASMA;

/* These and the DNP constraints are from Europa1 (NewPlan) tests. */
/* See the NewPlan CVS tree under, e.g., ModuleTests/Parser/dnp. */
/* --wedgingt@email.arc.nasa.gov 2004 Oct 14 */
//Constants for DNP domain.
// AKJ: These have been cut by 100 to reduce their numerical size
// original value: ObsLo 100
#define RFPLO (1)
// original value: ObsHi 1000
#define RFPHI (10)
// original value: ObsLo 50000
#define RMOSLO (500)
// original value: ObsHi 100000
#define RMOSHI (1000)
// ?????
#define ERDURMIN (100)
// Obviously, its' 0:
#define MINSTORE (0)
// Original value:   9000000000
#define MAXSTORE (90000000)
//Need to find out what this really should be
#define MIN_PLAYBACK_DUR (1)

BOUNDS_RECORD_END_STORAGE::BOUNDS_RECORD_END_STORAGE(const LabelStr& name,
                                                     const LabelStr& propagatorName,
                                                     const ConstraintEngineId& constraintEngine,
                                                     const std::vector<ConstrainedVariableId>& variables)
  : Constraint(name, propagatorName, constraintEngine, variables) {
  check_error(m_variables.size() == 5);
  check_error(getCurrentDomain(m_variables[0]).isNumeric());
  check_error(getCurrentDomain(m_variables[1]).isNumeric());
  check_error(getCurrentDomain(m_variables[2]).isNumeric()); //!!Should be Observation_Level
  check_error(getCurrentDomain(m_variables[3]).isNumeric()); //!!Should be Observation_Level
  check_error(getCurrentDomain(m_variables[4]).isNumeric());
}

void BOUNDS_RECORD_END_STORAGE::handleExecute() {
  IntervalIntDomain& outDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[0]));
  IntervalIntDomain& startDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[1]));
  IntervalIntDomain& rfpDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[2]));
  IntervalIntDomain& rmosDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[3]));
  IntervalIntDomain& durDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[4]));
  if (!rfpDom.isSingleton() || !rmosDom.isSingleton())
    return;
  int rfpVal = (int)rfpDom.getSingletonValue();
  int rmosVal = (int)rmosDom.getSingletonValue();
  int minRate;
  if (rfpVal == 0.0) /*!!ObsLo */
    minRate = RFPLO;
  else
    if (rfpVal == 1.0) /*!!ObsHi */
      minRate = RFPHI;
    else
      if (rmosVal == 0.0) /*!!ObsLo */
        minRate = RMOSLO;
      else
        if (rmosVal == 1.0) /*!!ObsHi */
          minRate = RMOSHI;
        else /* rfpVal == ObsNo && rmosVal == ObsNo */
          minRate = 0;
  int lowStart = (int)startDom.getLowerBound();
  if (lowStart < MINSTORE)
    lowStart = MINSTORE;
  int hiStart = (int)startDom.getUpperBound();
  if (hiStart > MAXSTORE)
    hiStart = MAXSTORE;
  int lo = 0, hi = 0;
  if (minRate > 0) {
    int lowDur = (int)durDom.getLowerBound();
    if (lowDur < MINSTORE/minRate)
      lowDur = MINSTORE/minRate;
    int hiDur = (int)durDom.getUpperBound();
    if (hiDur > MAXSTORE/minRate)
      hiDur = MAXSTORE/minRate;
    lo = lowDur*minRate;
    hi = hiDur*minRate;
  }
  lo += lowStart;
  hi += hiStart;
  if (lo > hi)
    outDom.empty();
  else
    outDom.intersect(IntervalIntDomain(lo, hi));
}

BOUNDS_RECORD_START_STORAGE::BOUNDS_RECORD_START_STORAGE(const LabelStr& name,
                                                         const LabelStr& propagatorName,
                                                         const ConstraintEngineId& constraintEngine,
                                                         const std::vector<ConstrainedVariableId>& variables)
  : Constraint(name, propagatorName, constraintEngine, variables) {
  check_error(m_variables.size() == 5);
  check_error(getCurrentDomain(m_variables[0]).isNumeric());
  check_error(getCurrentDomain(m_variables[1]).isNumeric());
  check_error(getCurrentDomain(m_variables[2]).isNumeric()); //!!Should be Observation_Level
  check_error(getCurrentDomain(m_variables[3]).isNumeric()); //!!Should be Observation_Level
  check_error(getCurrentDomain(m_variables[4]).isNumeric());
}

void BOUNDS_RECORD_START_STORAGE::handleExecute() {
  IntervalIntDomain& outDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[0]));
  IntervalIntDomain& endDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[1]));
  IntervalIntDomain& rfpDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[2]));
  IntervalIntDomain& rmosDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[3]));
  IntervalIntDomain& durDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[4]));
  if (!rfpDom.isSingleton() || !rmosDom.isSingleton())
    return;
  int rfpVal = (int)rfpDom.getSingletonValue();
  int rmosVal = (int)rmosDom.getSingletonValue();
  int minRate;
  if (rfpVal == 0.0) /*!!ObsLo */
    minRate = RFPLO;
  else
    if (rfpVal == 1.0) /*!!ObsHi */
      minRate = RFPHI;
    else
      if (rmosVal == 0.0) /*!!ObsLo */
        minRate = RMOSLO;
      else
        if (rmosVal == 1.0) /*!!ObsHi */
          minRate = RMOSHI;
        else /* rfpVal == ObsNo && rmosVal == ObsNo */
          //!!minRate = 0;
          return; /* No actual limit on the storage. */
  int lowEnd = (int)endDom.getLowerBound();
  if (lowEnd < MINSTORE)
    lowEnd = MINSTORE;
  int hiEnd = (int)endDom.getUpperBound();
  if (hiEnd > MAXSTORE)
    hiEnd = MAXSTORE;
  int lo = lowEnd, hi = hiEnd;
  int lowDur = (int)durDom.getLowerBound();
  if (lowDur < MINSTORE/minRate)
    lowDur = MINSTORE/minRate;
  int hiDur = (int)durDom.getUpperBound();
  if (hiDur > MAXSTORE/minRate)
    hiDur = MAXSTORE/minRate;
  lo = lowDur*minRate;
  if (lo > hiDur*minRate)
    lo -= hiDur*minRate;
  else
    lo = 0;
  hi = hiDur*minRate;
  if (hi > lowDur*minRate)
    hi -= lowDur*minRate;
  else
    hi = 0;
  lo += lowEnd;
  hi += hiEnd;
  if (lo > hi)
    outDom.empty();
  else
    outDom.intersect(IntervalIntDomain(lo, hi));
}

COMPUTE_PLAYBACK_DURATION::COMPUTE_PLAYBACK_DURATION(const LabelStr& name,
                                                     const LabelStr& propagatorName,
                                                     const ConstraintEngineId& constraintEngine,
                                                     const std::vector<ConstrainedVariableId>& variables)
  : Constraint(name, propagatorName, constraintEngine, variables) {
  check_error(m_variables.size() == 6);
  check_error(getCurrentDomain(m_variables[0]).isNumeric());
  check_error(getCurrentDomain(m_variables[1]).isNumeric());
  check_error(getCurrentDomain(m_variables[2]).isNumeric());
  check_error(getCurrentDomain(m_variables[3]).isNumeric());
  check_error(getCurrentDomain(m_variables[4]).isNumeric());
  check_error(getCurrentDomain(m_variables[5]).isNumeric());
}

void COMPUTE_PLAYBACK_DURATION::handleExecute() {
  IntervalIntDomain& outDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[0]));
  IntervalIntDomain& startStorDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[1]));
  IntervalIntDomain& endStorDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[2]));
  IntervalIntDomain& rateDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[3]));
  // unused in original: IntervalIntDomain& startDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[4]));
  // unused in original: IntervalIntDomain& endDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[5]));
  int sSDUB = (int)startStorDom.getUpperBound();
  if (sSDUB > MAXSTORE)
    sSDUB = MAXSTORE;
  int eSDLB = (int)endStorDom.getLowerBound();
  if (eSDLB < MINSTORE)
    eSDLB = MINSTORE;
  int eSDUB = (int)endStorDom.getUpperBound();
  if (eSDUB > MAXSTORE)
    eSDUB = MAXSTORE;
  int sSDLB = (int)startStorDom.getLowerBound();
  if (sSDLB < MINSTORE)
    sSDLB = MINSTORE;
  int rDUB = (int)rateDom.getUpperBound();
  if (rDUB > MAXSTORE)
    rDUB = MAXSTORE;
  int rDLB = (int)rateDom.getLowerBound();
  if (rDLB < 1)
    rDLB = 1;
  int erdur = ERDURMIN;
  if (sSDUB - eSDLB >= rDLB*ERDURMIN)
    erdur = (sSDUB - eSDLB)/rDLB;
  int mindur = MIN_PLAYBACK_DUR;
  if (sSDLB - eSDUB >= rDUB*MIN_PLAYBACK_DUR) {
    mindur = (sSDLB - eSDUB)/rDUB;
    if (mindur < MIN_PLAYBACK_DUR)
      mindur = MIN_PLAYBACK_DUR;
  }
  if (mindur > erdur)
    outDom.empty();
  else
    outDom.intersect(IntervalIntDomain(mindur, erdur));
}

BOUNDS_PLAYBACK_START_STORAGE::BOUNDS_PLAYBACK_START_STORAGE(const LabelStr& name,
                                                             const LabelStr& propagatorName,
                                                             const ConstraintEngineId& constraintEngine,
                                                             const std::vector<ConstrainedVariableId>& variables)
  : Constraint(name, propagatorName, constraintEngine, variables) {
  check_error(m_variables.size() == 4);
  check_error(getCurrentDomain(m_variables[0]).isNumeric());
  check_error(getCurrentDomain(m_variables[1]).isNumeric());
  check_error(getCurrentDomain(m_variables[2]).isNumeric());
  check_error(getCurrentDomain(m_variables[3]).isNumeric());
}

void BOUNDS_PLAYBACK_START_STORAGE::handleExecute() {
  IntervalIntDomain& outDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[0]));
  IntervalIntDomain& endStorDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[1]));
  IntervalIntDomain& rateDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[2]));
  IntervalIntDomain& durDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[3]));
  int endMin = (int)endStorDom.getLowerBound();
  if (endMin < MINSTORE)
    endMin = MINSTORE;
  int endMax = (int)endStorDom.getUpperBound();
  if (endMax > MAXSTORE)
    endMax = MAXSTORE;
  int minRate = (int)rateDom.getLowerBound();
  if (minRate < 1)
    minRate = 1;
  int maxRate = (int)rateDom.getUpperBound();
  if (maxRate > MAXSTORE)
    maxRate = MAXSTORE;
  int minDur = (int)durDom.getLowerBound();
  if (minDur < 1)
    minDur = 1;
  int maxDur = (int)durDom.getUpperBound();
  if (maxDur > MAXSTORE)
    maxDur = MAXSTORE;
  int lower = endMin + minRate*minDur;
  if (lower < MINSTORE)
    lower = MINSTORE;
  int upper = MAXSTORE;
  if (maxRate != MAXSTORE && maxDur != MAXSTORE)
    if ((MAX_FINITE_TIME - endMax)/maxRate/maxDur > 0) {
      upper = endMax + maxRate*maxDur;
      if (upper > MAXSTORE)
        upper = MAXSTORE;
    }
  if (lower > upper)
    outDom.empty();
  else
    outDom.intersect(IntervalIntDomain(lower, upper));
}

BOUNDS_PLAYBACK_END_STORAGE::BOUNDS_PLAYBACK_END_STORAGE(const LabelStr& name,
                                                         const LabelStr& propagatorName,
                                                         const ConstraintEngineId& constraintEngine,
                                                         const std::vector<ConstrainedVariableId>& variables)
  : Constraint(name, propagatorName, constraintEngine, variables) {
  check_error(m_variables.size() == 4);
  check_error(getCurrentDomain(m_variables[0]).isNumeric());
  check_error(getCurrentDomain(m_variables[1]).isNumeric());
  check_error(getCurrentDomain(m_variables[2]).isNumeric());
  check_error(getCurrentDomain(m_variables[3]).isNumeric());
}

void BOUNDS_PLAYBACK_END_STORAGE::handleExecute() {
  IntervalIntDomain& outDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[0]));
  IntervalIntDomain& startStorDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[1]));
  IntervalIntDomain& rateDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[2]));
  IntervalIntDomain& durDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[3]));
  int startMin = MINSTORE;
  if (startStorDom.getLowerBound() > MINSTORE)
    startMin = (int)startStorDom.getLowerBound();
  int startMax = MAXSTORE;
  if (startStorDom.getUpperBound() < MAXSTORE)
    startMax = (int)startStorDom.getUpperBound();
  int minRate = 1;
  if (rateDom.getLowerBound() > 1)
    minRate = (int)rateDom.getLowerBound();
  int maxRate = MAXSTORE;
  if (rateDom.getUpperBound() < MAXSTORE)
    maxRate = (int)rateDom.getUpperBound();
  int minDur = 1;
  if (durDom.getLowerBound() > 1)
    minDur = (int)durDom.getLowerBound();
  int maxDur = MAXSTORE;
  if (durDom.getUpperBound() < MAXSTORE)
    maxDur = (int)durDom.getUpperBound();
  int lower = 0;
  if (maxRate != MAXSTORE && maxDur != MAXSTORE)
    if (MAX_FINITE_TIME/maxRate/maxDur > 0) {
      lower = startMin - maxRate*maxDur;
      if (lower < MINSTORE)
        lower = MINSTORE;
    }
  int upper = startMax - minRate*minDur;
  if (lower > upper)
    outDom.empty();
  else
    outDom.intersect(IntervalIntDomain(lower, upper));
}

FIGURE_EARLIER_OP_IDS::FIGURE_EARLIER_OP_IDS(const LabelStr& name,
                                             const LabelStr& propagatorName,
                                             const ConstraintEngineId& constraintEngine,
                                             const std::vector<ConstrainedVariableId>& variables)
  : Constraint(name, propagatorName, constraintEngine, variables) {
  check_error(m_variables.size() == 2);
  check_error(getCurrentDomain(m_variables[0]).isNumeric());
  check_error(getCurrentDomain(m_variables[1]).isNumeric());
  // readTestCases() presently has no way to tell that numbers in files should be integer rather than real, so ...
  // check_error(getCurrentDomain(m_variables[0]).minDelta() >= 1.0);
  // check_error(getCurrentDomain(m_variables[1]).minDelta() >= 1.0);
}

void FIGURE_EARLIER_OP_IDS::handleExecute() {
  IntervalIntDomain& outDom = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[0]));
  IntervalIntDomain& ids = static_cast<IntervalIntDomain&>(getCurrentDomain(m_variables[1]));
  if (ids.getUpperBound() < 1)
    outDom.empty();
  else
    if (ids.getUpperBound() < PLUS_INFINITY)
      outDom.intersect(IntervalIntDomain(0, (int)(ids.getUpperBound() - 1)));
}
