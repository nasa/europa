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

bool loggingEnabled(){
  static const char* TRUE_VALUE = "1";
  static const char *envStr = getenv("PROTOTYPE_ENABLE_LOGGING");
  static const bool enabled = (envStr != NULL && strcmp(envStr, TRUE_VALUE) == 0);
  return enabled;
}
