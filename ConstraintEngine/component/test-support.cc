#include "TestSupport.hh"

ConstraintEngineId DefaultEngineAccessor::s_instance;

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
