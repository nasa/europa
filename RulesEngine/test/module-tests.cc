#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "RulesEngine.hh"
#include "Rule.hh"
#include "RuleContext.hh"
#include "ObjectFilter.hh"
#include "DbLogger.hh"
#include "TestRule.hh"
#include "PartialPlanWriter.hh"

#include "../ConstraintEngine/TestSupport.hh"
#include "../ConstraintEngine/Utils.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "TokenTemporalVariable.hh"
#include "../ConstraintEngine/Domain.hh"
#include "../ConstraintEngine/DefaultPropagator.hh"
#include "../ConstraintEngine/EqualityConstraintPropagator.hh"

#include <iostream>
#include <string>
#include <cassert>

#ifdef __sun
#include <strstream>
typedef std::strstream sstream;
#else
#include <sstream>
typedef std::stringstream sstream;
#endif


class DefaultSchemaAccessor{
public:
  static const SchemaId& instance(){
    if (s_instance.isNoId()){
      s_instance = (new Schema())->getId();
    }

    return s_instance;
  }

  static void reset(){
    if(!s_instance.isNoId()){
      delete (Schema*) s_instance;
      s_instance = SchemaId::noId();
    }
  }

private:
  static SchemaId s_instance;
};

SchemaId DefaultSchemaAccessor::s_instance;

#define SCHEMA DefaultSchemaAccessor::instance()

#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce;\
    Schema schema;\
    PlanDatabase db(ce.getId(), schema.getId());\
    new DefaultPropagator(LabelStr("Default"), ce.getId());\
    if(loggingEnabled()){\
     new CeLogger(std::cout, ce.getId());\
     new DbLogger(std::cout, db.getId());\
     new PlanWriter::PartialPlanWriter(db.getId(), ce.getId());\
    }\
    RulesEngine re(db.getId()); \
    Object& object = *(new Object(db.getId(), LabelStr("AllObjects"), LabelStr("o1")));\
    if(autoClose) db.close();

class RulesEngineTest {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testActivation);
    runTest(testRuleFiringAndCleanup);
    runTest(testObjectFilteringConstraint);
    runTest(testFilterAndConstrain);
    return true;
  }
private:
  static bool testBasicAllocation(){
    DEFAULT_SETUP(ce, db, schema, false);
    TestRule r(LabelStr("Type::Predicate"));
    return true;
  }

  static bool testActivation(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    TestRule r(LabelStr("AllObjects::Predicate"));

    IntervalToken tokenA(db.getId(),
		     LabelStr("AllObjects::Predicate"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    assert(Rule::getRules().size() == 1);
    assert(re.getRuleInstances().empty());

    int num_constraints = ce.getConstraints().size();
    // Activate and confirm the rule instance is created
    tokenA.activate();
    assert(re.getRuleInstances().size() == 1);
    // New constraint added to listen to rule variables
    assert(ce.getConstraints().size() == (unsigned int) (num_constraints + 1));

    // Deactivate to ensure the rule instance is removed
    tokenA.cancel();
    assert(re.getRuleInstances().empty());
    assert(ce.getConstraints().size() == (unsigned int) num_constraints);

    // Activate again to test deletion through automatic cleanup.
    tokenA.activate();
    assert(re.getRuleInstances().size() == 1);
    return true;
  }

  static bool testRuleFiringAndCleanup(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();
    TestRule r(LabelStr("AllObjects::Predicate"));

    IntervalToken tokenA(db.getId(), 
		     LabelStr("AllObjects::Predicate"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    assert(ce.propagate());
    assert(db.getTokens().size() == 1);

    tokenA.activate();
    assert(ce.propagate());
    assert(db.getTokens().size() == 1);

    tokenA.getObject()->specify(timeline.getId());
    assert(ce.propagate());
    // 2 tokens added since fire will trigger twice due to composition
    assert(db.getTokens().size() == 3);
    assert(tokenA.getSlaves().size() == 2);
    TokenId slave = *(tokenA.getSlaves().begin());
    assert(slave->getDuration()->getDerivedDomain().isSingleton()); // Due to constraint on local variable, propagated through interim constraint

    // Test reset which should backtrack the rule
    tokenA.getObject()->reset();
    assert(ce.propagate());
    assert(db.getTokens().size() == 1);

    // Set again, and deactivate
    tokenA.getObject()->specify(timeline.getId());
    assert(ce.propagate());
    assert(db.getTokens().size() == 3);
    tokenA.cancel();
    assert(db.getTokens().size() == 1);

    // Now repeast to ensure correct automatic cleanup
    tokenA.activate();
    assert(ce.propagate());
    assert(db.getTokens().size() == 3); // Rule should fire since specified domain already set!
    return true;
  }
  static bool testObjectFilteringConstraint(){
    PlanDatabase db(ENGINE, SCHEMA);

    static const int NUM_OBJECTS = 10;

    std::list<ObjectId> objects;

    // Allocate objects with 2 field variables
    for (int i=0;i<NUM_OBJECTS;i++){
      //std::stringstream ss;
      sstream ss;
      ss << "Object" << i;
      std::string objectName(ss.str());
      ObjectId object = (new Object(db.getId(), LabelStr("AllObjects"), LabelStr(objectName.c_str()), true))->getId();
      object->addVariable(IntervalIntDomain(i, i));
      object->addVariable(BoolDomain());
      object->close();
      objects.push_back(object);
    }

    // Set up the object variable
    Variable<ObjectSet> objectVar(db.getConstraintEngine(), objects);

    // Set up the filter variable
    EnumeratedDomain filterBaseDomain = ObjectFilter::constructUnion(objects, 0);
    Variable<EnumeratedDomain> filterVar(db.getConstraintEngine(), filterBaseDomain);

    // Construct the filter constraint
    ObjectFilter filter(LabelStr("Default"), db.getConstraintEngine(), objectVar.getId(), 0, filterVar.getId());

    assert(db.getConstraintEngine()->propagate());

    // Iterate and restrict the filter by 1 each time. It should restrict the object variable by 1 also
    EnumeratedDomain workingDomain(filterBaseDomain);
    assert(filterBaseDomain.getSize() == NUM_OBJECTS);
    assert(workingDomain.getSize() == NUM_OBJECTS);
    assert(objectVar.getDerivedDomain().getSize() == NUM_OBJECTS);

    for (int i=1; i< NUM_OBJECTS; i++){
      workingDomain.remove(i);
      filterVar.specify(workingDomain);
      int resultingSize = objectVar.getDerivedDomain().getSize();
      assert(resultingSize == (filterBaseDomain.getSize() - i));
    }

    filterVar.reset();
    assert(objectVar.getDerivedDomain().getSize() == filterBaseDomain.getSize());

    objectVar.specify(objects.front());
    assert(filterVar.getDerivedDomain().isSingleton());
    return true;
  }

  /**
   * This case will emulate the following in nddl
   * class Fact {
   *  int p1;
   *  int p2;
   * }

   */
  static bool testFilterAndConstrain(){
    PlanDatabase db(ENGINE, SCHEMA);

    static const int NUM_OBJECTS = 10;

    std::list<ObjectId> objects;

    // Allocate objects with 2 field variables
    for (int i=0;i<NUM_OBJECTS;i++){
      //std::stringstream ss;
      sstream ss;
      ss << "Object" << i;
      std::string objectName(ss.str());
      ObjectId object = (new Object(db.getId(), LabelStr("AllObjects"), LabelStr(objectName.c_str()), true))->getId();
      object->addVariable(IntervalIntDomain(i, i)); // p1
      object->addVariable(IntervalIntDomain(NUM_OBJECTS - i, NUM_OBJECTS - i)); // p2
      object->close();
      objects.push_back(object);
    }

    // Prepare variables
    Variable<ObjectSet> objectVar(db.getConstraintEngine(), objects);

    EnumeratedDomain filterBaseDomain0 = ObjectFilter::constructUnion(objects, 0);
    Variable<EnumeratedDomain> filterVar0(db.getConstraintEngine(), filterBaseDomain0);
    ObjectFilter filter0(LabelStr("Default"), db.getConstraintEngine(), objectVar.getId(), 0, filterVar0.getId());

    EnumeratedDomain filterBaseDomain1 = ObjectFilter::constructUnion(objects, 1);
    Variable<EnumeratedDomain> filterVar1(db.getConstraintEngine(), filterBaseDomain1);
    ObjectFilter filter1(LabelStr("Default"), db.getConstraintEngine(), objectVar.getId(), 1, filterVar1.getId());

    // OK - now create 2 variables to constrain against, using the equals constraint
    Variable<EnumeratedDomain> v0(db.getConstraintEngine(), filterBaseDomain0);
    EqualConstraint c0(LabelStr("eq"), LabelStr("Default"), db.getConstraintEngine(), makeScope(v0.getId(), filterVar0.getId()));

    Variable<EnumeratedDomain> v1(db.getConstraintEngine(), filterBaseDomain1);
    EqualConstraint c1(LabelStr("eq"), LabelStr("Default"), db.getConstraintEngine(), makeScope(v1.getId(), filterVar1.getId()));

    assert(objectVar.getDerivedDomain().getSize() == NUM_OBJECTS);
    assert(v0.getDerivedDomain() == filterBaseDomain0);
    assert(v1.getDerivedDomain() == filterBaseDomain1);

    // Specify one variable to a singleton
    v0.specify(3);
    assert(v1.getDerivedDomain().isSingleton());
    assert(v1.getDerivedDomain().getSingletonValue() == (NUM_OBJECTS - 3 ));

    return true;
  }
};

int main(){
  initConstraintLibrary();
  
  // Special designations for temporal relations
  REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "precede", "Default");

  // Support for Token implementations
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");

  REGISTER_NARY(EqualConstraint, "eq", "Default");

  // Allocate default schema initially so tests don't fail because of ID's
  SCHEMA;
  runTestSuite(RulesEngineTest::test);
  std::cout << "Finished" << std::endl;
}
