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
      new DefaultPropagator(s_instance);
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
  cout << "      " << name; \
  int id_count = Europa::IdTable::size();\
  bool result = test(); \
  DefaultEngineAccessor::reset(); \
  if(result && Europa::IdTable::size() == id_count) \
    cout << " passed." << endl; \
  else \
    cout << " FAILED." << endl; \
}

#define runTestSuite(test, name) { \
  cout << name << "***************" << endl; \
  if(test()) \
    cout << name << " passed." << endl; \
  else \
    cout << name << " FAILED." << endl; \
}

// Register constraint Factories
REGISTER_UNARY(SubsetOfConstraint, "SubsetOf");
REGISTER_NARY(EqualConstraint, "Equal");
REGISTER_NARY(AddEqualConstraint, "AddEqual");

#endif
