#include "TestSupport.hh"

ConstraintEngineId DefaultEngineAccessor::s_instance;



bool loggingEnabled() {
  static const char *envStr = getenv("PROTOTYPE_ENABLE_LOGGING");
  static const bool enabled = (envStr != NULL && atoi(envStr) != 0);
  return(enabled);
}
