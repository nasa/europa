#ifndef _H_TestSupport
#define _H_TestSupport


#include "ConstraintEngine.hh"
#include "DefaultPropagator.hh"
#include "ConstraintLibrary.hh"
#include "Constraints.hh"

using namespace Prototype;

class DefaultEngineAccessor{
public:
  static const ConstraintEngineId& instance(){
    if (s_instance.isNoId()){
      s_instance = (new ConstraintEngine())->getId();
      new DefaultPropagator(LabelStr("Default"), s_instance);
    }

    return s_instance;
  }

  static void reset(){
    if(!s_instance.isNoId()){
      delete (ConstraintEngine*) s_instance;
      s_instance = ConstraintEngineId::noId();
    }
  }

private:
  static ConstraintEngineId s_instance;
};

ConstraintEngineId DefaultEngineAccessor::s_instance;

#define ENGINE DefaultEngineAccessor::instance()

#define runTest(test, name) { \
  std::cout << "      " << name; \
  int id_count = Europa::IdTable::size();\
  bool result = test(); \
  DefaultEngineAccessor::reset(); \
  if(result && Europa::IdTable::size() == id_count) \
    std::cout << " passed." << std::endl; \
  else \
    std::cout << " FAILED." << std::endl; \
}

#define runTestSuite(test, name) { \
  std::cout << name << "***************" << std::endl; \
  if(test()) \
    std::cout << name << " passed." << std::endl; \
  else \
    std::cout << name << " FAILED." << std::endl; \
}

void initConstraintLibrary(){
  static bool s_runAlready(false);

  if(!s_runAlready){
    // Register constraint Factories
    REGISTER_UNARY(SubsetOfConstraint, "SubsetOf", "Default");
    REGISTER_NARY(EqualConstraint, "Equal", "Default");
    REGISTER_NARY(AddEqualConstraint, "AddEqual", "Default");\
    s_runAlready = true;
  }
}

#endif
