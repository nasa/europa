#include "TestSupport.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "EnumeratedDomain.hh"
#include "LabelStr.hh"
#include "DomainListener.hh"
#include "Domain.hh"
#include "module-tests.hh"
#include <cmath>

namespace Prototype {

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
      runTest(testInsertAndRemove);
      runTest(testValidComparisonWithEmpty_gnats2403);
      runTest(testIntervalSingletonValues);
      runTest(testIntervalIntValues);
      return(true);
    }

  private:
    static bool testAllocation() {
      IntervalIntDomain intDomain(10, 20);
      assert(intDomain.isFinite());

      double lb, ub;
      intDomain.getBounds(lb,ub);
      assert(lb<ub);
      assert(!intDomain.isMember(1.889));
      assert(!intDomain.isOpen());
      IntervalIntDomain d1(intDomain);
      d1.empty();
      assert(d1.isEmpty());

      AbstractDomain& d2 = static_cast<AbstractDomain&>(intDomain);
      assert(!d2.isEmpty());

      IntervalIntDomain d3(static_cast<IntervalIntDomain&>(intDomain));
      IntervalIntDomain d4;

      assert( ! (d3 == d4));
      d3.relax(d4);
      assert(d3 == d4);

      assert(d2 != d4);
      d2.relax(d4);
      assert(d2 == d4);
      return(true);
    }

    static bool testRelaxation() {
      ChangeListener l_listener;
      IntervalIntDomain dom0; // Will have very large default range
      IntervalIntDomain dom1(-100, 100, l_listener.getId());
      dom1.relax(dom0);
      DomainListener::ChangeType change;
      bool res = l_listener.checkAndClearChange(change);
      assert(res && change == DomainListener::RELAXED);
      assert(dom1.isSubsetOf(dom0));
      assert(dom0.isSubsetOf(dom1));
      assert(dom1 == dom0);

      IntervalIntDomain dom2(-300, 100);
      dom1.intersect(dom2);
      res = l_listener.checkAndClearChange(change);
      assert(res);
      assert(dom1 == dom2);
      dom1.relax(dom2);
      res = l_listener.checkAndClearChange(change);
      assert(!res);
      return(true);
    }

    static bool testIntersection() {
      ChangeListener l_listener;
      IntervalIntDomain dom0(l_listener.getId()); // Will have very large default range

      // Execute intersection and verify results
      IntervalIntDomain dom1(-100, 100);
      dom0.intersect(dom1);
      DomainListener::ChangeType change;
      bool res = l_listener.checkAndClearChange(change);
      assert(res);
      assert(dom0 == dom1);
    
      // verify no change triggered if none should take place.
      dom0.intersect(dom1);
      res = l_listener.checkAndClearChange(change);
      assert(!res);

      // Verify only the upper bound changes
      IntervalIntDomain dom2(-200, 50);
      dom0.intersect(dom2);
      res = l_listener.checkAndClearChange(change);
      assert(res);
      assert(dom0.getLowerBound() == dom1.getLowerBound());
      assert(dom0.getUpperBound() == dom2.getUpperBound());
    
      // Make an intersection that leads to an empty domain
      IntervalIntDomain dom3(500, 1000);
      dom0.intersect(dom3);
      res = l_listener.checkAndClearChange(change);
      assert(res);
      assert(dom0.isEmpty());

      IntervalDomain dom4(0.98, 101.23);
      IntervalDomain dom5(80, 120.44);
      IntervalDomain dom6(80, 101.23);
      dom4.equate(dom5);
      assert(dom4 == dom6);
      assert(dom5 == dom6);
      return(true);
    }

    static bool testSubset() {
      IntervalIntDomain dom0(10, 35);
      IntervalDomain dom1(0, 101);
      assert(dom0.isSubsetOf(dom1));
      assert(! dom1.isSubsetOf(dom0));

      // Handle cases where domains are equal
      IntervalIntDomain dom2(dom0);
      assert(dom2 == dom0);
      assert(dom0.isSubsetOf(dom2));
      assert(dom2.isSubsetOf(dom0));

      // Handle case with no intersection
      IntervalIntDomain dom3(0, 9);
      assert(! dom3.isSubsetOf(dom0));
      assert(! dom0.isSubsetOf(dom3));

      // Handle case with partial intersection
      IntervalIntDomain dom4(0, 20);
      assert(! dom4.isSubsetOf(dom0));
      assert(! dom0.isSubsetOf(dom4));

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
      assert(actualString == expectedString);
      std::string anotherActualString = d1.toString();
      assert(anotherActualString == expectedString);
      return(true);
    }

    static bool testBoolDomain() {
      BoolDomain dom0;
      assert(dom0.getSize() == 2);
      assert(dom0.getUpperBound() == true);
      assert(dom0.getLowerBound() == false);

      IntervalIntDomain dom1(0, 100);
      dom1.intersect(dom0);
      assert(dom1 == dom0);
      return(true);
    }

    static bool testDifference() {
      IntervalDomain dom0(1, 10);
      IntervalDomain dom1(11, 20);
      bool res = dom0.difference(dom1);
      assert(!res);
      res = dom1.difference(dom0);
      assert(!res);

      IntervalDomain dom2(dom0);
      res = dom2.difference(dom0);
      assert(res);
      assert(dom2.isEmpty());

      IntervalIntDomain dom3(5, 100);
      res = dom3.difference(dom0);
      assert(res);
      assert(dom3.getLowerBound() == 11);
      res = dom3.difference(dom1);
      assert(res);
      assert(dom3.getLowerBound() == 21);

      IntervalDomain dom4(0, 20);
      res = dom4.difference(dom1);
      assert(res);
      double newValue = (dom1.getLowerBound() - dom4.minDelta());
      assert(dom4.getUpperBound() == newValue);

      EnumeratedDomain dom5(3.14159265);
      assert(dom5.getSize() == 1);

      std::list<double> vals;
      vals.push_back(dom5.getSingletonValue());
      vals.push_back(1.2);
      vals.push_back(2.1);
      vals.push_back(PLUS_INFINITY);
      vals.push_back(MINUS_INFINITY);
      vals.push_back(EPSILON);
      EnumeratedDomain dom6(vals);

      assert(dom6.getSize() == 6);
      assert(fabs(dom5.minDelta() - dom6.minDelta()) < EPSILON); // Should be ==, but allow some leeway.
      assert(dom6.intersects(dom5));

      dom6.difference(dom5);
      assert(!(dom6.intersects(dom5)));
      assert(dom6.getSize() == 5);

      dom6.difference(dom5);
      assert(!(dom6.intersects(dom5)));
      assert(dom6.getSize() == 5);

      return(true);
    }

    static bool testOperatorEquals() {
      IntervalDomain dom0(1, 28);
      IntervalDomain dom1(50, 100);
      dom0 = dom1;
      assert(dom0 == dom1);
      return(true);
    }

    static bool testInfinitesAndInts() {
      IntervalDomain dom0;
      assert(dom0.translateNumber(MINUS_INFINITY) == MINUS_INFINITY);
      assert(dom0.translateNumber(MINUS_INFINITY - 1) == MINUS_INFINITY);
      assert(dom0.translateNumber(MINUS_INFINITY + 1) == MINUS_INFINITY + 1);
      assert(dom0.translateNumber(PLUS_INFINITY + 1) == PLUS_INFINITY);
      assert(dom0.translateNumber(PLUS_INFINITY - 1) == PLUS_INFINITY - 1);
      assert(dom0.translateNumber(2.8) == 2.8);

      IntervalIntDomain dom1;
      assert(dom1.translateNumber(2.8, false) == 2);
      assert(dom1.translateNumber(2.8, true) == 3);
      assert(dom1.translateNumber(PLUS_INFINITY - 0.2, false) == PLUS_INFINITY - 1);
      return(true);
    }

    static bool testEnumDomInsertAndRemove() {
      // Should add a loop like the one in
      //   testIntervalDomInsertAndRemove(). --wedgingt 2004 Mar 8

      EnumeratedDomain enumDom1;
      assert(enumDom1.isOpen());
      assert(enumDom1.isNumeric());
      enumDom1.insert(3.14159265);
      enumDom1.close();
      assert(enumDom1.isMember(3.14159265));
      assert(enumDom1.isSingleton());
      assert(enumDom1.isFinite());
      enumDom1.remove(5.0);
      assert(enumDom1.isMember(3.14159265));
      assert(enumDom1.isSingleton());
      assert(enumDom1.isFinite());

      double minDiff = enumDom1.minDelta();
      assert(minDiff >= EPSILON && EPSILON > 0.0);

      const double onePlus = 1.0 + 2.0*EPSILON;

      enumDom1.remove(3.14159265 - onePlus*minDiff);
      assert(enumDom1.isMember(3.14159265));
      assert(enumDom1.isSingleton());
      assert(enumDom1.isFinite());
      enumDom1.remove(3.14159265 + onePlus*minDiff);
      assert(enumDom1.isMember(3.14159265));
      assert(enumDom1.isSingleton());
      assert(enumDom1.isFinite());
      enumDom1.remove(3.14159265 - minDiff/onePlus);
      assert(enumDom1.isEmpty());
      enumDom1.insert(3.14159265);
      assert(enumDom1.isMember(3.14159265));
      assert(enumDom1.isSingleton());
      assert(enumDom1.isFinite());
      enumDom1.remove(3.14159265 + minDiff/onePlus);
      assert(enumDom1.isEmpty());
      enumDom1.insert(3.14159265);
      assert(enumDom1.isMember(3.14159265));
      assert(enumDom1.isSingleton());

      std::list<double> vals;
      vals.push_back(enumDom1.getSingletonValue());
      vals.push_back(1.2);
      vals.push_back(2.1);
      vals.push_back(PLUS_INFINITY);
      vals.push_back(MINUS_INFINITY);
      vals.push_back(EPSILON);
      EnumeratedDomain enumDom2(vals);

      assert(!(enumDom2.isOpen()));
      assert(enumDom2.isNumeric());
      assert(enumDom2.isFinite());
      assert(enumDom2.getSize() == 6);

      double minDiff2 = enumDom2.minDelta();
      assert(fabs(minDiff - minDiff2) < EPSILON);

      enumDom2.remove(1.2 - minDiff2/onePlus);
      assert(enumDom2.getSize() == 5);

      enumDom2.remove(MINUS_INFINITY);
      assert(enumDom2.getSize() == 4);

      // Remove a value near but not "matching" a member and
      //   verify the domain has not changed.
      enumDom2.remove(enumDom1.getSingletonValue() - onePlus*minDiff2);
      assert(enumDom2.intersects(enumDom1));
      assert(enumDom2.getSize() == 4);

      // Remove a value near but not equal a member and
      //   verify the member was removed via intersection.
      enumDom2.remove(enumDom1.getSingletonValue() - minDiff2/onePlus);
      assert(!(enumDom2.intersects(enumDom1)));
      assert(enumDom2.getSize() == 3);

      // Add a value near a value from another domain
      //   verify that the resulting domain intersects the other domain.
      enumDom2.insert(enumDom1.getSingletonValue() + minDiff2/onePlus);
      assert(enumDom2.intersects(enumDom1));
      assert(enumDom2.getSize() == 4);

      // Add the value in the other domain and
      //   verify the domain is not affected.
      enumDom2.insert(enumDom1.getSingletonValue());
      assert(enumDom2.intersects(enumDom1));
      assert(enumDom2.getSize() == 4);

      // Remove a value that should not be a member but is
      //   only slightly too large to "match" the new member.
      enumDom2.remove(enumDom1.getSingletonValue() + minDiff2/onePlus + onePlus*minDiff2);
      assert(enumDom2.intersects(enumDom1));
      assert(enumDom2.getSize() == 4);

      // Remove a value "matching" the added value but larger
      //   and verify the domain no longer intersects the other domain.
      enumDom2.remove(enumDom1.getSingletonValue() + 2.0*minDiff2/onePlus);
      assert(enumDom2.getSize() == 3);
      assert(!(enumDom2.intersects(enumDom1)));

      return(true);
    }

    static bool testIntervalDomInsertAndRemove() {
      assert(EPSILON > 0.0); // Otherwise, loop will be infinite.

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
        assert(iDom.isSingleton());
        assert(iDom.isMember(val));
        assert(iDom.isMember(val + minDiff/onePlus));
        assert(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val + minDiff/onePlus);
        assert(iDom.isEmpty());
        assert(!(iDom.isMember(val)));

        iDom.insert(val);
        assert(iDom.isSingleton());
        assert(iDom.isMember(val));
        assert(iDom.isMember(val + minDiff/onePlus));
        assert(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val - onePlus*minDiff);
        assert(iDom.isSingleton());
        assert(iDom.isMember(val));
        assert(iDom.isMember(val + minDiff/onePlus));
        assert(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val - minDiff/onePlus);
        assert(iDom.isEmpty());
        assert(!(iDom.isMember(val)));

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
      assert(iiDom.getSize() == 16);

      iiDom.remove(-6);
      assert(iiDom.getSize() == 16);

      iiDom.remove(11);
      assert(iiDom.getSize() == 16);

      iiDom.remove(PLUS_INFINITY);
      assert(iiDom.getSize() == 16);

      iiDom.insert(-5);
      assert(iiDom.getSize() == 16);

      iiDom.insert(-1);
      assert(iiDom.getSize() == 16);

      iiDom.insert(10);
      assert(iiDom.getSize() == 16);

      iiDom.insert(11);
      assert(iiDom.getSize() == 17);

      iiDom.insert(-6);
      assert(iiDom.getSize() == 18);

      iiDom.remove(PLUS_INFINITY);
      assert(iiDom.getSize() == 18);

      iiDom.remove(-7);
      assert(iiDom.getSize() == 18);

      iiDom.remove(11);
      assert(iiDom.getSize() == 17);

      return(true);
    }

    static bool testInsertAndRemove() {
      return(testEnumDomInsertAndRemove() &&
             testIntervalDomInsertAndRemove() &&
             testIntervalIntDomInsertAndRemove());
    }

    static bool testValidComparisonWithEmpty_gnats2403(){
      IntervalIntDomain d0;
      IntervalIntDomain d1;
      EnumeratedDomain d2;
      d2.insert(1);
      d2.close();

      assert(d0 == d1);
      assert(d1 != d2);
      d0.empty();
      assert(d0 != d1);
      assert(d0 != d2);

      assert(AbstractDomain::canBeCompared(d0, d2));

      EnumeratedDomain d3(d2);
      d2.empty();
      assert(AbstractDomain::canBeCompared(d2, d3));
      assert(d3 != d2);

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
      return(true);
    }

  private:

    static bool testEnumerationOnly() {
      std::list<double> values;
      values.push_back(-98.67);
      values.push_back(-0.01);
      values.push_back(1);
      values.push_back(2);
      values.push_back(10);
      values.push_back(11);

      EnumeratedDomain d0(values);
      EnumeratedDomain d1(values);
      assert(d0 == d1);
      assert(d0.isSubsetOf(d1));
      assert(d0.isMember(-98.67));
      d0.remove(-0.01);
      assert(!d0.isMember(-0.01));
      assert(d0.isSubsetOf(d1));
      assert(!d1.isSubsetOf(d0));

      return(true);
    }

    static bool testBasicLabelOperations() {
      int initialCount = Prototype::LabelStr::getSize();
      Prototype::LabelStr l1("L1");
      Prototype::LabelStr l2("L2");
      Prototype::LabelStr l3("L3");
      assert(l1 < l2 && l2 < l3);

      Prototype::LabelStr la("L");
      Prototype::LabelStr l4("L30");
      Prototype::LabelStr lb("L");
      assert(la == lb);
      assert(la < l4);

      Prototype::LabelStr copy1(l1);
      assert(l1 == copy1);
      assert(l2 != copy1);

      assert((Prototype::LabelStr::getSize() - initialCount) == 5);
      assert(l1.toString() == "L1");

      assert(LabelStr::isString(l1.getKey()));
      assert(!LabelStr::isString(PLUS_INFINITY+1));
      return(true);
    }

    static bool testLabelSetAllocations() {
      std::list<Prototype::LabelStr> values;
      values.push_back(Prototype::LabelStr("L1"));
      values.push_back(Prototype::LabelStr("L4"));
      values.push_back(Prototype::LabelStr("L2"));
      values.push_back(Prototype::LabelStr("L5"));
      values.push_back(Prototype::LabelStr("L3"));

      ChangeListener l_listener;
      LabelSet ls0(values, true, l_listener.getId());
      assert(!ls0.isOpen());

      Prototype::LabelStr l2("L2");
      assert(ls0.isMember(l2));
      DomainListener::ChangeType change;
      ls0.remove(l2);
      bool res = l_listener.checkAndClearChange(change);
      assert(res && change == DomainListener::VALUE_REMOVED);
      assert(!ls0.isMember(l2));

      Prototype::LabelStr l3("L3");
      ls0.set(l3);
      assert(ls0.isMember(l3));
      assert(ls0.getSize() == 1);

      LabelSet ls1(values, true);
      ls0.relax(ls1);
      res = l_listener.checkAndClearChange(change);
      assert(res && change == DomainListener::RELAXED);
      assert(ls0 == ls1);
      return(true);
    }

    static bool testEquate() {
      std::list<Prototype::LabelStr> baseValues;
      baseValues.push_back(Prototype::LabelStr("A"));
      baseValues.push_back(Prototype::LabelStr("B"));
      baseValues.push_back(Prototype::LabelStr("C"));
      baseValues.push_back(Prototype::LabelStr("D"));
      baseValues.push_back(Prototype::LabelStr("E"));
      baseValues.push_back(Prototype::LabelStr("F"));
      baseValues.push_back(Prototype::LabelStr("G"));
      baseValues.push_back(Prototype::LabelStr("H"));

      ChangeListener l_listener;
      LabelSet ls0(baseValues, true, l_listener.getId());
      LabelSet ls1(baseValues, true, l_listener.getId());

      assert(ls0 == ls1);
      assert(ls0.getSize() == 8);
      bool res = ls0.equate(ls1);
      assert(res == false); // Implying no change occured

      Prototype::LabelStr lC("C");
      ls0.remove(lC);
      assert(!ls0.isMember(lC));
      assert(ls1.isMember(lC));
      res = ls0.equate(ls1);
      assert(res); // It should have changed
      assert(!ls1.isMember(lC));

      LabelSet ls2(baseValues, true, l_listener.getId());
      ls2.remove(Prototype::LabelStr("A"));
      ls2.remove(Prototype::LabelStr("B"));
      ls2.remove(Prototype::LabelStr("C"));
      ls2.remove(Prototype::LabelStr("D"));
      ls2.remove(Prototype::LabelStr("E"));

      LabelSet ls3(baseValues, true, l_listener.getId());
      Prototype::LabelStr lA("A");
      Prototype::LabelStr lB("B");
      ls3.remove(lA);
      ls3.remove(lB);
      ls3.remove(lC);
      res = ls2.equate(ls3);
      assert(res);
      assert(ls2 == ls3);

      LabelSet ls4(baseValues, true, l_listener.getId());
      ls4.remove(Prototype::LabelStr("A"));
      ls4.remove(Prototype::LabelStr("B"));
      ls4.remove(Prototype::LabelStr("C"));
      ls4.remove(Prototype::LabelStr("D"));
      ls4.remove(Prototype::LabelStr("E"));

      LabelSet ls5(baseValues, true, l_listener.getId());
      ls5.remove(Prototype::LabelStr("F"));
      ls5.remove(Prototype::LabelStr("G"));
      ls5.remove(Prototype::LabelStr("H"));

      DomainListener::ChangeType change;
      ls4.equate(ls5);
      res = l_listener.checkAndClearChange(change);
      assert(res && change == DomainListener::EMPTIED);
      assert(ls4.isEmpty() || ls5.isEmpty());
      assert(!(ls4.isEmpty() && ls5.isEmpty()));

      std::list<double> enumVals;
      enumVals.push_back(1.0);
      enumVals.push_back(2.5);
      enumVals.push_back(-0.25);
      enumVals.push_back(3.375);
      enumVals.push_back(-1.75);
      EnumeratedDomain ed1(enumVals);
      EnumeratedDomain ed3(enumVals);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      EnumeratedDomain ed2(enumVals);
      EnumeratedDomain ed4(enumVals);

      ed1.equate(ed2);
      assertTrue(ed1 == ed2);

      ed1.equate(ed3);
      assertTrue(ed1 == ed3);

      enumVals.clear();
      enumVals.push_back(0.0);
      EnumeratedDomain ed0(enumVals);

      ed1.equate(ed0);

      // This is actually false because equate() only empties
      // one of the domains when the intersection is empty.
      // assertTrue(ed0 == ed1);

      assertFalse(ed0 == ed1);
      assertTrue(ed1.isEmpty() != ed0.isEmpty());

      ed0 = EnumeratedDomain(enumVals);
      assertTrue(!ed0.isEmpty());

      ed0.equate(ed2);
      assertTrue(ed2 != ed0 && ed2.isEmpty() != ed0.isEmpty());

      enumVals.push_back(20.0); // Now 0.0 and 20.0
      ed0 = EnumeratedDomain(enumVals);
      assertTrue(!ed0.isEmpty() && !ed0.isSingleton());

      IntervalDomain id0(-10.0, 10.0);

      id0.equate(ed0);
      assertTrue(ed0.isSingleton() && ed0.getSingletonValue() == 0.0);
      assertTrue(id0.isSingleton() && id0.getSingletonValue() == 0.0);

      ed0 = EnumeratedDomain(enumVals); // Now 0.0 and 20.0
      assertTrue(!ed0.isEmpty() && !ed0.isSingleton());

      IntervalDomain id1(0.0, 5.0);

      ed0.equate(id1);
      assertTrue(ed0.isSingleton() && ed0.getSingletonValue() == 0.0);
      assertTrue(id1.isSingleton() && id1.getSingletonValue() == 0.0);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      enumVals.push_back(1.5);
      EnumeratedDomain ed5(enumVals);
      IntervalDomain id2(2.5, 3.0);

      ed5.equate(id2);
      assertTrue(ed5.isSingleton() && ed5.getSingletonValue() == 2.5);
      assertTrue(id2.isSingleton() && id2.getSingletonValue() == 2.5);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      enumVals.push_back(1.5);
      enumVals.push_back(-2.0);
      EnumeratedDomain ed6(enumVals);
      IntervalDomain id3(-1.0, 3.0);

      id3.equate(ed6);
      assertTrue(ed6.getSize() == 2);
      assertTrue(id3 == IntervalDomain(1.5, 2.5));

      IntervalDomain id4(1.0, 1.25);

      ed6.equate(id4);
      assertTrue(ed6.isEmpty() != id4.isEmpty());

      enumVals.clear();
      enumVals.push_back(1.0);
      EnumeratedDomain ed7(enumVals);
      IntervalDomain id5(1.125, PLUS_INFINITY);

      id5.equate(ed7);
      assertTrue(ed7.isEmpty() != id5.isEmpty());

      return(true);
    }

    static bool testValueRetrieval() {
      std::list<Prototype::LabelStr> values;
      values.push_back(Prototype::LabelStr("A"));
      values.push_back(Prototype::LabelStr("B"));
      values.push_back(Prototype::LabelStr("C"));
      values.push_back(Prototype::LabelStr("D"));
      values.push_back(Prototype::LabelStr("E"));

      LabelSet l1(values, true);
      std::list<Prototype::LabelStr> results;
      l1.getValues(results);

      LabelSet l2(results, true);

      assert(l1 == l2);
      LabelStr lbl("C");
      l1.set(lbl);
      assert(lbl == l1.getSingletonValue());
      return(true);
    }

    static bool testIntersection() {
      std::list<Prototype::LabelStr> values;
      values.push_back(Prototype::LabelStr("A"));
      values.push_back(Prototype::LabelStr("B"));
      values.push_back(Prototype::LabelStr("C"));
      values.push_back(Prototype::LabelStr("D"));
      values.push_back(Prototype::LabelStr("E"));
      values.push_back(Prototype::LabelStr("F"));
      values.push_back(Prototype::LabelStr("G"));
      values.push_back(Prototype::LabelStr("H"));
      values.push_back(Prototype::LabelStr("I"));
      LabelSet ls1(values);

      LabelSet ls2(values);
      ls2.remove(Prototype::LabelStr("A"));
      ls2.remove(Prototype::LabelStr("C"));
      ls2.remove(Prototype::LabelStr("E"));
      assert(ls2.isSubsetOf(ls1));
      assert(!ls1.isSubsetOf(ls2));

      LabelSet ls3(ls1);

      ls1.intersect(ls2);
      assert(ls1 == ls2);
      assert(ls2.isSubsetOf(ls1));

      ls1.relax(ls3);
      assert(ls2.isSubsetOf(ls1));
      assert(ls1 == ls3);

      LabelSet ls4(values);
      ls4.remove(Prototype::LabelStr("A"));
      ls4.remove(Prototype::LabelStr("B"));
      ls4.remove(Prototype::LabelStr("C"));
      ls4.remove(Prototype::LabelStr("D"));
      ls4.remove(Prototype::LabelStr("E"));
      ls4.remove(Prototype::LabelStr("F"));
      ls4.remove(Prototype::LabelStr("G"));

      ls3.remove(Prototype::LabelStr("H"));
      ls3.remove(Prototype::LabelStr("I"));
      ls4.intersect(ls3);
      assert(ls4.isEmpty());

      {
	EnumeratedDomain d0;
	d0.insert(0);
	d0.insert(1);
	d0.insert(2);
	d0.insert(3);
	d0.close();

	EnumeratedDomain d1;
	d1.insert(-1);
	d1.insert(2);
	d1.insert(4);
	d1.insert(5);
	d1.close();

	d0.intersect(d1);
	assert(d0.getSize() == 1);
      }

      return(true);
    }

    static bool testDifference() {
      EnumeratedDomain dom0;
      dom0.insert(1);
      dom0.insert(3);
      dom0.insert(2);
      dom0.insert(8);
      dom0.insert(10);
      dom0.insert(6);
      dom0.close();

      IntervalIntDomain dom1(11, 100);
      bool res = dom0.difference(dom1);
      assert(!res);

      IntervalIntDomain dom2(5, 100);
      res = dom0.difference(dom2);
      assert(res);
      assert(dom0.getUpperBound() == 3);

      IntervalIntDomain dom3(0, 100);
      res = dom0.difference(dom3);
      assert(res);
      assert(dom0.isEmpty());

      return(true);
    }

    static bool testOperatorEquals() {
      EnumeratedDomain dom0;
      dom0.insert(1);
      dom0.insert(3);
      dom0.insert(2);
      dom0.insert(8);
      dom0.insert(10);
      dom0.insert(6);
      dom0.close();

      EnumeratedDomain dom1;
      dom1.insert(1);
      dom1.insert(3);
      dom1.insert(2);
      dom1.close();

      EnumeratedDomain dom2(dom0);

      assert(dom0 != dom1);
      dom0 = dom1;
      assert(dom0 == dom1);

      dom1 = dom2;
      assert(dom1 == dom2);

      return(true);
    }

    static bool testEmptyOnClosure(){
      std::list<Prototype::LabelStr> values;
      {
	ChangeListener l_listener;
	LabelSet ls0(values, true, l_listener.getId());
	DomainListener::ChangeType change;
	bool res = l_listener.checkAndClearChange(change);
	assert(res && change == DomainListener::EMPTIED);
      }

      {
	LabelSet ls0(values, true); // Will be empty on closure, but no listener attached
	DomainListener::ChangeType change;
	ChangeListener l_listener;
	ls0.setListener(l_listener.getId());
	bool res = l_listener.checkAndClearChange(change);
	assert(res && change == DomainListener::EMPTIED);
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
      runTest(testEquality);
      runTest(testIntersection);
      runTest(testSubset);
      runTest(testIntDomain);
      runTest(testCopying);
      return(true);
    }

  private:

    static bool testEquality() {
      EnumeratedDomain dom;
      dom.insert(1.0);
      dom.insert(2.0);
      dom.close();

      EnumeratedDomain dom0(dom);
      dom0.set(1.0);

      IntervalDomain dom1(1.0);
      assert(dom1 == dom0);
      assert(dom0 == dom1);

      IntervalIntDomain dom2(1);
      assert(dom1 == dom2);

      dom0.reset(dom);
      IntervalIntDomain dom3(1, 2);
      assert(dom0 == dom3);
      return(true);
    }

    static bool testIntersection() {
      EnumeratedDomain dom0;
      dom0.insert(0);
      dom0.insert(0.98);
      dom0.insert(1.0);
      dom0.insert(1.89);
      dom0.insert(2.98);
      dom0.insert(10);
      dom0.close();
      assert(dom0.getSize() == 6);
      IntervalIntDomain dom1(1, 8);
      EnumeratedDomain dom2(dom0);

      dom0.intersect(dom1);
      assert(dom0.getSize() == 1);
      assert(dom0.isMember(1.0));

      IntervalDomain dom3(1, 8);
      dom2.intersect(dom3);
      assert(dom2.getSize() == 3);

      BoolDomain dom4;
      dom2.intersect(dom4);
      assert(dom2.getSize() == 1);
      return(true);
    }

    static bool testSubset() {
      EnumeratedDomain dom0;
      dom0.insert(0);
      dom0.insert(0.98);
      dom0.insert(1.0);
      dom0.insert(1.89);
      dom0.insert(2.98);
      dom0.insert(10);
      dom0.close();

      IntervalDomain dom1(0, 10);
      assert(dom0.isSubsetOf(dom1));

      IntervalIntDomain dom2(0, 10);
      assert(!dom0.isSubsetOf(dom2));

      dom0.remove(0.98);
      dom0.remove(1.89);
      dom0.remove(2.98);
      assert(dom0.isSubsetOf(dom2));

      assert(dom2.isSubsetOf(dom1));
      assert(!dom1.isSubsetOf(dom2));
      return(true);
    }

    static bool testIntDomain() {
      Domain<int> dom0;
      dom0.insert(10);
      dom0.insert(12);
      dom0.close();

      Domain<float> dom1;
      dom1.insert(9.98);
      dom1.insert(9.037);
      dom1.close();

      assert(dom0.getType() == AbstractDomain::USER_DEFINED);
      assert(dom1.getType() == AbstractDomain::USER_DEFINED);

      // These two are correct conceptually, but incorrect
      // at this level because the Nddl compiler produces
      // a getTypeName() function in each class for object
      // domains, which Domain<TYPE> is also used for.
      //assert(dom0.getTypeName() != dom1.getTypeName());
      // Last three assert()s imply next one.
      //assert(!AbstractDomain::canBeCompared(dom0, dom1));

      Domain<int> dom2(10);
      assert(!dom2.isOpen());
      assert(dom2.isSingleton());

      assert(AbstractDomain::canBeCompared(dom0, dom2));
      assert(dom0 != dom2);

      assert(!dom0.isSubsetOf(dom2));
      assert(dom0.isSubsetOf(dom0));
      assert(dom2.isSubsetOf(dom0));
      assert(dom2.isSubsetOf(dom2));

      return(true);
    }

    static void testCopyingBoolDomains() {
      AbstractDomain *copyPtr;
      BoolDomain falseDom(false);
      BoolDomain trueDom(true);
      BoolDomain both;

      copyPtr = falseDom.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::BOOL);
      assertTrue(copyPtr->getTypeName() == LabelStr("BOOL"));
      assertTrue((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      delete copyPtr;

      copyPtr = trueDom.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::BOOL);
      assertTrue(copyPtr->getTypeName() == LabelStr("BOOL"));
      assertTrue((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      delete copyPtr;

      copyPtr = both.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::BOOL);
      assertTrue(copyPtr->getTypeName() == LabelStr("BOOL"));
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      assertFalse((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    static void testCopyingEnumeratedDomains() {
      AbstractDomain *copyPtr;
      EnumeratedDomain emptyOpen;
      std::list<double> values;
      values.push_back(0.0);
      values.push_back(1.1);
      values.push_back(2.7);
      values.push_back(3.1);
      EnumeratedDomain fourDom(values, false); // Open
      values.push_back(4.2);
      EnumeratedDomain fiveDom(values); // Closed
      EnumeratedDomain oneDom(2.7); // Singleton

      copyPtr = emptyOpen.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::REAL_ENUMERATION);
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
      assertTrue(copyPtr->getType() == AbstractDomain::REAL_ENUMERATION);
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_ENUMERATION"));
      assertTrue(copyPtr->isOpen());
      assertTrue(copyPtr->isEnumerated());
      copyPtr->close();
      assertTrue(copyPtr->getSize() == 4);
      assertTrue(copyPtr->isSubsetOf(fiveDom));
      delete copyPtr;

      copyPtr = fiveDom.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::REAL_ENUMERATION);
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_ENUMERATION"));
      assertFalse(copyPtr->isOpen());
      assertTrue(copyPtr->isEnumerated());
      assertTrue(copyPtr->getSize() == 5);
      assertTrue(fourDom.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = oneDom.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::REAL_ENUMERATION);
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
      IntervalDomain four(4.0);
      IntervalDomain empty;
      empty.empty();

      // Domains containing infinities should also be tested.

      copyPtr = empty.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::REAL_INTERVAL);
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
      assertTrue(copyPtr->getType() == AbstractDomain::REAL_INTERVAL);
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
      assertTrue(copyPtr->getType() == AbstractDomain::REAL_INTERVAL);
      assertTrue(copyPtr->getTypeName() == LabelStr("REAL_INTERVAL"));
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
      IntervalIntDomain four(4);
      IntervalIntDomain empty;
      empty.empty();
      // domains containing infinities should also be tested

      copyPtr = empty.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::INT_INTERVAL);
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
      assertTrue(copyPtr->getType() == AbstractDomain::INT_INTERVAL);
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
      assertTrue(copyPtr->getType() == AbstractDomain::INT_INTERVAL);
      assertTrue(copyPtr->getTypeName() == LabelStr("INT_INTERVAL"));
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

    static void testCopyingTemplateDomains() {
      AbstractDomain *copyPtr;
      Domain<Fruits> orangeOnly(orange);
      Domain<Colors> yellowOnly(yellow);
      std::list<Fruits> fruitList;
      Domain<Fruits> emptyFruit(fruitList);
      fruitList.push_back(lemon);
      fruitList.push_back(raspberry);
      Domain<Fruits> noOrange(fruitList);
      fruitList.push_back(orange);
      Domain<Fruits> fruitDom(fruitList);

      // Should test dynamic domains

      copyPtr = emptyFruit.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::USER_DEFINED);
      std::cerr << "\nCopy's type name is " << copyPtr->getTypeName().toString() << ".\n";

      // This assertion is false in the current implementation because
      // Domain<TYPE>::getTypeName() uses an implementation/compiler
      // dependent way to get the name of the type that uses the usual
      // C++ name "mangling" in all the compilers we have.
      // Skipping this check is probably OK since the type names are
      // compared within AbstractDomain::canBeCompared().
      // --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 22
      // assertTrue(copyPtr->getTypeName() == LabelStr("Fruits"));

      // This assertion is also incorrect in the implementation
      // despite being correct semanticly since this kind of
      // distinction is being handled in the Nddl compiler by
      // overriding getTypeName() appropriately.
      // assertFalse(AbstractDomain::canBeCompared(*copyPtr, yellowOnly));

      assertFalse(copyPtr->isOpen());
      assertFalse(copyPtr->isNumeric());
      assertTrue(copyPtr->isEnumerated());
      assertFalse(copyPtr->isInterval());
      assertTrue(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(orange));
      assertFalse(copyPtr->isSingleton());
      assertTrue(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 0);
      assertTrue(*copyPtr == emptyFruit);
      assertFalse(*copyPtr == orangeOnly);
      assertFalse(*copyPtr == fruitDom);
      copyPtr->reset(fruitDom);
      assertFalse(*copyPtr == emptyFruit);
      assertFalse(*copyPtr == orangeOnly);
      assertTrue(*copyPtr == fruitDom);
      delete copyPtr;

      copyPtr = orangeOnly.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::USER_DEFINED);

      // See comment above near similar assertion.
      // assertTrue(copyPtr->getTypeName() == LabelStr("Fruits"));

      // Again, as above.
      // assertFalse(AbstractDomain::canBeCompared(*copyPtr, yellowOnly));

      assertFalse(copyPtr->isOpen());
      assertFalse(copyPtr->isNumeric());
      assertTrue(copyPtr->isEnumerated());
      assertTrue(copyPtr->isFinite());
      assertFalse(copyPtr->isMember(lemon));
      assertTrue(copyPtr->isMember(orange));
      assertTrue(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 1);
      assertFalse(*copyPtr == emptyFruit);
      assertTrue(*copyPtr == orangeOnly);
      assertFalse(*copyPtr == fruitDom);
      assertFalse(copyPtr->intersect(fruitDom));
      assertTrue(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(emptyFruit.isSubsetOf(*copyPtr));
      assertTrue(orangeOnly.isSubsetOf(*copyPtr));
      assertFalse(fruitDom.isSubsetOf(*copyPtr));
      assertTrue(copyPtr->isSubsetOf(Domain<Fruits>(orange)));
      assertTrue(copyPtr->isSubsetOf(fruitDom));
      assertFalse(copyPtr->isSubsetOf(noOrange));
      delete copyPtr;

      copyPtr = fruitDom.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::USER_DEFINED);
      
      // See comment above near similar assertion.
      // assertTrue(copyPtr->getTypeName() == LabelStr("Fruits"));

      // assertFalse(AbstractDomain::canBeCompared(*copyPtr, yellowOnly));

      assertFalse(copyPtr->isOpen());
      assertFalse(copyPtr->isNumeric());
      assertTrue(copyPtr->isEnumerated());
      assertTrue(copyPtr->isFinite());
      assertTrue(copyPtr->isMember(lemon));
      assertTrue(copyPtr->isMember(orange));
      assertFalse(copyPtr->isMember(blueberry));
      assertFalse(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(copyPtr->getSize() == 3);
      assertFalse(*copyPtr == emptyFruit);
      assertFalse(*copyPtr == orangeOnly);
      assertTrue(*copyPtr == fruitDom);
      assertTrue(copyPtr->intersect(orangeOnly));
      assertTrue(copyPtr->isSingleton());
      assertFalse(copyPtr->isEmpty());
      assertTrue(emptyFruit.isSubsetOf(*copyPtr));
      assertTrue(orangeOnly.isSubsetOf(*copyPtr));
      assertFalse(fruitDom.isSubsetOf(*copyPtr));
      assertTrue(copyPtr->isSubsetOf(Domain<Fruits>(orange)));
      assertTrue(copyPtr->isSubsetOf(fruitDom));
      assertFalse(copyPtr->isSubsetOf(noOrange));
      delete copyPtr;

      copyPtr = yellowOnly.copy();
      assertTrue(copyPtr->getType() == AbstractDomain::USER_DEFINED);
      
      // See comment above near similar assertion.
      // assertTrue(copyPtr->getTypeName() == LabelStr("Colors"));

      assertTrue(AbstractDomain::canBeCompared(*copyPtr, yellowOnly));

      //assertFalse(AbstractDomain::canBeCompared(*copyPtr, orangeOnly));

      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    static bool testCopying() {

      // These five functions were mistakenly put in DomainTest.cc originally.
      // They also test the new getTypeName() member functions a little and
      // those member functions' effects on AbstractDomain::canBeCompared().
      testCopyingBoolDomains();
      testCopyingEnumeratedDomains();
      testCopyingIntervalDomains();
      testCopyingIntervalIntDomains();
      testCopyingTemplateDomains();

      BoolDomain boolDom;
      AbstractDomain *copy = boolDom.copy();
      check_error(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.remove(true);
      assertTrue(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      check_error(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.remove(false);
      assertTrue(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      check_error(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.insert(true);
      assertTrue(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      check_error(copy != 0);
      assertTrue(*copy == boolDom && copy != &boolDom);
      boolDom.remove(true);
      assertTrue(*copy != boolDom);
      delete copy;

      IntervalIntDomain iiDom(-2, PLUS_INFINITY);
      copy = iiDom.copy();
      check_error(copy != 0);
      assertTrue(*copy == iiDom && copy != &iiDom);
      iiDom.remove(PLUS_INFINITY);
      assertTrue(*copy != iiDom);
      copy->remove(PLUS_INFINITY);
      assertTrue(*copy == iiDom && copy != &iiDom);
      delete copy;

      IntervalDomain iDom(MINUS_INFINITY);
      copy = iDom.copy();
      check_error(copy != 0);
      assertTrue(*copy == iDom && copy != &iDom);
      iDom.remove(MINUS_INFINITY);
      assertTrue(*copy != iDom);
      copy->remove(MINUS_INFINITY);
      assertTrue(*copy == iDom && copy != &iDom);
      delete copy;

      EnumeratedDomain eDom(2.7);
      copy = eDom.copy();
      check_error(copy != 0);
      assertTrue(*copy == eDom && copy != &eDom);
      eDom.insert(PLUS_INFINITY);
      assertTrue(*copy != eDom);
      eDom.remove(PLUS_INFINITY);
      assertTrue(*copy == eDom && copy != &eDom);
      delete copy;

      return(true);
    }
  };
}

bool DomainTests::test() {
  runTestSuite(Prototype::IntervalDomainTest::test);
  runTestSuite(Prototype::EnumeratedDomainTest::test);
  runTestSuite(Prototype::MixedTypeTest::test);
  return(true);
}
