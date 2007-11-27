
#include "TransactionInterpreterInitializer.hh"
#include "TransactionInterpreter.hh"
#include "TransactionInterpreterResources.hh"
#include "Schema.hh"

namespace EUROPA {

  void TransactionInterpreterInitializer::initialize()
  {
	  InterpretedDbClientTransactionPlayer::createDefaultObjectFactory("Object", true);
	  REGISTER_OBJECT_FACTORY(TimelineObjectFactory, Timeline);	    	    	  	

      Id<Schema> schema = Schema::instance();
	  schema->declareObjectType("Resource");
	  schema->declareObjectType("Reusable");
	  schema->declareObjectType("Reservoir");

	  REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource);	    	    	  
	  REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float);	    	    	  
	  REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float:float:float);	    	    	  
	  REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float:float:float:float:float);	    	    	  
	  new ResourceChangeTokenFactory("Resource.change");

	  REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable);	    	    	  
	  REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float);	    	    	  
	  REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float:float);	    	    	  
	  REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float:float:float);	    	    	  
	  new ReusableUsesTokenFactory("Reusable.uses");

	  REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir);	    	    	  
	  REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float);	    	    	  
	  REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float:float:float);	    	    	  
	  REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float:float:float:float:float);	    	    	  
	  new ReservoirProduceTokenFactory("Reservoir.produce");      
	  new ReservoirConsumeTokenFactory("Reservoir.consume");      
  }

  void TransactionInterpreterInitializer::uninitialize()
  {
	  // TODO: Implement this correctly
  }

}
