#include "NddlDefs.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "intType.hh"
#include "floatType.hh"

namespace Prototype {

  void initNDDL(){
    static bool sl_alreadyDone(false);
    if(!sl_alreadyDone){
      sl_alreadyDone = true;

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
}
