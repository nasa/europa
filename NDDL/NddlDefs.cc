#include "NddlDefs.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "intType.hh"
#include "floatType.hh"

namespace EUROPA {

  static bool & nddlInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  void initNDDL(){
    if(!nddlInitialized()){
      nddlInitialized() = true;

      /* Initialize dependendent module */
      initConstraintEngine();

      /* Allocate NDDL Type Factories */
      new BoolTypeFactory("bool");
      new StringTypeFactory("string");
      new SymbolTypeFactory("symbol");
      new intTypeFactory();
      new floatTypeFactory();
    }
  }

  void uninitNDDL(){
    if(nddlInitialized()){
      uninitConstraintEngine();
      TypeFactory::purgeAll();
      nddlInitialized() = false;
    }
  }

}
