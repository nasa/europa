#include "Utils.hh"
#include "LabelStr.hh"
#include "Domains.hh"
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
      EUROPA_runTest(testAllocation);
      EUROPA_runTest(testRelaxation);
      EUROPA_runTest(testPrecision);
      EUROPA_runTest(testIntersection);
      EUROPA_runTest(testSubset);
      EUROPA_runTest(testPrinting);
      EUROPA_runTest(testBoolDomain);
      EUROPA_runTest(testDifference);
      EUROPA_runTest(testOperatorEquals);
      EUROPA_runTest(testInfinitesAndInts);
      EUROPA_runTest(testEnumSet);
      EUROPA_runTest(testInsertAndRemove);
      EUROPA_runTest(testValidComparisonWithEmpty_gnats2403);
      EUROPA_runTest(testIntervalSingletonValues);
      EUROPA_runTest(testIntervalIntValues);
      return(true);
    }

  private:
    static bool testAllocation() {
      IntervalDomain realDomain(10.2,20.4);
      CPPUNIT_ASSERT(!realDomain.isEmpty());
      CPPUNIT_ASSERT(!realDomain.isFinite());
      CPPUNIT_ASSERT_EQUAL((unsigned int) PLUS_INFINITY, realDomain.getSize());

      IntervalIntDomain intDomain(10, 20);
      CPPUNIT_ASSERT(intDomain.isFinite());

      double lb, ub;
      intDomain.getBounds(lb,ub);
      CPPUNIT_ASSERT(!realDomain.isFinite());
      CPPUNIT_ASSERT(!realDomain.isFinite());
      CPPUNIT_ASSERT(!realDomain.isFinite());
      IntervalIntDomain d1(intDomain);
      d1.empty();
      CPPUNIT_ASSERT(d1.isEmpty());

      AbstractDomain& d2 = static_cast<AbstractDomain&>(intDomain);
      CPPUNIT_ASSERT(!d2.isEmpty());

      IntervalIntDomain d3(static_cast<IntervalIntDomain&>(intDomain));
      IntervalIntDomain d4;

      CPPUNIT_ASSERT(!(d3 == d4));
      d3.relax(d4);
      CPPUNIT_ASSERT((d3 == d4));

      CPPUNIT_ASSERT((d2 != d4));
      d2.relax(d4);
      CPPUNIT_ASSERT((d2 == d4));
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
      CPPUNIT_ASSERT(res && change == DomainListener::RELAXED);
      CPPUNIT_ASSERT(dom1.isSubsetOf(dom0));
      CPPUNIT_ASSERT(dom0.isSubsetOf(dom1));
      CPPUNIT_ASSERT(dom1 == dom0);

      IntervalIntDomain dom2(-300, 100);
      dom1.intersect(dom2);
      res = l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom1 == dom2);
      dom1.relax(dom2);
      res = l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(!res);
      return(true);
    }

    static bool testPrecision() {
      IntervalDomain dom0(-EPSILON, 0);
      CPPUNIT_ASSERT(dom0.isMember(-EPSILON));
      CPPUNIT_ASSERT(dom0.isMember(-EPSILON -EPSILON/10));
      CPPUNIT_ASSERT(!dom0.isMember(-EPSILON -EPSILON));

      IntervalDomain dom1(-EPSILON, EPSILON/10);
      CPPUNIT_ASSERT(dom1 == dom0);

      IntervalDomain dom2(-EPSILON, -EPSILON/10);
      CPPUNIT_ASSERT(dom2.intersects(dom0));
      return true;
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
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom0 == dom1);

      // verify no change triggered if none should take place.
      dom0.intersect(dom1);
      res = l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(!res);

      // Verify only the upper bound changes
      IntervalIntDomain dom2(-200, 50);
      dom0.intersect(dom2);
      res = l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom0.getLowerBound() == dom1.getLowerBound());
      CPPUNIT_ASSERT(dom0.getUpperBound() == dom2.getUpperBound());

      // Make an intersection that leads to an empty domain
      IntervalIntDomain dom3(500, 1000);
      dom0.intersect(dom3);
      res = l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom0.isEmpty());

      IntervalDomain dom4(0.98, 101.23);
      IntervalDomain dom5(80, 120.44);
      IntervalDomain dom6(80, 101.23);
      dom4.equate(dom5);
      CPPUNIT_ASSERT(dom4 == dom6);
      CPPUNIT_ASSERT(dom5 == dom6);

      IntervalDomain dom7(-1, 0);
      dom6.intersect(dom7);
      CPPUNIT_ASSERT(dom6.isEmpty());

      IntervalDomain dom8;
      IntervalDomain dom9;
      dom8.intersect(IntervalDomain(0.1, 0.10));
      dom9.intersect(IntervalDomain(0.10, 0.10));
      CPPUNIT_ASSERT(dom8.intersects(dom9));

      // Case added to recreate failure case for GNATS 3045
      IntervalDomain dom8a;
      IntervalDomain dom9a;
      dom8a.intersect(IntervalDomain(0.1, 0.1));
      dom9a.intersect(IntervalDomain(0.1, 0.1));
      CPPUNIT_ASSERT(dom8a.intersects(dom9a));
      CPPUNIT_ASSERT(dom8a.getUpperBound() == 0.1);
      CPPUNIT_ASSERT(dom8a.getLowerBound() == 0.1);
      CPPUNIT_ASSERT(dom9a.getUpperBound() == 0.1);
      CPPUNIT_ASSERT(dom9a.getLowerBound() == 0.1);

      // Test at the limit of precision
      IntervalDomain dom10(0.0001);
      IntervalDomain dom11(0.0001);
      CPPUNIT_ASSERT(dom10.intersects(dom11));

      // Test at the limit of precision
      IntervalDomain dom12(-0.0001);
      IntervalDomain dom13(-0.0001);
      CPPUNIT_ASSERT(dom12.intersects(dom13));

      // Test beyond the limits of precission
      IntervalDomain dom14(-0.1 - EPSILON/10);
      IntervalDomain dom15(-0.1);
      CPPUNIT_ASSERT_MESSAGE(dom15.toString() + " should have a non-empty intersection " + dom14.toString(), dom14.intersects(dom15));
      CPPUNIT_ASSERT_MESSAGE(dom15.toString() + " should not cause a change for " + dom14.toString(), !dom14.intersect(dom15));

      IntervalIntDomain intBase(-3, 3);
      IntervalIntDomain dom16(intBase);
      IntervalDomain dom17(-2.9, 3);
      IntervalDomain dom18(-3, 2.9);
      IntervalDomain dom19(-2.9, 2.9);
      IntervalDomain dom20(-0.9, 3);
      IntervalDomain dom21(-3, 0.9);
      IntervalDomain dom22(0.3, 0.4);

      dom16.intersect(dom17);
      CPPUNIT_ASSERT(dom16.getLowerBound() == -2.0 && dom16.getUpperBound() == 3.0);
      dom16.relax(intBase);

      dom16.intersect(dom18);
      CPPUNIT_ASSERT(dom16.getLowerBound() == -3.0 && dom16.getUpperBound() == 2.0);
      dom16.relax(intBase);

      dom16.intersect(dom19);
      CPPUNIT_ASSERT(dom16.getLowerBound() == -2 && dom16.getUpperBound() == 2.0);
      dom16.relax(intBase);

      dom16.intersect(dom20);
      CPPUNIT_ASSERT(dom16.getLowerBound() == 0.0 && dom16.getUpperBound() == 3.0);
      dom16.relax(intBase);

      dom16.intersect(dom21);
      CPPUNIT_ASSERT(dom16.getLowerBound() == -3.0 && dom16.getUpperBound() == 0);
      dom16.relax(intBase);

      dom16.intersect(dom22);
      CPPUNIT_ASSERT(dom16.isEmpty());
      dom16.relax(intBase);

      return(true);
    }

    static bool testSubset() {
      IntervalIntDomain dom0(10, 35);
      IntervalDomain dom1(0, 101);
      CPPUNIT_ASSERT(dom0.isSubsetOf(dom1));
      CPPUNIT_ASSERT(! dom1.isSubsetOf(dom0));

      // Handle cases where domains are equal
      IntervalIntDomain dom2(dom0);
      CPPUNIT_ASSERT(dom2 == dom0);
      CPPUNIT_ASSERT(dom0.isSubsetOf(dom2));
      CPPUNIT_ASSERT(dom2.isSubsetOf(dom0));

      // Handle case with no intersection
      IntervalIntDomain dom3(0, 9);
      CPPUNIT_ASSERT(! dom3.isSubsetOf(dom0));
      CPPUNIT_ASSERT(! dom0.isSubsetOf(dom3));

      // Handle case with partial intersection
      IntervalIntDomain dom4(0, 20);
      CPPUNIT_ASSERT(! dom4.isSubsetOf(dom0));
      CPPUNIT_ASSERT(! dom0.isSubsetOf(dom4));

      // Handle intersection with infinites
      IntervalDomain dom5;
      IntervalDomain dom6(0, 100);
      CPPUNIT_ASSERT(dom6.isSubsetOf(dom5));
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
      std::string expectedString("int:CLOSED[1, 100]");
      CPPUNIT_ASSERT(actualString == expectedString);
      std::string anotherActualString = d1.toString();
      CPPUNIT_ASSERT(anotherActualString == expectedString);

      //intervalInt domain
      IntervalIntDomain intervalInt (1,100);
      intervalInt.set(1);
      std::string d1DisplayValueStr = intervalInt.toString(intervalInt.getSingletonValue());
      std::string expectedD1DisplayValue("1");
      CPPUNIT_ASSERT(d1DisplayValueStr == expectedD1DisplayValue);

      //intervalReal domain
      IntervalDomain intervalReal (1.5, 100.6);
      intervalReal.set(1.5);
      std::string d2DisplayValueStr = intervalReal.toString(intervalReal.getSingletonValue());
      std::string expectedD2DisplayValue("1.5");
      CPPUNIT_ASSERT(d2DisplayValueStr == expectedD2DisplayValue);

      // boolean domain
      BoolDomain boolDomainTrue(true);
      boolDomainTrue.set(true);
      std::string d3DisplayValueStr = boolDomainTrue.toString(boolDomainTrue.getSingletonValue());
      std::string expectedD3DisplayValue("true");
      CPPUNIT_ASSERT(d3DisplayValueStr == expectedD3DisplayValue);

      BoolDomain boolDomainFalse(false);
      boolDomainFalse.set(false);
      std::string d4DisplayValueStr = boolDomainFalse.toString(boolDomainFalse.getSingletonValue());
      std::string expectedD4DisplayValue("false");
      CPPUNIT_ASSERT(d4DisplayValueStr == expectedD4DisplayValue);

      // numeric domain
      NumericDomain numericDom(1.117);
      numericDom.set(1.117);
      std::string d5DisplayValueStr = numericDom.toString(numericDom.getSingletonValue());
      std::string expectedD5DisplayValue("1.117");
      CPPUNIT_ASSERT(d5DisplayValueStr == expectedD5DisplayValue);

      // string domain
      LabelStr theString("AString");
      StringDomain stringDom(theString);
      stringDom.set(theString);
      CPPUNIT_ASSERT(stringDom.isSingleton());
      std::string d6DisplayValueStr = stringDom.toString(stringDom.getSingletonValue());
      std::string expectedD6DisplayValue("AString");
      CPPUNIT_ASSERT(d6DisplayValueStr == expectedD6DisplayValue);

      // symbol domain
      LabelStr element("ASymbol");
      SymbolDomain symbolDom(element);
      symbolDom.set(element);
      std::string d7DisplayValueStr = symbolDom.toString(symbolDom.getSingletonValue());
      std::string expectedD7DisplayValue("ASymbol");
      CPPUNIT_ASSERT(d7DisplayValueStr == expectedD7DisplayValue);

      return(true);
    }

    static bool testBoolDomain() {
      BoolDomain dom0(true);
      CPPUNIT_ASSERT(dom0.getSize() == 1);
      CPPUNIT_ASSERT(dom0.getUpperBound() == true);
      CPPUNIT_ASSERT(dom0.getLowerBound() == true);

      BoolDomain dom1;
      CPPUNIT_ASSERT(dom1.getSize() == 2);
      CPPUNIT_ASSERT(dom1.getUpperBound() == true);
      CPPUNIT_ASSERT(dom1.getLowerBound() == false);

      dom1.intersect(dom0);
      CPPUNIT_ASSERT(dom1 == dom0);
      return(true);
    }

    static bool testDifference() {
      IntervalDomain dom0(1, 10);
      IntervalDomain dom1(11, 20);
      bool res = dom0.difference(dom1);
      CPPUNIT_ASSERT(!res);
      res = dom1.difference(dom0);
      CPPUNIT_ASSERT(!res);

      IntervalDomain dom2(dom0);
      res = dom2.difference(dom0);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom2.isEmpty());

      IntervalIntDomain dom3(5, 100);
      res = dom3.difference(dom0);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom3.getLowerBound() == 11);
      res = dom3.difference(dom1);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom3.getLowerBound() == 21);

      IntervalDomain dom4(0, 20);
      res = dom4.difference(dom1);
      CPPUNIT_ASSERT(res);
      double newValue = (dom1.getLowerBound() - dom4.minDelta());
      CPPUNIT_ASSERT(dom4.getUpperBound() == newValue);

      NumericDomain dom5(3.14159265);
      CPPUNIT_ASSERT(dom5.getSize() == 1);

      std::list<double> vals;
      vals.push_back(dom5.getSingletonValue());
      vals.push_back(1.2);
      vals.push_back(2.1);
      vals.push_back(PLUS_INFINITY);
      vals.push_back(MINUS_INFINITY);
      vals.push_back(EPSILON);
      NumericDomain dom6(vals);

      CPPUNIT_ASSERT(dom6.getSize() == 6);
      CPPUNIT_ASSERT(fabs(dom5.minDelta() - dom6.minDelta()) < EPSILON); // Should be ==, but allow some leeway.
      CPPUNIT_ASSERT(dom6.intersects(dom5));

      dom6.difference(dom5);
      CPPUNIT_ASSERT(!(dom6.intersects(dom5)));
      CPPUNIT_ASSERT(dom6.getSize() == 5);

      dom6.difference(dom5);
      CPPUNIT_ASSERT(!(dom6.intersects(dom5)));
      CPPUNIT_ASSERT(dom6.getSize() == 5);

      return(true);
    }

    static bool testOperatorEquals() {
      IntervalDomain dom0(1, 28);
      IntervalDomain dom1(50, 100);
      dom0 = dom1;
      CPPUNIT_ASSERT(dom0 == dom1);
      return(true);
    }

    static bool testInfinitesAndInts() {
      IntervalDomain dom0;
      CPPUNIT_ASSERT(dom0.translateNumber(MINUS_INFINITY) == MINUS_INFINITY);
      CPPUNIT_ASSERT(dom0.translateNumber(MINUS_INFINITY - 1) == MINUS_INFINITY);
      CPPUNIT_ASSERT(dom0.translateNumber(MINUS_INFINITY + 1) == MINUS_INFINITY + 1);
      CPPUNIT_ASSERT(dom0.translateNumber(PLUS_INFINITY + 1) == PLUS_INFINITY);
      CPPUNIT_ASSERT(dom0.translateNumber(PLUS_INFINITY - 1) == PLUS_INFINITY - 1);
      CPPUNIT_ASSERT(dom0.translateNumber(2.8) == 2.8);

      IntervalIntDomain dom1;
      CPPUNIT_ASSERT(dom1.translateNumber(2.8, false) == 2);
      CPPUNIT_ASSERT(dom1.translateNumber(2.8, true) == 3);
      CPPUNIT_ASSERT(dom1.translateNumber(PLUS_INFINITY - 0.2, false) == PLUS_INFINITY - 1);
      return(true);
    }

    static bool testEnumSet(){

      EnumeratedDomain dom0(FloatDT::instance());
      CPPUNIT_ASSERT(dom0.isOpen());
      dom0.insert(1); // required for isMember precondition of set.
      dom0.set(1);
      CPPUNIT_ASSERT(dom0.isSingleton());
      return true;
    }

    static bool testEnumDomInsertAndRemove() {
      // Should add a loop like the one in
      //   testIntervalDomInsertAndRemove(). --wedgingt 2004 Mar 8

      NumericDomain enumDom1;
      CPPUNIT_ASSERT(enumDom1.isOpen());
      CPPUNIT_ASSERT(enumDom1.isNumeric());
      enumDom1.insert(3.14159265);
      enumDom1.close();
      CPPUNIT_ASSERT(enumDom1.isMember(3.14159265));
      CPPUNIT_ASSERT(enumDom1.isSingleton());
      CPPUNIT_ASSERT(enumDom1.isFinite());
      enumDom1.remove(5.0);
      CPPUNIT_ASSERT(enumDom1.isMember(3.14159265));
      CPPUNIT_ASSERT(enumDom1.isSingleton());
      CPPUNIT_ASSERT(enumDom1.isFinite());

      double minDiff = enumDom1.minDelta();
      CPPUNIT_ASSERT(minDiff >= EPSILON && EPSILON > 0.0);

      const double onePlus = 1.0 + 2.0*EPSILON;

      enumDom1.remove(3.14159265 - onePlus*minDiff);
      CPPUNIT_ASSERT(enumDom1.isMember(3.14159265));
      CPPUNIT_ASSERT(enumDom1.isSingleton());
      CPPUNIT_ASSERT(enumDom1.isFinite());
      enumDom1.remove(3.14159265 + onePlus*minDiff);
      CPPUNIT_ASSERT(enumDom1.isMember(3.14159265));
      CPPUNIT_ASSERT(enumDom1.isSingleton());
      CPPUNIT_ASSERT(enumDom1.isFinite());
      enumDom1.remove(3.14159265 - minDiff/onePlus);
      CPPUNIT_ASSERT(enumDom1.isEmpty());
      enumDom1.insert(3.14159265);
      CPPUNIT_ASSERT(enumDom1.isMember(3.14159265));
      CPPUNIT_ASSERT(enumDom1.isSingleton());
      CPPUNIT_ASSERT(enumDom1.isFinite());
      enumDom1.remove(3.14159265 + minDiff/onePlus);
      CPPUNIT_ASSERT(enumDom1.isEmpty());
      enumDom1.insert(3.14159265);
      CPPUNIT_ASSERT(enumDom1.isMember(3.14159265));
      CPPUNIT_ASSERT(enumDom1.isSingleton());

      std::list<double> vals;
      vals.push_back(enumDom1.getSingletonValue());
      vals.push_back(1.2);
      vals.push_back(2.1);
      vals.push_back(PLUS_INFINITY);
      vals.push_back(MINUS_INFINITY);
      vals.push_back(EPSILON);
      NumericDomain enumDom2(vals);

      CPPUNIT_ASSERT(!(enumDom2.isOpen()));
      CPPUNIT_ASSERT(enumDom2.isNumeric());
      CPPUNIT_ASSERT(enumDom2.isFinite());
      CPPUNIT_ASSERT(enumDom2.getSize() == 6);

      double minDiff2 = enumDom2.minDelta();
      CPPUNIT_ASSERT(fabs(minDiff - minDiff2) < EPSILON);

      enumDom2.remove(1.2 - minDiff2/onePlus);
      CPPUNIT_ASSERT(enumDom2.getSize() == 5);

      enumDom2.remove(MINUS_INFINITY);
      CPPUNIT_ASSERT(enumDom2.getSize() == 4);

      // Remove a value near but not "matching" a member and
      //   verify the domain has not changed.
      enumDom2.remove(enumDom1.getSingletonValue() - onePlus*minDiff2);
      CPPUNIT_ASSERT(enumDom2.intersects(enumDom1));
      CPPUNIT_ASSERT(enumDom2.getSize() == 4);

      // Remove a value near but not equal a member and
      //   verify the member was removed via intersection.
      enumDom2.remove(enumDom1.getSingletonValue() - minDiff2/onePlus);
      CPPUNIT_ASSERT(!(enumDom2.intersects(enumDom1)));
      CPPUNIT_ASSERT(enumDom2.getSize() == 3);

      // Add a value near a value from another domain
      //   verify that the resulting domain intersects the other domain.
      enumDom2.insert(enumDom1.getSingletonValue() + minDiff2/onePlus);
      CPPUNIT_ASSERT(enumDom2.intersects(enumDom1));
      CPPUNIT_ASSERT(enumDom2.getSize() == 4);

      // Add the value in the other domain and
      //   verify the domain is not affected.
      enumDom2.insert(enumDom1.getSingletonValue());
      CPPUNIT_ASSERT(enumDom2.intersects(enumDom1));
      CPPUNIT_ASSERT(enumDom2.getSize() == 4);

      // Remove a value that should not be a member but is
      //   only slightly too large to "match" the new member.
      enumDom2.remove(enumDom1.getSingletonValue() + minDiff2/onePlus + onePlus*minDiff2);
      CPPUNIT_ASSERT(enumDom2.intersects(enumDom1));
      CPPUNIT_ASSERT(enumDom2.getSize() == 4);

      // Remove a value "matching" the added value but larger
      //   and verify the domain no longer intersects the other domain.
      enumDom2.remove(enumDom1.getSingletonValue() + 2.0*minDiff2/onePlus);
      CPPUNIT_ASSERT(enumDom2.getSize() == 3);
      CPPUNIT_ASSERT(!(enumDom2.intersects(enumDom1)));

      return(true);
    }
    /* NO LONGER SUPPORT INSERT AND REMOVE ON INTERVAL DOMAINS
    static bool testIntervalDomInsertAndRemove() {
      CPPUNIT_ASSERT(EPSILON > 0.0); // Otherwise, loop will be infinite.

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
        CPPUNIT_ASSERT(iDom.isSingleton());
        CPPUNIT_ASSERT(iDom.isMember(val));
        CPPUNIT_ASSERT(iDom.isMember(val + minDiff/onePlus));
        CPPUNIT_ASSERT(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val + minDiff/onePlus);
        CPPUNIT_ASSERT(iDom.isEmpty());
        CPPUNIT_ASSERT(!(iDom.isMember(val)));

        iDom.insert(val);
        CPPUNIT_ASSERT(iDom.isSingleton());
        CPPUNIT_ASSERT(iDom.isMember(val));
        CPPUNIT_ASSERT(iDom.isMember(val + minDiff/onePlus));
        CPPUNIT_ASSERT(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val - onePlus*minDiff);
        CPPUNIT_ASSERT(iDom.isSingleton());
        CPPUNIT_ASSERT(iDom.isMember(val));
        CPPUNIT_ASSERT(iDom.isMember(val + minDiff/onePlus));
        CPPUNIT_ASSERT(iDom.isMember(val - minDiff/onePlus));

        iDom.remove(val - minDiff/onePlus);
        CPPUNIT_ASSERT(iDom.isEmpty());
        CPPUNIT_ASSERT(!(iDom.isMember(val)));

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
      CPPUNIT_ASSERT(iiDom.getSize() == 16);

      iiDom.remove(-6);
      CPPUNIT_ASSERT(iiDom.getSize() == 16);

      iiDom.remove(11);
      CPPUNIT_ASSERT(iiDom.getSize() == 16);

      iiDom.remove(PLUS_INFINITY);
      CPPUNIT_ASSERT(iiDom.getSize() == 16);

      iiDom.insert(-5);
      CPPUNIT_ASSERT(iiDom.getSize() == 16);

      iiDom.insert(-1);
      CPPUNIT_ASSERT(iiDom.getSize() == 16);

      iiDom.insert(10);
      CPPUNIT_ASSERT(iiDom.getSize() == 16);

      iiDom.insert(11);
      CPPUNIT_ASSERT(iiDom.getSize() == 17);

      iiDom.insert(-6);
      CPPUNIT_ASSERT(iiDom.getSize() == 18);

      iiDom.remove(PLUS_INFINITY);
      CPPUNIT_ASSERT(iiDom.getSize() == 18);

      iiDom.remove(-7);
      CPPUNIT_ASSERT(iiDom.getSize() == 18);

      iiDom.remove(11);
      CPPUNIT_ASSERT(iiDom.getSize() == 17);

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

      CPPUNIT_ASSERT(d0 == d1);
      CPPUNIT_ASSERT(d1 != d2);
      d0.empty();
      CPPUNIT_ASSERT(d0 != d1);
      CPPUNIT_ASSERT(d0 != d2);

      CPPUNIT_ASSERT(AbstractDomain::canBeCompared(d0, d2));

      NumericDomain d3(d2);
      d2.empty();
      CPPUNIT_ASSERT(AbstractDomain::canBeCompared(d2, d3));
      CPPUNIT_ASSERT(d3 != d2);

      return true;
    }

    static bool testIntervalSingletonValues() {
      for (double value = -2.0 ; value <= 1.5 ; value += 0.1) {
        IntervalDomain id(value, value);
        std::list<double> values;
        id.getValues(values);
        CPPUNIT_ASSERT(values.size() == 1);
        CPPUNIT_ASSERT(values.front() == value);
      }
      for (double value = 2.0 ; value >= 1.5 ; value -= 0.1) {
        IntervalDomain id(value, value);
        std::list<double> values;
        id.getValues(values);
        CPPUNIT_ASSERT(values.size() == 1);
        CPPUNIT_ASSERT(values.front() == value);
      }
      IntervalDomain id(0, 0);
      std::list<double> values;
      id.getValues(values);
      CPPUNIT_ASSERT(values.size() == 1);
      CPPUNIT_ASSERT(values.front() == 0);
      return true;
    }

    static bool testIntervalIntValues() {
      std::list<double> values;

      IntervalIntDomain i0(10, 20);
      i0.getValues(values);
      CPPUNIT_ASSERT(values.size() == 11);
      CPPUNIT_ASSERT(values.front() == 10); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 11); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 12); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 13); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 14); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 15); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 16); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 17); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 18); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 19); values.pop_front();
      CPPUNIT_ASSERT(values.front() == 20); values.pop_front();

      IntervalIntDomain i1(-4, 3);
      i1.getValues(values);
      CPPUNIT_ASSERT(values.size() == 8);
      CPPUNIT_ASSERT(values.front() == -4); values.pop_front();
      CPPUNIT_ASSERT(values.front() == -3); values.pop_front();
      CPPUNIT_ASSERT(values.front() == -2); values.pop_front();
      CPPUNIT_ASSERT(values.front() == -1); values.pop_front();
      CPPUNIT_ASSERT(values.front() ==  0); values.pop_front();
      CPPUNIT_ASSERT(values.front() ==  1); values.pop_front();
      CPPUNIT_ASSERT(values.front() ==  2); values.pop_front();
      CPPUNIT_ASSERT(values.front() ==  3); values.pop_front();

      IntervalIntDomain i2(-10, 10);
      IntervalDomain& i3 = i2;
      IntervalDomain* i4 = &i2;
      CPPUNIT_ASSERT(i2.minDelta() == 1);
      CPPUNIT_ASSERT(i3.minDelta() == 1);
      CPPUNIT_ASSERT(i4->minDelta() == 1);
      CPPUNIT_ASSERT(dynamic_cast<IntervalDomain*>(&i2)->minDelta() == 1);
      //std::cout << "FOO: " << static_cast<IntervalDomain>(i2).minDelta() << std::endl;
      //CPPUNIT_ASSERT(static_cast<IntervalDomain>(i2).minDelta() == 1);
      //CPPUNIT_ASSERT(((IntervalDomain)i2).minDelta() == 1);

      return true;
    }

  };



  class EnumeratedDomainTest {
  public:

    static bool test() {
      EUROPA_runTest(testStrings);
      EUROPA_runTest(testEnumerationOnly);
      EUROPA_runTest(testBasicLabelOperations);
      EUROPA_runTest(testLabelSetAllocations);
      EUROPA_runTest(testEquate);
      EUROPA_runTest(testValueRetrieval);
      EUROPA_runTest(testIntersection);
      EUROPA_runTest(testDifference);
      EUROPA_runTest(testOperatorEquals);
      EUROPA_runTest(testEmptyOnClosure);
      EUROPA_runTest(testOpenEnumerations);
      return true;
    }

  private:

    static bool testStrings() {
      StringDomain s1;
      StringDomain s2;

      // Open domains always intersect
      CPPUNIT_ASSERT(s1.intersects(s2));

      // Should be able to call an intersection operation and they will continue to intersect
      s1.intersect(s2);
      CPPUNIT_ASSERT(s1.intersects(s2));
      CPPUNIT_ASSERT(s1.isOpen());
      CPPUNIT_ASSERT(s2.isOpen());

      // If we add a value to s2, and interesect there should be no impact to s1
      s2.insert("string_1");
      CPPUNIT_ASSERT(s1.intersects(s2));
      CPPUNIT_ASSERT(s2.intersects(s1));
      s1.intersect(s2);

      // Now if we close s2, we should reduce to the singleton, and intersection applies that restriction to s1
      s2.close();
      CPPUNIT_ASSERT(!s2.isEmpty());
      CPPUNIT_ASSERT(s2.isMember("string_1"));

      CPPUNIT_ASSERT(s1.intersects(s2));
      s1.intersect(s2);
      CPPUNIT_ASSERT(!s1.isOpen());

      StringDomain s3;
      s1.reset(s3);
      CPPUNIT_ASSERT(s1.isOpen());

      return true;
    }

    static bool testOpenEnumerations() {
      EnumeratedDomain e1(FloatDT::instance());
      ChangeListener l1;
      DomainListener::ChangeType change;
      e1.setListener(l1.getId());

      CPPUNIT_ASSERT(e1.isOpen());
      CPPUNIT_ASSERT(e1.isFinite());
      CPPUNIT_ASSERT(e1.isEmpty());
      e1.insert(1.0);
      CPPUNIT_ASSERT(e1.isOpen());

      e1.remove(1.0);
      CPPUNIT_ASSERT(l1.checkAndClearChange(change));
      CPPUNIT_ASSERT(change == DomainListener::VALUE_REMOVED);
      e1.insert(1.0);

      e1.close();
      CPPUNIT_ASSERT(e1.isClosed());
      CPPUNIT_ASSERT(l1.checkAndClearChange(change));
      CPPUNIT_ASSERT(change == DomainListener::CLOSED);

      e1.open();
      CPPUNIT_ASSERT(e1.isOpen());
      CPPUNIT_ASSERT(l1.checkAndClearChange(change));
      CPPUNIT_ASSERT(change == DomainListener::OPENED);

      e1.insert(2.0);
      e1.insert(3.0);
      e1.insert(4.0);

      std::list<double> vals;
      vals.push_back(2.0);
      vals.push_back(3.0);
      EnumeratedDomain e2(FloatDT::instance(),vals);

      CPPUNIT_ASSERT(e2.isClosed());

      e1.insert(4.0);
      e1.set(4.0);
      CPPUNIT_ASSERT(e1.isClosed());
      e1.open();
      e1.insert(1.0);
      e1.insert(2.0);
      e1.insert(3.0);

      e1.intersect(e2);
      CPPUNIT_ASSERT(e1.isClosed());

      e2.open();
      CPPUNIT_ASSERT(e2.isOpen());
      e1.intersect(e2);
      CPPUNIT_ASSERT(e1.isClosed());

      e2.close();
      e1.open();
      e1.insert(4.0);
      e2.reset(e1);
      CPPUNIT_ASSERT(e2.isOpen());

      CPPUNIT_ASSERT(e1 == e2);
      e2.close();
      CPPUNIT_ASSERT(e1 != e2);
      e1.close();
      CPPUNIT_ASSERT(e1 == e2);

      e1.open();
      e2.open();
      CPPUNIT_ASSERT(e1.intersects(e2));
      e2.close();
      CPPUNIT_ASSERT(e1.intersects(e2));
      e1.close();
      CPPUNIT_ASSERT(e1.intersects(e2));

      e1.open();
      e2.open();
      CPPUNIT_ASSERT(e1.isSubsetOf(e2));
      e2.close();
      CPPUNIT_ASSERT(!e1.isSubsetOf(e2));
      e1.close();
      CPPUNIT_ASSERT(e1.isSubsetOf(e2));

      e1.open();
      e2.open();

      e1.equate(e2);
      CPPUNIT_ASSERT(e1.isOpen() && e2.isOpen());
      e2.close();
      e1.equate(e2);
      CPPUNIT_ASSERT(e1.isClosed() && e2.isClosed());
      e2.open();
      e1.equate(e2);
      CPPUNIT_ASSERT(e1.isClosed() && e2.isClosed());
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
      CPPUNIT_ASSERT(d0 == d1);


      {
	double value(0);
	std::stringstream sstr;
	sstr << -0.01;
	CPPUNIT_ASSERT(d0.convertToMemberValue(sstr.str(), value));
	CPPUNIT_ASSERT(value == -0.01);
      }

      {
	double value(0);
	std::stringstream sstr;
	sstr << 88.46;
	CPPUNIT_ASSERT(!d0.convertToMemberValue(sstr.str(), value));
	CPPUNIT_ASSERT(value == 0);
      }

      CPPUNIT_ASSERT(d0.isSubsetOf(d1));
      CPPUNIT_ASSERT(d0.isMember(-98.67));
      d0.remove(-0.01);
      CPPUNIT_ASSERT(!d0.isMember(-0.01));
      CPPUNIT_ASSERT(d0.isSubsetOf(d1));
      CPPUNIT_ASSERT(!d1.isSubsetOf(d0));

      return(true);
    }

    static bool testBasicLabelOperations() {
      int initialCount = EUROPA::LabelStr::getSize();
      EUROPA::LabelStr dt_l1("DT_L1");
      EUROPA::LabelStr dt_l2("DT_L2");
      EUROPA::LabelStr dt_l3("DT_L3");
      CPPUNIT_ASSERT(dt_l1 < dt_l2 && dt_l2 < dt_l3);

      EUROPA::LabelStr la("L");
      EUROPA::LabelStr l4("L30");
      EUROPA::LabelStr lb("L");
      CPPUNIT_ASSERT(la == lb);
      CPPUNIT_ASSERT(la < l4);

      EUROPA::LabelStr copy1(dt_l1);
      CPPUNIT_ASSERT(dt_l1 == copy1);
      CPPUNIT_ASSERT(dt_l2 != copy1);

      CPPUNIT_ASSERT((EUROPA::LabelStr::getSize() - initialCount) == 5);
      CPPUNIT_ASSERT(dt_l1.toString() == "DT_L1");

      CPPUNIT_ASSERT(LabelStr::isString(dt_l1.getKey()));
      CPPUNIT_ASSERT(!LabelStr::isString(PLUS_INFINITY+1));
      return(true);
    }

    static bool testLabelSetAllocations() {
      std::list<double> values;
      values.push_back(EUROPA::LabelStr("DT_L1"));
      values.push_back(EUROPA::LabelStr("L4"));
      values.push_back(EUROPA::LabelStr("DT_L2"));
      values.push_back(EUROPA::LabelStr("L5"));
      values.push_back(EUROPA::LabelStr("DT_L3"));

      ChangeListener l_listener;
      LabelSet ls0(values);
      ls0.setListener(l_listener.getId());
      CPPUNIT_ASSERT(!ls0.isOpen());

      EUROPA::LabelStr dt_l2("DT_L2");
      CPPUNIT_ASSERT(ls0.isMember(dt_l2));
      DomainListener::ChangeType change;
      ls0.remove(dt_l2);
      bool res = l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(res && change == DomainListener::VALUE_REMOVED);
      CPPUNIT_ASSERT(!ls0.isMember(dt_l2));

      EUROPA::LabelStr dt_l3("DT_L3");
      ls0.set(dt_l3);
      CPPUNIT_ASSERT(ls0.isMember(dt_l3));
      CPPUNIT_ASSERT(ls0.getSize() == 1);

      LabelSet ls1(values);
      ls0.relax(ls1);
      res = l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(res && change == DomainListener::RELAXED);
      CPPUNIT_ASSERT(ls0 == ls1);
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

      CPPUNIT_ASSERT(ls0 == ls1);
      CPPUNIT_ASSERT(ls0.getSize() == 8);
      bool res = ls0.equate(ls1);
      CPPUNIT_ASSERT(res == false); // Implying no change occured

      EUROPA::LabelStr lC("C");
      ls0.remove(lC);
      CPPUNIT_ASSERT(!ls0.isMember(lC));
      CPPUNIT_ASSERT(ls1.isMember(lC));
      res = ls0.equate(ls1);
      CPPUNIT_ASSERT(res); // It should have changed
      CPPUNIT_ASSERT(!ls1.isMember(lC));

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
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(ls2 == ls3);

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
      CPPUNIT_ASSERT(res && change == DomainListener::EMPTIED);
      CPPUNIT_ASSERT(ls4.isEmpty() || ls5.isEmpty());
      CPPUNIT_ASSERT(!(ls4.isEmpty() && ls5.isEmpty()));

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
      CPPUNIT_ASSERT(ed1 == ed2);

      ed1.equate(ed3);
      CPPUNIT_ASSERT(ed1 == ed3);

      enumVals.clear();
      enumVals.push_back(0.0);
      NumericDomain ed0(enumVals);

      ed1.equate(ed0);

      // This is actually false because equate() only empties
      // one of the domains when the intersection is empty.
      // CPPUNIT_ASSERT(ed0 == ed1);

      CPPUNIT_ASSERT(!(ed0 == ed1));
      CPPUNIT_ASSERT(ed1.isEmpty() != ed0.isEmpty());

      ed0 = NumericDomain(enumVals);
      CPPUNIT_ASSERT(!ed0.isEmpty());

      ed0.equate(ed2);
      CPPUNIT_ASSERT(ed2 != ed0 && ed2.isEmpty() != ed0.isEmpty());

      enumVals.push_back(20.0); // Now 0.0 and 20.0
      ed0 = NumericDomain(enumVals);
      CPPUNIT_ASSERT(!ed0.isEmpty() && !ed0.isSingleton());

      IntervalDomain id0(-10.0, 10.0);

      id0.equate(ed0);
      //CPPUNIT_ASSERT(ed0.isSingleton() && ed0.getSingletonValue() == 0.0, ed0.toString());
      //CPPUNIT_ASSERT(id0.isSingleton() && id0.getSingletonValue() == 0.0, id0.toString());

      ed0 = NumericDomain(enumVals); // Now 0.0 and 20.0
      CPPUNIT_ASSERT(!ed0.isEmpty() && !ed0.isSingleton());

      IntervalDomain id1(0.0, 5.0);

      ed0.equate(id1);
      CPPUNIT_ASSERT(ed0.isSingleton() && ed0.getSingletonValue() == 0.0);
      CPPUNIT_ASSERT(id1.isSingleton() && id1.getSingletonValue() == 0.0);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      enumVals.push_back(1.5);
      NumericDomain ed5(enumVals);
      IntervalDomain id2(2.5, 3.0);

      ed5.equate(id2);
      CPPUNIT_ASSERT(ed5.isSingleton() && ed5.getSingletonValue() == 2.5);
      CPPUNIT_ASSERT(id2.isSingleton() && id2.getSingletonValue() == 2.5);

      enumVals.clear();
      enumVals.push_back(3.375);
      enumVals.push_back(2.5);
      enumVals.push_back(1.5);
      enumVals.push_back(-2.0);
      NumericDomain ed6(enumVals);
      IntervalDomain id3(-1.0, 3.0);

      id3.equate(ed6);
      CPPUNIT_ASSERT(ed6.getSize() == 2);
      CPPUNIT_ASSERT(id3 == IntervalDomain(1.5, 2.5));

      IntervalDomain id4(1.0, 1.25);

      ed6.equate(id4);
      CPPUNIT_ASSERT(ed6.isEmpty() != id4.isEmpty());

      enumVals.clear();
      enumVals.push_back(1.0);
      NumericDomain ed7(enumVals);
      IntervalDomain id5(1.125, PLUS_INFINITY);

      id5.equate(ed7);
      CPPUNIT_ASSERT(ed7.isEmpty() != id5.isEmpty());

      return(true);
    }

    static bool testValueRetrieval() {
      std::list<double> values;
      values.push_back(EUROPA::LabelStr("A"));
      values.push_back(EUROPA::LabelStr("B"));
      values.push_back(EUROPA::LabelStr("C"));
      values.push_back(EUROPA::LabelStr("D"));
      values.push_back(EUROPA::LabelStr("E"));

      LabelSet dt_l1(values);
      std::list<double> results;
      dt_l1.getValues(results);

      LabelSet dt_l2(results);

      CPPUNIT_ASSERT(dt_l1 == dt_l2);
      LabelStr lbl("C");
      dt_l1.set(lbl);
      CPPUNIT_ASSERT(lbl == dt_l1.getSingletonValue());
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
      CPPUNIT_ASSERT(ls1.convertToMemberValue(std::string("H"), value));
      CPPUNIT_ASSERT(value == LabelStr("H"));
      CPPUNIT_ASSERT(!ls1.convertToMemberValue(std::string("LMN"), value));

      LabelSet ls2(values);
      ls2.remove(EUROPA::LabelStr("A"));
      ls2.remove(EUROPA::LabelStr("C"));
      ls2.remove(EUROPA::LabelStr("E"));
      CPPUNIT_ASSERT(ls2.isSubsetOf(ls1));
      CPPUNIT_ASSERT(!ls1.isSubsetOf(ls2));

      LabelSet ls3(ls1);
      ls1.intersect(ls2);
      CPPUNIT_ASSERT(ls1 == ls2);
      CPPUNIT_ASSERT(ls2.isSubsetOf(ls1));

      ls1.relax(ls3);
      CPPUNIT_ASSERT(ls2.isSubsetOf(ls1));
      CPPUNIT_ASSERT(ls1 == ls3);

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
      CPPUNIT_ASSERT(ls4.isEmpty());

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
	CPPUNIT_ASSERT(d0.getSize() == 1);

	// Also test bounds intersection
	d1.intersect(0, 4.6);
	CPPUNIT_ASSERT(d1.getSize() == 2);
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
      CPPUNIT_ASSERT(!res);

      IntervalIntDomain dom2(5, 100);
      res = dom0.difference(dom2);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom0.getUpperBound() == 3);

      IntervalIntDomain dom3(0, 100);
      res = dom0.difference(dom3);
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(dom0.isEmpty());

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

      CPPUNIT_ASSERT(dom0 != dom1);
      dom0 = dom1;
      CPPUNIT_ASSERT(dom0 == dom1);

      dom1 = dom2;
      CPPUNIT_ASSERT(dom1 == dom2);

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
	CPPUNIT_ASSERT(res && change == DomainListener::EMPTIED);
      }

      {
	LabelSet ls0(values); // Will be empty on closure, but no listener attached
	DomainListener::ChangeType change;
	ChangeListener l_listener;
	ls0.setListener(l_listener.getId());
	bool res = l_listener.checkAndClearChange(change);
	CPPUNIT_ASSERT(res && change == DomainListener::EMPTIED);
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
      EUROPA_runTest(testOpenAndClosed);
      EUROPA_runTest(testInfinityBounds);
      EUROPA_runTest(testEquality);
      EUROPA_runTest(testIntersection);
      EUROPA_runTest(testSubset);
      EUROPA_runTest(testIntDomain);
      EUROPA_runTest(testDomainComparatorConfiguration);
      EUROPA_runTest(testCopying);
      EUROPA_runTest(testSymbolicVsNumeric);
      return(true);
    }

  private:
    static bool testOpenAndClosed(){
      NumericDomain a, b;
      ChangeListener l_listener;
      b.setListener(l_listener.getId());
      a.insert(1.0);
      a.insert(2.0);
      a.close();

      b.intersect(a);
      DomainListener::ChangeType change;
      l_listener.checkAndClearChange(change);
      CPPUNIT_ASSERT(!b.isEmpty());
      CPPUNIT_ASSERT(change != DomainListener::EMPTIED);
      return true;
    }

    static bool testInfinityBounds(){
      IntervalDomain dom0;
      CPPUNIT_ASSERT(!dom0.areBoundsFinite());
      IntervalDomain dom1(0, PLUS_INFINITY);
      CPPUNIT_ASSERT(!dom1.areBoundsFinite());
      IntervalDomain dom2(0, PLUS_INFINITY-1);
      CPPUNIT_ASSERT(dom2.areBoundsFinite());
      NumericDomain dom3;
      CPPUNIT_ASSERT(!dom3.areBoundsFinite());
      SymbolDomain dom4;
      CPPUNIT_ASSERT(dom4.areBoundsFinite());
      NumericDomain dom5;
      dom5.insert(0);
      dom5.insert(1);
      CPPUNIT_ASSERT(!dom5.areBoundsFinite());
      dom5.close();
      CPPUNIT_ASSERT(dom5.areBoundsFinite());
      NumericDomain dom6(PLUS_INFINITY);
      CPPUNIT_ASSERT(!dom6.areBoundsFinite());
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
      CPPUNIT_ASSERT(dom1 == dom0);
      CPPUNIT_ASSERT(dom0 == dom1);

      IntervalIntDomain dom2(1);
      CPPUNIT_ASSERT(dom1 == dom2);

      dom0.reset(dom);
      IntervalIntDomain dom3(1, 2);
      CPPUNIT_ASSERT(dom0 == dom3);
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
      CPPUNIT_ASSERT(dom0.getSize() == 6);
      IntervalIntDomain dom1(1, 8);
      NumericDomain dom2(dom0);

      dom0.intersect(dom1);
      CPPUNIT_ASSERT(dom0.getSize() == 1);
      CPPUNIT_ASSERT(dom0.isMember(1.0));

      IntervalDomain dom3(1, 8);
      dom2.intersect(dom3);
      CPPUNIT_ASSERT(dom2.getSize() == 3);

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
      CPPUNIT_ASSERT(dom0.isSubsetOf(dom1));

      IntervalIntDomain dom2(0, 10);
      CPPUNIT_ASSERT(!dom0.isSubsetOf(dom2));

      dom0.remove(0.98);
      dom0.remove(1.89);
      dom0.remove(2.98);
      CPPUNIT_ASSERT(dom0.isSubsetOf(dom2));

      CPPUNIT_ASSERT(dom2.isSubsetOf(dom1));
      CPPUNIT_ASSERT(!dom1.isSubsetOf(dom2));
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
      CPPUNIT_ASSERT(!dom2.isOpen());
      CPPUNIT_ASSERT(dom2.isSingleton());

      CPPUNIT_ASSERT(AbstractDomain::canBeCompared(dom0, dom2));
      CPPUNIT_ASSERT(dom0 != dom2);

      CPPUNIT_ASSERT(!dom0.isSubsetOf(dom2));
      CPPUNIT_ASSERT(dom0.isSubsetOf(dom0));
      CPPUNIT_ASSERT(dom2.isSubsetOf(dom0));
      CPPUNIT_ASSERT(dom2.isSubsetOf(dom2));

      return(true);
    }

    static void testCopyingBoolDomains() {
      AbstractDomain *copyPtr;
      BoolDomain falseDom(false);
      BoolDomain trueDom(true);
      BoolDomain both;
      BoolDomain customDom(true);

      copyPtr = falseDom.copy();
      CPPUNIT_ASSERT(copyPtr->isBool());
      CPPUNIT_ASSERT((dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      CPPUNIT_ASSERT(!(dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      delete copyPtr;

      copyPtr = trueDom.copy();
      CPPUNIT_ASSERT(copyPtr->isBool());
      CPPUNIT_ASSERT((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      CPPUNIT_ASSERT(!(dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      delete copyPtr;

      copyPtr = both.copy();
      CPPUNIT_ASSERT(copyPtr->isBool());
      CPPUNIT_ASSERT(!(dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
      CPPUNIT_ASSERT(!(dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      delete copyPtr;

      copyPtr = customDom.copy();
      CPPUNIT_ASSERT(copyPtr->isBool());
      CPPUNIT_ASSERT(copyPtr->getTypeName().toString() == BoolDT::NAME());
      CPPUNIT_ASSERT((dynamic_cast<BoolDomain*>(copyPtr))->isTrue());
      CPPUNIT_ASSERT(!(dynamic_cast<BoolDomain*>(copyPtr))->isFalse());
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
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("float"));
      CPPUNIT_ASSERT(copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isNumeric());
      CPPUNIT_ASSERT(copyPtr->isEnumerated());
      copyPtr->insert(3.1);
      //assertFalse(copyPtr->isSingleton()); Or should that provoke an error? wedgingt 2004 Mar 3
      copyPtr->close();
      CPPUNIT_ASSERT(copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      delete copyPtr;

      copyPtr = fourDom.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("float"));
      CPPUNIT_ASSERT(copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isEnumerated());
      copyPtr->close();
      CPPUNIT_ASSERT(copyPtr->getSize() == 4);
      CPPUNIT_ASSERT(copyPtr->isSubsetOf(fiveDom));
      delete copyPtr;

      copyPtr = fiveDom.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("float"));
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isEnumerated());
      CPPUNIT_ASSERT(copyPtr->getSize() == 5);
      CPPUNIT_ASSERT(!fourDom.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = oneDom.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("float"));
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isEnumerated());
      CPPUNIT_ASSERT(copyPtr->isSingleton());

      // Can't call this with a dynamic domain, so close it first.
      fourDom.close();
      CPPUNIT_ASSERT(copyPtr->isSubsetOf(fourDom));

      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    static void testCopyingIntervalDomains() {
      AbstractDomain *copyPtr;
      IntervalDomain one2ten(1.0, 10.9);
      IntervalIntDomain four(4,4);
      IntervalDomain empty;
      empty.empty();

      // Domains containing infinities should also be tested.

      copyPtr = empty.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("float"));
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isNumeric());
      CPPUNIT_ASSERT(!copyPtr->isEnumerated());
      CPPUNIT_ASSERT(copyPtr->isFinite());
      CPPUNIT_ASSERT(!copyPtr->isMember(0.0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(copyPtr->isEmpty());
      CPPUNIT_ASSERT(copyPtr->getSize() == 0);
      CPPUNIT_ASSERT(*copyPtr == empty);
      CPPUNIT_ASSERT(!(*copyPtr == one2ten));
      copyPtr->relax(IntervalDomain(-3.1, 11.0));
      CPPUNIT_ASSERT(copyPtr->isMember(0.0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(empty.isEmpty());
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(empty.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = one2ten.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("float"));
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isNumeric());
      CPPUNIT_ASSERT(!copyPtr->isEnumerated());
      CPPUNIT_ASSERT(!copyPtr->isFinite());
      CPPUNIT_ASSERT(!copyPtr->isMember(0.0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(*copyPtr == one2ten);
      copyPtr->relax(IntervalDomain(-3.1, 11.0));
      CPPUNIT_ASSERT(copyPtr->isMember(0.0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(!(*copyPtr == one2ten));
      CPPUNIT_ASSERT(one2ten.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = four.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName().toString() == IntDT::NAME());
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isNumeric());
      CPPUNIT_ASSERT(!copyPtr->isEnumerated());
      CPPUNIT_ASSERT(copyPtr->isFinite());
      CPPUNIT_ASSERT(!copyPtr->isMember(0.0));
      CPPUNIT_ASSERT(copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(copyPtr->getSize() == 1);
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(*copyPtr == four);
      CPPUNIT_ASSERT(!(*copyPtr == one2ten));
      copyPtr->relax(IntervalDomain(-3, 11));
      CPPUNIT_ASSERT(copyPtr->isMember(0.0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(!(*copyPtr == four));
      CPPUNIT_ASSERT(four.isSubsetOf(*copyPtr));
      delete copyPtr;

      // Cannot check that expected errors are detected until
      //   new error handling support is in use.
    }

    static void testCopyingIntervalIntDomains() {
      AbstractDomain *copyPtr;
      IntervalIntDomain one2ten(1, 10);
      IntervalIntDomain four(4,4);
      IntervalIntDomain empty;
      empty.empty();
      // domains containing infinities should also be tested

      copyPtr = empty.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("int"));
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isNumeric());
      CPPUNIT_ASSERT(!copyPtr->isEnumerated());
      CPPUNIT_ASSERT(copyPtr->isFinite());
      CPPUNIT_ASSERT(!copyPtr->isMember(0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(copyPtr->isEmpty());
      CPPUNIT_ASSERT(copyPtr->getSize() == 0);
      CPPUNIT_ASSERT(*copyPtr == empty);
      CPPUNIT_ASSERT(!(*copyPtr == one2ten));
      copyPtr->relax(IntervalDomain(-3, 11));
      CPPUNIT_ASSERT(copyPtr->isMember(0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(empty.isEmpty());
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(empty.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = one2ten.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName() == LabelStr("int"));
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isNumeric());
      CPPUNIT_ASSERT(!copyPtr->isEnumerated());
      CPPUNIT_ASSERT(copyPtr->isFinite());
      CPPUNIT_ASSERT(!copyPtr->isMember(0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(copyPtr->getSize() == 10);
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(*copyPtr == one2ten);
      copyPtr->relax(IntervalIntDomain(-3, 11));
      CPPUNIT_ASSERT(copyPtr->getSize() == 15);
      CPPUNIT_ASSERT(copyPtr->isMember(0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(!(*copyPtr == one2ten));
      CPPUNIT_ASSERT(one2ten.isSubsetOf(*copyPtr));
      delete copyPtr;

      copyPtr = four.copy();
      CPPUNIT_ASSERT(copyPtr->getTypeName().toString() == IntDT::NAME());
      CPPUNIT_ASSERT(!copyPtr->isOpen());
      CPPUNIT_ASSERT(copyPtr->isNumeric());
      CPPUNIT_ASSERT(!copyPtr->isEnumerated());
      CPPUNIT_ASSERT(copyPtr->isFinite());
      CPPUNIT_ASSERT(!copyPtr->isMember(0));
      CPPUNIT_ASSERT(copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(copyPtr->getSize() == 1);
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(*copyPtr == four);
      CPPUNIT_ASSERT(!(*copyPtr == one2ten));
      copyPtr->relax(IntervalIntDomain(-3, 11));
      CPPUNIT_ASSERT(copyPtr->getSize() == 15);
      CPPUNIT_ASSERT(copyPtr->isMember(0));
      CPPUNIT_ASSERT(!copyPtr->isSingleton());
      CPPUNIT_ASSERT(!copyPtr->isEmpty());
      CPPUNIT_ASSERT(!(*copyPtr == empty));
      CPPUNIT_ASSERT(!(*copyPtr == four));
      CPPUNIT_ASSERT(four.isSubsetOf(*copyPtr));
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
      CPPUNIT_ASSERT(AbstractDomain::canBeCompared(dom0, dom1));

      // Switch for the bogus one - and make sure it fails as expected
      BogusComparator bogus;
      CPPUNIT_ASSERT(!AbstractDomain::canBeCompared(dom0, dom1));

      // Allocate the standard comparator, and ensure it compares once again
      DomainComparator standard;
      CPPUNIT_ASSERT(AbstractDomain::canBeCompared(dom0, dom1));

      // Now allocate a new one again
      DomainComparator* dc = new BogusComparator();
      CPPUNIT_ASSERT(!AbstractDomain::canBeCompared(dom0, dom1));

      // Deallocate and thus cause revert to standard comparator
      delete dc;
      CPPUNIT_ASSERT(AbstractDomain::canBeCompared(dom0, dom1));

      // 2 numeric enumerations should be comparable, even if type names differ
      {
    RestrictedDT dt0("NumberDomain0",FloatDT::instance(),IntervalDomain());
	NumericDomain d0(dt0.getId());
    RestrictedDT dt1("NumberDomain1",FloatDT::instance(),IntervalDomain());
	NumericDomain d1(dt1.getId());
	IntervalIntDomain d2;
	CPPUNIT_ASSERT(AbstractDomain::canBeCompared(d0, d1));
	CPPUNIT_ASSERT(AbstractDomain::canBeCompared(d0, d2));
      }

      // 2 non numeric enumerations can only be compared if their base domains intersect
      {
    RestrictedDT dt0("SymbolicType",SymbolDT::instance(),SymbolDomain());
	SymbolDomain d0(dt0.getId());
    RestrictedDT dt1("SymbolicType",StringDT::instance(),StringDomain());
	StringDomain d1(dt1.getId());
    RestrictedDT dt2("OtherDomainType",SymbolDT::instance(),SymbolDomain());
	SymbolDomain d2(dt2.getId());
	CPPUNIT_ASSERT(!AbstractDomain::canBeCompared(d0, d1));
	CPPUNIT_ASSERT(AbstractDomain::canBeCompared(d0, d2));
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
      CPPUNIT_ASSERT(copy != 0);
      CPPUNIT_ASSERT(*copy == boolDom && copy != &boolDom);
      boolDom.set(false);
      CPPUNIT_ASSERT(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      CPPUNIT_ASSERT(copy != 0);
      CPPUNIT_ASSERT(*copy == boolDom && copy != &boolDom);
      boolDom.empty();
      CPPUNIT_ASSERT(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      CPPUNIT_ASSERT(copy != 0);
      CPPUNIT_ASSERT(*copy == boolDom && copy != &boolDom);
      boolDom.relax(BoolDomain(true));
      CPPUNIT_ASSERT(*copy != boolDom);
      delete copy;
      copy = boolDom.copy();
      CPPUNIT_ASSERT(copy != 0);
      CPPUNIT_ASSERT(*copy == boolDom && copy != &boolDom);
      boolDom.remove(true);
      CPPUNIT_ASSERT(*copy != boolDom);
      delete copy;

      IntervalIntDomain iiDom(-2, PLUS_INFINITY);
      copy = iiDom.copy();
      CPPUNIT_ASSERT(copy != 0);
      CPPUNIT_ASSERT(*copy == iiDom && copy != &iiDom);
      iiDom = IntervalIntDomain(-2, PLUS_INFINITY-1);
      CPPUNIT_ASSERT(*copy != iiDom);
      delete copy;

      IntervalDomain iDom(MINUS_INFINITY);
      copy = iDom.copy();
      CPPUNIT_ASSERT(copy != 0);
      CPPUNIT_ASSERT(*copy == iDom && copy != &iDom);
      iDom.empty();
      CPPUNIT_ASSERT(*copy != iDom);
      copy->empty();
      CPPUNIT_ASSERT(*copy == iDom);
      delete copy;

      NumericDomain eDom;
      eDom.insert(2.7);
      copy = eDom.copy();
      CPPUNIT_ASSERT(copy != 0);
      CPPUNIT_ASSERT(*copy == eDom && copy != &eDom);
      eDom.insert(PLUS_INFINITY);
      CPPUNIT_ASSERT(*copy != eDom);
      eDom.remove(PLUS_INFINITY);
      CPPUNIT_ASSERT(*copy == eDom && copy != &eDom);
      delete copy;

      return(true);
    }

    static bool testSymbolicVsNumeric() {

      BoolDomain bDom(false);
      IntervalIntDomain iiDom(-2, PLUS_INFINITY);
      IntervalDomain iDom(MINUS_INFINITY);
      NumericDomain nDom(2.7);
      EnumeratedDomain eDom(StringDT::instance()); // non numeric enum
      eDom.insert(LabelStr("myEnumMember"));
      eDom.set(LabelStr("myEnumMember").getKey());
      EnumeratedDomain enDom(FloatDT::instance()); // numeric enum
      SymbolDomain sDom;
      StringDomain stDom;

			// change for gnats 3242
      CPPUNIT_ASSERT(bDom.isNumeric());
      CPPUNIT_ASSERT(iiDom.isNumeric());
      CPPUNIT_ASSERT(iDom.isNumeric());
      CPPUNIT_ASSERT(nDom.isNumeric());
      CPPUNIT_ASSERT(eDom.isSymbolic());
      CPPUNIT_ASSERT(enDom.isNumeric());
      CPPUNIT_ASSERT(sDom.isSymbolic());
      CPPUNIT_ASSERT(stDom.isSymbolic());

      return(true);
    }

  };
}

bool DomainTests::test() {
  /*runTestSuite(EUROPA::IntervalDomainTest::test);
  runTestSuite(EUROPA::EnumeratedDomainTest::test);
  runTestSuite(EUROPA::MixedTypeTest::test);*/
  EUROPA::IntervalDomainTest::test();
  EUROPA::EnumeratedDomainTest::test();
  EUROPA::MixedTypeTest::test();
  return(true);
}


