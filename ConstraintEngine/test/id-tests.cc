#include "Id.hh"
#include "CommonDefs.hh"
#include "IdTable.hh"
#include "TestSupport.hh"
#include "module-tests.hh"
#include <list>

using namespace Prototype;

#ifndef PROTOTYPE_FAST_VERSION
#define non_fast_only_assert(T) assert(T)
#else
#define non_fast_only_assert(T) //NO-OP
#endif

/**
 * Support classes to enable testing
 * Foo: Basic allocation and deallocation.
 * Bar: Casting behaviour
 * EmbeddedClass: Error handling and checking for release and alloction patterns with embedded id's.
 */
class Foo
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

int Foo::counter(0);


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
  assert(cbId.isInvalid());
  return true;
}

bool IdTests::testBadAllocationErrorHandling()
{
#ifndef PROTOTYPE_FAST_VERSION

  // Ensure allocation of a null pointer triggers error
  expectedError = idMgrInvalidItemPtrError;

  Id<Foo> fId0((Foo*) 0);
  assert(fId0.isInvalid());

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
  return true;
}

bool IdTests::testIdConversion()
{
  Id<Foo> fooId(new Bar());
  Id<Bar> barId(fooId);
  barId.release();
  Id<Foo> fooId3(new Bar());
  Id<Bar> barId3;
  barId3 = fooId3;
  barId3.release();
  assert(Foo::getCount() == 0);
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

