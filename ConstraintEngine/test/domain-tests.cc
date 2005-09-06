#include "TestSupport.hh"
#include "LabelStr.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "EnumeratedDomain.hh"
#include "StringDomain.hh"
#include "NumericDomain.hh"
#include "SymbolDomain.hh"
#include "DomainListener.hh"
#include "module-tests.hh"
#include <sstream>
#include <cmath>

namespace EUROPA {

  class ChangeListener : public DomainListener {
  public:
    ChangeListener()
      : m_changed(false), m_change(RESET) {
    }

    void notifyChange(const ChangeType& change) {
      m_changed = true;
      m_change = change;
    }

    bool checkAndClearChange(ChangeType& change) {
      bool result = m_changed;
      change = m_change;
      m_changed = false;
      return(result);
    }

  private:
    bool m_changed;
    ChangeType m_change;
  };

  class IntervalDomainTest {
  public:
    static bool test() {
      runTest(testAllocation); 
      runTest(testRelaxation); 
      runTest(testIntersection);  
      runTest(testSubset);
      runTest(testPrinting);
      runTest(testBoolDomain);
      runTest(testDifference);
      runTest(testOperatorEquals);
      runTest(testInfinitesAndInts);
      runTest(testEnumSet); 
      runTest(testInsertAndRemove);
      runTest(testValidComparisonWithEmpty_gnats2403);
      runTest(testIntervalSingletonValues);
      runTest(testIntervalIntValues);
      return(true);
    }

  private:
    static bool testAllocation() {
      IntervalDomain realDomain(10.2,20.4);
      assertFalse(realDomain.isEmpty());
      assertFalse(realDomain.isFinite());
      assert(realDomain.getSize() == PLUS_INFINITY);

      IntervalIntDomain intDomain(10, 20);
      assertTrue(intDomain.isFinite());

      double lb, ub;
      intDomain.getBounds(lb,ub);
      assertTrue(lb<ub);
      assertTrue(!intDomain.isMember(1.889));
      assertTrue(!intDomain.isOpen());
      IntervalIntDomain d1(intDomain);
      d1.empty();
      assertTrue(d1.isEmpty());

      AbstractDomain& d2 = static_cast<AbstractDomain&>(intDomain);
      assertTrue(!d2.isEmpty());

      IntervalIntDomain d3(static_cast<IntervalIntDomain&>(intDomain));
      IntervalIntDomain d4;

      assertTrue( ! (d3 == d4));
      d3.relax(d4);
      assertTrue(d3 == d4);

      assertTrue(d2 != d4);
      d2.relax(d4);
      assertTrue(d2 == d4);
      return(true);
    }

    static bool testRelaxation() {
      ChangeListener l_listener;
      IntervalIntDomain dom0; // Will have very large default range
      IntervalIntDomain dom1(-100, 100);
      dom1.setListener(l_listener.getId());
      dom1.relax(dom0);
      DomainListener::ChangeType change;
      bool res = l_listener.checkAndClearChange(change);
      assertTrue(res && change == DomainListener::RELAXED);
      assertTrue(dom1.isSubsetOf(dom0));
      assertTrue(dom0.isSubsetOf(dom1));
      assertTrue(dom1 == dom0);

      IntervalIntDomain dom2(-300, 100);
      dom1.intersect(dom2);
      res = l_listener.checkAndClearChange(change);
      assertTrue(res);
      assertTrue(dom1 == dom2);
      dom1.relax(dom2);
      res = l_listener.checkAndClearChange(change);
      assertTrue(!res);
      return(true);
    }

    static bool testIntersection() {
      ChangeListener l_listener;
      IntervalIntDomain dom0; // Will have very large default range
      dom0.setListener(l_listener.getId());

      // Execute intersection and verify results
      IntervalIntDomain dom1(-100, 100);
      dom0.intersect(dom1);
      DomainListener::ChangeType change;
      bool res = l_listener.checkAndClearChange(change);
      assertTrue(res);
      assertTrue(dom0 == dom1);
    
      // verify no change triggered if none should take place.
      dom0.intersect(dom1);
      res = l_listener.checkAndClearChange(change);
      assertTrue(!res);

      // Verify only the upper bound changes
      IntervalIntDomain dom2(-200, 50);
      dom0.intersect(dom2);
      res = l_listener.checkAndClearChange(change);
      assertTrue(res);
      assertTrue(dom0.getLowerBound() == dom1.getLowerBound());
      assertTrue(dom0.getUpperBound() == dom2.getUpperBound());
    
      // Make an intersection that leads to an empty domain
      IntervalIntDomain dom3(500, 1000);
      dom0.intersect(dom3);
      res = l_listener.checkAndClearChange(change);
      assertTrue(res);
      assertTrue(dom0.isEmpty());

      IntervalDomain dom4(0.98, 101.23);
      IntervalDomain dom5(80, 120.44);
      IntervalDomain dom6(80, 101.23);
      dom4.equate(dom5);
      assertTrue(dom4 == dom6);
      assertTrue(dom5 == dom6);

      IntervalDomain dom7(-1, 0);
      dom6.intersect(dom7);
      assertTrue(dom6.isEmpty());


      IntervalDomain dom8;
      IntervalDomain dom9;
      dom8.intersect(IntervalDomain(0.1, 0.10));
      dom9.intersect(IntervalDomain(0.10, 0.10));
      assertTrue(dom8.intersects(dom9));
          
      // Case added to recreate failure case for GNATS 3045
      
      IntervalDomain dom8a;
      IntervalDomain dom9a;
      dom8a.intersect(IntervalDomain(0.1, 0.1));
      dom9a.intersect(IntervalDomain(0.1, 0.1));
      assertTrue(dom8a.intersects(dom9a));
          
       assertTrue (dom8a.getUpperBound() == 0.1);
       assertTrue (dom8a.getLowerBound() == 0.1);
       assertTrue (dom9a.getUpperBound() == 0.1);
       assertTrue (dom9a.getLowerBound() == 0.1);

      // Test at the limit of precision
      IntervalDomain dom10(0.0001);
      IntervalDomain dom11(0.0001);
      assertTrue(dom10.intersects(dom11));

      // Test at the limit of precision
      IntervalDomain dom12(-0.0001);
      IntervalDomain dom13(-0.0001);
      assertTrue(dom12.intersects(dom13));

      // Test beyond the limits of precission
      IntervalDomain dom14(-0.1 - EPSILON/10);
      IntervalDomain dom15(-0.1);
      // POSSIBLE BUG? assertTrue(dom14.intersects(dom15), dom14.toString() + " cannot intersect " + dom15.toString());
      return(true);
    }

    static bool testSubset() {
      IntervalIntDomain dom0(10, 35);
      IntervalDomain dom1(0, 101);
      assertTrue(dom0.isSubsetOf(dom1));
      assertTrue(! dom1.isSubsetOf(dom0));

      // Handle cases where domains are equal
      IntervalIntDomain dom2(dom0);
      assertTrue(dom2 == dom0);
      assertTrue(dom0.isSubsetOf(dom2));
      assertTrue(dom2.isSubsetOf(dom0));

      // Handle case with no intersection
      IntervalIntDomain dom3(0, 9);
      assertTrue(! dom3.isSubsetOf(dom0));
      assertTrue(! dom0.isSubsetOf(dom3));

      // Handle case with partial intersection
      IntervalIntDomain dom4(0, 20);
      assertTrue(! dom4.isSubsetOf(dom0));
      assertTrue(! dom0.isSubsetOf(dom4));

      // Handle intersection with infinites
      IntervalDomain dom5; 
      IntervalDomain dom6(0, 100);
      assertTrue(dom6.isSubsetOf(dom5)); 
      return(true);
    }

    static bool testListener() {
      return(true);
    }

    static bool testPrinting() {
      IntervalIntDomain d1(1, 100);
      std::stringstream ss1;
      d1 >> ss1;
      std::string actualString = ss1.str();
      std::string expectedString("INT_INTERVAL:CLOSED[1, 100]");
      assertTrue(actualString == expectedString);
      std::string anotherActualString = d1.toString();
      assertTrue(anotherActualString == expectedString);

      std::string integerType = "integer";
      IntervalIntDomain d2(1, 100, integerType.c_str());
      std::stringstream ss2;
      d2 >> ss2;
      std::string actualString2 = ss2.str();
      std::string expectedString2("integer:CLOSED[1, 100]");
      assertTrue(actualString2 == expectedString2);
      std::string anotherActualString2 = d2.toString();
      assertTrue(anotherActualString2 == expectedString2);

      return(true);
    }

    static bool testBoolDomain() {
      BoolDomain dom0(true);
      assertTrue(dom0.getSize() == 1);
      assertTrue(dom0.getUpperBound() == true);
      assertTrue(dom0.getLowerBound() == true);

      BoolDomain dom1;
      assertTrue(dom1.getSize() == 2);
      assertTrue(dom1.getUpperBound() == true);
      assertTrue(dom1.getLowerBound() == false);

      dom1.intersect(dom0);
      assertTrue(dom1 == dom0);
      return(true);
    }

    static bool testDifference() {
      IntervalDomain dom0(1, 10);
      IntervalDomain dom1(11, 20);
      bool res = dom0.difference(dom1);
      assertTrue(!res);
      res = dom1.difference(dom0);
      assertTrue(!res);

      IntervalDomain dom2(dom0);
      res = dom2.difference(dom0);
      assertTrue(res);
      assertTrue(dom2.isEmpty());

      IntervalIntDomain dom3(5, 100);
      res = dom3.difference(dom0);
      assertTrue(res);
      assertTrue(dom3.getLowerBound() == 11);
      res = dom3.difference(dom1);
      assertTrue(res);
      assertTrue(dom3.getLowerBound() == 21);

      IntervalDomain dom4(0, 20);
      res = dom4.difference(dom1);
      assertTrue(res);
      double newValue = (dom1.getLowerBound() - dom4.minDelta());
      assertTrue(dom4.getUpperBound() == newValue);

      NumericDomain dom5(3.14159265);
      assertTrue(dom5.getSize() == 1);

      std::list<double> vals;
      vals.push_back(dom5.getSingletonValue());
      vals.push_back(1.2);
      vals.push_back(2.1);
      vals.push_back(PLUS_INFINITY);
      vals.push_back(MINUS_INFINITY);
      vals.push_back(EPSILON);
      NumericDomain dom6(vals);

      assertTrue(dom6.getSize() == 6);
      assertTrue(fabs(dom5.minDelta() - dom6.minDelta()) < EPSILON); // Should be ==, but allow some leeway.
      assertTrue(dom6.intersects(dom5));

      dom6.difference(dom5);
      assertTrue(!(dom6.intersects(dom5)));
      assertTrue(dom6.getSize() == 5);

      dom6.difference(dom5);
      assertTrue(!(dom6.intersects(dom5)));
      assertTrue(dom6.getSize() == 5);

      return(true);
    }

    static bool testOperatorEquals() {
      IntervalDomain dom0(1, 28);
      IntervalDomain dom1(50, 100);
      dom0 = dom1;
      assertTrue(dom0 == dom1);
      return(true);
    }

    static bool testInfinitesAndInts() {
      IntervalDomain dom0;
      assertTrue(dom0.translateNumber(MINUS_INFINITY) == MINUS_INFINITY);
      assertTrue(dom0.translateNumber(MINUS_INFINITY - 1) == MINUS_INFINITY);
      assertTrue(dom0.translateNumber(MINUS_INFINITY + 1) == MINUS_INFINITY + 1);
      assertTrue(dom0.translateNumber(PLUS_INFINITY + 1) == PLUS_INFINITY);
      assertTrue(dom0.translateNumber(PLUS_INFINITY - 1) == PLUS_INFINITY - 1);
      assertTrue(dom0.translateNumber(2.8) == 2.8);

      IntervalIntDomain dom1;
      assertTrue(dom1.translateNumber(2.8, false) == 2);
      assertTrue(dom1.translateNumber(2.8, true) == 3);
      assertTrue(dom1.translateNumber(PLUS_INFINITY - 0.2, false) == PLUS_INFINITY - 1);
      return(true);
    }

    static bool testEnumSet(){
     
      EnumeratedDomain dom0(true, "Enum");
      assertTrue(dom0.isOpen());
      dom0.insert(1); // required for isMember precondition of set.
      dom0.set(1);
      assertTrue(dom0.isSingleton());
      return true;
    }

    static bool testEnumDomInsertAndRemove() {
      // Should add a loop like the one in
      //   testIntervalDomInsertAndRemove(). --wedgingt 2004 Mar 8

      NumericDomain enumDom1;
      assertTrue(enumDom1.isOpen());
      assertTrue(enumDom1.isNumeric());
      enumDom1.insert(3.14159265);
      enumDom1.close();
      assertTrue(enumDom1.isMember(3.14159265));
      assertTrue(enumDom1.isSingleton());
      assertTrue(enumDom1.isFinite());
      enumDom1.remove(5.0);
      assertTrue(enumDom1.isMember(3.14159265));
      assertTrue(enumDom1.isSingleton());
      assertTrue(enumDom1.isFinite());

      double minDiff = enumDom1.minDelta();
      assertTrue(minDiff >= EPSILON && EPSILON > 0.0);

      const double onePlus = 1.0 + 2.0*EPSILON;

      enumDom1.remove(3.14159265 - onePlus*minDiff);
      assertTrue(enumDom1.isMember(3.14159265));
      assertTrue(enumDom1.isSingleton());
      assertTrue(enumDom1.isFinite());
      enumDom1.remove(3.14159265 + onePlus*minDiff);
      assertTrue(enumDom1.isMember(3.14159265));
      assertTrue(enumDom1.isSingleton());
      assertTrue(enumDom1.isFinite());
      enumDom1.remove(3.14159265 - minDiff/onePlus);
      assertTrue(enumDom1.isEmpty());
      enumDom1.insert(3.14159265);
      assertTrue(enumDom1.isMember(3.14159265));
      assertTrue(enumDom1.isSingleton());
      assertTrue(enumDom1.isFinite());
      enumDom1.remove(3.14159265 + minDiff/onePlus);
      assertTrue(enumDom1.isEmpty());
      enumDom1.insert(3.14159265);
      assertTrue(enumDom1.isMember(3.14159265));
      assertTrue(enumDom1.isSingleton());

      std::list<double> vals;
      vals.push_back(enumDom1.getSingletonValue());
      vals.push_back(1.2);
      vals.push_back(2.1);
      vals.push_back(PLUS_INFINITY);
      vals.push_back(MINUS_INFINITY);
      vals.push_back(EPSILON);
      NumericDomain enumDom2(vals);

      assertTrue(!(enumDom2.isOpen()));
      assertTrue(enumDom2.isNumeric());
      assertTrue(enumDom2.isFinite());
      assertTrue(enumDom2.getSize() == 6);

      double minDiff2 = enumDom2.minDelta();
      assertTrue(fabs(minDiff - minDiff2) < EPSILON);

      enumDom2.remove(1.2 - minDiff2/onePlus);
      assertTrue(enumDom2.getSize() == 5);

      enumDom2.remove(MINUS_INFINITY);
      assertTrue(enumDom2.getSize() == 4);

      // Remove a value near but not "matching" a member and
      //   verify the domain has not changed.
      enumDom2.remove(enumDom1.getSingletonValue() - onePlus*minDiff2);
      assertTrue(enumDom2.intersects(enumDom1));
      assertTrue(enumDom2.getSize() == 4);

      // Remove a value near but not equal a member and
      //   verify the member was removed via intersection.
      enumDom2.remove(enumDom1.getSingletonValue() - minDiff2/onePlus);
      assertTrue(!(enumDom2.intersects(enumDom1)));
      assertTrue(enumDom2.getSize() == 3);

      // Add a value near a value from another domain
      //   verify that the resulting domain intersects the other domain.
      enumDom2.insert(enumDom1.getSingletonValue() + minDiff2/onePlus);
      assertTrue(enumDom2.intersects(enumDom1));
      assertTrue(enumDom2.getSize() == 4);

      // Add the value in the other domain and
      //   verify the domain is not affected.
      enumDom2.insert(enumDom1.getSingletonValue());
      assertTrue(enumDom2.intersects(enumDom1));
      assertTrue(enumDom2.getSize() == 4);

      // Remove a value that should not be a member but is
      //   only slightly too large to "match" the new member.
      enumDom2.remove(enumDom1.getSingletonValue() + minDiff2/onePlus + onePlus*minDiff2);
      assertTrue(enumDom2.intersects(enumDom1));
      assertTrue(enumDom2.getSize() == 4);

      // Remove a value "matching" the added value but larger
      //   and verify the domain no longer intersects the other domain.
      enumDom2.remove(enumDom1.getSingletonValue() + 2.0*minDiff2/onePlus);
      assertTrue(enumDom2.getSize() == 3);
      assertTrue(!(enumDom2.intersects(enumDom1)));

      return(true);
    }
    /* NO LONGER SUPPORT INSERT AND REMOVE ON INTERVAL DOMAINS
    static bool testIntervalDomInsertAndRemove() {
      assertTrue(EPSILON > 0.0); // Otherwise, loop will be infinite.

      // Making this any closer to 1.0 fails first iDom.isEmpty() assert,
      //   at least on SunOS 5.8 with g++ 2.95.2.
      const double onePlus = 1.001;

      // For IntervalDomains, insert() and remove() have very
      // restricted usefulness, since only singleton and empty domains
      // work with insert and remove unless the value given is already
      // in (for insert) or already outside (for remove) the domain,
      // so the tests can be fairly extensive yet "automated".

      // Note, however, that broadening the extent of this loop to
      // values for which the hardware will not be able to distinguish
      // between x and x + minDelta will not work and the current
      // implementation of IntervalDomain does not support such
      // (member) values.  However, the values used for infinity by
      // the temporal network implementation is much narrower than
      // that (presently, at least): 268435455 (CommonDefs.hh).

      for (double val = -2.6e8; val <= 2.6e8; ) {
        IntervalDomain iDom(val);
        double minDiff = iDom.minDelta();

        iDom.remove(val + onePlus*minDiff);
        assertTrue(iDom.isSingleton());
        assertTrue(iDom.isMember(val));
        assertTrue(iDom.isMember(val + minDiff/onePlus));
        assertTrue(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val + minDiff/onePlus);
        assertTrue(iDom.isEmpty());
        assertTrue(!(iDom.isMember(val)));

        iDom.insert(val);
        assertTrue(iDom.isSingleton());
        assertTrue(iDom.isMember(val));
        assertTrue(iDom.isMember(val + minDiff/onePlus));
        assertTrue(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val - onePlus*minDiff);
        assertTrue(iDom.isSingleton());
        assertTrue(iDom.isMember(val));
        assertTrue(iDom.isMember(val + minDiff/onePlus));
        assertTrue(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val - minDiff/onePlus);
        assertTrue(iDom.isEmpty());
        assertTrue(!(iDom.isMember(val)));

        if (val < 0.0)
          if (val > -EPSILON)
            val = 0.0;
          else
            val /= 3.14159265;
        else
          if (val > 0.0)
            val *= 2.7182818;
          else
            val = onePlus*EPSILON;
      }

      return(true);
    }

    static bool testIntervalIntDomInsertAndRemove() {
      IntervalIntDomain iiDom(-5, 10);
      assertTrue(iiDom.getSize() == 16);

      iiDom.remove(-6);
      assertTrue(iiDom.getSize() == 16);

      iiDom.remove(11);
      assertTrue(iiDom.getSize() == 16);

      iiDom.remove(PLUS_INFINITY);
      assertTrue(iiDom.getSize() == 16);

      iiDom.insert(-5);
      assertTrue(iiDom.getSize() == 16);

      iiDom.insert(-1);
      assertTrue(iiDom.getSize() == 16);

      iiDom.insert(10);
      assertTrue(iiDom.getSize() == 16);

      iiDom.insert(11);
      assertTrue(iiDom.getSize() == 17);

      iiDom.insert(-6);
      assertTrue(iiDom.getSize() == 18);

      iiDom.remove(PLUS_INFINITY);
      assertTrue(iiDom.getSize() == 18);

      iiDom.remove(-7);
      assertTrue(iiDom.getSize() == 18);

      iiDom.remove(11);
      assertTrue(iiDom.getSize() == 17);

      return(true);
    }
    */

    static bool testInsertAndRemove() {
      return(true);//testEnumDomInsertAndRemove());
    }

    static bool testValidComparisonWithEmpty_gnats2403(){
      IntervalIntDomain d0;
      IntervalIntDomain d1;
      NumericDomain d2;
      d2.insert(1);
      d2.close();

      assertTrue(d0 == d1);
      assertTrue(d1 != d2);
      d0.empty();
      assertTrue(d0 != d1);
      assertTrue(d0 != d2);

      assertTrue(AbstractDomain::canBeCompared(d0, d2));

      NumericDomain d3(d2);
      d2.empty();
      assertTrue(AbstractDomain::canBeCompared(d2, d3));
      assertTrue(d3 != d2);

      return true;
    }

    static bool testIntervalSingletonValues() {
      for (double value = -2.0 ; value <= 1.5 ; value += 0.1) {
        IntervalDomain id(value, value);
        std::list<double> values;
        id.getValues(values);
        assertTrue(values.size() == 1);
        assertTrue(values.front() == value);
      }
      for (double value = 2.0 ; value >= 1.5 ; value -= 0.1) {
        IntervalDomain id(value, value);
        std::list<double> values;
        id.getValues(values);
        assertTrue(values.size() == 1);
        assertTrue(values.front() == value);
      }
      IntervalDomain id(0, 0);
      std::list<double> values;
      id.getValues(values);
      assertTrue(values.size() == 1);
      assertTrue(values.front() == 0);
      return true;
    }

    static bool testIntervalIntValues() {
      std::list<double> values;

      IntervalIntDomain i0(10, 20);
      i0.getValues(values);
      assertTrue(values.size() == 11);
      assertTrue(values.front() == 10); values.pop_front();
      assertTrue(values.front() == 11); values.pop_front();
      assertTrue(values.front() == 12); values.pop_front();
      assertTrue(values.front() == 13); values.pop_front();
      assertTrue(values.front() == 14); values.pop_front();
      assertTrue(values.front() == 15); values.pop_front();
      assertTrue(values.front() == 16); values.pop_front();
      assertTrue(values.front() == 17); values.pop_front();
      assertTrue(values.front() == 18); values.pop_front();
      assertTrue(values.front() == 19); values.pop_front();
      assertTrue(values.front() == 20); values.pop_front();

      IntervalIntDomain i1(-4, 3);
      i1.getValues(values);
      assertTrue(values.size() == 8);
      assertTrue(values.front() == -4); values.pop_front();
      assertTrue(values.front() == -3); values.pop_front();
      assertTrue(values.front() == -2); values.pop_front();
      assertTrue(values.front() == -1); values.pop_front();
      assertTrue(values.front() ==  0); values.pop_front();
      assertTrue(values.front() ==  1); values.pop_front();
      assertTrue(values.front() ==  2); values.pop_front();
      assertTrue(values.front() ==  3); values.pop_front();

      return true;
    }

  };

  

  class EnumeratedDomainTest {
  public:

    static bool test() {
      runTest(testEnumerationOnly);
      runTest(testBasicLabelOperations);
      runTest(testLabelSetAllocations);
      runTest(testEquate);
      runTest(testValueRetrieval);
      runTest(testIntersection);
      runTest(testDifference);
      runTest(testOperatorEquals);
      runTest(testEmptyOnClosure);
      runTest(testOpenEnumerations);
      return(true);
    }

  private:

    static bool testOpenEnumerations() {
      EnumeratedDomain e1(true, "Test");
      ChangeListener l1;
      DomainListener::ChangeType change;
      e1.setListener(l1.getId());

      assertTrue(e1.isOpen());
      assertTrue(e1.isFinite());
      assertTrue(e1.isEmpty());
      e1.insert(1.0);
      assertTrue(e1.isOpen());

      e1.remove(1.0);
      assertTrue(l1.checkAndClearChange(change));
      assertTrue(change == DomainListener::VALUE_REMOVED);
      e1.insert(1.0);

      e1.close();
      assertTrue(e1.isClosed());
      assertTrue(l1.checkAndClearChange(change));
      assertTrue(change == DomainListener::CLOSED);

      e1.open();
      assertTrue(e1.isOpen());
      assertTrue(l1.checkAndClearChange(change));
      assertTrue(change == DomainListener::OPENED);

      e1.insert(2.0);
      e1.insert(3.0);
      e1.insert(4.0);

      std::list<double> vals;
      vals.push_back(2.0);
      vals.push_back(3.0);
      EnumeratedDomain e2(vals, true, "Test");

      assertTrue(e2.isClosed());

      e1.set(e2);
      assertTrue(e1.isClosed());
      
      e1.open();
      assertTrue(e1.isOpen());

      e1.insert(4.0);
      e1.set(4.0);
      assertTrue(e1.isClosed());
      e1.open();
      e1.insert(1.0);
      e1.insert(2.0);
      e1.insert(3.0);

      e1.intersect(e2);
      assertTrue(e1.isClosed());

      e1.open();
      e2.open();
      e1.intersect(e2);
      assertTrue(e1.isOpen());
      assertTrue(e2.isOpen());
      e2.close();

      e1.insert(4.0);
      e2.reset(e1);
      assertTrue(e2.isOpen());

      assertTrue(e1 == e2);
      e2.close();
      assertTrue(e1 == e2);
      e1.close();
      assertTrue(e1 == e2);

      e1.open();
      e2.open();
      assertTrue(e1.intersects(e2));
      e2.close();
      assertTrue(e1.intersects(e2));
      e1.close();
      assertTrue(e1.intersects(e2));
      
      e1.open();
      e2.open();
      assertTrue(e1.isSubsetOf(e2));
      e2.close();
      assertTrue(e1.isSubsetOf(e2));
      e1.close();
      assertTrue(e1.isSubsetOf(e2));

      e1.open();
      e2.open();

      e1.equate(e2);
      assertTrue(e1.isOpen() && e2.isOpen());
      e2.close();
      e1.equate(e2);
      assertTrue(e1.isClosed() && e2.isClosed());
      e2.open();
      e1.equate(e2);
      assertTrue(e1.isClosed() && e2.isClosed());
      return true;
    }

    static bool testEnumerationOnly() {
      std::list<double> values;
      values.push_back(-98.67);
      values.push_back(-0.01);
      values.push_back(1);
      values.push_back(2);
      values.push_back(10);
      values.push_back(11);

      NumericDomain d0(values);
      NumericDomain d1(values);
      assertTrue(d0 == d1);


      {
	double value(0);
	std::stringstream sstr;
	sstr << -0.01;
	assertTrue(d0.convertToMemberValue(sstr.str(), value));
	assertTrue(value == -0.01);
      }

      {
	double value(0);
	std::stringstream sstr;
	sstr << 88.46;
	assertFalse(d0.convertToMemberValue(sstr.str(), value));
	assertTrue(value == 0);
      }

      assertTrue(d0.isSubsetOf(d1));
      assertTrue(d0.isMember(-98.67));
      d0.remove(-0.01);
      assertTrue(!d0.isMember(-0.01));
      assertTrue(d0.isSubsetOf(d1));
      assertTrue(!d1.isSubsetOf(d0));

      return(true);
    }

    static bool testBasicLabelOperations() {
      int initialCount = EUROPA::LabelStr::getSize();
      EUROPA::LabelStr l1("L1");
      EUROPA::LabelStr l2("L2");
      EUROPA::LabelStr l3("L3");
      assertTrue(l1 < l2 && l2 < l3);

      EUROPA::LabelStr la("L");
      EUROPA::LabelStr l4("L30");
      EUROPA::LabelStr lb("L");
      assertTrue(la == lb);
      assertTrue(la < l4);

      EUROPA::LabelStr copy1(l1);
      assertTrue(l1 == copy1);
      assertTrue(l2 != copy1);

      assertTrue((EUROPA::LabelStr::getSize() - initialCount) == 5);
      assertTrue(l1.toString() == "L1");

      assertTrue(LabelStr::isString(l1.getKey()));
      assertTrue(!LabelStr::isString(PLUS_INFINITY+1));
      return(true);
    }

    static bool testLabelSetAllocations() {
      std::list<double> values;
      values.push_back(EUROPA::LabelStr("L1"));
      values.push_back(EUROPA::LabelStr("L4"));
      values.push_back(EUROPA::LabelStr("L2"));
      values.push_back(EUROPA::LabelStr("L5"));
      values.push_back(EUROPA::LabelStr("L3"));

      ChangeListener l_listener;
      LabelSet ls0(values);
      ls0.setListener(l_listener.getId());
      assertTrue(!ls0.isOpen());

      EUROPA::LabelStr l2("L2");
      assertTrue(ls0.isMember(l2));
      DomainListener::ChangeType change;
      ls0.remove(l2);
      bool res = l_listener.checkAndClearChange(change);
      assertTrue(res && change == DomainListener::VALUE_REMOVED);
      assertTrue(!ls0.isMember(l2));

      EUROPA::LabelStr l3("L3");
      ls0.set(l3);
      assertTrue(ls0.isMember(l3));
      assertTrue(ls0.getSize() == 1);

      LabelSet ls1(values);
      ls0.relax(ls1);
      res = l_listener.checkAndClearChange(change);
      assertTrue(res && change == DomainListener::RELAXED);
      assertTrue(ls0 == ls1);
      return(true);
    }

    static bool testEquate() {
      std::list<double> baseValues;
      baseValues.push_back(EUROPA::LabelStr("A"));
      baseValues.push_back(EUROPA::LabelStr("B"));
      baseValues.push_back(EUROPA::LabelStr("C"));
      baseValues.push_back(EUROPA::LabelStr("D"));
      baseValues.push_back(EUROPA::LabelStr("E"));
      baseValues.push_back(EUROPA::LabelStr("F"));
      baseValues.push_back(EUROPA::LabelStr("G"));
      baseValues.push_back(EUROPA::LabelStr("H"));

      ChangeListener l_listener;
      LabelSet ls0(baseValues);
      ls0.setListener(l_listener.getId());
      LabelSet ls1(baseValues);
      ls1.setListener(l_listener.getId());

      assertTrue(ls0 == ls1);
      assertTrue(ls0.getSize() == 8);
      bool res = ls0.equate(ls1);
      assertTrue(res == false); // Implying no change occured

      EUROPA::LabelStr lC("C");
      ls0.remove(lC);
      assertTrue(!ls0.isMember(lC));
      assertTrue(ls1.isMember(lC));
      res = ls0.equate(ls1);
      assertTrue(res); // It should have changed
      assertTrue(!ls1.isMember(lC));

      LabelSet ls2(baseValues);
      ls2.setListener(l_listener.getId());
      ls2.remove(EUROPA::LabelStr("A"));
      ls2.remove(EUROPA::LabelStr("B"));
      ls2.remove(EUROPA::LabelStr("C"));
      ls2.remove(EUROPA::LabelStr("D"));
      ls2.remove(EUROPA::LabelStr("E"));

      LabelSet ls3(baseValues);
      ls3.setListener(l_listener.getId());
      EUROPA::LabelStr lA("A");
      EUROPA::LabelStr lB("B");
      ls3.remove(lA);
      ls3.remove(lB);
      ls3.remove(lC);
      res = ls2.equate(ls3);
      assertTrue(res);
      assertTrue(ls2 == ls3);

      LabelSet ls4(baseValues);
      ls4.setListener(l_listener.getId());
      ls4.remove(EUROPA::LabelStr("A"));
      ls4.remove(EUROPA::LabelStr("B"));
      ls4.remove(EUROPA::LabelStr("C"));
      ls4.remove(EUROPA::LabelStr("D"));
      ls4.remove(EUROPA::LabelStr("E"));

      LabelSet ls5(baseValues);
      ls5.setListener(l_listener.getId());
      ls5.remove(EUROPA::LabelStr("F"));
      ls5.remove(EUROPA::LabelStr("G"));
      ls5.remove(EUROPA::LabelStr("H"));

      DomainListener::ChangeType change;
      ls4.equate(ls5);
      res = l_listener.checkAndClearChange(change);
      assertTrue(res && change == DomainListener::EMPTIED);
      assertTrue(ls4.isEmpty() || ls5.isEmpty());
      assertTrue(!(ls4.isEmpty() && ls5.isEmpty()));

      std::list<double> enumVals;
      enumVals.push_back(1.0);
      enumVals.push_back(2.5);
      enumVals.push_back(-0.25);
      enumVals.push_back(3.375);
      enumVals.push_back(-1.75);
      NumericDomain ed1(enumVals);
      NumericDomain ed3(enumVals);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      NumericDomain ed2(enumVals);
      NumericDomain ed4(enumVals);

      ed1.equate(ed2);
      assertTrue(ed1 == ed2);

      ed1.equate(ed3);
      assertTrue(ed1 == ed3);

      enumVals.clear();
      enumVals.push_back(0.0);
      NumericDomain ed0(enumVals);

      ed1.equate(ed0);

      // This is actually false because equate() only empties
      // one of the domains when the intersection is empty.
      // assertTrue(ed0 == ed1);

      assertFalse(ed0 == ed1);
      assertTrue(ed1.isEmpty() != ed0.isEmpty());

      ed0 = NumericDomain(enumVals);
      assertTrue(!ed0.isEmpty());

      ed0.equate(ed2);
      assertTrue(ed2 != ed0 && ed2.isEmpty() != ed0.isEmpty());

      enumVals.push_back(20.0); // Now 0.0 and 20.0
      ed0 = NumericDomain(enumVals);
      assertTrue(!ed0.isEmpty() && !ed0.isSingleton());

      IntervalDomain id0(-10.0, 10.0);

      id0.equate(ed0);
      assertTrue(ed0.isSingleton() && ed0.getSingletonValue() == 0.0);
      assertTrue(id0.isSingleton() && id0.getSingletonValue() == 0.0);

      ed0 = NumericDomain(enumVals); // Now 0.0 and 20.0
      assertTrue(!ed0.isEmpty() && !ed0.isSingleton());

      IntervalDomain id1(0.0, 5.0);

      ed0.equate(id1);
      assertTrue(ed0.isSingleton() && ed0.getSingletonValue() == 0.0);
      assertTrue(id1.isSingleton() && id1.getSingletonValue() == 0.0);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      enumVals.push_back(1.5);
      NumericDomain ed5(enumVals);
      IntervalDomain id2(2.5, 3.0);

      ed5.equate(id2);
      assertTrue(ed5.isSingleton() && ed5.getSingletonValue() == 2.5);
      assertTrue(id2.isSingleton() && id2.getSingletonValue() == 2.5);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      enumVals.push_back(1.5);
      enumVals.push_back(-2.0);
      NumericDomain ed6(enumVals);
      IntervalDomain id3(-1.0, 3.0);

      id3.equate(ed6);
      assertTrue(ed6.getSize() == 2);
      assertTrue(id3 == IntervalDomain(1.5, 2.5));

      IntervalDomain id4(1.0, 1.25);

      ed6.equate(id4);
      assertTrue(ed6.isEmpty() != id4.isEmpty());

      enumVals.clear();
      enumVals.push_back(1.0);
      NumericDomain ed7(enumVals);
      IntervalDomain id5(1.125, PLUS_INFINITY);

      id5.equate(ed7);
      assertTrue(ed7.isEmpty() != id5.isEmpty());

      return(true);
    }

    static bool testValueRetrieval() {
      std::list<double> values;
      values.push_back(EUROPA::LabelStr("A"));
      values.push_back(EUROPA::LabelStr("B"));
      values.push_back(EUROPA::LabelStr("C"));
      values.push_back(EUROPA::LabelStr("D"));
      values.push_back(EUROPA::LabelStr("E"));

      LabelSet l1(values);
      std::list<double> results;
      l1.getValues(results);

      LabelSet l2(results);

      assertTrue(l1 == l2);
      LabelStr lbl("C");
      l1.set(lbl);
      assertTrue(lbl == l1.getSingletonValue());
      return(true);
    }

    static bool testIntersection() {
      std::list<double> values;
      values.push_back(EUROPA::LabelStr("A"));
      values.push_back(EUROPA::LabelStr("B"));
      values.push_back(EUROPA::LabelStr("C"));
      values.push_back(EUROPA::LabelStr("D"));
      values.push_back(EUROPA::LabelStr("E"));
      values.push_back(EUROPA::LabelStr("F"));
      values.push_back(EUROPA::LabelStr("G"));
      values.push_back(EUROPA::LabelStr("H"));
      values.push_back(EUROPA::LabelStr("I"));
      LabelSet ls1(values);

      double value(0);
      assertTrue(ls1.convertToMemberValue(std::string("H"), value));
      assertTrue(value == LabelStr("H"));
      assertFalse(ls1.convertToMemberValue(std::string("LMN"), value));

      LabelSet ls2(values);
      ls2.remove(EUROPA::LabelStr("A"));
      ls2.remove(EUROPA::LabelStr("C"));
      ls2.remove(EUROPA::LabelStr("E"));
      assertTrue(ls2.isSubsetOf(ls1));
      assertTrue(!ls1.isSubsetOf(ls2));

      LabelSet ls3(ls1);
      ls1.intersect(ls2);
      assertTrue(ls1 == ls2);
      assertTrue(ls2.isSubsetOf(ls1));

      ls1.relax(ls3);
      assertTrue(ls2.isSubsetOf(ls1));
      assertTrue(ls1 == ls3);

      LabelSet ls4(values);
      ls4.remove(EUROPA::LabelStr("A"));
      ls4.remove(EUROPA::LabelStr("B"));
      ls4.remove(EUROPA::LabelStr("C"));
      ls4.remove(EUROPA::LabelStr("D"));
      ls4.remove(EUROPA::LabelStr("E"));
      ls4.remove(EUROPA::LabelStr("F"));
      ls4.remove(EUROPA::LabelStr("G"));

      ls3.remove(EUROPA::LabelStr("H"));
      ls3.remove(EUROPA::LabelStr("I"));
      ls4.intersect(ls3);
      assertTrue(ls4.isEmpty());

      {
	NumericDomain d0;
	d0.insert(0);
	d0.insert(1);
	d0.insert(2);
	d0.insert(3);
	d0.close();

	NumericDomain d1;
	d1.insert(-1);
	d1.insert(2);
	d1.insert(4);
	d1.insert(5);
	d1.close();

	d0.intersect(d1);
	assertTrue(d0.getSize() == 1);
      }

      return(true);
    }

    static bool testDifference() {
      NumericDomain dom0;
      dom0.insert(1);
      dom0.insert(3);
      dom0.insert(2);
      dom0.insert(8);
      dom0.insert(10);
      dom0.insert(6);
      dom0.close();

      IntervalIntDomain dom1(11, 100);
      bool res = dom0.difference(dom1);
      assertTrue(!res);

      IntervalIntDomain dom2(5, 100);
      res = dom0.difference(dom2);
      assertTrue(res);
      assertTrue(dom0.getUpperBound() == 3);

      IntervalIntDomain dom3(0, 100);
      res = dom0.difference(dom3);
      assertTrue(res);
      assertTrue(dom0.isEmpty());

      return(true);
    }

    static bool testOperatorEquals() {
      NumericDomain dom0;
      dom0.insert(1);
      dom0.insert(3);
      dom0.insert(2);
      dom0.insert(8);
      dom0.insert(10);
      dom0.insert(6);
      dom0.close();

      NumericDomain dom1;
      dom1.insert(1);
      dom1.insert(3);
      dom1.insert(2);
      dom1.close();

      NumericDomain dom2(dom0);

      assertTrue(dom0 != dom1);
      dom0 = dom1;
      assertTrue(dom0 == dom1);

      dom1 = dom2;
      assertTrue(dom1 == dom2);

      return(true);
    }

    static bool testEmptyOnClosure(){
      std::list<double> values;
      {
	ChangeListener l_listener;
	LabelSet ls0(values);
	ls0.setListener(l_listener.getId());
	DomainListener::ChangeType change;
	bool res = l_listener.checkAndClearChange(change);
	assertTrue(res && change == DomainListener::EMPTIED);
      }

      {
	LabelSet ls0(values); // Will be empty on closure, but no listener attached
	DomainListener::ChangeType change;
	ChangeListener l_listener;
	ls0.setListener(l_listener.getId());
	bool res = l_listener.checkAndClearChange(change);
	assertTrue(res && change == DomainListener::EMPTIED);
      }

      return(true);
    }
  };

  // These have to be "global" (outside any class, at least) or some
  // compilers complain about uses as template type.
  typedef enum Fruits { orange, lemon, blueberry, raspberry } Fruits;
  typedef enum Colors { orangeColor, yellow, blue, red } Colors;

  class MixedTypeTest {
  public:

    static bool test() {
      runTest(testInfinityBounds);
      runTest(testEquality);
      runTest(testIntersection);
      runTest(testSubset);
      runTest(testIntDomain);
      runTest(testDomainComparatorConfiguration);
      runTest(testCopying);
      runTest(testSymbolicVsNumeric);
      return(true);
    }

  private:

    static bool testInfinityBounds(){
      IntervalDomain dom0;
      assertFalse(dom0.areBoundsFinite());
      IntervalDomain dom1(0, PLUS_INFINITY);
      assertFalse(dom1.areBoundsFinite());
      IntervalDomain dom2(0, PLUS_INFINITY-1);
      assertTrue(dom2.areBoundsFinite());
      NumericDomain dom3;
      assertFalse(dom3.areBoundsFinite());
      SymbolDomain dom4;
      assertTrue(dom4.areBoundsFinite());
      NumericDomain dom5;
      dom5.insert(0);
      dom5.insert(1);
      assertFalse(dom5.areBoundsFinite());
      dom5.close();
      assertTrue(dom5.areBoundsFinite());
      NumericDomain dom6(PLUS_INFINITY);
      assertFalse(dom6.areBoundsFinite());
      return true;
    }

    static bool testEquality() {
      NumericDomain dom;
      dom.insert(1.0);
      dom.insert(2.0);
      dom.close();

      NumericDomain dom0(dom);
      dom0.set(1.0);

      IntervalDomain dom1(1.0);
      assertTrue(dom1 == dom0);
      assertTrue(dom0 == dom1);

      IntervalIntDomain dom2(1);
      assertTrue(dom1 == dom2);

      dom0.reset(dom);
      IntervalIntDomain dom3(1, 2);
      assertTrue(dom0 == dom3);
      return(true);
    }

    static bool testIntersection() {
      NumericDomain dom0;
      dom0.insert(0);
      dom0.insert(0.98);
      dom0.insert(1.0);
      dom0.insert(1.89);
      dom0.insert(2.98);
      dom0.insert(10);
      dom0.close();
      assertTrue(dom0.getSize() == 6);
      IntervalIntDomain dom1(1, 8);
      NumericDomain dom2(dom0);

      dom0.intersect(dom1);
      assertTrue(dom0.getSize() == 1);
      assertTrue(dom0.isMember(1.0));

      IntervalDomain dom3(1, 8);
      dom2.intersect(dom3);
      assertTrue(dom2.getSize() == 3);

      return(true);
    }

    static bool testSubset() {
      NumericDomain dom0;
      dom0.insert(0);
      dom0.insert(0.98);
      dom0.insert(1.0);
      dom0.insert(1.89);
      dom0.insert(2.98);
      dom0.insert(10);
      dom0.close();

      IntervalDomain dom1(0, 10);
      assertTrue(dom0.isSubsetOf(dom1));

      IntervalIntDomain dom2(0, 10);
      assertTrue(!dom0.isSubsetOf(dom2));

      dom0.remove(0.98);
      dom0.remove(1.89);
      dom0.remove(2.98);
      assertTrue(dom0.isSubsetOf(dom2));

      assertTrue(dom2.isSubsetOf(dom1));
      assertTrue(!dom1.isSubsetOf(dom2));
      return(true);
    }

    static bool testIntDomain() {
      NumericDomain dom0;
      dom0.insert(10);
      dom0.insert(12);
      dom0.close();

      NumericDomain dom1;
      dom1.insert(9.98);
      dom1.insert(9.037);
      dom1.close();

      NumericDomain dom2(10);
      assertTrue(!dom2.isOpen());
      assertTrue(dom2.isSingleton());

      assertTrue(AbstractDomain::canBeCompared(dom0, dom2));
      assertTrue(dom0 != dom2);

      assertTrue(!dom0.isSubsetOf(dom2));
      assertTrue(dom0.isSubsetOf(dom0));
      assertTrue(dom2.isSubsetOf(dom0));
      assertTrue(dom2.isSubsetOf(dom2));

      return(true);
    }

    static void testCopyingBoolDomains() {
      AbstractDomain *copyPtr;
      BoolDomain falseDom(false);
      BoolDomain trueDom(true);
      BoolDomain both;
      BoolDomain customDom(true, "boolean");

      copyPtr = falseDom.copy();
      assertTrue(copyPtr->isBool());
      assertTrue(copyPtr->getTypeName() == LabelStr("BOOL"));
      assertTrue((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      delete copyPtr;

      copyPtr = trueDom.copy();
      assertTrue(copyPtr->isBool());
      assertTrue(copyPtr->getTypeName() == LabelStr("BOOL"));
      assertTrue((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      delete copyPtr;

      copyPtr = both.copy();
      assertTrue(copyPtr->isBool());
      assertTrue(copyPtr->getTypeName() == LabelStr("BOOL"));
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      delete copyPtr;

      copyPtr = customDom.copy();
      assertTrue(copyPtr->isBool());
      assertTrue(copyPtr->getTypeName() == LabelStr("boolean"));
      assertTrue((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    static void testCopyingEnumeratedDomains() {
      AbstractDomain *copyPtr;
      NumericDomain emptyOpen;
      NumericDomain fourDom;
      std::list<double> values;
      values.push_back(0.0);
      fourDom.insert(0.0);
      values.push_back(1.1);
      fourDom.insert(1.1);
      values.push_back(2.7);
      fourDom.insert(2.7);
      values.push_back(3.1);
      fourDom.insert(3.1);
      values.push_back(4.2);
      NumericDomain fiveDom(values); // Closed
      NumericDomain oneDom(2.7); // Singleton

      copyPtr = emptyOpen.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_ENUMERATION"));
      assertTrue(copyPtr->isOpen());
      assertTrue(copyPtr->isNumeric());
      assertTrue(copyPtr->isEnumerated());
      copyPtr->insert(3.1);
      //assertFalse(copyPtr->isSingleton()); Or should that provoke an error? wedgingt 2004 Mar 3
      copyPtr->close();
      assertTrue(copyPtr->isSingleton());
      assertFalse(copyPtr->isOpen());
      delete copyPtr;

      copyPtr = fourDom.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_ENUMERATION"));
      assertTrue(copyPtr->isOpen());
      assertTrue(copyPtr->isEnumerated());
      copyPtr->close();
      assertTrue(copyPtr->getSize() == 4);
      assertTrue(copyPtr->isSubsetOf(fiveDom));
      delete copyPtr;

      copyPtr = fiveDom.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_ENUMERATION"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isEnumerated());
      assertTrue(copyPtr->getSize() == 5);
      assertTrue(fourDom.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = oneDom.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_ENUMERATION"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isEnumerated());
      assertTrue(copyPtr->isSingleton());

      // Can't call this with a dynamic domain, so close it first.
      fourDom.close();
      assertTrue(copyPtr->isSubsetOf(fourDom));

      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    static void testCopyingIntervalDomains() {
      AbstractDomain *copyPtr;
      IntervalDomain one2ten(1.0, 10.9);
      IntervalDomain four(4.0, 4.0, "fourType");
      IntervalDomain empty;
      empty.empty();

      // Domains containing infinities should also be tested.

      copyPtr = empty.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_INTERVAL"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isNumeric());
      assertFalse(copyPtr->isEnumerated());
      assertTrue(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(0.0));
      assertFalse(copyPtr->isSingleton());
      assertTrue(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 0);
      assertTrue(*copyPtr == empty);
      assertFalse(*copyPtr == one2ten);
      copyPtr->relax(IntervalDomain(-3.1, 11.0));
      assertTrue(copyPtr->isMember(0.0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(empty.isEmpty());
      assertFalse(*copyPtr == empty);
      assertTrue(empty.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = one2ten.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_INTERVAL"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isNumeric());
      assertFalse(copyPtr->isEnumerated());
      assertFalse(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(0.0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertFalse(*copyPtr == empty);
      assertTrue(*copyPtr == one2ten);
      copyPtr->relax(IntervalDomain(-3.1, 11.0));
      assertTrue(copyPtr->isMember(0.0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertFalse(*copyPtr == one2ten);
      assertTrue(one2ten.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = four.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("fourType"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isNumeric());
      assertFalse(copyPtr->isEnumerated());
      assertTrue(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(0.0));
      assertTrue(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 1);
      assertFalse(*copyPtr == empty);
      assertTrue(*copyPtr == four);
      assertFalse(*copyPtr == one2ten);
      copyPtr->relax(IntervalDomain(-3.1, 11.0));
      assertTrue(copyPtr->isMember(0.0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertFalse(*copyPtr == empty);
      assertFalse(*copyPtr == four);
      assertTrue(four.isSubsetOf(*copyPtr));
      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    static void testCopyingIntervalIntDomains() {
      AbstractDomain *copyPtr;
      IntervalIntDomain one2ten(1, 10);
      IntervalIntDomain four(4, 4, "fourType");
      IntervalIntDomain empty;
      empty.empty();
      // domains containing infinities should also be tested

      copyPtr = empty.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("INT_INTERVAL"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isNumeric());
      assertFalse(copyPtr->isEnumerated());
      assertTrue(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(0));
      assertFalse(copyPtr->isSingleton());
      assertTrue(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 0);
      assertTrue(*copyPtr == empty);
      assertFalse(*copyPtr == one2ten);
      copyPtr->relax(IntervalDomain(-3, 11));
      assertTrue(copyPtr->isMember(0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(empty.isEmpty());
      assertFalse(*copyPtr == empty);
      assertTrue(empty.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = one2ten.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("INT_INTERVAL"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isNumeric());
      assertFalse(copyPtr->isEnumerated());
      assertTrue(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 10);
      assertFalse(*copyPtr == empty);
      assertTrue(*copyPtr == one2ten);
      copyPtr->relax(IntervalIntDomain(-3, 11));
      assertTrue(copyPtr->getSize() == 15);
      assertTrue(copyPtr->isMember(0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertFalse(*copyPtr == one2ten);
      assertTrue(one2ten.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = four.copy();
      assertTrue(copyPtr->getTypeName() == LabelStr("fourType"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isNumeric());
      assertFalse(copyPtr->isEnumerated());
      assertTrue(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(0));
      assertTrue(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 1);
      assertFalse(*copyPtr == empty);
      assertTrue(*copyPtr == four);
      assertFalse(*copyPtr == one2ten);
      copyPtr->relax(IntervalIntDomain(-3, 11));
      assertTrue(copyPtr->getSize() == 15);
      assertTrue(copyPtr->isMember(0));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertFalse(*copyPtr == empty);
      assertFalse(*copyPtr == four);
      assertTrue(four.isSubsetOf(*copyPtr));
      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    class BogusComparator: public DomainComparator {
    public:
      BogusComparator(): DomainComparator(){}

      bool canCompare(const AbstractDomain& domx, const AbstractDomain& domy) const {
	return false;
      }

    };

    static bool testDomainComparatorConfiguration() {
      IntervalIntDomain dom0;
      IntervalIntDomain dom1;

      // Using the default comparator - they should be comparable
      assertTrue(AbstractDomain::canBeCompared(dom0, dom1));

      // Switch for the bogus one - and make sure it fails as expected
      BogusComparator bogus;
      assertFalse(AbstractDomain::canBeCompared(dom0, dom1));

      // Allocate the standard comparator, and ensure it compares once again
      DomainComparator standard;
      assertTrue(AbstractDomain::canBeCompared(dom0, dom1));

      // Now allocate a new one again
      DomainComparator* dc = new BogusComparator();
      assertFalse(AbstractDomain::canBeCompared(dom0, dom1));

      // Deallocate and thus cause revert to standard comparator
      delete dc;
      assertTrue(AbstractDomain::canBeCompared(dom0, dom1));

      // 2 numeric enumerations should be comparable, even if type names differ
      {
	NumericDomain d0("NumberDomain0");
	NumericDomain d1("NumberDomain1");
	IntervalIntDomain d2;
	assertTrue(AbstractDomain::canBeCompared(d0, d1));
	assertTrue(AbstractDomain::canBeCompared(d0, d2));
      }

      // 2 non numeric enumerations can only be compared if they are of the same type
      {
	SymbolDomain d0("SymbolicDomain");
	StringDomain d1("SymbolicDomain");
	SymbolDomain d2("OtherDomainType");
	assertTrue(AbstractDomain::canBeCompared(d0, d1));
	assertFalse(AbstractDomain::canBeCompared(d0, d2));
      }
      return true;
    }

    static bool testCopying() {

      // These five functions were mistakenly put in DomainTest.cc originally.
      // They also test the new getTypeName() member functions a little and
      // those member functions' effects on AbstractDomain::canBeCompared().
      testCopyingBoolDomains();
      testCopyingEnumeratedDomains();
      testCopyingIntervalDomains();
      testCopyingIntervalIntDomains();

      BoolDomain boolDom;
      AbstractDomain *copy = boolDom.copy();
      assertTrue(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.set(false);
      assertTrue(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      assertTrue(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.empty();
      assertTrue(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      assertTrue(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.relax(BoolDomain(true));
      assertTrue(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      assertTrue(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.remove(true);
      assertTrue(*copy != boolDom);
      delete copy;

      IntervalIntDomain iiDom(-2, PLUS_INFINITY);
      copy = iiDom.copy();
      assertTrue(copy != 0);
      assertTrue(*copy == iiDom && copy != &iiDom);
      iiDom.set(IntervalIntDomain(-2, PLUS_INFINITY-1));
      assertTrue(*copy != iiDom);
      copy->set(IntervalIntDomain(-2, PLUS_INFINITY-1));
      assertTrue(*copy == iiDom && copy != &iiDom);
      delete copy;

      IntervalDomain iDom(MINUS_INFINITY);
      copy = iDom.copy();
      assertTrue(copy != 0);
      assertTrue(*copy == iDom && copy != &iDom);
      iDom.empty();
      assertTrue(*copy != iDom);
      copy->empty();
      assertTrue(*copy == iDom);
      delete copy;

      NumericDomain eDom;
      eDom.insert(2.7);
      copy = eDom.copy();
      assertTrue(copy != 0);
      assertTrue(*copy == eDom && copy != &eDom);
      eDom.insert(PLUS_INFINITY);
      assertTrue(*copy != eDom);
      eDom.remove(PLUS_INFINITY);
      assertTrue(*copy == eDom && copy != &eDom);
      delete copy;

      return(true);
    }

    static bool testSymbolicVsNumeric() {

      BoolDomain bDom(false);
      IntervalIntDomain iiDom(-2, PLUS_INFINITY);
      IntervalDomain iDom(MINUS_INFINITY);
      NumericDomain nDom(2.7);
      EnumeratedDomain eDom(false, "MyEnum"); // non numeric enum
      eDom.insert(LabelStr("myEnumMember"));
      eDom.set(LabelStr("myEnumMember").getKey());
      EnumeratedDomain enDom(true, "MyEnum"); // numeric enum
      SymbolDomain sDom("mySymbol");
      StringDomain stDom("myName");

      assertTrue(bDom.isSymbolic());
      assertTrue(iiDom.isNumeric());
      assertTrue(iDom.isNumeric());
      assertTrue(nDom.isNumeric());
      assertTrue(eDom.isSymbolic());
      assertTrue(enDom.isNumeric());
      assertTrue(sDom.isSymbolic());
      assertTrue(stDom.isSymbolic());

      return(true);
    }

  };
}

bool DomainTests::test() {
  runTestSuite(EUROPA::IntervalDomainTest::test);
  runTestSuite(EUROPA::EnumeratedDomainTest::test);
  runTestSuite(EUROPA::MixedTypeTest::test);
  return(true);
}


