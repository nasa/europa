/**
 * @file TransactionInterpreter.cc
 * @brief Core classes to interpret XML statements
 * @author Javier Barreiro
 * @date December, 2006
 */

#include "TransactionInterpreter.hh"
#include "tinyxml.h"
#include "Debug.hh"
#include "Error.hh"
//#include "AbstractDomain.hh"
#include "DbClient.hh"
#include "EnumeratedTypeFactory.hh"
#include "Object.hh" 
#include "ObjectFactory.hh"
#include "TokenVariable.hh"
#include "TypeFactory.hh"
#include "Schema.hh"

// To support Rule Interpretation
#include "NddlRules.hh"
#include "NddlUtils.hh"

// Hack!! the macro in NddlRules.hh only works with code-generation
// so this is redefined here. this is brittle though
// TODO: change code-generation code to work with this macro instead
namespace NDDL {
#define relation(relationname, origin, originvar, target, targetvar) {\
 std::vector<ConstrainedVariableId> vars;\
 vars.push_back(getSlave(LabelStr(origin))->get##originvar());\
 vars.push_back(getSlave(LabelStr(target))->get##targetvar());\
 rule_constraint(relationname,vars);\
  } 
}  

/*
 * Here is what can be used as a main to run an interpreted version through the DSA
 * 
bool runDSATest(const char* plannerConfig, const char* txSource)
{
    DSA::DSA& dsa = DSA::DSA::instance();
    dsa.addPlan(txSource,true);
    dsa.solverConfigure(plannerConfig,0,100);
    dsa.solverSolve(500,500);
    dsa.writePlan(std::cout);
    
	return false;
}

int main(int argc, const char ** argv){
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  const char* txSource = argv[1];
  const char* plannerConfig = argv[2];
  
  if (!runDSATest(plannerConfig,txSource))
      return 0;
}
 */
  

namespace EUROPA {

  /*
   * 
   * InterpretedDbClientTransactionPlayer
   * 
   */ 
       
  void createDefaultObjectFactory(const char* className, bool canCreateObjects)
  {
      std::vector<std::string> constructorArgNames;
      std::vector<std::string> constructorArgTypes;
      std::vector<Expr*> constructorBody;
      ExprConstructorSuperCall* superCallExpr = NULL;
      
      // If it can't create objects, generate default super call
      if (!canCreateObjects) 
            superCallExpr = new ExprConstructorSuperCall(Schema::instance()->getParent(className),std::vector<Expr*>());                 	
      
      // The ObjectFactory constructor automatically registers the factory
      new InterpretedObjectFactory(
          className,
          className,
          constructorArgNames,
          constructorArgTypes,
          superCallExpr,
          constructorBody,
          canCreateObjects
      );       
  }
  
  InterpretedDbClientTransactionPlayer::InterpretedDbClientTransactionPlayer(const DbClientId & client)
      : DbClientTransactionPlayer(client) 
  {  	  
  	  m_systemClasses.insert("Object");
  	  m_systemClasses.insert("Timeline");
  	  
  	  // TODO: this should be done once after the schema is initialized, not for every TransactionPlayer
  	  // TODO: check to make sure this is done only once
  	  createDefaultObjectFactory("Object", true);
      REGISTER_OBJECT_FACTORY(TimelineObjectFactory, Timeline);	    	    	  
  }

  InterpretedDbClientTransactionPlayer::~InterpretedDbClientTransactionPlayer() 
  {
  }

  const char* safeStr(const char* str)
  {
  	return (str !=NULL ? str : "NULL");
  }

  std::ostringstream dbgout;
  const char* ident="    ";
  
  void InterpretedDbClientTransactionPlayer::playDeclareClass(const TiXmlElement& element) 
  {
    const char* className = element.Attribute("name");
    Id<Schema> schema = Schema::instance();
    schema->declareObjectType(className);
    dbgout.str("");
    dbgout << "Declared class " << className << std::endl;
    std::cout << dbgout.str();
  }
  
  void InterpretedDbClientTransactionPlayer::playDefineClass(const TiXmlElement& element) 
  {
    const char* className = element.Attribute("name");

    const char* parentClassName = element.Attribute("extends");
    parentClassName = (parentClassName == NULL ? "Object" : parentClassName);

    Id<Schema> schema = Schema::instance();
    schema->addObjectType(className,parentClassName);
    
    // The TypeFactory constructor automatically registers the factory
    // TODO: This should probably be done by Schema::addObjectType
    new EnumeratedTypeFactory(
        className,
        className,
        ObjectDomain(className)
    );             

    dbgout.str("");
    dbgout << "class " << className  
                       << (parentClassName != NULL ? " extends " : "") 
                        << (parentClassName != NULL ? parentClassName : "") 
                        << " {" << std::endl;
                          
    bool definedConstructor = false;                          
    for(const TiXmlElement* child = element.FirstChildElement(); child; child = child->NextSiblingElement() ) {    	
    	const char * tagname = child->Value();
    	
	    if (strcmp(tagname, "var") == 0) 
	    	defineClassMember(schema,className,child);
	    else if (strcmp(tagname, "constructor") == 0) { 
	    	defineConstructor(schema,className,child);
	    	definedConstructor = true;	
	    }	    	    	
	    else if (strcmp(tagname, "predicate") == 0) 
	    	declarePredicate(schema,className,child);	    	
	    else if (strcmp(tagname, "enum") == 0) 
	    	defineEnum(schema,className,child);	    	
    }

    // Register a default factory with no arguments if one is not provided explicitly
    if (!definedConstructor &&
        m_systemClasses.find(className) == m_systemClasses.end()) { 
        	createDefaultObjectFactory(className, false);
        	dbgout << "    generated default constructor" << std::endl;
    }
    
    dbgout << "}" << std::endl;    
    std::cout << dbgout.str() << std::endl;     
  }

  Expr* InterpretedDbClientTransactionPlayer::valueToExpr(const TiXmlElement* element)
  {
    if (strcmp(element->Value(),"value") == 0) {
    	return new ExprConstant(xmlAsAbstractDomain(*element));
    }
    else if (strcmp(element->Value(),"id") == 0) {
      	const char* varName = element->Attribute("name");
      	return new ExprVariableRef(varName);
    }
    else
        check_error(ALWAYS_FAILS,std::string("Unexpected xml element:") + element->Value());
        
    return NULL;        
  }
  
  void InterpretedDbClientTransactionPlayer::defineClassMember(Id<Schema>& schema, const char* className,  const TiXmlElement* element)
  {	
	  const char* type = safeStr(element->Attribute("type"));	
	  const char* name = safeStr(element->Attribute("name"));	
	  dbgout << ident << type << " " << name << std::endl;
	  schema->addMember(className, type, name);
  }

  int InterpretedDbClientTransactionPlayer::defineConstructor(Id<Schema>& schema, const char* className,  const TiXmlElement* element)
  {	
  	  std::ostringstream signature;
  	  signature << className;

      std::vector<std::string> constructorArgNames;
      std::vector<std::string> constructorArgTypes;
      std::vector<Expr*> constructorBody;
      ExprConstructorSuperCall* superCallExpr = NULL;
        	  
      for(const TiXmlElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement() ) {
      	  if (strcmp(child->Value(),"arg") == 0) {
	          const char* type = safeStr(child->Attribute("type"));	
	          const char* name = safeStr(child->Attribute("name"));	
	          constructorArgNames.push_back(name);
	          constructorArgTypes.push_back(type);
	          signature << ":" << type;
      	  }
      	  else if (strcmp(child->Value(),"super") == 0) {
      	  	std::vector<Expr*> argExprs;     	  	
            for(const TiXmlElement* argChild = child->FirstChildElement(); argChild; argChild = argChild->NextSiblingElement() ) 
            	argExprs.push_back(valueToExpr(argChild));

      	  	superCallExpr = new ExprConstructorSuperCall(Schema::instance()->getParent(className),argExprs);
      	  }
      	  else if (strcmp(child->Value(),"assign") == 0) {
      	  	const TiXmlElement* rhsChild = child->FirstChildElement();

      	  	const char* lhs = child->Attribute("name");

      	  	Expr* rhs=NULL;
            // rhs type can be "value | interval | set" (constant), "id" (parameter) or "new" (new object)  
      	  	const char* rhsType = rhsChild->Value();
      	  	if (strcmp(rhsType,"value") == 0 || 
      	  	    strcmp(rhsType,"interval") == 0 ||
      	  	    strcmp(rhsType,"set") == 0) {
      	  		rhs = new ExprConstant(xmlAsAbstractDomain(*rhsChild));
      	  	}
      	  	else if (strcmp(rhsType,"id") == 0) {
          	  	const char* varName = rhsChild->Attribute("name");
          	  	rhs = new ExprVariableRef(varName);
      	  	}
      	  	else if (strcmp(rhsType,"new") == 0) {
	          	 const char* objectType = rhsChild->Attribute("type");      	  		
      	  		
	      	  	std::vector<Expr*> argExprs;     	  	
	            for(const TiXmlElement* argChild = rhsChild->FirstChildElement(); argChild; argChild = argChild->NextSiblingElement() ) {
	          	    if (strcmp(argChild->Value(),"value") == 0) {
	          	    	Expr* arg = new ExprConstant(xmlAsAbstractDomain(*argChild));
	          	    	argExprs.push_back(arg);
	          	    }
	          	    else if (strcmp(argChild->Value(),"id") == 0) {
	          	    	const char* varName = argChild->Attribute("name");
	          	    	Expr* arg = new ExprVariableRef(varName);
	          	    	argExprs.push_back(arg);
	          	    }
	            }

      	  		rhs = new ExprNewObject(
      	  		    m_client,
      	  		    objectType,
      	  		    lhs,
      	  		    argExprs
      	  		);
      	  	}

      	  	constructorBody.push_back(new ExprConstructorAssignment(lhs,rhs));
      	  }
      }	
      dbgout << ident << "constructor (" << signature.str() << ")" << std::endl; 

		// If constructor for super class isn't called explicitly, call default one with no args
        if (superCallExpr == NULL) 
            superCallExpr = new ExprConstructorSuperCall(Schema::instance()->getParent(className),std::vector<Expr*>());                 	
            
      // The ObjectFactory constructor automatically registers the factory
      new InterpretedObjectFactory(
          className,
          signature.str(),
          constructorArgNames,
          constructorArgTypes,
          superCallExpr,
          constructorBody
      ); 
      
      return constructorArgNames.size();
  }

  void InterpretedDbClientTransactionPlayer::declarePredicate(Id<Schema>& schema, const char* className,  const TiXmlElement* element)
  {	
      std::string predName = std::string(className) + "." + element->Attribute("name");	
      schema->addPredicate(predName.c_str());

      std::vector<LabelStr> parameterNames;
      std::vector<LabelStr> parameterTypes;
      
      dbgout << ident << "predicate " <<  predName << "(";
      for(const TiXmlElement* predArg = element->FirstChildElement(); predArg; predArg = predArg->NextSiblingElement() ) {
      	if (strcmp(predArg->Value(),"var") == 0) {
	      const char* type = safeStr(predArg->Attribute("type"));	
	      const char* name = safeStr(predArg->Attribute("name"));	
	      dbgout << type << " " << name << ",";
          schema->addMember(predName.c_str(), type, name);
          parameterNames.push_back(name);
          parameterTypes.push_back(type);
          // TODO: Check for value assignment
      	}
      	else if (strcmp(predArg->Value(),"invoke") == 0) {
      		// TODO: deal with this
      		dbgout << "constraint " << predArg->Attribute("name");
      	}             
      }	
      dbgout << ")" << std::endl;
      
      // The TokenFactory's constructor automatically registers the factory
      // TODO: pass along parameter domain restrictions and constraints
      new InterpretedTokenFactory(
          predName,
          parameterNames,
          parameterTypes
      );
  }
    
  void InterpretedDbClientTransactionPlayer::defineEnum(Id<Schema>& schema, const char* className,  const TiXmlElement* element)
  {	
  	  // Enum is scoped within the class but in the generated code it doesn't make a difference
  	  playDefineEnumeration(*element);
  }
  
  /*
   * figures out the type of a predicate given an instance
   * 
   */
  const char* predicateInstanceToType(const char* className,const char* predicateInstance)
  {
  	// TODO: implement this
  	return "YourObject.helloWorld";
  }
  
  void InterpretedDbClientTransactionPlayer::playDefineCompat(const TiXmlElement& element)
  {
      std::string predName = std::string(element.Attribute("class")) + "." + element.Attribute("name");	
      std::string source = std::string(element.Attribute("filename")) + 
          " line " + element.Attribute("line") +
          " column " + element.Attribute("column");	

      int slave_cnt = 0;      
      std::vector<RuleExpr*> ruleBody;
      
      for(const TiXmlElement* child = element.FirstChildElement()->FirstChildElement(); child; child = child->NextSiblingElement() ) {
      	if (strcmp(child->Value(),"invoke") == 0) {

      		std::vector<Expr*> constraintArgs;
            for(const TiXmlElement* arg = child->FirstChildElement(); arg; arg = arg->NextSiblingElement() ) 
            	constraintArgs.push_back(valueToExpr(arg));
            	
      		ruleBody.push_back(new ExprConstraint(child->Attribute("name"),constraintArgs));
      	}
      	else if (strcmp(child->Value(),"subgoal") == 0) {
      		const char* predicateInstance = NULL;
      		const char* name = NULL;
            for(const TiXmlElement* arg = child->FirstChildElement(); arg; arg = arg->NextSiblingElement() ) {
            	if (strcmp(arg->Value(),"predicateinstance") == 0) { 
            		predicateInstance = arg->Attribute("type");
            		name = arg->Attribute("name");
            	}            		
             	else 
      	            check_error(ALWAYS_FAILS,std::string("Unknown subgoal element:") + arg->Value());
            }
            check_error(predicateInstance != NULL,"predicate instance in a subgoal cannot be null");

            const char* predicateType = predicateInstanceToType(element.Attribute("class"), predicateInstance);    
            if (name == NULL) {
            	std::ostringstream tmpname;
            	tmpname << "slave" << (slave_cnt++);            
                name = LabelStr(tmpname.str()).c_str();
            }                 
      		ruleBody.push_back(new ExprSubgoal(name,predicateType,predicateInstance,child->Attribute("relation")));
      	}
      	else 
      	   check_error(ALWAYS_FAILS,std::string("Unknown Compatibility element:") + child->Value());
      }
      
      // The RuleFactory's constructor automatically registers the factory
      new InterpretedRuleFactory(predName,source,ruleBody);
  }
  
  void InterpretedDbClientTransactionPlayer::playDefineEnumeration(const TiXmlElement &element)
  {
      const char* enumName = element.Attribute("name");
      const TiXmlElement* setElement = element.FirstChildElement();
      check_error(strcmp(setElement->Value(),"set") == 0, "Expected value set as part of Enum definition");
      
      Id<Schema> schema = Schema::instance();
      schema->addEnum(enumName);
      
      const TiXmlElement* enumValue;
      for(enumValue = setElement->FirstChildElement(); enumValue; enumValue = enumValue->NextSiblingElement() ) {
      	if (strcmp(enumValue->Value(),"symbol") == 0) {
      		LabelStr symbolValue(enumValue->Attribute("value"));
            schema->addValue(enumName, symbolValue);
      	}
      	else {
      	    // TODO: deal with other types
      	    check_error(ALWAYS_FAILS,std::string("Don't know how to deal with enum values of type ") + enumValue->Value());
      	}
      }	      
  }

  void InterpretedDbClientTransactionPlayer::playDefineType(const TiXmlElement &)
  {
      // TODO: jrb
  }  
  
    /*
     * DataRef
     */   
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
    	// TODO: should DataRef delete the AbstractDomain?, it'll prevent leaks in many cases
    }
  	    
    AbstractDomain* DataRef::getValue() { return m_value.domain; }
    
    const AbstractDomain* DataRef::getConstValue() const { return m_value.constDomain; }
  	        
    /*
     * TIVariable
     */   
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

  	TIVariable* EvalContext::getVar(const char* name)
  	{
  		std::map<std::string,TIVariable>::iterator it = 
  		    m_variables.find(name);
  		    
        if( it != m_variables.end() ) 
            return &(it->second);
  	    else if (m_parent != NULL)
  	    	return m_parent->getVar(name);
  	    else 
  	        return NULL;
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
  		ObjectId object = context.getVar("this")->getDataRef().getValue()->getSingletonValue();

  		std::vector<const AbstractDomain*> arguments;

        evalArgs(context,arguments);  		
  		ObjectFactory::evalConstructorBody(object,m_superClassName,arguments);
  		
  		return DataRef::null;
  	}      	 

    void ExprConstructorSuperCall::evalArgs(EvalContext& context, std::vector<const AbstractDomain*>& arguments) const
    {
  		for (unsigned int i=0; i < m_argExprs.size(); i++) {
			DataRef arg = m_argExprs[i]->eval(context);
			arguments.push_back(arg.getConstValue());
  		}
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
  		ObjectId object = context.getVar("this")->getDataRef().getValue()->getSingletonValue();
     	const AbstractDomain* domain = m_rhs->eval(context).getConstValue();
     	check_error(object->getVariable(m_lhs) == ConstrainedVariableId::noId());
     	object->addVariable(*domain,m_lhs);
     	std::cout << "Initialized variable:" << object->getName().toString() << "." << m_lhs << " in constructor" << std::endl;
  		
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
     	TIVariable* rhs = context.getVar(m_varName);

     	if (rhs == NULL) 
     	    // TODO: throw exception
     	    return DataRef::null;
     	    
     	return rhs->getDataRef();
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
  		
  		// TODO: when this is the rhs of an assignment in a constructor, this must create an object specifying 
  		// the enclosing object (for which the constructor is being executed) as the parent.
  		// The way it is now, objects are created :
  		// - directly in C++ through new (in the generated code) if the stmt is inside a NDDL constructor
  		// - through the factories if the stmtm is in the initial state
  		// This is a problem, everybody should go through the factory
  		// faking it for now, but this is a hack

  		// This assumes for now this is only called inside a constructor  		
  		ObjectId thisObject = context.getVar("this")->getDataRef().getValue()->getSingletonValue();
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
  	
  	
  	ExprConstraint::ExprConstraint(const char* name,const std::vector<Expr*> args)
  	    : m_name(name)
  	    , m_args(args)
  	{
  	}

  	ExprConstraint::~ExprConstraint()
  	{
  	}

  	DataRef ExprConstraint::eval(EvalContext& context) const
  	{
  		std::vector<ConstrainedVariableId> vars;
  		for (unsigned int i=0; i < m_args.size(); i++) {
			DataRef arg = m_args[i]->eval(context);
			// TODO: Implement DataRef::getVariable
			//vars.push_back(arg.getVariable());
  		}

  		m_ruleInstance->createConstraint(m_name,vars);
  		std::cout << "Evaluated Constraint : " << m_name.toString() << std::endl;
  		return DataRef::null;
  	}  
  	    
  
    // TODO: predicateType that comes from xml is something like object.fw.fasting. That needs to be translated
    // into something that the token factory understands : ObjectType "." PredicateName, for instance FastingWindow.fasting
  	ExprSubgoal::ExprSubgoal(const char* name,
  	                         const char* predicateType, 
  	                         const char* predicateInstance, 
  	                         const char* relation)
  	    : m_name(name)
  	    , m_predicateType(predicateType)
  	    , m_predicateInstance(predicateInstance)
  	    , m_relation(relation)
    {
    }  	      	    
  	
  	ExprSubgoal::~ExprSubgoal()
  	{
  	}

  	DataRef ExprSubgoal::eval(EvalContext& context) const  
  	{
  		std::cout << "Creating subgoal " << m_predicateType.toString() << ":" << m_name.toString() << std::endl;
  		m_ruleInstance->createSubgoal(m_name,m_predicateType,m_relation);
  		std::cout << "Create subgoal " << m_predicateType.toString() << ":" << m_name.toString() << std::endl;
  		return DataRef::null;
  	}  
  	     
    /*
     * InterpretedObjectFactory
     */     	     
	InterpretedObjectFactory::InterpretedObjectFactory(
	                               const char* className,
	                               const LabelStr& signature, 
	                               const std::vector<std::string>& constructorArgNames,
	                               const std::vector<std::string>& constructorArgTypes,
	                               ExprConstructorSuperCall* superCallExpr,
	                               const std::vector<Expr*>& constructorBody,
	                               bool canMakeNewObject)
	    : ConcreteObjectFactory(signature) 
	    , m_className(className)
	    , m_constructorArgNames(constructorArgNames)
	    , m_constructorArgTypes(constructorArgTypes)
	    , m_superCallExpr(superCallExpr)
	    , m_constructorBody(constructorBody)
	    , m_canMakeNewObject(canMakeNewObject)
	{
	}
	
	InterpretedObjectFactory::~InterpretedObjectFactory()
	{
		if (m_superCallExpr != NULL)
    		delete m_superCallExpr;
		
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
	  
	    ObjectId instance = makeNewObject(planDb, objectType, objectName,arguments);
	    evalConstructorBody(instance,arguments);
	    instance->close();
	  
	    return instance;
	} 

    bool InterpretedObjectFactory::checkArgs(const std::vector<const AbstractDomain*>& arguments) const
    {
    	// TODO: implement this
    	return true;
    }
    	
    /*
     * If a class has a native C++ ancestor the ancestor instance must be instanciated instead
     * - Every exported C++ class (like Timeline) must extend Object
     * - Every exported C++ class must register an InterpretedObjectFactory for each one of its constructors 
     *   that overrides this method and calls the appropriate constructor
     * 
	 */  
	ObjectId InterpretedObjectFactory::makeNewObject( 
	                        const PlanDatabaseId& planDb,
	                        const LabelStr& objectType, 
	                        const LabelStr& objectName,
	                        const std::vector<const AbstractDomain*>& arguments) const
	{
	  
      // go up the hierarchy and give the parents a chance to create the object, this allows native classes to be exported
      // TODO: some effort can be saved by keeping track of whether a class has a native ancestor different from Object.
      // If it doesn't, the object can be created right away and this traversal up the hierarchy can be skipped
	  if (m_canMakeNewObject) {
    	  std::cout << "Created Object:" << objectName.toString() << " type:" << objectType.toString() << std::endl;
	      return (new Object(planDb, objectType, objectName,true))->getId();
	  }
	  else {
	        check_error(m_superCallExpr != NULL, std::string("Failed to find factory for object ") + objectName.toString() + " of type "+objectType.toString());
	        // TODO: argumentsForSuper are eval'd twice, once here and once when the constructor body is eval'd
	        //  figure out how to do it only once
	    	std::vector<const AbstractDomain*> argumentsForSuper;
     	    EvalContext evalContext(NULL);
     	    m_superCallExpr->evalArgs(evalContext,argumentsForSuper);
	        return ObjectFactory::makeNewObject(planDb, m_superCallExpr->getSuperClassName(), objectType, objectName,argumentsForSuper);
	  }
	}
	
	void InterpretedObjectFactory::evalConstructorBody(ObjectId& instance, 
	                                           const std::vector<const AbstractDomain*>& arguments) const
	{
        // TODO: need to pass in eval context from outside
	    EvalContext evalContext(NULL);
	    
        // Add new object and arguments to eval context
		ObjectDomain thisDomain(instance,m_className.c_str());		
		evalContext.addVar("this",&thisDomain);
		
		for (unsigned int i=0;i<m_constructorArgNames.size();i++) 
			evalContext.addVar(m_constructorArgNames[i].c_str(),arguments[i]);
		
		if (m_superCallExpr != NULL)
		    m_superCallExpr->eval(evalContext);
		
		for (unsigned int i=0; i < m_constructorBody.size(); i++) 
			m_constructorBody[i]->eval(evalContext);

        /*		
		const std::vector<ConstrainedVariableId>& vars = instance->getVariables();
		std::cout << "    Vars for " << m_className.toString() << " " << instance->getName().toString() << " are:";
		for (unsigned j=0; j < vars.size(); j++) {
			std::cout << vars[j]->getName().toString() << ",";
		}
		std::cout << std::endl;
	    */
			
	    // Initialize any variables that were not explicitly initialized
	    const Schema::NameValueVector& members = Schema::instance()->getMembers(m_className);
	    for (unsigned int i=0; i < members.size(); i++) {
	    	std::string varName = instance->getName().toString() + "." + members[i].second.toString();
	        if (instance->getVariable(varName) == ConstrainedVariableId::noId()) {
	    	    AbstractDomain* baseDomain = TypeFactory::baseDomain(members[i].first.c_str()).copy(); 
	            instance->addVariable(
	                *baseDomain,
	                members[i].second.c_str()
	            );
	            delete baseDomain;
	            std::cout << "Used default initializer for " << m_className.toString() << "." << members[i].second.toString() << std::endl; 
	        } 
	    }
	}	
	
    /*
     * InterpretedToken
     */     	     
    InterpretedToken::InterpretedToken(const PlanDatabaseId& planDatabase, 
                     const LabelStr& predicateName, 
                     const std::vector<LabelStr>& parameterNames,
                     const std::vector<LabelStr>& parameterTypes, 
                     const bool& rejectable, 
                     const bool& close) 
                     
        : IntervalToken(planDatabase, 
                        predicateName,
                        rejectable,
                        IntervalIntDomain(),                  // start
                        IntervalIntDomain(),                  // end
                        IntervalIntDomain(1, PLUS_INFINITY),  // duration
                        Token::noObject(),                    // Object Name
                        false) 
    {
    	commonInit(parameterNames, parameterTypes, close);
    }
  	                     
    InterpretedToken::InterpretedToken(const TokenId& master, 
                     const LabelStr& predicateName, 
                     const LabelStr& relation, 
                     const std::vector<LabelStr>& parameterNames,
                     const std::vector<LabelStr>& parameterTypes, 
                     const bool& close)
        : IntervalToken(master, 
                        relation,
                        predicateName,
                        IntervalIntDomain(),                 // start
                        IntervalIntDomain(),                 // end
                        IntervalIntDomain(1, PLUS_INFINITY), // duration
                        Token::noObject(),                   // Object Name
                        false) 
    {
    	commonInit(parameterNames, parameterTypes, close);
    }
        
    InterpretedToken::~InterpretedToken()
    {
    }

    // Hack to get around limitation of TokenVariable that requires the domain of the
    // variable to be known at compile time. that needs to be fixed. in the mean time
    // this Abstract domain wrpper allows us to create generic TokenVariables
    class MonoDomain : public AbstractDomain 
    {
      public:
        MonoDomain(AbstractDomain& wrappedDomain)
            : AbstractDomain(false,false,"")
            , m_wrappedDomain(wrappedDomain)
        {
        }

        AbstractDomain& getWrappedDomain() const { return m_wrappedDomain; }
        
        MonoDomain(const AbstractDomain& rhs)
            : AbstractDomain(false,false,"")
            , m_wrappedDomain((dynamic_cast<const MonoDomain&>(rhs)).getWrappedDomain())
        {
        }
        
        virtual void close()  { m_wrappedDomain.close(); }
        virtual void open()   { m_wrappedDomain.open(); }
        virtual bool isClosed() const { return m_wrappedDomain.isClosed(); }
        virtual bool isOpen() const   { return m_wrappedDomain.isOpen(); }

        virtual bool isFinite() const  { return m_wrappedDomain.isFinite(); }
        virtual bool isNumeric() const { return m_wrappedDomain.isNumeric(); }
        virtual bool isBool() const    { return m_wrappedDomain.isBool(); }
        virtual bool isString() const  { return m_wrappedDomain.isString(); }

        virtual void setListener(const DomainListenerId& listener) { m_wrappedDomain.setListener(listener); }
        virtual const DomainListenerId& getListener() const { return m_wrappedDomain.getListener(); }

        virtual const LabelStr& getTypeName() const { return m_wrappedDomain.getTypeName(); }

        virtual bool isEnumerated() const { return m_wrappedDomain.isEnumerated(); }
        virtual bool isInterval() const { return m_wrappedDomain.isInterval(); }

        virtual bool isSingleton() const { return m_wrappedDomain.isSingleton(); }
        virtual bool isEmpty() const { return m_wrappedDomain.isEmpty(); };

        virtual void empty() { m_wrappedDomain.empty(); }

        virtual unsigned int getSize() const { return m_wrappedDomain.getSize(); }

        virtual void operator>>(ostream& os) const { os << m_wrappedDomain; }

        virtual double getUpperBound() const { return m_wrappedDomain.getUpperBound(); }
        virtual double getLowerBound() const { return m_wrappedDomain.getLowerBound(); }

        virtual double getSingletonValue() const { return m_wrappedDomain.getSingletonValue(); }

        virtual bool getBounds(double& lb, double& ub) const { return m_wrappedDomain.getBounds(lb,ub); }

        virtual void set(double value) { m_wrappedDomain.set(value); }

        virtual void reset(const AbstractDomain& dom) { m_wrappedDomain.reset(dom); }

        virtual void relax(const AbstractDomain& dom) { m_wrappedDomain.isFinite(); }

        virtual void relax(double value) { m_wrappedDomain.relax(value); }

        virtual void insert(double value) { m_wrappedDomain.insert(value); }

        virtual void insert(const std::list<double>& values) { m_wrappedDomain.insert(values); }

        virtual void remove(double value) { m_wrappedDomain.remove(value); }

        virtual bool intersect(const AbstractDomain& dom) { return m_wrappedDomain.intersect(dom); }

        virtual bool intersect(double lb, double ub) { return m_wrappedDomain.intersect(lb,ub); }

        virtual bool difference(const AbstractDomain& dom) { return m_wrappedDomain.difference(dom); }

        virtual AbstractDomain& operator=(const AbstractDomain& dom) { return m_wrappedDomain=dom; }

        virtual bool isMember(double value) const { return m_wrappedDomain.isMember(value); }

        virtual bool operator==(const AbstractDomain& dom) const { return m_wrappedDomain == dom; }

        virtual bool operator!=(const AbstractDomain& dom) const { return m_wrappedDomain != dom; }

        virtual bool isSubsetOf(const AbstractDomain& dom) const { return m_wrappedDomain.isSubsetOf(dom); }

        virtual bool intersects(const AbstractDomain& dom) const { return m_wrappedDomain.intersects(dom); }

        virtual bool equate(AbstractDomain& dom) { return m_wrappedDomain.equate(dom); }

        virtual void getValues(std::list<double>& results) const { return m_wrappedDomain.getValues(results); };

        virtual double minDelta() const { return m_wrappedDomain.minDelta(); }

        virtual double translateNumber(double number, bool asMin) const { return m_wrappedDomain.translateNumber(number,asMin); }

        virtual bool convertToMemberValue(const std::string& strValue, double& dblValue) const { return m_wrappedDomain.convertToMemberValue(strValue,dblValue); }

        virtual bool areBoundsFinite() const { return m_wrappedDomain.areBoundsFinite(); }

        virtual AbstractDomain *copy() const { return m_wrappedDomain.copy(); }

        virtual std::string toString() const { return m_wrappedDomain.toString(); }

        virtual std::string toString(double value) const { return m_wrappedDomain.toString(value); }

      protected:
          virtual void testPrecision(const double& value) const { /*noop*/ }
          
          AbstractDomain& m_wrappedDomain;               
    };
  
    void InterpretedToken::commonInit(
                     const std::vector<LabelStr>& parameterNames,
                     const std::vector<LabelStr>& parameterTypes, 
                     const bool& autoClose)
    {
    	// TODO: initialize parameters that have exprs 
    	// TODO: add any constraints included in the declaration
    	
	    for (unsigned int i=0; i < parameterNames.size(); i++) {
	        if (getVariable(parameterNames[i]) == ConstrainedVariableId::noId()) {
	    	    AbstractDomain* d = TypeFactory::baseDomain(parameterTypes[i].c_str()).copy();
	    	    // This is a hack needed because TokenVariable is parametrized by the domain arg to addParameter 
	    	    MonoDomain baseDomain(*d);
	            addParameter(
	                baseDomain,
	                parameterNames[i]
	            );
	            // TODO: will d be leaked?
	            //std::cout << "Used default initializer for " << m_className.toString() << "." << members[i].second.toString() << std::endl; 
	        } 
	    }
    	
        if (autoClose)
            close();    	
    }
      
    /*
     * InterpretedTokenFactory
     */     	     
    InterpretedTokenFactory::InterpretedTokenFactory(
                                       const LabelStr& predicateName,
                                       const std::vector<LabelStr>& parameterNames,
                                       const std::vector<LabelStr>& parameterTypes) 
	    : ConcreteTokenFactory(predicateName) 
	    , m_parameterNames(parameterNames)
	    , m_parameterTypes(parameterTypes)
	{ 
	} 
	
	TokenId InterpretedTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable) const 
	{ 
		// TODO: add token parameters and constraints
	    TokenId token = (new InterpretedToken(planDb, name, m_parameterNames, m_parameterTypes, rejectable, true))->getId(); 
	    return token; 
	} 
	
	TokenId InterpretedTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const 
	{ 
		// TODO: add token parameters and constraints
	    TokenId token = (new InterpretedToken(master, name, relation, m_parameterNames, m_parameterTypes , true))->getId(); 
	    return token; 
	 } 
	 
    /*
     * InterpretedRuleInstance
     */     	     
    InterpretedRuleInstance::InterpretedRuleInstance(const RuleId& rule, 
                                                     const TokenId& token, 
                                                     const PlanDatabaseId& planDb,
                                                     const std::vector<RuleExpr*>& body)
        : RuleInstance(rule, token, planDb)
        , m_body(body)
    {
    }

    InterpretedRuleInstance::~InterpretedRuleInstance()
    {
    }

    void InterpretedRuleInstance::handleExecute()
    {
        // TODO: need to pass in eval context from outside
	    EvalContext evalContext(NULL);
	    
        // TODO: Add object and token to the context
        
    	std::cout << "Executing interpreted rule:" << getRule()->getName().toString() << std::endl;
		for (unsigned int i=0; i < m_body.size(); i++) {
			m_body[i]->setRuleInstance(this);
			m_body[i]->eval(evalContext);
		}		
    	std::cout << "Executed interpreted rule:" << getRule()->getName().toString() << std::endl;
    }
    
    void InterpretedRuleInstance::createConstraint(const LabelStr& name, std::vector<ConstrainedVariableId>& vars)
    {
    	// TODO: Hardcoded constraint!
        vars.push_back(NDDL::var(getId(),std::string("duration")));
        vars.push_back(ruleVariable(IntervalIntDomain(10,10, "int")));
        addConstraint(LabelStr(name),vars);    	
    }

    void InterpretedRuleInstance::createSubgoal(const LabelStr& name,const LabelStr& predicateType, const LabelStr& relation)
    {
  		TokenId slave = TokenFactory::createInstance(m_token,LabelStr(predicateType),LabelStr(relation));
  		addSlave(slave,name);
  		
  		// TODO: for qualified names like "object.helloWorld" must add constraint to constraint the object variable on the slave token:
  		// for instance:
  		// TODO: hardcoded constraint!!!
  		{
  			std::vector<ConstrainedVariableId> vars;
            vars.push_back(NDDL::var(getId(),"object"));
            vars.push_back(slave->getObject());
            addConstraint(LabelStr("eq"),vars);    	
  		} 
  		
  		const char* relationName = relation.c_str();
  		
  		if (strcmp(relationName,"meets") == 0) {
  			meets("this",name);
  		} 
  		else if (strcmp(relationName,"met_by") == 0) {
  			met_by("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"contains") == 0) {
  			contains("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"before") == 0) {
  			before("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"after") == 0) {
  			after("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"starts") == 0) {
  			starts("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"ends") == 0) {
  			ends("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"ends_after") == 0) {
  			ends_after("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"ends_before") == 0) {
  			ends_before("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"ends_after_start") == 0) {
  			ends_after_start("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"starts_before_end") == 0) {
  			starts_before_end("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"starts_during") == 0) {
  			starts_during("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"contains_start") == 0) {
  			contains_start("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"ends_during") == 0) {
  			ends_during("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"contains_end") == 0) {
  			contains_end("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"starts_after") == 0) {
  			starts_after("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"starts_before") == 0) {
  			starts_before("this",name);
  		}   	  		   
  		else if (strcmp(relationName,"equals") == 0) {
  			equals("this",name);
  		}   	  		   
    }
    

    /*
     * InterpretedRuleFactory
     */     	     
    InterpretedRuleFactory::InterpretedRuleFactory(const LabelStr& predicate, 
                                                   const LabelStr& source, 
                                                   const std::vector<RuleExpr*>& body)
      : Rule(predicate,source)
      , m_body(body)
    {
    }
        
    InterpretedRuleFactory::~InterpretedRuleFactory()
    {
    }
        
    RuleInstanceId InterpretedRuleFactory::createInstance(
                                  const TokenId& token, 
                                  const PlanDatabaseId& planDb, 
                                  const RulesEngineId &rulesEngine) const
    {
    	// TODO: Pass expressions in Rule Body
        InterpretedRuleInstance *foo = new InterpretedRuleInstance(m_id, token, planDb, m_body);
        foo->setRulesEngine(rulesEngine);
        return foo->getId();
    }	
}

