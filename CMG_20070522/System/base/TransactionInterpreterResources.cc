#include "TransactionInterpreterResources.hh"

#include "NddlResource.hh"

namespace EUROPA {

  ResourceObjectFactory::ResourceObjectFactory(const LabelStr& signature) 
    : NativeObjectFactory("Resource",signature) 
  {
  }
  	 
  ResourceObjectFactory::~ResourceObjectFactory() 
  {
  } 
  	
  ObjectId ResourceObjectFactory::makeNewObject( 
						const PlanDatabaseId& planDb, 
						const LabelStr& objectType, 
						const LabelStr& objectName, 
						const std::vector<const AbstractDomain*>& arguments) const 
  { 
    Id<NDDL::NddlResource>  instance = (new NDDL::NddlResource(planDb, objectType, objectName,true))->getId();
    	
    std::vector<float> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back((float)(arguments[i]->getSingletonValue()));
	   	    
    if (argValues.size() == 0) 
      instance->constructor();
    else if (argValues.size() == 3)
      instance->constructor(argValues[0],argValues[1],argValues[2]);
    else if (argValues.size() == 5)
      instance->constructor(argValues[0],argValues[1],argValues[2],argValues[3],argValues[4]);
    else if (argValues.size() == 7)
      instance->constructor(argValues[0],argValues[1],argValues[2],argValues[3],argValues[4],argValues[5],argValues[6]);
    else {
      std::ostringstream os;
      os << "Unexpected number of args in Resource constructor:" << argValues.size();
      check_runtime_error(ALWAYS_FAILS,os.str());
    }	
	    		
    instance->handleDefaults(false /*don't close the object yet*/); 	    	
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString()); 

    return instance;	
  }   
    
  TokenId ResourceChangeTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Resource.change"); 
    return new NDDL::NddlResource::change(planDb,name,rejectable,isFact,true);
  }
	
  TokenId ResourceChangeTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Resource.change"); 
    return (new NDDL::NddlResource::change(master,name,relation,true))->getId();
  }     
		  
  ReusableObjectFactory::ReusableObjectFactory(const LabelStr& signature) 
    : NativeObjectFactory("Reusable",signature) 
  {
  }
  	 
  ReusableObjectFactory::~ReusableObjectFactory() 
  {
  } 
  	
  ObjectId ReusableObjectFactory::makeNewObject( 
						const PlanDatabaseId& planDb, 
						const LabelStr& objectType, 
						const LabelStr& objectName, 
						const std::vector<const AbstractDomain*>& arguments) const 
  { 
    Id<NDDL::NddlReusable>  instance = (new NDDL::NddlReusable(planDb, objectType, objectName,true))->getId();
    	
    std::vector<float> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back((float)(arguments[i]->getSingletonValue()));
	   	    
    if (argValues.size() == 0) 
      instance->constructor();
    else if (argValues.size() == 2)
      instance->constructor(argValues[0],argValues[1]);
    else if (argValues.size() == 3)
      instance->constructor(argValues[0],argValues[1],argValues[2]);
    else if (argValues.size() == 4)
      instance->constructor(argValues[0],argValues[1],argValues[2],argValues[3]);
    else {
      std::ostringstream os;
      os << "Unexpected number of args in Reusable constructor:" << argValues.size();
      check_runtime_error(ALWAYS_FAILS,os.str());
    }	
	    		
    instance->handleDefaults(false /*don't close the object yet*/); 	    	
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString()); 

    return instance;	
  }   
    
  TokenId ReusableUsesTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reusable.uses"); 
    return new NDDL::NddlReusable::uses(planDb,name,rejectable,isFact,true);
  }
	
  TokenId ReusableUsesTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reusable.uses"); 
    return (new NDDL::NddlReusable::uses(master,name,relation,true))->getId();
  }        


  TransactionInterpreterResourcesInitializer::TransactionInterpreterResourcesInitializer()
  {
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
  }
  
  TransactionInterpreterResourcesInitializer TransactionInterpreterResourcesInitializer::s_instance;
  
  TransactionInterpreterResourcesInitializer& TransactionInterpreterResourcesInitializer::getInstance() 
  { 
      return s_instance; 
  } 
}
