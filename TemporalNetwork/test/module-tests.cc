#include "TestSupport.hh"
#include "TemporalNetwork.hh"

#include <iostream>
#include <string>
#include <cassert>
#include <list>

#ifdef __sun
#include <strstream>
typedef std::strstream sstream;
#else
#include <sstream>
typedef std::stringstream sstream;
#endif

class TemporalNetworkTest {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testTimlineCanPrecedeTest);
    runTest(testFixForReversingEndpoints);
    return true;
  }

private:
  static bool testBasicAllocation(){
    assert(g_noTime() != g_infiniteTime() && g_noTime() != -g_infiniteTime() && g_infiniteTime() != -g_infiniteTime());
    TemporalNetwork tn;
    TimepointId origin = tn.getOrigin();
    Time delta = g_noTime();
    Time epsilon = g_noTime();
    tn.getTimepointBounds(origin, delta, epsilon);
    assert(delta == 0 && epsilon == 0);

    tn.calcDistanceBounds(origin, origin, delta, epsilon);
    assert(delta == 0 && epsilon == 0);
    return true;
  }

  static bool testTimlineCanPrecedeTest(){
    TemporalNetwork tn;
    TimepointId a_end = tn.addTimepoint();
    TimepointId b_start = tn.addTimepoint();
    TimepointId b_end = tn.addTimepoint();
    TimepointId c_start = tn.addTimepoint();
    TemporalConstraintId a_before_b = tn.addTemporalConstraint(a_end, b_start, 0, g_infiniteTime());
    TemporalConstraintId start_before_end = tn.addTemporalConstraint(b_start, b_end, 1, g_infiniteTime());
    TemporalConstraintId a_meets_c = tn.addTemporalConstraint(a_end, c_start, 0, 0);
    assert(tn.isConsistent());

    Time dist_lb, dist_ub;
    tn.calcDistanceBounds(c_start, b_end, dist_lb, dist_ub);
    assert(dist_lb > 0);

    // Force failure where b meets c
    TemporalConstraintId b_meets_c = tn.addTemporalConstraint(b_end, c_start, 0, 0);
    assert(!tn.isConsistent());

    // Cleanup
    tn.removeTemporalConstraint(b_meets_c);
    tn.removeTemporalConstraint(a_meets_c);
    tn.removeTemporalConstraint(start_before_end);
    tn.removeTemporalConstraint(a_before_b);
    tn.deleteTimepoint(c_start);
    tn.deleteTimepoint(b_end);
    tn.deleteTimepoint(b_start);
    tn.deleteTimepoint(a_end);
    return true;
  }

  static bool testFixForReversingEndpoints(){
    TemporalNetwork tn;

    // Allocate timepoints
    TimepointId x = tn.getOrigin();
    TimepointId y = tn.addTimepoint();
    TimepointId z = tn.addTimepoint();

    TemporalConstraintId fromage = tn.addTemporalConstraint(x, y, (Time)0, g_infiniteTime());
    TemporalConstraintId tango = tn.addTemporalConstraint(y, x, 200, 200);

    assert(!tn.isConsistent());

    tn.removeTemporalConstraint(fromage);
    tn.removeTemporalConstraint(tango);
    assert(tn.isConsistent()); // Consistency restored

    TemporalConstraintId c0 = tn.addTemporalConstraint(y, x, -200, g_infiniteTime());
    TemporalConstraintId c1 = tn.addTemporalConstraint(x, z, 0, g_infiniteTime());
    TemporalConstraintId c2 = tn.addTemporalConstraint(z, y, (Time)0, g_infiniteTime());
    TemporalConstraintId c3 = tn.addTemporalConstraint(x, y, 200, g_infiniteTime());
    assert(tn.isConsistent());

    // Clean up
    tn.removeTemporalConstraint(c0);
    tn.removeTemporalConstraint(c1);
    tn.removeTemporalConstraint(c2);
    tn.removeTemporalConstraint(c3);
    tn.deleteTimepoint(y);
    tn.deleteTimepoint(z);
    return true;
  }
};
int main() {
  //initConstraintLibrary();
  runTestSuite(TemporalNetworkTest::test);
  std::cout << "Finished" << std::endl;
}
