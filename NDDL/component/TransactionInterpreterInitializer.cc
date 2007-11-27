
#include "TransactionInterpreterInitializer.hh"
#include "TransactionInterpreter.hh"

namespace EUROPA {

  void TransactionInterpreterInitializer::initialize()
  {
	  InterpretedDbClientTransactionPlayer::createDefaultObjectFactory("Object", true);
      REGISTER_OBJECT_FACTORY(TimelineObjectFactory, Timeline);	    	    	  	
  }

  void TransactionInterpreterInitializer::uninitialize()
  {
	  // TODO: Implement this correctly
  }

}

