#ifndef _H_TestSupport
#define _H_TestSupport


#include "ConstraintEngine.hh"
#include "DefaultPropagator.hh"
#include "ConstraintFactory.hh"
#include "Constraints.hh"
#include "Error.hh"
#include "Utils.hh"
#include "Debug.hh"


using namespace EUROPA;

class DefaultEngineAccessor {
public:
  static const ConstraintEngineId& instance() {
    if (s_instance.isNoId()) {
        CESchema* ces = new CESchema();
      s_instance = (new ConstraintEngine(ces->getId()))->getId();
      new DefaultPropagator(LabelStr("Default"), s_instance);
      new DefaultPropagator(LabelStr("Temporal"), s_instance);
    }
    return s_instance;
  }

  static void reset() {
    if (!s_instance.isNoId()) {
        const CESchemaId& tfm = s_instance->getCESchema();
      delete (ConstraintEngine*) s_instance;
      delete (CESchema*) tfm;
      s_instance = ConstraintEngineId::noId();
     }
  }

private:
  static ConstraintEngineId s_instance;
};

#define ENGINE DefaultEngineAccessor::instance()

#define EUROPA_runTest(test, args...) {			\
  try { \
  unsigned int id_count = IdTable::size(); \
  bool result = test(args); \
  DefaultEngineAccessor::reset(); \
  Entity::garbageCollect(); \
  if (result && IdTable::size() <= id_count) { \
    debugMsg("Test"," PASSED."); \
  } \
  else \
    if (result) { \
      std::cerr << " FAILED = DID NOT CLEAN UP ALLOCATED IDs:\n"; \
      IdTable::output(std::cerr); \
      std::cerr << "\tWere " << id_count << " IDs before; " << IdTable::size() << " now"; \
      std::cerr << std::endl; \
      throw Error::GeneralMemoryError(); \
    } else { \
      std::cerr << "      " << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error err){ \
   err.print(std::cout); \
  }\
  }

#define runTestSuite(test) { \
  try{ \
      debugMsg("Test", #test << "******************************"); \
      if (test()) { \
          debugMsg("Test", #test << " PASSED."); \
      } \
      else {\
          debugMsg("Test", #test << " FAILED."); \
      } \
  }\
  catch (Error err) {\
   err.print(std::cerr);\
  }\
}

#endif


