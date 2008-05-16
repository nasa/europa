#ifndef _H_PlanDatabaseTestSupport
#define _H_PlanDatabaseTestSupport

#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Utils.hh"
#include "DefaultPropagator.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "StringDomain.hh"
#include "BoolDomain.hh"
#include "Object.hh"

using namespace EUROPA;

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngineId ce = (new ConstraintEngine())->getId(); \
    SchemaId schema = SCHEMA; \
    initDbTestSchema(schema); \
    PlanDatabaseId db = (new PlanDatabase(ce, schema))->getId(); \
    { new DefaultPropagator(LabelStr("Default"), ce);\
      new DefaultPropagator(LabelStr("Temporal"), ce);\
    } \
    Object* objectPtr = new Object(db, LabelStr(DEFAULT_OBJECT_TYPE), LabelStr("o1")); \
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

const char* DEFAULT_OBJECT_TYPE = "DEFAULT_OBJECT_TYPE";
const char* DEFAULT_PREDICATE = "DEFAULT_OBJECT_TYPE.DEFAULT_PREDICATE";

#define SCHEMA Schema::testInstance()


void initDbTestSchema(const SchemaId& schema) {
  schema->reset();

  schema->addPrimitive(IntervalDomain::getDefaultTypeName());
  schema->addPrimitive(IntervalIntDomain::getDefaultTypeName());
  schema->addPrimitive(EnumeratedDomain::getDefaultTypeName());

  // Set up object types and compositions for testing - builds a recursive structure
  schema->addObjectType(LabelStr(DEFAULT_OBJECT_TYPE));
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id0");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id1");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id2");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id3");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id4");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id5");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id6");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id7");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id8");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "id9");

  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o0");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o1");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o2");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o3");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o4");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o5");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o6");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o7");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o8");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelStr(DEFAULT_OBJECT_TYPE), "o9");

  // Set up primitive object type member variables for testing
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), IntervalDomain::getDefaultTypeName(), "IntervalVar");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), IntervalIntDomain::getDefaultTypeName(), "IntervalIntVar");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), BoolDomain::getDefaultTypeName(), "BoolVar");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), LabelSet::getDefaultTypeName(), "LabelSetVar");
  schema->addMember(LabelStr(DEFAULT_OBJECT_TYPE), EnumeratedDomain::getDefaultTypeName(), "EnumeratedVar");

  // Set up predicates for testing
  schema->addPredicate(LabelStr(DEFAULT_PREDICATE));
  schema->addMember(LabelStr(DEFAULT_PREDICATE), IntervalDomain::getDefaultTypeName(), "IntervalParam");
  schema->addMember(LabelStr(DEFAULT_PREDICATE), IntervalIntDomain::getDefaultTypeName(), "IntervalIntParam");
  schema->addMember(LabelStr(DEFAULT_PREDICATE), BoolDomain::getDefaultTypeName(), "BoolParam");
  schema->addMember(LabelStr(DEFAULT_PREDICATE), LabelSet::getDefaultTypeName(), "LabelSetParam");
  schema->addMember(LabelStr(DEFAULT_PREDICATE), EnumeratedDomain::getDefaultTypeName(), "EnumeratedParam");
}


#endif
