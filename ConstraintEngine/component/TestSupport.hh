#ifndef _H_TestSupport
#define _H_TestSupport


#include "ConstraintEngine.hh"
#include "DefaultPropagator.hh"
#include "ConstraintLibrary.hh"
#include "Constraints.hh"
#include "Error.hh"
#include "Utils.hh"

using namespace EUROPA;

class DefaultEngineAccessor {
public:
  static const ConstraintEngineId& instance() {
    if (s_instance.isNoId()) {
      s_instance = (new ConstraintEngine())->getId();
      new DefaultPropagator(LabelStr("Default"), s_instance);
      new DefaultPropagator(LabelStr("Temporal"), s_instance);
    }
    return s_instance;
  }

  static void reset() {
    if (!s_instance.isNoId()) {
      delete (ConstraintEngine*) s_instance;
      s_instance = ConstraintEngineId::noId();
    }
  }

private:
  static ConstraintEngineId s_instance;
};

#define ENGINE DefaultEngineAccessor::instance()

#define runTest(test) { \
  try { \
  std::cout << "   " << #test << " "; \
  unsigned int id_count = IdTable::size(); \
  bool result = test(); \
  DefaultEngineAccessor::reset(); \
  if (result && IdTable::size() <= id_count) \
    std::cout << " PASSED." << std::endl; \
  else \
    if (result) { \
      std::cout << " FAILED = DID NOT CLEAN UP ALLOCATED IDs:\n"; \
      IdTable::output(std::cout); \
      std::cout << "\tWere " << id_count << " IDs before; " << IdTable::size() << " now"; \
      std::cout << std::endl; \
      throw Error::GeneralMemoryError(); \
    } else { \
      std::cout << "      " << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error err){ \
   err.print(std::cout); \
  }\
  }

#define runTestSuite(test) { \
  try{ \
  std::cout << #test << "******************************" << std::endl;\
  if (test()) \
    std::cout << #test << " PASSED." << std::endl; \
  else \
    std::cout << #test << " FAILED." << std::endl; \
  }\
  catch (Error err){\
   err.print(std::cout);\
  }\
  }

#endif
