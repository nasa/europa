#ifndef _H_PlanDatabaseTestSupport
#define _H_PlanDatabaseTestSupport

#include "PlanDatabaseDefs.hh"
#include "Utils.hh"
#include "Schema.hh"
#include "DefaultPropagator.hh"
#include "PlanDbModuleTests.hh"

using namespace EUROPA;

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngineId ce = (new ConstraintEngine())->getId(); \
    SchemaId schema = SCHEMA; \
    initDbTestSchema(schema); \
    PlanDatabaseId db = (new PlanDatabase(ce, schema))->getId(); \
    { DefaultPropagator* dp = new DefaultPropagator(LabelStr("Default"), ce); \
      assert(dp != 0); } \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce); \
      dbLId = (new DbLogger(std::cout, db))->getId(); \
    } \
    Object* objectPtr = new Object(db, DEFAULT_OBJECT_TYPE(), LabelStr("o1")); \
    assert(objectPtr != 0); \
    Object& object = *objectPtr; \
    assert(objectPtr->getId() == object.getId()); \
    if (autoClose) \
      db->close();\
    {

#define DEFAULT_TEARDOWN() \
    }\
    Entity::purgeStarted();\
    delete (PlanDatabase*) db;\
    delete (ConstraintEngine*) ce;\
    Entity::purgeEnded();

#endif
