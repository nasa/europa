#ifndef _H_PlanDatabaseTestSupport
#define _H_PlanDatabaseTestSupport

#include "PlanDatabaseDefs.hh"
#include "Utils.hh"
#include "Schema.hh"
#include "DefaultPropagator.hh"
//#include "PlanDbModuleTests.hh"

using namespace EUROPA;

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngineId ce = (new ConstraintEngine())->getId(); \
    SchemaId schema = SCHEMA; \
    initDbTestSchema(schema); \
    PlanDatabaseId db = (new PlanDatabase(ce, schema))->getId(); \
    { DefaultPropagator* dp = new DefaultPropagator(LabelStr("Default"), ce); \
      assert(dp != 0); } \
    Object* objectPtr = new Object(db, DEFAULT_OBJECT_TYPE(), LabelStr("o1")); \
    assert(objectPtr != 0); \
    Object& object = *objectPtr; \
    assert(objectPtr->getId() == object.getId()); \
    if (autoClose) \
      db->close();\
    {

#define DEFAULT_TEARDOWN() \
    } \
    Entity::purgeStarted(); \
    delete (PlanDatabase*) db; \
    delete (ConstraintEngine*) ce; \
    Entity::purgeEnded();

#define DEFAULT_TEARDOWN_MULTI(ce, db) \
    }\
    Entity::purgeStarted();\
    delete (PlanDatabase*) db;\
    delete (ConstraintEngine*) ce;\
    Entity::purgeEnded();

const LabelStr& DEFAULT_OBJECT_TYPE(){
  static const LabelStr sl_local("DEFAULT_OBJECT_TYPE");
  return sl_local;
}

const LabelStr& DEFAULT_PREDICATE(){
  static const LabelStr sl_local("DEFAULT_OBJECT_TYPE.DEFAULT_PREDICATE");
  return sl_local;
}

#define SCHEMA Schema::instance()


  void initDbTestSchema(const SchemaId& schema) {
    schema->reset();
    // Set up object types and compositions for testing - builds a recursive structure
    schema->addObjectType(DEFAULT_OBJECT_TYPE());
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id0");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id1");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id2");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id3");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id4");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id5");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id6");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id7");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id8");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id9");

    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o0");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o1");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o2");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o3");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o4");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o5");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o6");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o7");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o8");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o9");

    // Set up primitive object type member variables for testing
    schema->addMember(DEFAULT_OBJECT_TYPE(), IntervalDomain::getDefaultTypeName(), "IntervalVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), IntervalIntDomain::getDefaultTypeName(), "IntervalIntVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), BoolDomain::getDefaultTypeName(), "BoolVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), LabelSet::getDefaultTypeName(), "LabelSetVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), EnumeratedDomain::getDefaultTypeName(), "EnumeratedVar");

    // Set up predicates for testing
    schema->addPredicate(DEFAULT_PREDICATE());
    schema->addMember(DEFAULT_PREDICATE(), IntervalDomain::getDefaultTypeName(), "IntervalParam");
    schema->addMember(DEFAULT_PREDICATE(), IntervalIntDomain::getDefaultTypeName(), "IntervalIntParam");
    schema->addMember(DEFAULT_PREDICATE(), BoolDomain::getDefaultTypeName(), "BoolParam");
    schema->addMember(DEFAULT_PREDICATE(), LabelSet::getDefaultTypeName(), "LabelSetParam");
    schema->addMember(DEFAULT_PREDICATE(), EnumeratedDomain::getDefaultTypeName(), "EnumeratedParam");
  }

#endif
