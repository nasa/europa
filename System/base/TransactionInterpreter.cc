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
#include "DbClient.hh"
#include "EnumeratedTypeFactory.hh"
#include "Object.hh" 
#include "ObjectFactory.hh"
#include "TokenVariable.hh"
#include "TypeFactory.hh"
#include "Schema.hh"
#include "Utils.hh"

#include "NddlResource.hh"
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
 * TODO: temp variables : an easy way to deal with garbage collection could be to register them with the PlanDatabase when they're created
 * and flush every time we're done interpreting an XML statement
 * TODO: handle assignments and constraints in predicate declaration
 * TODO: add named subgoals to eval context
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
  	  //m_systemClasses.insert("Resource"); // TODO: export NddlResource
  	  m_systemClasses.insert("Reusable");
  	  m_systemTokens.insert("Reusable.uses");
  	  
  	  // TODO: this should be done once after the schema is initialized, not for every TransactionPlayer
  	  // TODO: check to make sure this is done only once
  	  createDefaultObjectFactory("Object", true);
      REGISTER_OBJECT_FACTORY(TimelineObjectFactory, Timeline);	    	    	  

      //REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource);	    	    	  

      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable);	    	    	  
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:int:int);	    	    	  
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:int:int:int);	    	    	  
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:int:int:int:int);	    	    	  
      new ReusableUsesTokenFactory("Reusable.uses");
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
    debugMsg("XMLInterpreter",dbgout.str());
  }
  
  void InterpretedDbClientTransactionPlayer::playDefineClass(const TiXmlElement& element) 
  {
    const char* className = element.Attribute("name");

    const char* parentClassName = element.Attribute("extends");
    parentClassName = (parentClassName == NULL ? "Object" : parentClassName);

    // TODO: should do nothing here for native classes, need to make PLasma.nddl consistent with this
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
            if (m_systemClasses.find(className) == m_systemClasses.end()) {
	    	    defineConstructor(schema,className,child);
	    	    definedConstructor = true;
            }
            else {	
            	// TODO: should always be displayed as WARNING!
    	        debugMsg("XMLInterpreter","Skipping constructor declaration for System class : " << className);
	        }	  
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
    debugMsg("XMLInterpreter",dbgout.str());     
  }

  Expr* InterpretedDbClientTransactionPlayer::valueToExpr(const TiXmlElement* element)
  {  	
    check_runtime_error(element != NULL,"Unexpected NULL element, expected value or id element");
    
    if (strcmp(element->Value(),"value") == 0) {
     	return new ExprConstant(m_client,element->Attribute("type"),xmlAsAbstractDomain(*element));
    }
    else if (strcmp(element->Value(),"id") == 0) {
      	const char* varName = element->Attribute("name");
      	return new ExprVariableRef(varName);
    }
    else
        check_runtime_error(ALWAYS_FAILS,std::string("Unexpected xml element:") + element->Value() + ", expected value or id element");
        
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
      	  		rhs = new ExprConstant(m_client,rhsChild->Attribute("type"),xmlAsAbstractDomain(*rhsChild));
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
	          	    	Expr* arg = new ExprConstant(m_client,argChild->Attribute("type"),xmlAsAbstractDomain(*argChild));
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
      std::vector<LabelStr> assignVars;
      std::vector<Expr*> assignValues;
      
      dbgout << ident << "predicate " <<  predName << "(";
      for(const TiXmlElement* predArg = element->FirstChildElement(); predArg; predArg = predArg->NextSiblingElement() ) {
      	if (strcmp(predArg->Value(),"var") == 0) {
	      const char* type = safeStr(predArg->Attribute("type"));	
	      const char* name = safeStr(predArg->Attribute("name"));	
	      dbgout << type << " " << name << ",";
          schema->addMember(predName.c_str(), type, name);
          parameterNames.push_back(name);
          parameterTypes.push_back(type);
      	}
      	if (strcmp(predArg->Value(),"assign") == 0) {
	      const char* type = safeStr(predArg->Attribute("type"));	
	      const char* name = safeStr(predArg->Attribute("name"));
	      bool inherited = (predArg->Attribute("inherited") != NULL ? true : false);	
	      dbgout << type << " " << name << ",";
	      if (!inherited)
              schema->addMember(predName.c_str(), type, name);
          assignVars.push_back(name);
          assignValues.push_back(valueToExpr(predArg->FirstChildElement()));                 
      	}
      	else if (strcmp(predArg->Value(),"invoke") == 0) {
      		dbgout << "constraint " << predArg->Attribute("name");
      		// TODO: deal with this
      		std::cerr << "Constraint in predicate declaration for predicate " << predName << " not supported yet" << std::endl;
      	}             
      }	
      dbgout << ")" << std::endl;

      if (m_systemTokens.find(predName) != m_systemTokens.end()) {
          // TODO: should always be displayed as WARNING!
	      debugMsg("XMLInterpreter","Skipping factory registration for System token : " << predName);     
      }       
      // The TokenFactory's constructor automatically registers the factory
      // TODO: pass along constraints
      new InterpretedTokenFactory(
          predName,
          parameterNames,
          parameterTypes,
          assignVars,
          assignValues
      );
  }
    
  void InterpretedDbClientTransactionPlayer::defineEnum(Id<Schema>& schema, const char* className,  const TiXmlElement* element)
  {	
  	  // Enum is scoped within the class but in the generated code it doesn't make a difference
  	  playDefineEnumeration(*element);
  }

  bool isClass(LabelStr className)
  {  	  
  	  return Schema::instance()->isObjectType(className);
  }
    
  LabelStr getTokenVarClass(LabelStr className,LabelStr var)
  {
      if (strcmp(var.c_str(),"object") == 0) {
      	return className;
      }
      else {
      	check_runtime_error(ALWAYS_FAILS,std::string("Undefined variable:")+var.c_str());
      	return LabelStr("");
      }
  }

  LabelStr getObjectVarClass(LabelStr className,LabelStr var)
  {
      const SchemaId& schema = Schema::instance();
      check_runtime_error(schema->hasMember(className,var),className.toString()+" has no member called "+var.toString());
      return schema->getMemberType(className,var);
  }

  /*
   * figures out the type of a predicate given an instance
   * 
   */
  LabelStr predicateInstanceToType(const char* className,const char* predicateInstance)
  {
  	// see ModelAccessor.getSlaveObjectType() in NDDL compiler  	
  	LabelStr str(predicateInstance);
  	
  	unsigned int tokenCnt = str.countElements(".");
  	
  	if (tokenCnt == 1) {
  		std::string retval = std::string(className)+"."+predicateInstance;
  		// TODO: make sure type is valid
  		return LabelStr(retval);
  	}
  	else if (tokenCnt == 2) {
  		LabelStr prefix(str.getElement(0,"."));
  		LabelStr suffix(str.getElement(1,"."));
  		
  		if (prefix.toString() == std::string("object")) {
      		std::string retval = std::string(className)+"."+suffix.toString();
  	    	// TODO: make sure type is valid
  		    return LabelStr(retval.c_str());
  		}
  		else if (isClass(prefix)) {
  	    	// TODO: make sure type is valid
  		     return LabelStr(predicateInstance);
  		}
  		else {
  		    check_error(ALWAYS_FAILS,std::string("Invalid predicate:") + predicateInstance);
            return LabelStr("NO_VALUE");
  		}
  	}
  	else {
  	    LabelStr var = str.getElement(0,".");
  	    // TODO: include token parameters and token local variables
  		LabelStr clazz = getTokenVarClass(className,var);
  		for (unsigned int i=1;i<tokenCnt-1;i++) {
  			LabelStr var = str.getElement(i,".");
  			clazz = getObjectVarClass(clazz,var);
  		}
  		
  		LabelStr predicate = str.getElement(tokenCnt-1,".");
  		std::string retval = clazz.toString() + "." + predicate.toString();  	    
  	    // TODO: make sure type is valid
  		return LabelStr(retval);  		
  	}  	  	
  }
  
  void InterpretedDbClientTransactionPlayer::playDefineCompat(const TiXmlElement& element)
  {
  	  const char* className = element.Attribute("class");
      std::string predName = std::string(className) + "." + element.Attribute("name");	
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

            const char* predicateType = predicateInstanceToType(className, predicateInstance).c_str();    
            if (name == NULL) {
            	std::ostringstream tmpname;
            	tmpname << "slave" << (slave_cnt++);            
                name = LabelStr(tmpname.str()).c_str();
            }                 
      		ruleBody.push_back(new ExprSubgoal(name,predicateType,predicateInstance,child->Attribute("relation")));
      	}
      	else if (strcmp(child->Value(),"var") == 0) {
      		LabelStr name(child->Attribute("name"));
      		LabelStr type(child->Attribute("type"));
      		// TODO: deal with domain restrictions ???
      		if (child->FirstChildElement() != NULL)
      	        check_runtime_error(ALWAYS_FAILS,std::string("Can't deal with domain restrictions for local var ") + name.toString());      		
      		ruleBody.push_back(new ExprLocalVar(name,type));
      	}
      	else if (strcmp(child->Value(),"if") == 0) {
      		// TODO: implement this
      		ruleBody.push_back(new ExprIf());
      	}
      	else 
      	   check_runtime_error(ALWAYS_FAILS,std::string("Unknown Compatibility element:") + child->Value());
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
    
    DataRef::DataRef()
        : m_value(ConstrainedVariableId::noId())
    {
    }
    
    DataRef::DataRef(const ConstrainedVariableId& v)
        : m_value(v)
    {
    }
    
    DataRef::~DataRef()
    {
    }
  	    
    ConstrainedVariableId& DataRef::getValue() { return m_value; }
    
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

  	void EvalContext::addVar(const char* name,const ConstrainedVariableId& v)
  	{
  		m_variables[name] = v;
  		debugMsg("XMLInterpreter","Added var:" << name << " to EvalContext");
  	} 

  	ConstrainedVariableId EvalContext::getVar(const char* name)
  	{
  		std::map<std::string,ConstrainedVariableId>::iterator it = 
  		    m_variables.find(name);
  		    
        if( it != m_variables.end() ) 
            return it->second;
  	    else if (m_parent != NULL)
  	    	return m_parent->getVar(name);
  	    else 
  	        return ConstrainedVariableId::noId();
  	}

  	void EvalContext::addToken(const char* name,const TokenId& t)
  	{
  		m_tokens[name] = t;
  	} 

  	TokenId EvalContext::getToken(const char* name)
  	{
  		std::map<std::string,TokenId>::iterator it = 
  		    m_tokens.find(name);
  		    
        if( it != m_tokens.end() ) 
            return it->second;
  	    else if (m_parent != NULL)
  	    	return m_parent->getToken(name);
  	    else 
  	        return TokenId::noId();
  	}

    std::string EvalContext::toString() const
    {
    	std::ostringstream os;
    	
    	if (m_parent == NULL)
    	    os << "EvalContext {" << std::endl;    
    	else
    	    os << m_parent->toString();
    	    
  		std::map<std::string,ConstrainedVariableId>::const_iterator varIt = m_variables.begin();    	
    	os << "    vars {";    
    	for (;varIt != m_variables.end();++varIt)
    	    os << varIt->first << " " << varIt->second->toString() << ",";
    	os << "    }" << std::endl;
    	
  		std::map<std::string,TokenId>::const_iterator tokenIt = m_tokens.begin();    	
    	os << "    tokens {";    
    	for (;tokenIt != m_tokens.end();++tokenIt)
    	    os << tokenIt->first << " " << tokenIt->second->getPredicateName().toString() << ",";
    	os << "    }"  << std::endl;
    	
    	if (m_parent == NULL)
    	    os << "}" << std::endl;
    	    
    	return os.str();        	    
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
  		ObjectId object = context.getVar("this")->derivedDomain().getSingletonValue();

  		std::vector<const AbstractDomain*> arguments;

        evalArgs(context,arguments);  		
  		ObjectFactory::evalConstructorBody(object,m_superClassName,arguments);
  		
  		return DataRef::null;
  	}      	 

    void ExprConstructorSuperCall::evalArgs(EvalContext& context, std::vector<const AbstractDomain*>& arguments) const
    {
  		for (unsigned int i=0; i < m_argExprs.size(); i++) {
			DataRef arg = m_argExprs[i]->eval(context);
			arguments.push_back(&(arg.getValue()->derivedDomain()));
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
  		ObjectId object = context.getVar("this")->derivedDomain().getSingletonValue();
     	check_error(object->getVariable(m_lhs) == ConstrainedVariableId::noId());
     	ConstrainedVariableId rhsValue = m_rhs->eval(context).getValue(); 
     	const AbstractDomain& domain = rhsValue->derivedDomain();
     	object->addVariable(domain,m_lhs);
     	debugMsg("XMLInterpreter","Initialized variable:" << object->getName().toString() << "." << m_lhs << " in constructor");
  		
  		return DataRef::null;
  	} 

    /*
     * ExprConstant
     */   
    ExprConstant::ExprConstant(DbClientId& dbClient, const char* type, const AbstractDomain* domain)
    {
		m_var = dbClient->createVariable(
		    type,
		    *domain,
		    "TMP_VAR",
		    true
		);    	
    }
    
  	ExprConstant::~ExprConstant()
  	{
  		// TODO: delete temp variable?
  	}

  	DataRef ExprConstant::eval(EvalContext& context) const
  	{
  		return DataRef(m_var);
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
  		// TODO: do this only once
  		std::vector<std::string> vars;
  		tokenize(m_varName.toString(),vars,".");
  		std::string varName=vars[0];
  		
  		unsigned int idx = 1;

        // TODO: this is confusing because variables at higher levels will hide tokens at lower levels. FIXME  		
     	ConstrainedVariableId rhs = context.getVar(vars[0].c_str());
        
     	if (rhs.isNoId()) {
     		TokenId tok = context.getToken(vars[0].c_str());
     		if (tok == TokenId::noId())  
     	        check_runtime_error(ALWAYS_FAILS,std::string("Couldn't find variable ")+varName+" in Evaluation Context");
     		
     		//debugMsg("XMLInterpreter", vars[0] << " is a token");
     		rhs = tok->getVariable(vars[1]);
     		varName += "." + vars[1];
         	if (rhs.isNoId()) 
     	        check_runtime_error(ALWAYS_FAILS,std::string("Couldn't find variable ")+varName+" in token of type " +tok->getPredicateName().toString());
     	        
     		idx=2;
     	}
     	else {
     		//debugMsg("XMLInterpreter", vars[0] << " is a variable");
     		//debugMsg("XMLInterpreter", context.toString());
     	}
     	
     	
     	for (;idx<vars.size();idx++) {
     		// TODO: should probably make sure it's an object var first
     		//debugMsg("XMLInterpreter","vars.size() is:" << vars.size() << " idx is:" << idx);
     		ObjectId object = rhs->derivedDomain().getSingletonValue();
     		rhs = object->getVariable(vars[idx]);
     		varName += "." + vars[idx];
         	if (rhs.isNoId()) 
     	        check_runtime_error(ALWAYS_FAILS,std::string("Couldn't find variable ")+varName+" in Evaluation Context");
     	}
     	    
     	return DataRef(rhs);
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
			arguments.push_back(&(arg.getValue()->derivedDomain()));
  		}
  		
  		// TODO: when this is the rhs of an assignment in a constructor, this must create an object specifying 
  		// the enclosing object (for which the constructor is being executed) as the parent.
  		// The way it is now, objects are created :
  		// - directly in C++ through new (in the generated code) if the stmt is inside a NDDL constructor
  		// - through the factories if the stmt is in the initial state
  		// This is a problem, everybody should go through the factory
  		// faking it for now, but this is a hack

  		// This assumes for now this is only called inside a constructor  		
  		ObjectId thisObject = context.getVar("this")->derivedDomain().getSingletonValue();
  		LabelStr name(std::string(thisObject->getName().toString() + "." + m_objectName.toString()));  		
     	ObjectId newObject = m_dbClient->createObject(
     	    m_objectType.c_str(), 
     	    name.c_str(), 
     	    arguments
     	);
     	newObject->setParent(thisObject);
     	 
	    return DataRef(newObject->getThis());
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
			vars.push_back(arg.getValue());
  		}

  		m_ruleInstance->createConstraint(m_name,vars);
  		debugMsg("XMLInterpreter","Evaluated Constraint : " << m_name.toString());
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
  		debugMsg("XMLInterpreter","Creating subgoal " << m_predicateType.toString() << ":" << m_name.toString());
  		TokenId slave = m_ruleInstance->createSubgoal(m_name,m_predicateType,m_predicateInstance,m_relation);
  		context.addToken(m_name.c_str(),slave);
  		debugMsg("XMLInterpreter","Created  subgoal " << m_predicateType.toString() << ":" << m_name.toString());
  		return DataRef::null;
  	}  
  	 
  	ExprLocalVar::ExprLocalVar(const LabelStr& name,const LabelStr& type)
  	    : m_name(name)
  	    , m_type(type)
  	{
  		m_baseDomain = TypeFactory::baseDomain(type.c_str()).copy();
  	}
  	
  	ExprLocalVar::~ExprLocalVar()
  	{
  	}

  	DataRef ExprLocalVar::eval(EvalContext& context) const
  	{
  		ConstrainedVariableId localVar = m_ruleInstance->addLocalVariable(
  		    *m_baseDomain,
  		    false, // can't be specified
  		    m_name
  		);
  		context.addVar(m_name.c_str(),localVar);
  		debugMsg("XMLInterpreter","Added RuleInstance local var:" << m_name.toString());
  		return DataRef::null;
  	}  
  	  
  	ExprIf::ExprIf()
  	{
  	}
  	
  	ExprIf::~ExprIf()
  	{
  	}

  	DataRef ExprIf::eval(EvalContext& context) const
  	{
  		std::cerr << "ERROR:if staments not supported yet" << std::endl;
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
	    evalConstructorBody(planDb->getClient(),instance,arguments);
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
    	  debugMsg("XMLInterpreter","Created Object:" << objectName.toString() << " type:" << objectType.toString());
	      return (new Object(planDb, objectType, objectName,true))->getId();
	  }
	  else {
	        check_error(m_superCallExpr != NULL, std::string("Failed to find factory for object ") + objectName.toString() + " of type "+objectType.toString());
	        // TODO: argumentsForSuper are eval'd twice, once here and once when the constructor body is eval'd
	        //  figure out how to do it only once
	    	std::vector<const AbstractDomain*> argumentsForSuper;
     	    EvalContext evalContext(NULL);
     	    m_superCallExpr->evalArgs(evalContext,argumentsForSuper);
	        ObjectId retval = ObjectFactory::makeNewObject(planDb, m_superCallExpr->getSuperClassName(), objectType, objectName,argumentsForSuper);
  		    return retval;
	  }
	}
	
	void InterpretedObjectFactory::evalConstructorBody(
	                                           DbClientId dbClient,
	                                           ObjectId& instance, 
	                                           const std::vector<const AbstractDomain*>& arguments) const
	{
        // TODO: need to pass in eval context from outside
	    EvalContext evalContext(NULL);
	    
        // Add new object and arguments to eval context
		evalContext.addVar("this",instance->getThis());
		
		for (unsigned int i=0;i<m_constructorArgNames.size();i++) {
    		std::ostringstream argName;
    		argName << "arg" << i;
			// TODO: destroy these temporary variables
			ConstrainedVariableId arg = dbClient->createVariable(
			    m_constructorArgTypes[i].c_str(),
			    *(arguments[i]),
			    argName.str().c_str(),
			    true
			);
			evalContext.addVar(m_constructorArgNames[i].c_str(),arg);
		}
		
		if (m_superCallExpr != NULL)
		    m_superCallExpr->eval(evalContext);
		
		for (unsigned int i=0; i < m_constructorBody.size(); i++) 
			m_constructorBody[i]->eval(evalContext);

    	// TODO: destroy temporary variables created for args

        /*		
		const std::vector<ConstrainedVariableId>& vars = instance->getVariables();
		debugMsg("XMLInterpreter","    Vars for " << m_className.toString() << " " << instance->getName().toString() << " are:";
		for (unsigned j=0; j < vars.size(); j++) {
			debugMsg("XMLInterpreter",vars[j]->getName().toString() << ",";
		}
		debugMsg("XMLInterpreter",std::endl;
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
	            debugMsg("XMLInterpreter","Used default initializer for " << m_className.toString() << "." << members[i].second.toString()); 
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
                     const std::vector<LabelStr>& assignVars,
                     const std::vector<Expr*>& assignValues,
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
    	commonInit(parameterNames, parameterTypes, assignVars, assignValues, close);
    }
  	                     
    InterpretedToken::InterpretedToken(const TokenId& master, 
                     const LabelStr& predicateName, 
                     const LabelStr& relation, 
                     const std::vector<LabelStr>& parameterNames,
                     const std::vector<LabelStr>& parameterTypes, 
                     const std::vector<LabelStr>& assignVars,
                     const std::vector<Expr*>& assignValues,
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
    	commonInit(parameterNames, parameterTypes, assignVars, assignValues, close);
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
                     const std::vector<LabelStr>& assignVars,
                     const std::vector<Expr*>& assignValues,
                     const bool& autoClose)
    {
    	// TODO: initialize parameters that have exprs 
    	// TODO: add any constraints included in the declaration
    	
        //debugMsg("XMLInterpreter","Token " << getName().toString() << " has " << parameterNames.size() << " parameters"); 
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
	            /*
	            debugMsg("XMLInterpreter","Token " << getName().toString() << " added Parameter " 
	                                               << parameterTypes[i].toString() << " " << parameterNames[i].toString());
	            */                                    
	        } 
	    }
	    
	    // Take care of initializations that were part of the predicate declaration
	    EvalContext context(NULL); // TODO: provide token context if we want to allow anything other than constants
	    for (unsigned int i=0; i < assignVars.size(); i++)
	        getVariable(assignVars[i])->restrictBaseDomain(assignValues[i]->eval(context).getValue()->baseDomain()); 
    	
        if (autoClose)
            close();    	
    }
      
    /*
     * InterpretedTokenFactory
     */     	     
    InterpretedTokenFactory::InterpretedTokenFactory(
                                       const LabelStr& predicateName,
                                       const std::vector<LabelStr>& parameterNames,
                                       const std::vector<LabelStr>& parameterTypes, 
                                       const std::vector<LabelStr>& assignVars,
                                       const std::vector<Expr*>& assignValues)
	    : ConcreteTokenFactory(predicateName) 
	    , m_parameterNames(parameterNames)
	    , m_parameterTypes(parameterTypes)
	    , m_assignVars(assignVars)
	    , m_assignValues(assignValues)
	{ 
	} 
	
	TokenId InterpretedTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable) const 
	{ 
	    TokenId token = (new InterpretedToken(
	        planDb, 
	        name, 
	        m_parameterNames, 
	        m_parameterTypes,
	        m_assignVars,
	        m_assignValues, 
	        rejectable, 
	        true))->getId(); 
	    return token; 
	} 
	
	TokenId InterpretedTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const 
	{ 
	    TokenId token = (new InterpretedToken(
	        master, 
	        name, 
	        relation, 
	        m_parameterNames, 
	        m_parameterTypes, 
	        m_assignVars,
	        m_assignValues, 
	        true))->getId(); 
	    return token; 
    } 
	
	
    /*
     * RuleInstanceEvalContext
     * Puts RuleInstance variables like duration, start, end, in context
     */     	     
  	 RuleInstanceEvalContext::RuleInstanceEvalContext(EvalContext* parent, const RuleInstanceId& ruleInstance) 
  	     : EvalContext(parent) 
  	     , m_ruleInstance(ruleInstance)
  	 {
  	 }
     
     RuleInstanceEvalContext::~RuleInstanceEvalContext()
     {
     }   	
  	    
  	 ConstrainedVariableId RuleInstanceEvalContext::getVar(const char* name)
  	 {
  	 	ConstrainedVariableId var = m_ruleInstance->getVariable(LabelStr(name));
  	 	
  	 	if (!var.isNoId()) {
  	 		//debugMsg("XMLInterpreter","Found var in rule instance:" << name);
  	 	    return var;
  	 	}  	 	    
  	 	else
  	 	    return EvalContext::getVar(name);
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
	    RuleInstanceEvalContext evalContext(NULL,getId());
	    
    	debugMsg("XMLInterpreter","Executing interpreted rule:" << getRule()->getName().toString());
		for (unsigned int i=0; i < m_body.size(); i++) {
			m_body[i]->setRuleInstance(this);
			m_body[i]->eval(evalContext);
		}		
    	debugMsg("XMLInterpreter","Executed  interpreted rule:" << getRule()->getName().toString());
    }
    
    void InterpretedRuleInstance::createConstraint(const LabelStr& name, std::vector<ConstrainedVariableId>& vars)
    {
        addConstraint(LabelStr(name),vars);    	
    }

  	// see ModelAccessor.isConstrained in Nddl compiler 
    bool isConstrained(const LabelStr& predicateInstance)
    {
    	unsigned int tokenCnt = predicateInstance.countElements(".");
    	
    	// If the predicate is not qualified that means it belongs to the object in scope
    	if (tokenCnt == 1)
    	    return true;
    	    
    	// If the prefix is a class, it means it can be any object instance, so it must not be constrained    
        LabelStr prefix(predicateInstance.getElement(0,"."));
        if (!isClass(prefix))
            return true;
    	
    	return false;
    }
    
    TokenId InterpretedRuleInstance::createSubgoal(
                                        const LabelStr& name,
                                        const LabelStr& predicateType, 
                                        const LabelStr& predicateInstance, 
                                        const LabelStr& relation)
    {
  		TokenId slave = TokenFactory::createInstance(m_token,LabelStr(predicateType),LabelStr(relation));
  		addSlave(slave,name);  		

  		// For qualified names like "object.helloWorld" must add constraint to the object variable on the slave token
  		// See RuleWriter.allocateSlave in Nddl compiler 
  		if (isConstrained(predicateInstance)) {
  			std::vector<ConstrainedVariableId> vars;
  			    
         	unsigned int tokenCnt = predicateInstance.countElements(".");
  		    // TODO: getting the object to constrain should be handled through a qualified var Expr
  			if (tokenCnt <= 2) { // equivalent of sameObject() in NddlRules.hh
                vars.push_back(NDDL::var(getId(),"object"));
  			}
  			else {  // equivalent of constrainObject() in NddlRules.hh
  				// TODO: this can be done more efficiently
  				int cnt = predicateInstance.countElements(".");
  				std::string prefix(predicateInstance.getElement(0,".").toString());
  				std::string tokenName(predicateInstance.getElement(cnt-1,".").toString());
  				std::string asString = predicateInstance.toString();
  				std::string suffix = asString.substr(prefix.size()+1,asString.size()-(prefix.size()+tokenName.size()+2));
  				//debugMsg("XMLInterpreter","Subgoal slave object constraint. prefix=" << prefix << " suffix=" << suffix << " tokenName=" << tokenName);
                vars.push_back(varFromObject(prefix,suffix));
  			}
  			
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
  		
  		return slave; 			
    }
    
    ConstrainedVariableId InterpretedRuleInstance::addLocalVariable( 
                       const AbstractDomain& baseDomain,
				       bool canBeSpecified,
				       const LabelStr& name)                   
	{
        ConstrainedVariableId localVariable = TypeFactory::createVariable(
            baseDomain.getTypeName().c_str(),
            m_planDb->getConstraintEngine(),
		    baseDomain,
		    canBeSpecified,
		    name.c_str(),
		    m_id,
		    m_variables.size()
	    )->getId();
		  
        // Only allowed add a variable for an executed rule instance
        check_error(isExecuted());

        m_variables.push_back(localVariable);
        addVariable(localVariable, name);
        return localVariable;
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
    
    
    // Factories for exported C++ classes
  	TimelineObjectFactory::TimelineObjectFactory(const LabelStr& signature) 
  	    : NativeObjectFactory("Timeline",signature) 
  	{
  	} 
    
    TimelineObjectFactory::~TimelineObjectFactory() 
  	{
  	} 

   	ObjectId TimelineObjectFactory::makeNewObject( 
	                        const PlanDatabaseId& planDb, 
	                        const LabelStr& objectType, 
	                        const LabelStr& objectName, 
	                        const std::vector<const AbstractDomain*>& arguments) const 
	{ 
	    ObjectId instance =  (new Timeline(planDb, objectType, objectName,true))->getId();
	    debugMsg("XMLInterpreter","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString()); 
	    
	    return instance; 
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
	   	debugMsg("XMLInterpreter","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString()); 

    	return instance;	
    }   
    
	TokenId ReusableUsesTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable) const
	{
		return new NDDL::NddlReusable::uses(planDb,name,rejectable,true);
	}
	
	TokenId ReusableUsesTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
	{
		return new NDDL::NddlReusable::uses(master,name,relation,true);
	}        
}

/*
 * Here is what can be used as a main to run an interpreted version of NDDL-XML through PSEngine
 * It's also possible to run NDDL (what is support it of it so far) fully interpreted
 * through the java version of PSEngine, see NDDLHelloWorld example in PlanWorks/PSUI  

#include "Nddl.hh" 
#include "SolverAssembly.hh" 
#include "PSEngine.hh" 
#include "Debug.hh"

using namespace EUROPA;

bool runPSEngineTest(const char* plannerConfig, const char* txSource);
void printFlaws(int it, PSList<std::string>& flaws);

int main(int argc, const char ** argv)
{
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file and planner config file ." << std::endl;
    return -1;
  }

  const char* txSource = argv[1];
  const char* plannerConfig = argv[2];
  
  if (!runPSEngineTest(plannerConfig,txSource)) 
      return -1;
      
  return 0;
}
 
bool runPSEngineTest(const char* plannerConfig, const char* txSource)
{
    try {
	  PSEngine engine;
	
	  engine.start();
	  engine.executeTxns(txSource,true,true);
	
	  PSSolver* solver = engine.createSolver(plannerConfig);
	  solver->configure(0,100);
	
	  for (int i = 0; i<50; i++) {
		solver->step();
		PSList<std::string> flaws = solver->getFlaws();
		if (flaws.size() == 0)
		    break;
		printFlaws(i,flaws);
	  }
	
	  delete solver;	
	  engine.shutdown();

	  return true;
	}
	catch (Error& e) {
		std::cerr << "PSEngine failed:" << e.getMsg() << std::endl;
		return false;
	}	
}

void printFlaws(int it, PSList<std::string>& flaws)
{
	debugMsg("XMLInterpreter","Iteration:" << it << " " << flaws.size() << " flaws");
	
	for (int i=0; i<flaws.size(); i++) {
		debugMsg("XMLInterpreter","    " << (i+1) << " - " << flaws.get(i));
	}
}

*/
  

