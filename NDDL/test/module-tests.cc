#include <iostream>
#include "NddlUtils.hh"
#include "TestSupport.hh"

using namespace Prototype;
using namespace NDDL;

class NddlSchemaTest {
public:
  static bool test() {
    runTest(testObjectPredicateRelationships);
    return true;
  }

private:

  static bool testObjectPredicateRelationships(){
    NddlSchema schema(Labelstr("TestSchema"));
    schema.addType(LabelStr("Resource"));
    schema.addObjectParent(LabelStr("Resource"), LabelStr("NddlResource"));
    schema.addPredicate(LabelStr("Resource.change"));
    schema.addType(LabelStr("Battery"));
    schema.addObjectParent(LabelStr("Battery"), LabelStr("NddlResource"));
    schema.addObjectParent(LabelStr("Battery"), LabelStr("Resource"));
    schema.addObjectPredicate(LabelStr("Resource"), LabelStr("Resource.change"));
    schema.addType(LabelStr("World"));
    schema.addPredicate(LabelStr("World.initialState"));
    schema.addObjectPredicate(LabelStr("World"), LabelStr("World.initialState"));

    assert(schema.isPredicateDefined(LabelStr("Resource.change")));
    assert(schema.isPredicateDefined(LabelStr("World.initialState")));
    assert(!schema.isPredicateDefined(LabelStr("NOPREDICATE")));
    assert(schema.isTypeDefined(LabelStr("Resource")));
    assert(schema.isTypeDefined(LabelStr("World")));
    assert(schema.isTypeDefined(LabelStr("Battery")));
    assert(!schema.isTypeDefined(LabelStr("NOTYPE")));

    assert(schema.canBeAssigned(LabelStr("World"), LabelStr("World.initialState")));
    assert(schema.canBeAssigned(LabelStr("Resource"), LabelStr("Resource.change")));
    assert(schema.canBeAssigned(LabelStr("Battery"), LabelStr("Resource.change")));

    assert(!schema.isA(LabelStr("Resource"), LabelStr("Battery")));
    assert(schema.isA(LabelStr("Battery"), LabelStr("Resource")));
    assert(schema.isA(LabelStr("Battery"), LabelStr("Battery")));

    return true;
  }
};

int main() {
  runTestSuite(NddlSchemaTest::test);
  std::cout << "Finished" << std::endl;
}
