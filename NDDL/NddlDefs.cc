#include "NddlDefs.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "intType.hh"
#include "floatType.hh"

namespace Prototype {

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
      new BoolTypeFactory(LabelStr("bool"));
      new StringTypeFactory(LabelStr("string"));
      new SymbolTypeFactory(LabelStr("symbol"));
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
