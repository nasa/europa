#include "ConstraintEngineDefs.hh"
#include "BoolTypeFactory.hh"
#include "IntervalIntTypeFactory.hh"
#include "IntervalTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"

namespace EUROPA {

  static bool & constraintEngineInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  void initConstraintEngine(){
    if(!constraintEngineInitialized()){
      constraintEngineInitialized() = true;

      /* Allocate Standard Type Factories */
      new BoolTypeFactory();
      new IntervalIntTypeFactory();
      new IntervalTypeFactory();
      new StringTypeFactory();
      new SymbolTypeFactory();
    }
  }

  void uninitConstraintEngine(){
    if(constraintEngineInitialized()){
      TypeFactory::purgeAll();
      constraintEngineInitialized() = false;
    }
  }

}
