#include "ConstraintEngineDefs.hh"
#include "BoolTypeFactory.hh"
#include "IntervalIntTypeFactory.hh"
#include "IntervalTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"

namespace Prototype {

  void initConstraintEngine(){
    static bool sl_alreadyDone(false);
    if(!sl_alreadyDone){
      sl_alreadyDone = true;

      /* Allocate Standard Type Factories */
      new BoolTypeFactory();
      new IntervalIntTypeFactory();
      new IntervalTypeFactory();
      new StringTypeFactory();
      new SymbolTypeFactory();
    }
  }
}
