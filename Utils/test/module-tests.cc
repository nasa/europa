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
#include "LockManager.hh"
#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <pthread.h>


#ifndef EUROPA_FAST
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
    else { \
      std::cout << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error err) { \
    err.print(std::cout); \
  } \
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

using namespace EUROPA;

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
    bool success = true;
    Error::doThrowExceptions();
    int var = 1;
    assertTrue(var == 1);
    assertTrue(Error::printingErrors());
    assertTrue(Error::displayWarnings());
    assertTrue(Error::throwEnabled());
    try {
      check_error(Error::printingErrors(), "not printing errors by default!");
      check_error(Error::displayWarnings(), "display warnings off by default!");
      check_error(var == 1);
      check_error(var == 1, "check_error(var == 1)");
      check_error(var == 1, Error("check_error(var == 1)"));
      condWarning(var == 1, "var is not 1");
      warn("everything worked in first try() block of main()");
    } 
    catch (Error e) {
      __x__(e);
      success = false;
    }
    // check_error will not throw the errors for EUROPA_FAST
#if !defined(EUROPA_FAST) && !defined(__CYGWIN__)
    assertTrue(Error::throwEnabled());
    try {
      check_error(var == 2);
      __y__("check_error(var == 2) did not throw an exception");
      success = false;
    } 
    catch (Error e) {
      __z__(e, Error("var == 2", __FILE__, __LINE__ - 5));
    }
    try {
      check_error(var == 2, "check_error(var == 2)");
      __y__("check_error(var == 2, blah) did not throw an exception");
      success = false;
    } 
    catch (Error e) {
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 5));
    }
    try {
      check_error(var == 2, Error("check_error(var == 2)"));
      __y__("check_error(var == 2, Error(blah)) did not throw an exception");
      success = false;
    } 
    catch (Error e) {
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 5));
    }
    try {
      check_error(var == 2, "check_error(var == 2)", TestError::BadThing());
      __y__("check_error(var == 2, TestError::BadThing()) did not throw an exception");
      success = false;
    }
    catch(Error e) {
      assertTrue(e.getType() == "BadThing");
      std::cerr << "Caught expected " << e.getType() << std::endl;
    }
#endif
    return(success);
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
    bool success = true;
    // check_error will not throw the errors for EUROPA_FAST
#if !defined(EUROPA_FAST) && defined(DEBUG_MESSAGE_SUPPORT)
    Error::doThrowExceptions();
    assertTrue(Error::throwEnabled());
    //!!Add a test of DebugMessage that should throw an error here.
    //!!  Skipped for lack of time presently. --wedgingt@email.arc.nasa.gov
    Error::doNotThrowExceptions();
    assertTrue(!Error::throwEnabled());
#endif
    return(success);
  }

  static bool testDebugFiles() {
    for (int i = 1; i < 7; i++)
      runDebugTest(i);
    return(true);
  }

  static void runDebugTest(int cfgNum) {
#if !defined(EUROPA_FAST) && defined(DEBUG_MESSAGE_SUPPORT)
    std::stringstream cfgName;
    cfgName << "debug" << cfgNum << ".cfg";
    std::string cfgFile(cfgName.str());
    cfgName << ".output";
    std::string cfgOut(cfgName.str());

    Error::doNotThrowExceptions();
    Error::doNotDisplayErrors();
    std::ofstream debugOutput(cfgOut.c_str());
    check_error(debugOutput, "could not open debug output file");
    DebugMessage::setStream(debugOutput);
    std::ifstream debugStream(cfgFile.c_str());
    check_error(debugStream, "could not open debug config file",
                DebugErr::DebugConfigError());
    if (!DebugMessage::readConfigFile(debugStream))
      handle_error(!DebugMessage::readConfigFile(debugStream),
                   "problems reading debug config file",
                   DebugErr::DebugConfigError());
    
    debugMsg("main1", "done opening files");
    condDebugMsg(std::cout.good(), "main1a", "std::cout is good");
    debugStmt("main2a", int s = 0; for (int i = 0; i < 5; i++) { s += i; } std::cerr << "Sum is " << s << '\n'; );
    debugMsg("main2", "primary testing done");
    Error::doThrowExceptions();
    Error::doDisplayErrors();
    DebugMessage::setStream(std::cerr);
#endif
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
  Root() {
  }

  virtual ~Root() {
  }
};

class Foo: public Root {
public:
  Foo() {
    counter++;
  }

  virtual ~Foo() {
    counter--;
  }

  void increment() {
    counter++;
  }

  void decrement() {
    counter--;
  }

  bool doConstFunc() const {
    return(true);
  }

  static int getCount() {
    return(counter);
  }

private:
  static int counter;
};

int Foo::counter(0);

class Bar: public Foo {
public:
  Bar() {
  }
};

class Baz: public Foo
{
public:
  Baz(){
  }
};

class Bing: public Root 
{
public:
  Bing(){
  }
};

void overloadFunc(const Id<Bing>& arg) {
  assert(true);
}

void overloadFunc(const Id<Foo>& arg) {
  assert(true);
}

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

bool IdTests::test() {
  LockManager::instance().lock();
  runTest(testBasicAllocation);
  runTest(testCollectionSupport);
  runTest(testDoubleConversion);
  runTest(testCastingSupport);
  runTest(testTypicalConversionsAndComparisons);
  runTest(testBadAllocationErrorHandling);
  runTest(testBadIdUsage);
  runTest(testIdConversion);
  runTest(testConstId);
  LockManager::instance().unlock();
  return(true);
}

bool IdTests::testBasicAllocation() {
#ifndef EUROPA_FAST
  unsigned int initialSize = IdTable::size();
#endif
  Foo *fooPtr = new Foo();
  Id<Foo> fId1(fooPtr);
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
  bool success = true;
  // check_error will not throw the errors for EUROPA_FAST
#ifndef EUROPA_FAST
  // Ensure allocation of a null pointer triggers error
  //LabelStr expectedError = IdErr::IdMgrInvalidItemPtrError();

  Error::doThrowExceptions();
#if !defined(__CYGWIN__)
  // This exception simply isn't being caught on Cygwin for some reason.
  try {
    Id<Foo> fId0((Foo*) 0);
    check_error(false, "Id<Foo> fId0((Foo*) 0); failed to error out.");
    success = false;
  }
  catch(Error e){
    if(e.getType() == "Error")
      assert(false);
  }
  catch(IdErr idErr) {
    std::cerr << "Caught expected IdErr::IdMgrInvalidItemPtrError" << std::endl;
    // No operator==() implemented ...
    // __z__(idErr, IdErr::IdMgrInvalidItemPtrError());
  }
#endif
  Error::doNotThrowExceptions();

  Foo* foo = new Foo();
  Id<Foo> fId1(foo);
  fId1.remove();
  Id<Foo> fId3(foo);
  fId3.release();
#endif

  return(success);
}

bool IdTests::testBadIdUsage()
{
  bool success = true;
  Id<Root> barId(new Bar());
  Error::doThrowExceptions();
#if !defined(__CYGWIN__)
  // This exception isn't being caught on Cygwin.
  try {
    Id<Bing> bingId = barId;
    check_error(false, "Id<Bing> bingId = barId; failed to error out.");
    success = false;
  }
  catch(Error e){
    if(e.getType() == "Error")
      assert(false);
  }
#endif
  Error::doNotThrowExceptions();
  barId.release();
  return(success);
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
    runTest(testComparisons);
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
    assertFalse(LabelStr::isString(1));

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

  static bool testComparisons(){
    LabelStr lbl1("A");
    LabelStr lbl2("G");
    LabelStr lbl3("B");
    LabelStr lbl4("B");
    assert(lbl1 < lbl2);
    assert(lbl2 > lbl4);
    assert(lbl2 != lbl4);
    assert(lbl4 == lbl3);
    return true;
  }
};

class MultithreadTest {
public:
  static bool test() {
    runTest(testConnection);
    return true;
  }
private:
  static bool testConnection() {
    const int numthreads = 100;
    new ThreadedLockManager(); //ensure that we have a threaded model
    LockManager::instance().connect(LabelStr("Test"));
    LockManager::instance().lock();
    assert(LockManager::instance().getCurrentUser() == LabelStr("Test"));
    LockManager::instance().unlock();
    pthread_t threads[numthreads];
    for(int i = 0; i < numthreads; i++)
      pthread_create(&threads[i], NULL, connectionTestThread, NULL);
    for(int i = numthreads - 1; i >= 0; i--)
      pthread_join(threads[i], NULL);
    new LockManager(); //return to your regularly scheduled threading
    LockManager::instance().connect();
    return true;
  }
  
  static void* connectionTestThread(void* arg) {
    const int numconnects = 100;
    bool toggle = false;
 
    for(int i = 0; i < numconnects; i++) {
      if(toggle)
        LockManager::instance().connect(LabelStr("FIRST_USER"));
      else
        LockManager::instance().connect(LabelStr("SECOND_USER"));
      
      LockManager::instance().lock();

      if(toggle) {
        assertTrue(LockManager::instance().getCurrentUser() == LabelStr("FIRST_USER"),
                   "Failed to get expected user.  Instead got " + LockManager::instance().getCurrentUser().toString());
      }
      else {
        assertTrue(LockManager::instance().getCurrentUser() == LabelStr("SECOND_USER"),
                   "Failed to get expected user.  Instead got " + LockManager::instance().getCurrentUser().toString());
      }

      LockManager::instance().unlock();
      LockManager::instance().disconnect();
      toggle = !toggle;
    }
    pthread_exit(0);
    return NULL;
  }
};

int main() {
  LockManager::instance().connect();
  runTestSuite(ErrorTest::test);
  runTestSuite(DebugTest::test);
  runTestSuite(IdTests::test);
  runTestSuite(LabelTests::test);
  runTestSuite(MultithreadTest::test);

  std::cout << "Finished" << std::endl;
  exit(0);
}
