/**
 * @file TransactionInterpreter.cc
 * @brief Core classes to interpret XML statements
 * @author Javier Barreiro
 * @date December, 2006
 */

#include "TransactionInterpreter.hh"
#include "DbClient.hh"
#include "Error.hh"
#include "Object.hh" 
#include "ObjectFactory.hh"

namespace EUROPA {

    DataRef DataRef::null;
    
    DataRef::DataRef(AbstractDomain* d)
    {
    	m_value.domain = d;
    }
    
    DataRef::DataRef(const AbstractDomain* d)
    {
    	m_value.constDomain = d;
    }
    
    DataRef::~DataRef()
    {
    }
  	    
    AbstractDomain* DataRef::getValue() { return m_value.domain; }
    
    const AbstractDomain* DataRef::getConstValue() const { return m_value.constDomain; }
  	    
    
    TIVariable::TIVariable(const char* name,const AbstractDomain* value)
        : m_name(name)
        , m_value(value)
    {
    }

    TIVariable::~TIVariable()
    {
    }

    const char* TIVariable::getName() const { return m_name; }
    DataRef& TIVariable::getDataRef() { return m_value; }

    /*
     * EvalContext
     */
    EvalContext::EvalContext(EvalContext* parent)
        : m_parent(parent)
    {
    } 
    
    EvalContext::~EvalContext()
    {
    }

  	void EvalContext::addVar(const char* name,const AbstractDomain* value)
  	{
  		TIVariable v(name,value);
  		m_variables[name] = v;
  	} 

  	TIVariable& EvalContext::getVar(const char* name)
  	{
  		std::map<std::string,TIVariable>::iterator it = 
  		    m_variables.find(name);
  		    
        if( it != m_variables.end() ) {
            return it->second;
  	    }
  	    else {
  	    	check_error(m_parent != NULL);
  	    	return m_parent->getVar(name);
  	    }
  	}

    /*
     * ExprConstructorSuperCall
     */   
  	ExprConstructorSuperCall::ExprConstructorSuperCall(const LabelStr& superClassName, const std::vector<Expr*>& argExprs)
  	    : m_superClassName(superClassName)
  	    , m_argExprs(argExprs)
  	{
  	}
  	
  	ExprConstructorSuperCall::~ExprConstructorSuperCall()
  	{
  	}
  	
  	DataRef ExprConstructorSuperCall::eval(EvalContext& context) const
  	{
  		ObjectId object = context.getVar("this").getDataRef().getValue()->getSingletonValue();

  		std::vector<const AbstractDomain*> arguments;
  		for (unsigned int i=0; i < m_argExprs.size(); i++) {
			DataRef arg = m_argExprs[i]->eval(context);
			arguments.push_back(arg.getConstValue());
  		}
  		
  		ObjectFactory::invokeConstructor(object,m_superClassName,arguments);
  		
  		return DataRef::null;
  	}      	 

    /*
     * ExprConstructorAssignment
     */   
  	ExprConstructorAssignment::ExprConstructorAssignment(const char* lhs, 
  	                                                     Expr* rhs)
  	    : m_lhs(lhs)
  	    , m_rhs(rhs)
  	{
  	}
  	
  	ExprConstructorAssignment::~ExprConstructorAssignment()
  	{
  	}
  	    
  	DataRef ExprConstructorAssignment::eval(EvalContext& context) const
  	{
  		ObjectId object = context.getVar("this").getDataRef().getValue()->getSingletonValue();
     	const AbstractDomain* domain = m_rhs->eval(context).getConstValue();
     	check_error(object->getVariable(m_lhs) == ConstrainedVariableId::noId());
     	object->addVariable(*domain,m_lhs);
  		
  		return DataRef::null;
  	} 

    /*
     * ExprConstant
     */   
    ExprConstant::ExprConstant(const AbstractDomain* d)
        : m_data(d)
    {
    }
    
  	ExprConstant::~ExprConstant()
  	{
  	}

  	DataRef ExprConstant::eval(EvalContext& context) const
  	{
  		return m_data;
  	}  
  	    

    /*
     * ExprVariableRef
     */   
    ExprVariableRef::ExprVariableRef(const char* varName)
        : m_varName(varName)
    {
    }
  	
  	ExprVariableRef::~ExprVariableRef()
  	{
  	}

  	DataRef ExprVariableRef::eval(EvalContext& context) const
  	{
     	TIVariable& rhs = context.getVar(m_varName);
     	return rhs.getDataRef();
  	}  

    /*
     * ExprNewObject
     */   
    ExprNewObject::ExprNewObject(const DbClientId& dbClient,
	                             const LabelStr& objectType, 
	                             const LabelStr& objectName,
	                             const std::vector<Expr*>& argExprs)
	    : m_dbClient(dbClient)
	    , m_objectType(objectType)
	    , m_objectName(objectName)
	    , m_argExprs(argExprs)
	{
	}
	    
    ExprNewObject::~ExprNewObject()
    {
    }

  	DataRef ExprNewObject::eval(EvalContext& context) const
  	{  
  		std::vector<const AbstractDomain*> arguments;
  		for (unsigned int i=0; i < m_argExprs.size(); i++) {
			DataRef arg = m_argExprs[i]->eval(context);
			arguments.push_back(arg.getConstValue());
  		}
  		
  		// TODO: when this is the rhs of an assignment in a constructor, 
  		// this must create an object specifying the enclosing object (for which the constructor is being executed)
  		// as the parent.
  		// This is a problem, everybody should go through the factory, the way it is now, inside a constructor objects are
  		// created directly in C++ through new (in the generated code) and are created through the factories if
  		// they're in the initial state
  		// faking it for now, but this is a hack

  		// This assumes for now this is only called inside a constructor  		
  		ObjectId thisObject = context.getVar("this").getDataRef().getValue()->getSingletonValue();
  		LabelStr name(std::string(thisObject->getName().toString() + "." + m_objectName.toString()));  		
     	ObjectId newObject = m_dbClient->createObject(
     	    m_objectType.c_str(), 
     	    name.c_str(), 
     	    arguments
     	);
     	newObject->setParent(thisObject);
     	 
	    // TODO: must not leak ObjectDomain                
	    return DataRef(new ObjectDomain(newObject,m_objectType.c_str()));
  	}
  	     
    /*
     * InterpretedObjectFactory
     */     	     
	InterpretedObjectFactory::InterpretedObjectFactory(
	                               const char* className,
	                               const LabelStr& signature, 
	                               const std::vector<std::string>& constructorArgNames,
	                               const std::vector<std::string>& constructorArgTypes,
	                               const std::vector<Expr*>& constructorBody)
	    : ConcreteObjectFactory(signature) 
	    , m_className(className)
	    , m_constructorArgNames(constructorArgNames)
	    , m_constructorArgTypes(constructorArgTypes)
	    , m_constructorBody(constructorBody)
	{
	}
	
	InterpretedObjectFactory::~InterpretedObjectFactory()
	{
		for (unsigned int i=0; i < m_constructorBody.size(); i++) 
		    delete m_constructorBody[i];
	}
	
	ObjectId InterpretedObjectFactory::createInstance(
	                        const PlanDatabaseId& planDb,
	                        const LabelStr& objectType, 
	                        const LabelStr& objectName,
	                        const std::vector<const AbstractDomain*>& arguments) const 
	{
	  check_error(checkArgs(arguments));
	  
      ObjectId instance = makeNewObject(planDb, objectType, objectName);	  
	  constructor(instance,arguments);
	  instance->close();
	  
	  return instance;
	} 

    bool InterpretedObjectFactory::checkArgs(const std::vector<const AbstractDomain*>& arguments) const
    {
    	// TODO: implement this
    	return true;
    }
    	
	ObjectId InterpretedObjectFactory::makeNewObject( 
	                        const PlanDatabaseId& planDb,
	                        const LabelStr& objectType, 
	                        const LabelStr& objectName) const
	{
	  // TODO: If a class has a native C++ ancestor the ancestor instance must be instanciated instead
	  // Every native ancestor must extend Object 
	  // This should be implemented so that the appropriate factory for the superclass gets a shot at creating the object first
	  ObjectId instance = (new Object(planDb, objectType, objectName,true))->getId();
	  
	  // NOTE: here we'd normally add variables and initialize with default values, 
	  // making sure superclasses are handled as well
	  // However, initially we'll follow the approach from code generation, where variables are
	  // added as they're initialized
	  
	  return instance;
	}
	
	void InterpretedObjectFactory::constructor(ObjectId& instance, 
	                                           const std::vector<const AbstractDomain*>& arguments) const
	{
		// TODO: need to pass in global eval context somehow
		EvalContext evalContext(NULL);

        // Add new object and arguments to eval context
		ObjectDomain thisDomain(instance,m_className.c_str());		
		evalContext.addVar("this",&thisDomain);
		
		for (unsigned int i=0;i<m_constructorArgNames.size();i++) 
			evalContext.addVar(m_constructorArgNames[i].c_str(),arguments[i]);
		
		for (unsigned int i=0; i < m_constructorBody.size(); i++) 
			m_constructorBody[i]->eval(evalContext);
			
	    // Initialize variables that were not explicitly initialized, including inherited vars?
		/*
		   std::vector<std::string> varNames = getVarNamesFromClassName(m_className);
		   std::vector<std::string> varTypes = getVarTypesFromClassName(m_className);		   
		   for (unsigned int i=0; i < varNames.size(); i++) {
		       if (instance.getVariable(varNames[i] == ConstrainedVariableId::noId())) {
		           instance.addVariable(getDefaultDomain(varTypes[i]),varNames[i]);
		       } 
		   }
	    */ 	    
	}	
}

