#include <iostream>
#include "NddlUtils.hh"
#include "TestSupport.hh"

// Support for default setup
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "Schema.hh"
#include "DefaultPropagator.hh"
#include "Object.hh"
#include "Constraints.hh"
#include "PlanDatabaseTestSupport.hh"

#include "LockManager.hh"

using namespace EUROPA;
using namespace NDDL;

class ObjectFilterConstraintTest {
public:
  static bool test() {
    runTest(testBasicAllocation);
    runTest(testPropagation);
    runTest(testFiltering);
    return true;
  }

private:
  static bool testBasicAllocation() {
    DEFAULT_SETUP(ce, db, false);

    // Allocate an object with fields
    ObjectId object = makeObjectForTesting(db, LabelStr("objectName"), 1, LabelStr("A"), true, 2.1);
    Variable<ObjectDomain> filterVariable(ce, ObjectDomain(object, DEFAULT_OBJECT_TYPE().c_str()));

    // Allocate a number of filter variables, one for each field
    Variable<LabelSet> v1(ce, makeLabelSetDomain());
    Variable<EnumeratedDomain> v3(ce, makeEnumeratedDomain());

    // Create the constraint with filter set up
    std::vector<ObjectFilterCondition*> filter;
    filter.push_back(new ObjectFilterCondition(makeLabelSetDomain(), v1.getId(), 1, ObjectFilterConstraint::eq));
    filter.push_back(new ObjectFilterCondition(makeEnumeratedDomain(), v3.getId(), 3, ObjectFilterConstraint::eq));

    ObjectFilterConstraint c0(LabelStr("ObjectFilter"), 
			      LabelStr("Default"),
			      ce,
			      filterVariable.getId(),
			      ObjectFilterConstraint::CONSTRAIN,
			      filter);

    assertTrue(ce->propagate());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testPropagation() {
    DEFAULT_SETUP(ce, db, false);
    // Allocate a number of objects
    std::list<ObjectId> objects;
    objects.push_back(makeObjectForTesting(db, LabelStr("object0"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object1"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object2"), 1, LabelStr("B"), true, 3.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object3"), 1, LabelStr("B"), true, 4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object4"), 1, LabelStr("A"), false,4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object5"), 2, LabelStr("A"), true, 6.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object6"), 1, LabelStr("B"), false, 7.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object7"), 2, LabelStr("B"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object8"), 1, LabelStr("A"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object9"), 1, LabelStr("A"), true, 0.1));
    db->close();

    Variable<ObjectDomain> filterVariable(ce, ObjectDomain(objects, DEFAULT_OBJECT_TYPE().c_str()));

    // Allocate a number of filter variables, one for each field    
    Variable<LabelSet> v1(ce, makeLabelSetDomain());
    Variable<EnumeratedDomain> v3(ce, makeEnumeratedDomain());

    // Create the constraint with filter set up
    std::vector<ObjectFilterCondition*> filter;
    filter.push_back(new ObjectFilterCondition(makeLabelSetDomain(), v1.getId(), 1, ObjectFilterConstraint::eq));
    filter.push_back(new ObjectFilterCondition(makeEnumeratedDomain(), v3.getId(), 3, ObjectFilterConstraint::eq));

    ObjectFilterConstraint c0(LabelStr("ObjectFilter"), 
			      LabelStr("Default"),
			      ce,
			      filterVariable.getId(),
			      ObjectFilterConstraint::CONSTRAIN,
			      filter);

    assertTrue(ce->propagate());

    // Confirm that the object variable has not yet been restricted
    assertTrue(filterVariable.getDerivedDomain().getSize() == 10);

    // Confirm that the filter variables have been restricted.
    assertTrue(v1.getDerivedDomain().getSize() == 2);
    assertTrue(v1.getDerivedDomain().isMember(LabelStr("A")));
    assertTrue(v1.getDerivedDomain().isMember(LabelStr("B")));
    assertTrue(v3.getDerivedDomain().getSize() == 7);
    assertTrue(!v3.getDerivedDomain().isMember(9.1));

    // Now select for one filter and refine objects
    v1.specify(LabelStr("A"));
    assertTrue(filterVariable.getDerivedDomain().getSize() == 6);
    assertTrue(v3.getDerivedDomain().getSize() == 5); // Also pruned, as objects are removed

    // Now select other and confirm again
    v3.specify(8.1);
    assertTrue(filterVariable.getDerivedDomain().isSingleton());
    assertTrue(v3.getDerivedDomain().isSingleton());
    assertTrue(v1.getDerivedDomain().isSingleton());

    // Reset and confirm repropagation is correct. Non chronologically.
    v1.reset();
    assertTrue(filterVariable.getDerivedDomain().getSize() == 2); // @ objects with 8.1

    v3.reset();
    // Now specify in a different order. Should get the same results
    v3.specify(8.1);
    v1.specify(LabelStr("A"));
    assertTrue(filterVariable.getDerivedDomain().isSingleton());
    assertTrue(v3.getDerivedDomain().isSingleton());
    assertTrue(v1.getDerivedDomain().isSingleton());

    // Propagate and confirm restrictions
    DEFAULT_TEARDOWN();
    return true;
  }


  static bool testFiltering() {
    DEFAULT_SETUP(ce, db, false);
    // Allocate a number of objects
    std::list<ObjectId> objects;
    objects.push_back(makeObjectForTesting(db, LabelStr("object0"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object1"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object2"), 1, LabelStr("B"), true, 3.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object3"), 1, LabelStr("B"), true, 4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object4"), 1, LabelStr("A"), false,4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object5"), 2, LabelStr("A"), true, 6.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object6"), 1, LabelStr("B"), false, 7.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object7"), 2, LabelStr("B"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object8"), 1, LabelStr("A"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object9"), 1, LabelStr("A"), true, 8.1));
    db->close();

    Variable<ObjectDomain> filterVariable(ce, ObjectDomain(objects, DEFAULT_OBJECT_TYPE().c_str()));

    // Allocate a number of filter variables, one for each field    
    Variable<LabelSet> v1(ce, makeLabelSetDomain());
    Variable<EnumeratedDomain> v3(ce, makeEnumeratedDomain());

    // Create the constraint with filter set up
    std::vector<ObjectFilterCondition*> filter;
    filter.push_back(new ObjectFilterCondition(makeLabelSetDomain(), v1.getId(), 1, ObjectFilterConstraint::eq));
    filter.push_back(new ObjectFilterCondition(makeEnumeratedDomain(), v3.getId(), 3, ObjectFilterConstraint::eq));

    ObjectFilterConstraint c0(LabelStr("ObjectFilter"), 
			      LabelStr("Default"),
			      ce,
			      filterVariable.getId(),
			      ObjectFilterConstraint::FILTER,
			      filter);

    v1.specify(LabelStr("A")); // Fixing A only will still leave the filter var empty
    v3.specify(8.1); // Fixing A only will still leave the filter var empty

    assertTrue(ce->propagate());
    assertTrue(c0.getFilteredObjects().getSize() == 2);

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief Helper method to allocate object with fields to support testing
   */
  static ObjectId makeObjectForTesting(const PlanDatabaseId& db, 
				       const LabelStr& name,
				       unsigned int arg0,
				       const LabelStr& arg1,
				       bool arg2,
				       float arg3){
    ObjectId object = (new Object(db, DEFAULT_OBJECT_TYPE(), name, true))->getId();
    object->addVariable(IntervalIntDomain(arg0), "IntervalIntVar");
    object->addVariable(LabelSet(arg1), "LabelSetVar");
    object->addVariable(BoolDomain(arg2), "BoolVar");
    object->addVariable(IntervalDomain(arg3), "IntervalVar");
    object->close();
    return object;
  }

  static const LabelSet makeLabelSetDomain(){
    LabelSet lblSet;
    lblSet.insert(LabelStr("A"));
    lblSet.insert(LabelStr("B"));
    lblSet.insert(LabelStr("C"));
    lblSet.insert(LabelStr("D"));
    lblSet.insert(LabelStr("E"));
    lblSet.insert(LabelStr("F"));
    lblSet.insert(LabelStr("G"));
    lblSet.close();
    return lblSet;
  }

  static const EnumeratedDomain makeEnumeratedDomain(){
    EnumeratedDomain dom(true, EnumeratedDomain::getDefaultTypeName().c_str());
    dom.insert(0.1);
    dom.insert(1.1);
    dom.insert(2.1);
    dom.insert(3.1);
    dom.insert(4.1);
    dom.insert(5.1);
    dom.insert(6.1);
    dom.insert(7.1);
    dom.insert(8.1);
    dom.insert(9.1);
    dom.close();
    return dom;
  }

};

class UtilitiesTest {
public:
  static bool test() {
    std::string tokenizedString("A$B$C$D$");
    const std::list<double>& tokens = listFromString(tokenizedString, false);
    assertTrue(tokens.size() == 4);
    std::string newString;
    for(std::list<double>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      LabelStr value(*it);
      newString += value.toString() + "$";
    }
    assertTrue(newString == tokenizedString);

    std::string numberStr("1$2.45$3.04$-8.9$");
    const std::list<double>& numbers= listFromString(numberStr, true);
    double sum = 0;

    for(std::list<double>::const_iterator it = numbers.begin(); it != numbers.end(); ++it){
      double number = *it;
      sum += number;
    }
    assertTrue(numbers.size() == 4);
    assertTrue(sum == (1 + 2.45 + 3.04 - 8.9));    
    return true;
  }
};


int main() {
  LockManager::instance().connect();
  LockManager::instance().lock();

  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");

  // Pre-allocate a schema
  SCHEMA;

  initNDDL();

  runTestSuite(ObjectFilterConstraintTest::test);
  runTestSuite(UtilitiesTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
