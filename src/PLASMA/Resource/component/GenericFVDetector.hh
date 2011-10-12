#ifndef _H_GenericFVDetector
#define _H_GenericFVDetector

#include "FVDetector.hh"
#include "Instant.hh"

namespace EUROPA {
  /**
     * @class GenericFVDetector
     * @brief Detects all resource flaws and violations (6 of each)
     *
     * First, there are the upper/lower levels (each of is actually two profiles (max and min), since producers/consumers can have variable
     * quantities), which must be compared with upper/lower limits.  How violations
     * and flaws are determined here depends on whether we assume an open or closed world (hence the need to subclass).  In the open
     * world, more instants might be created - for example, having a lower level above the upper limit could be ok if new consumption
     * occurrences could take it back down again.
     *
     * Second, besides upper/lower levels, there are 4 numbers to check:  {cumulative,instantaneous} X {consumption,production}
     *     For each, a violation occurs if current min > max allowed
     *     For each, a flaw occurs if current max > max allowed
     *
     * This class checks all of the above cases, even though most situations only require a subset.  For example, reusable
     * resources don't care about production (those tests will always pass and are therefore unnecessary).  If this becomes
     * a computational burden, we could split the above 6 cases into components and a user could select whatever subset is required.
     *
     * NOTE:  consumption/production quantities don't need open vs. closed world differences because new pieces can only increase
     * production/consumption and can therefore not help keep the amounts below their limits.
     *
     * HISTORICAL NOTE:
     * a) TimelineFVDetector used the open-world assumption and did not check any flaws
     * b) ReuslabeFVDetector used the close-world assumption, ignored production violations and flaws, and reported consumption flaws
     *    as violations.
     */

    class GenericFVDetector : public FVDetector {
    public:
      GenericFVDetector(const ResourceId res);
      bool detect(const InstantId inst);

    protected:
      edouble m_maxInstConsumption, m_maxInstProduction;
      edouble m_maxCumulativeConsumption, m_maxCumulativeProduction;

      // Second version requires subclassing to handle open vs. closed world assumption
      Resource::ProblemType getResourceViolation(const InstantId inst) const;
      virtual Resource::ProblemType getResourceLevelViolation(const InstantId inst) const = 0;

      // Handling flaws does more than just report there is one
      // Second version requires subclassing to handle open vs. closed world assumption
      void handleResourceFlaws(const InstantId inst);
      virtual void handleResourceLevelFlaws(const InstantId inst) = 0;

      edouble getLowerLimit(const InstantId& inst) const;
      edouble getUpperLimit(const InstantId& inst) const;
    };
}

#endif
