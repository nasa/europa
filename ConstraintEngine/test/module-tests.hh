#ifndef _H_ModuleTests
#define _H_ModuleTests


class DomainTests {
public:
  static bool test();
};

class IdTests {
public:
  static bool test();

private:
  static bool testBasicAllocation();
  static bool testTypicalConversionsAndComparisons();
  static bool testCollectionSupport();
  static bool testDoubleConversion();
  static bool testCastingSupport();
  static bool testBadAllocationErrorHandling();
  static bool testBadIdUsage();
  static bool testIdConversion();
  static bool testConstId();
};

#endif
