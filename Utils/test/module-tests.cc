//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

/**
   @file module-tests.cc
   @author Will Edgington

   @brief A small test of classes Error and TestData and the related
   macros.
   
   CMG: Added test for id's
*/

#include "Error.hh"
#include "Debug.hh"
#include "LabelStr.hh"
#include "TestData.hh"
#include "Id.hh"
#include <sstream>
#include <iostream>
#include <fstream>


#ifndef PLASMA_FAST
#define non_fast_only_assert(T) assert(T)
#else
#define non_fast_only_assert(T) //NO-OP
#endif

#define runTest(test) { \
  try { \
  std::cout << "      " << #test; \
  bool result = test(); \
  if (result) \
    std::cout << " PASSED." << std::endl; \
  else \
    if (result) { \
      std::cout << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error err){ \
   err.print(std::cout); \
  }\
  }

#define runTestSuite(test) { \
  try{ \
  std::cout << #test << "***************" << std::endl; \
  if (test()) \
    std::cout << #test << " PASSED." << std::endl; \
  else \
    std::cout << #test << " FAILED." << std::endl; \
  }\
  catch (Error err){\
   err.print(std::cout);\
  }\
  }

using namespace Prototype;

class TestError {
public:
  DECLARE_ERROR(BadThing);
};

class ErrorTest {
public:
  static bool test() {
    runTest(testExceptions);
    return true;
  }
private:
  static bool testExceptions() {
    Error::doThrowExceptions();
    int argc = 1;
    try {
      check_error(Error::printingErrors());
      check_error(Error::getStream() == std::cerr);
      check_error(Error::displayWarnings());
      check_error(argc == 1);
      check_error(argc == 1, "check_error(argc == 1)");
      check_error(argc == 1, Error("check_error(argc == 1)"));
      condWarning(argc == 1, "argc is not 1");
      warn("everything worked in first try() block of main()");
    } 
    catch (Error e) {
      __x__(e);
    }
    // check_error will not throw the errors for PLASMA_FAST
#ifndef PLASMA_FAST
    try {
      check_error(argc == 2);
      __y__("check_error(argc == 2) did not throw an exception");
    } 
    catch (Error e) {
      __z__(e, Error("argc == 2", __FILE__, __LINE__ - 4));
    }
    try {
      check_error(argc == 2, "check_error(argc == 2)");
      __y__("check_error(argc == 2, blah) did not throw an exception");
    } 
    catch (Error e) {
      __z__(e, Error("argc == 2", "check_error(argc == 2)", __FILE__, __LINE__ - 4));
    }
    try {
      check_error(argc == 2, Error("check_error(argc == 2)"));
      __y__("check_error(argc == 2, Error(blah)) did not throw an exception");
    } 
    catch (Error e) {
      __z__(e, Error("argc == 2", "check_error(argc == 2)", __FILE__, __LINE__ - 4));
    }
    try {
      check_error(argc == 2, "check_error(argc == 2)", TestError::BadThing());
      __y__("check_error(argc == 2, TestError::BadThing()) did not throw an exception");
    }
    catch(Error e) {
      assert(e.getType() == "BadThing");
      std::cerr << "Caught expected " << e.getType() << std::endl;
    }
#endif
    return true;
  }
};

class DebugTest {
public:
  static bool test() {
    runTest(testDebugError);
    runTest(testDebugFiles);
    return true;
  }
private:

  static bool testDebugError() {
    // check_error will not throw the errors for PLASMA_FAST
#ifndef PLASMA_FAST
    try {
      DebugMessage::enableAll();
      __y__("enabling all debug messages succeeded despite no debug stream");
    }
    catch(Error e) {
      __z__(e, Error("s_os != 0", "no debug stream has been assigned",
                     "../../Utils/core/Debug.cc", 91));
    }
#endif
    return true;
  }

  static bool testDebugFiles() {
    for(int i = 1; i < 7; i++)
      runDebugTest(i);
    return true;
  }

  static void runDebugTest(int cfgNum) {
    std::stringstream cfgName;
    cfgName << "debug" << cfgNum << ".cfg";
    std::string cfgFile(cfgName.str());
    cfgName << ".output";
    std::string cfgOut(cfgName.str());

    Error::doNotThrowExceptions();
    Error::doNotDisplayErrors();
    std::ifstream debugStream(cfgFile.c_str());
    check_error(debugStream, "could not open debug config file",
                DebugErr::DebugConfigError());
    if(!DebugMessage::readConfigFile(debugStream))
      handle_error(!DebugMessage::readConfigFile(debugStream),
                   "problems reading debug config file",
                   DebugErr::DebugConfigError());
    std::ofstream debugOutput(cfgOut.c_str());
    check_error(debugOutput, "could not open debug output file");
    DebugMessage::setStream(debugOutput);
    
    debugMsg("main1", "done opening files");
    condDebugMsg(std::cout.good(), "main1a", "std::cout is good");
    debugStmt("main2a", int s = 0; for (int i = 0; i < 5; i++) { s += i; } std::cerr << "Sum is " << s << '\n'; );
    debugMsg("main2", "primary testing done");
    Error::doThrowExceptions();
    Error::doDisplayErrors();
    DebugMessage::setStream(std::cerr);
  }
};
/**
 * Support classes to enable testing
 * Foo: Basic allocation and deallocation.
 * Bar: Casting behaviour
 * EmbeddedClass: Error handling and checking for release and alloction patterns with embedded id's.
 */
class Root {
public:
  Root(){}
  virtual ~Root(){}
};

class Foo: public Root
{
public:
  Foo() {counter++;}

  virtual ~Foo()
  {
    counter--;
  }

  void increment(){counter++;}
  void decrement(){counter--;}

  bool doConstFunc() const {return true;}

  static int getCount() {return counter;}
private:
  static int counter;
};

class Bar: public Foo
{
public:
  Bar(){}
};

class Baz: public Foo
{
public:
  Baz(){}
};

class Bing: public Root 
{
public:
  Bing(){}
};

int Foo::counter(0);

void overloadFunc(const Id<Bing>& arg){assert(true);}
void overloadFunc(const Id<Foo>& arg){assert(true);}

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

bool IdTests::test(){
  runTest(testBasicAllocation);
  runTest(testCollectionSupport);
  runTest(testDoubleConversion);
  runTest(testCastingSupport);
  runTest(testTypicalConversionsAndComparisons);
  runTest(testBadAllocationErrorHandling);
  runTest(testBadIdUsage);
  runTest(testIdConversion);
  runTest(testConstId);
  return true;
}

bool IdTests::testBasicAllocation()
{
  int initialSize = IdTable::size();
  Id<Foo> fId1(new Foo());
  assert(Foo::getCount() == 1);
  non_fast_only_assert(IdTable::size() == initialSize + 1);

  fId1->increment();
  assert(Foo::getCount() == 2);
  fId1->decrement();
  assert(Foo::getCount() == 1);

  Id<Foo> fId2 = fId1;
  assert(Foo::getCount() == 1);

  assert(fId1.isValid() && fId2.isValid());
  assert(!fId1.isInvalid() && !fId2.isInvalid());

  fId2.release();
  assert(Foo::getCount() == 0);
  non_fast_only_assert( fId1.isInvalid() &&  fId2.isInvalid());

  return true;
}


bool IdTests::testTypicalConversionsAndComparisons()
{
  Foo* foo1 = new Foo();
  Id<Foo> fId1(foo1);
  Id<Foo> fId2(fId1);
  assert(fId1 == fId2); // Equality operator
  assert(&*fId1 == &*fId2); // Dereferencing operator
  assert(foo1 == &*fId2); // Dereferencing operator
  assert(foo1 == (Foo*) fId2); // Dereferencing operator
  assert(foo1 == fId2.operator->());
  assert( ! (fId1 > fId2));
  assert( ! (fId1 < fId2));

  Foo* foo2 = new Foo();
  Id<Foo> fId3(foo2);
  assert(fId1 != fId3);

  fId1.release();
  fId3.release();
  return true;
}

bool IdTests::testCollectionSupport()
{
  // Test inclusion in a collection class - forces compilation test
  std::list< Id<Foo> > fooList;
  assert(fooList.size() == 0);
  return true;
}

bool IdTests::testDoubleConversion()
{
  Id<Foo> fId(new Foo());
  double fooAsDouble = (double) fId;
  Id<Foo> idFromDbl(fooAsDouble);
  assert(idFromDbl == fId);
  fId.release();
  return true;
}

bool IdTests::testCastingSupport()
{
  Foo* foo = new Foo();
  Id<Foo> fId(foo);
  Foo* fooByCast = (Foo*) fId;
  assert(foo == fooByCast);

  assert(Id<Bar>::convertable(fId) == false);
  fId.release();

  Foo* bar = new Bar();
  Id<Bar> bId((Bar*) bar);
  fId = bId;
  assert(Id<Bar>::convertable(fId) == true);
  bId.release();

  bId = Id<Bar>(new Bar());
  double ptrAsDouble = bId; // Cast to double

  const Id<Bar>& cbId(ptrAsDouble);
  assert(cbId.isValid());
  assert(cbId == bId);
  bId.release();
  non_fast_only_assert(cbId.isInvalid());

  Id<Baz> fId1(new Baz());
  // DOES NOT COMPILE: overloadFunc(fId1);
  fId1.release();
  return true;
}

bool IdTests::testBadAllocationErrorHandling()
{
  // check_error will not throw the errors for PLASMA_FAST
#ifndef PLASMA_FAST
  // Ensure allocation of a null pointer triggers error
  //LabelStr expectedError = IdErr::IdMgrInvalidItemPtrError();

  Error::doThrowExceptions();
  try {
    Id<Foo> fId0((Foo*) 0);
    check_error(false, "Id<Foo> fId0((Foo*) 0); failed to error out.");
  }
  catch(Error e){
    if(e.getType() == "Error")
      assert(false);
  }
  Error::doNotThrowExceptions();

  Foo* foo = new Foo();
  Id<Foo> fId1(foo);
  fId1.remove();
  Id<Foo> fId3(foo);
  fId3.release();
#endif

  return true;
}

bool IdTests::testBadIdUsage()
{
  Id<Root> barId(new Bar());
  Error::doThrowExceptions();
  try {
    Id<Bing> bingId = barId;
    check_error(false, "Id<Bing> bingId = barId; failed to error out.");
  }
  catch(Error e){
    if(e.getType() == "Error")
      assert(false);
  }
  Error::doNotThrowExceptions();
  barId.release();
  return true;
}

bool IdTests::testIdConversion()
{
  int count = Foo::getCount();
  Id<Foo> fooId(new Bar());
  Id<Bar> barId(fooId);
  barId.release();
  Id<Foo> fooId3(new Bar());
  Id<Bar> barId3;
  barId3 = fooId3;
  barId3.release();
  assert(Foo::getCount() == count);
  return true;
}

bool IdTests::testConstId()
{
  Id<Foo> fooId(new Foo());
  const Id<const Foo> constFooId(fooId);
  assert(constFooId->doConstFunc());
  fooId->increment();
  fooId.remove();
  return true;
}

class LabelTests {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testElementCounting);
    runTest(testElementAccess);
    return true;
  }

private:
  static bool compare(const LabelStr& str1, const LabelStr& str2){
    return str1 == str2;
  }

  static bool testBasicAllocation(){
    LabelStr lbl1("");
    LabelStr lbl2("This is a char*");
    LabelStr lbl3(lbl2.toString());
    assertTrue(lbl3 == lbl2);
    std::string labelStr2("This is another char*");
    LabelStr lbl4(labelStr2);
    assertTrue(lbl4 != lbl2);

    double key = lbl2.getKey();
    LabelStr lbl5(key);
    assertTrue(lbl5 == lbl2);
    assertTrue(LabelStr::isString(key));
    assertFalse(LabelStr::isString(ASSUMED_MINIMUM_MEMORY_ADDRESS + 1));

    assertTrue(compare(lbl3, lbl2));
    assertTrue(compare("This is another char*", "This is another char*"));
    return true;
  }

  static bool testElementCounting(){
    LabelStr lbl1("A 1B 1C 1D EFGH");
    assert(lbl1.countElements("1") == 4);
    assert(lbl1.countElements(" ") == 5);
    assert(lbl1.countElements("B") == 2);
    assert(lbl1.countElements(":") == 1);

    LabelStr lbl2("A:B:C:D:");
    assert(lbl2.countElements(":") == 4);
    return true;
  }

  static bool testElementAccess(){
    LabelStr lbl1("A 1B 1C 1D EFGH");
    LabelStr first(lbl1.getElement(0, " "));
    assert(first == LabelStr("A"));

    LabelStr last(lbl1.getElement(3, "1"));
    assert(last == LabelStr("D EFGH"));
    return true;
  }
};

int main() {
  runTestSuite(ErrorTest::test);
  runTestSuite(DebugTest::test);
  runTestSuite(IdTests::test);
  runTestSuite(LabelTests::test);

  std::cout << "Finished" << std::endl;
}
