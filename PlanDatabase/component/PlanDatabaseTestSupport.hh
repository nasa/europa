#ifndef _H_PlanDatabaseTestSupport
#define _H_PlanDatabaseTestSupport

#include "Utils.hh"
#include "IntervalIntDomain.hh"
#include "Domain.hh"
#include "DefaultPropagator.hh"

class DefaultSchemaAccessor {
public:

  static const SchemaId& instance() {
    if (s_instance.isNoId())
      s_instance = (new Schema())->getId();
    return(s_instance);
  }

  static void reset() {
    if (!s_instance.isNoId()) {
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
    ConstraintEngineId ce = (new ConstraintEngine())->getId(); \
    SchemaId schema = (new Schema())->getId(); \
    PlanDatabaseId db = (new PlanDatabase(ce, schema))->getId(); \
    { DefaultPropagator* dp = new DefaultPropagator(LabelStr("Default"), ce); \
      assert(dp != 0); } \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce); \
      dbLId = (new DbLogger(std::cout, db))->getId(); \
    } \
    Object* objectPtr = new Object(db, Schema::ALL_OBJECTS(), LabelStr("o1")); \
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
    delete (Schema*) schema;\
    delete (ConstraintEngine*) ce;\
    Entity::purgeEnded();

#endif
