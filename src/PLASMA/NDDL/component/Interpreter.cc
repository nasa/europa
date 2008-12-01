/**
 * @file Interpreter.cc
 * @brief Core classes to interpret XML statements
 * @author Javier Barreiro
 * @date December, 2006
 */

#include "Interpreter.hh"

#include "Debug.hh"
#include "Error.hh"

#include "ConstraintFactory.hh"
#include "DbClient.hh"
#include "EnumeratedDomain.hh"
#include "EnumeratedTypeFactory.hh"
#include "Object.hh"
#include "ObjectFactory.hh"
#include "TokenVariable.hh"
#include "TypeFactory.hh"
#include "Schema.hh"
#include "Utils.hh"

#include "NddlRules.hh"
#include "NddlUtils.hh"

#include <string.h>

// Hack!! the macro in NddlRules.hh only works with code-generation
// so this is redefined here. this is brittle though
// TODO: change code-generation code to work with this macro instead
namespace NDDL {
#ifdef relation
#undef relation
#endif

#define relation(relationname, origin, originvar, target, targetvar) {	\
    std::vector<ConstrainedVariableId> vars;				\
    vars.push_back(getSlave(LabelStr(origin))->originvar());	\
    vars.push_back(getSlave(LabelStr(target))->targetvar());	\
    rule_constraint(relationname,vars);					\
  }
}


namespace EUROPA {

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
    debugMsg("XMLInterpreter:EvalContext","Added var:" << name << " to EvalContext");
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

  ExprList::ExprList()
  {
  }

  ExprList::~ExprList()
  {
      for (unsigned int i=0;i<m_children.size();i++)
          delete m_children[i];
  }

  DataRef ExprList::eval(EvalContext& context) const
  {
      DataRef result;

      for (unsigned int i=0;i<m_children.size();i++)
          result = m_children[i]->eval(context);

      return result;
  }

  void ExprList::addChild(Expr* child)
  {
      m_children.push_back(child);
  }

  ExprNoop::ExprNoop(const std::string& str)
      : m_str(str)
  {
  }

  ExprNoop::~ExprNoop()
  {
  }

  DataRef ExprNoop::eval(EvalContext& context) const
  {
      std::cout << "Noop:" << m_str << std::endl;
      return DataRef::null;
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
    ObjectFactoryId objFactory = object->getPlanDatabase()->getSchema()->getObjectFactory(m_superClassName,arguments);
    objFactory->evalConstructorBody(object,arguments);

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
    object->addVariable(domain,m_lhs.c_str());
    debugMsg("XMLInterpreter:InterpretedObject","Initialized variable:" << object->getName().toString() << "." << m_lhs.toString() << " to " << rhsValue->derivedDomain().toString() << " in constructor");

    return DataRef::null;
  }

  /*
   * ExprConstant
   */
  ExprConstant::ExprConstant(DbClientId& dbClient, const char* type, const AbstractDomain* domain)
    : m_dbClient(dbClient)
    , m_type(type)
    , m_domain(domain)
  {
  }

  ExprConstant::~ExprConstant()
  {
  }

  DataRef ExprConstant::eval(EvalContext& context) const
  {
    // need to create a new variable every time this is evaluated, since propagation
    // will affect the variable, should provide immutable variables so that a single var
    // can be created for a constant
    // TODO: Who destroys this variable?, need to give handle to RuleInstance
    ConstrainedVariableId var = m_dbClient->createVariable(
							   m_type.c_str(),
							   *m_domain,
							   "TMP_VAR",
							   true, // isTmp
							   false // cannot be specified
							   );

    return DataRef(var);
  }


  /*
   * ExprVariableRef
   */
  ExprVariableRef::ExprVariableRef(const char* varName, const SchemaId& schema)
    : m_varName(varName)
    , m_schema(schema)
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

    ConstrainedVariableId rhs = context.getVar(vars[0].c_str());

    if (rhs.isNoId())
      check_runtime_error(ALWAYS_FAILS,std::string("Couldn't find variable ")+varName+" in Evaluation Context");

    for (unsigned int idx = 1;idx<vars.size();idx++) {
      check_error(m_schema->isObjectType(rhs->baseDomain().getTypeName()), std::string("Can't apply dot operator to:")+rhs->baseDomain().getTypeName().toString());
      check_runtime_error(rhs->derivedDomain().isSingleton(),varName+" must be singleton to be able to get to "+vars[idx]);
      ObjectId object = rhs->derivedDomain().getSingletonValue();
      rhs = object->getVariable(object->getName().toString()+"."+vars[idx]);
      varName += "." + vars[idx];

      if (rhs.isNoId())
	      check_runtime_error(ALWAYS_FAILS,std::string("Couldn't find variable ")+vars[idx]+
			    " in object \""+object->getName().toString()+"\" of type "+object->getType().toString());
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

    // when this is the rhs of an assignment in a constructor, an object var must be created to specify
    // the enclosing object (for which the constructor is being executed) as the parent.
    // TODO: The way it is now, when using code generation objects are created :
    // - directly in C++ through new (in the generated code) if the stmt is inside a NDDL constructor
    // - through the factories if the stmt is in the initial state
    // This is a problem, everybody should go through the factory
    // faking it for now, but this is a hack

    // This assumes for now this is only called inside a constructor, not as part of an initial-state
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


  /*
   * ExprRuleVariableRef
   */
  ExprRuleVariableRef::ExprRuleVariableRef(const char* varName)
    : m_varName(varName)
  {
    std::vector<std::string> vars;
    tokenize(m_varName,vars,".");

    if (vars.size() > 1) {
      m_parentName = vars[0];
      m_varName = m_varName.substr(m_parentName.length()+1);
      debugMsg("XMLInterpreter:InterpretedRule","Split " << varName << " into " << m_parentName << " and " << m_varName);
    }
    else {
      m_parentName = "";
      debugMsg("XMLInterpreter:InterpretedRule","Didn't split " << varName);
    }
  }

  ExprRuleVariableRef::~ExprRuleVariableRef()
  {
  }

  DataRef ExprRuleVariableRef::doEval(RuleInstanceEvalContext& context) const
  {
    ConstrainedVariableId var;

    if (m_parentName == "") {
      var = context.getVar(m_varName.c_str());
      check_runtime_error(!var.isNoId(),std::string("Couldn't find variable ")+m_varName+" in Evaluation Context");
    }
    else {
      TokenId tok = context.getToken(m_parentName.c_str());
      if (!tok.isNoId())
	var = context.getRuleInstance()->varfromtok(tok,m_varName);
      else {
	var = context.getVar(m_parentName.c_str());
	if (!var.isNoId())
	  var = context.getRuleInstance()->varFromObject(m_parentName,m_varName,false);
	else
	  check_runtime_error(ALWAYS_FAILS,std::string("Couldn't find variable ")+m_parentName+" in Evaluation Context");
      }
    }

    return DataRef(var);
  }

  ExprConstraint::ExprConstraint(const char* name,const std::vector<Expr*> args)
    : m_name(name)
    , m_args(args)
  {
  }

  ExprConstraint::~ExprConstraint()
  {
  }

  std::string varsToString(const std::vector<ConstrainedVariableId>& vars)
  {
    std::ostringstream os;
    for (unsigned int i=0; i < vars.size(); i++) {
      if (i>0) os << ",";
      os << vars[i]->toString();
    }

    return os.str();
  }

  DataRef ExprConstraint::doEval(RuleInstanceEvalContext& context) const
  {
    std::vector<ConstrainedVariableId> vars;
    for (unsigned int i=0; i < m_args.size(); i++) {
      DataRef arg = m_args[i]->eval(context);
      vars.push_back(arg.getValue());
    }

    context.getRuleInstance()->createConstraint(m_name,vars);
    debugMsg("XMLInterpreter:InterpretedRule","Evaluated Constraint : " << m_name.toString() << " - " << varsToString(vars));
    return DataRef::null;
  }


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

  // see ModelAccessor.isConstrained in Nddl compiler
  bool ExprSubgoal::isConstrained(RuleInstanceEvalContext& context, const LabelStr& predicateInstance) const
  {
    unsigned int tokenCnt = predicateInstance.countElements(".");

    // If the predicate is not qualified that means it belongs to the object in scope
    if (tokenCnt == 1)
      return true;

    // If the prefix is a class, it means it can be any object instance, so it must not be constrained
    LabelStr prefix(predicateInstance.getElement(0,"."));
    if (!context.isClass(prefix))
      return true;

    return false;
  }

  DataRef ExprSubgoal::doEval(RuleInstanceEvalContext& context) const
  {
    debugMsg("XMLInterpreter:InterpretedRule","Creating subgoal " << m_predicateType.toString() << ":" << m_name.toString());

    bool constrained = isConstrained(context,m_predicateInstance);
    ConstrainedVariableId owner;
    if (constrained) {
      unsigned int tokenCnt = m_predicateInstance.countElements(".");
      if (tokenCnt == 1)
	owner = context.getVar("object");
      else
	owner = context.getVar(m_predicateInstance.getElement(0,".").c_str());
    }

    TokenId slave = context.getRuleInstance()->createSubgoal(
							     m_name,
							     m_predicateType,
							     m_predicateInstance,
							     m_relation,
							     constrained,
							     owner
							     );

    context.addToken(m_name.c_str(),slave);
    debugMsg("XMLInterpreter:InterpretedRule","Created  subgoal " << m_predicateType.toString() << ":" << m_name.toString());
    return DataRef::null;
  }

  ExprLocalVar::ExprLocalVar(const LabelStr& name,
                             const LabelStr& type,
                             bool guarded,
                             Expr* domainRestriction,
                             const AbstractDomain& baseDomain)
    : m_name(name)
    , m_type(type)
    , m_guarded(guarded)
    , m_domainRestriction(domainRestriction)
    , m_baseDomain(baseDomain)
  {
  }

  ExprLocalVar::~ExprLocalVar()
  {
  }

  DataRef ExprLocalVar::doEval(RuleInstanceEvalContext& context) const
  {
    ConstrainedVariableId localVar;
    if (context.isClass(m_type))
      localVar = context.getRuleInstance()->addObjectVariable(
							      m_type,
							      ObjectDomain(m_type.c_str()),
							      m_guarded, // can't be specified
							      m_name
							      );
    else
      localVar = context.getRuleInstance()->addLocalVariable(
							     m_baseDomain,
							     m_guarded, // can't be specified
							     m_name
							     );

    if (m_domainRestriction != NULL)
      localVar->restrictBaseDomain(m_domainRestriction->eval(context).getValue()->derivedDomain());

    context.addVar(m_name.c_str(),localVar);
    debugMsg("XMLInterpreter:InterpretedRule","Added RuleInstance local var:" << localVar->toString());
    return DataRef::null;
  }

  ExprIf::ExprIf(const char* op, Expr* lhs,Expr* rhs,const std::vector<RuleExpr*>& ifBody)
    : m_op(op)
    , m_lhs(lhs)
    , m_rhs(rhs)
    , m_ifBody(ifBody)
  {
  }

  ExprIf::~ExprIf()
  {
  }

  DataRef ExprIf::doEval(RuleInstanceEvalContext& context) const
  {
    bool isOpEquals = (m_op == "equals");

    DataRef lhs = m_lhs->eval(context);

    // TODO: this assumes that the variable is always on the lhs and the value on the rhs
    // is this enforced by the parser?

    if (m_rhs != NULL) {
      DataRef rhs = m_rhs->eval(context);
      context.getRuleInstance()->addChildRule(
					      new InterpretedRuleInstance(
									  context.getRuleInstance()->getId(),
									  lhs.getValue(),
									  rhs.getValue()->lastDomain(),
									  isOpEquals,
									  m_ifBody
									  )
					      );
      debugMsg("XMLInterpreter:InterpretedRule","Evaluated IF " << m_op << " " << lhs.getValue()->toString() << " " << rhs.getValue()->toString());
    }
    else {
      context.getRuleInstance()->addChildRule(
					      new InterpretedRuleInstance(
									  context.getRuleInstance()->getId(),
									  makeScope(lhs.getValue()),
									  isOpEquals,
									  m_ifBody
									  )
					      );
      debugMsg("XMLInterpreter:InterpretedRule","Evaluated IF " << m_op << " " << lhs.getValue()->toString());
    }

    return DataRef::null;
  }

  ExprLoop::ExprLoop(const char* varName, const char* varType, const char* varValue,const std::vector<RuleExpr*>& loopBody)
    : m_varName(varName)
    , m_varType(varType)
    , m_varValue(varValue)
    , m_loopBody(loopBody)
  {
  }

  ExprLoop::~ExprLoop()
  {
  }

  DataRef ExprLoop::doEval(RuleInstanceEvalContext& context) const
  {
    context.getRuleInstance()->executeLoop(context,m_varName,m_varType,m_varValue,m_loopBody);
    debugMsg("XMLInterpreter:InterpretedRule","Evaluated LOOP " << m_varName.toString() << "," << m_varValue.toString());
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
    : ObjectFactory(signature)
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

  class ObjectFactoryEvalContext : public EvalContext
  {
  public:
    ObjectFactoryEvalContext(const PlanDatabaseId& planDb,
			     const std::vector<std::string>& argNames,
			     const std::vector<std::string>& argTypes,
			     const std::vector<const AbstractDomain*>& args)
      : EvalContext(NULL) // TODO: should pass in eval context from outside to have access to globals
    {
			debugMsg("ObjectFactoryEvalContext", ">> ");
      // Add arguments to eval context
      for (unsigned int i=0;i<argNames.size();i++) {
				debugMsg("ObjectFactoryEvalContext:createVariable", argTypes[i] << " " << argNames[i] << " = " << *(args[i]));
	      ConstrainedVariableId arg = planDb->getClient()->createVariable(
									argTypes[i].c_str(),
									*(args[i]),
									argNames[i].c_str(),
									true
									);
	       m_tmpVars.push_back(arg);
	       addVar(argNames[i].c_str(),arg);
      }
			debugMsg("ObjectFactoryEvalContext", "<< ");
    }

    virtual ~ObjectFactoryEvalContext()
    {
      for (unsigned int i=0;i<m_tmpVars.size();i++) {
	      // TODO: must release temporary vars, causing crash?
	      //m_tmpVars[i].release();
      }
    }

  protected:
    std::vector<ConstrainedVariableId> m_tmpVars;
  };

  ObjectId InterpretedObjectFactory::createInstance(
						    const PlanDatabaseId& planDb,
						    const LabelStr& objectType,
						    const LabelStr& objectName,
						    const std::vector<const AbstractDomain*>& arguments) const
  {
    check_runtime_error(checkArgs(arguments));

    debugMsg("InterpretedObjectFactory:createInstance", "Creating instance for type " << objectType.toString() << " with name " << objectName.toString());

	ObjectId instance = makeNewObject(planDb, objectType, objectName,arguments);
	evalConstructorBody(instance,arguments);
	instance->close();

    debugMsg("InterpretedObjectFactory:createInstance", "Created instance " << instance->toString() << " for type " << objectType.toString() << " with name " << objectName.toString());

    return instance;
  }

  bool InterpretedObjectFactory::checkArgs(const std::vector<const AbstractDomain*>& arguments) const
  {
    // TODO: implement this. is this even necessary?, parser should take care of it
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
      debugMsg("XMLInterpreter:InterpretedObject","Created Object:" << objectName.toString() << " type:" << objectType.toString());
      return (new Object(planDb, objectType, objectName,true))->getId();
    }
    else {
      check_error(m_superCallExpr != NULL, std::string("Failed to find factory for object ") + objectName.toString() + " of type "+objectType.toString());

	  ObjectFactoryEvalContext evalContext(
	      planDb,
		  m_constructorArgNames,
		  m_constructorArgTypes,
		  arguments
      );

      // TODO: argumentsForSuper are eval'd twice, once here and once when m_superCallExpr->eval(evalContext) is called
      //  figure out how to do it only once
      std::vector<const AbstractDomain*> argumentsForSuper;
      m_superCallExpr->evalArgs(evalContext,argumentsForSuper);

      ObjectFactoryId objFactory = planDb->getSchema()->getObjectFactory(m_superCallExpr->getSuperClassName(),argumentsForSuper);
      ObjectId retval = objFactory->makeNewObject(
          planDb,
          objectType,
          objectName,
          argumentsForSuper
      );

      return retval;
    }
  }

	void InterpretedObjectFactory::evalConstructorBody(
	                                           ObjectId& instance,
	                                           const std::vector<const AbstractDomain*>& arguments) const
	{
		// TODO: should pass in eval context from outside to have access to globals
	    ObjectFactoryEvalContext factoryEvalContext(
	        instance->getPlanDatabase(),
		    m_constructorArgNames,
		    m_constructorArgTypes,
		    arguments
        );
	    ObjectEvalContext evalContext(&factoryEvalContext,instance);

        if (m_superCallExpr != NULL)
    	    m_superCallExpr->eval(evalContext);

        for (unsigned int i=0; i < m_constructorBody.size(); i++)
            m_constructorBody[i]->eval(evalContext);

        // Initialize any variables that were not explicitly initialized
        const Schema::NameValueVector& members = instance->getPlanDatabase()->getSchema()->getMembers(m_className);
        for (unsigned int i=0; i < members.size(); i++) {
            std::string varName = instance->getName().toString() + "." + members[i].second.toString();
            if (instance->getVariable(varName) == ConstrainedVariableId::noId()) {
	            const AbstractDomain& baseDomain =
	                instance->getPlanDatabase()->getConstraintEngine()->getCESchema()->baseDomain(members[i].first.c_str());
	            instance->addVariable(
			      baseDomain,
			      members[i].second.c_str()
			    );
	            debugMsg("XMLInterpreter:InterpretedObject","Used default initializer for " << m_className.toString() << "." << members[i].second.toString());
            }
        }

        debugMsg("XMLInterpreter:evalConstructorBody",
	             "Evaluated constructor for " << instance->toString());
    }

	  /*
	   * ObjectEvalContext
	   * Puts Object member variables in context
	   */
	  ObjectEvalContext::ObjectEvalContext(EvalContext* parent, const ObjectId& objInstance)
	    : EvalContext(parent)
	    , m_obj(objInstance)
	  {
	  }

	  ObjectEvalContext::~ObjectEvalContext()
	  {
	  }

	  ConstrainedVariableId ObjectEvalContext::getVar(const char* name)
	  {
	    if (strcmp(name,"this") == 0)
	        return m_obj->getThis();

	    ConstrainedVariableId var = m_obj->getVariable(m_obj->getName().toString()+"."+name);

	    if (!var.isNoId()) {
	      debugMsg("XMLInterpreter:EvalContext:Object","Found var in object instance:" << name);
	      return var;
	    }
	    else {
	      debugMsg("XMLInterpreter:EvalContext:Object","Didn't find var in object instance:" << name);
	      return EvalContext::getVar(name);
	    }
	  }

    /*
     * InterpretedToken
     */
    InterpretedToken::InterpretedToken(const PlanDatabaseId& planDatabase,
                     const LabelStr& predicateName,
                     const std::vector<LabelStr>& parameterNames,
                     const std::vector<LabelStr>& parameterTypes,
                     const std::vector<Expr*>& parameterValues,
                     const std::vector<LabelStr>& assignVars,
                     const std::vector<Expr*>& assignValues,
                     const std::vector<ExprConstraint*>& constraints,
                     const bool& rejectable,
                     const bool& isFact,
                     const bool& close)
      : IntervalToken(planDatabase,
                        predicateName,
                        rejectable,
                        isFact,
                        IntervalIntDomain(),                  // start
                        IntervalIntDomain(),                  // end
                        IntervalIntDomain(1, PLUS_INFINITY),  // duration
                        Token::noObject(),                    // Object Name
                        false)
    {
    	commonInit(parameterNames, parameterTypes, parameterValues, assignVars, assignValues, constraints, close);
    	debugMsg("XMLInterpreter:InterpretedToken","Created token(" << getKey() << ") of type:" << predicateName.toString() << " objectVar=" << getVariable("object")->toString());
    }

  InterpretedToken::InterpretedToken(const TokenId& master,
				     const LabelStr& predicateName,
				     const LabelStr& relation,
				     const std::vector<LabelStr>& parameterNames,
				     const std::vector<LabelStr>& parameterTypes,
				     const std::vector<Expr*>& parameterValues,
				     const std::vector<LabelStr>& assignVars,
				     const std::vector<Expr*>& assignValues,
				     const std::vector<ExprConstraint*>& constraints,
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
    commonInit(parameterNames, parameterTypes, parameterValues, assignVars, assignValues, constraints, close);
    debugMsg("XMLInterpreter:InterpretedToken","Created slave token(" << getKey() << ") of type:" << predicateName.toString() << " objectVar=" << getVariable("object")->toString());
  }

  InterpretedToken::~InterpretedToken()
  {
  }

  // slight modification of the version in NddlRules.hh that can be used with a variable name
  // instead of a literal
  // TODO: make code generator use this instead
#define token_constraint1(name, vars)					\
  {									\
    ConstraintId c0 = m_planDatabase->getConstraintEngine()->createConstraint(		\
							  name,		\
							  vars);	\
    m_standardConstraints.insert(c0);					\
  }

  void InterpretedToken::commonInit(
				    const std::vector<LabelStr>& parameterNames,
				    const std::vector<LabelStr>& parameterTypes,
				    const std::vector<Expr*>& parameterValues,
				    const std::vector<LabelStr>& assignVars,
				    const std::vector<Expr*>& assignValues,
				    const std::vector<ExprConstraint*>& constraints,
				    const bool& autoClose)
  {
    debugMsg("XMLInterpreter","Token " << getName().toString() << " has " << parameterNames.size() << " parameters");

    // TODO: Pass in EvalContext
    TokenEvalContext context(NULL,getId()); // TODO: give access to class or global context?

    for (unsigned int i=0; i < parameterNames.size(); i++) {
      check_runtime_error(getVariable(parameterNames[i],false) == ConstrainedVariableId::noId(), "Token parameter "+parameterNames[i].toString()+ " already exists!");

      // This is a hack needed because TokenVariable is parametrized by the domain arg to addParameter
      ConstrainedVariableId parameter;

      // same as completeObjectParam in NddlRules.hh
      if(parameterValues[i] != NULL) {
        parameter = addParameter(
                                 parameterValues[i]->eval(context).getValue()->baseDomain(),
                                 parameterNames[i]
                                );
        if (context.isClass(parameterTypes[i]))
          getPlanDatabase()->makeObjectVariableFromType(parameterTypes[i], parameter);
      }
      else {
        if (context.isClass(parameterTypes[i])) {
          parameter = addParameter(
                                   ObjectDomain(parameterTypes[i].c_str()),
                                   parameterNames[i]
                                  );
          getPlanDatabase()->makeObjectVariableFromType(parameterTypes[i], parameter);
        }
        else {
          parameter = addParameter(
                                   getPlanDatabase()->getConstraintEngine()->getCESchema()->baseDomain(parameterTypes[i].c_str()),
                                   parameterNames[i]
                                  );
        }
      }

      debugMsg("XMLInterpreter:InterpretedToken","Token " << getName().toString() << " added Parameter "
	       << parameter->toString() << " " << parameterNames[i].toString());
    }

    if (autoClose)
      close();


    // Take care of initializations that were part of the predicate declaration
    for (unsigned int i=0; i < assignVars.size(); i++)
      getVariable(assignVars[i])->restrictBaseDomain(assignValues[i]->eval(context).getValue()->baseDomain());

    // Post parameter constraints
    for (unsigned int i=0; i < constraints.size(); i++) {
      const std::vector<Expr*>& args = constraints[i]->getArgs();
      std::vector<ConstrainedVariableId> constraintArgs;
      for (unsigned int j=0; j < args.size(); j++) {
	DataRef arg = args[j]->eval(context);
	constraintArgs.push_back(arg.getValue());
      }
      token_constraint1(constraints[i]->getName(),constraintArgs);
    }
  }

  /*
   * InterpretedTokenFactory
   */
  InterpretedTokenFactory::InterpretedTokenFactory(
						   const LabelStr& predicateName,
						   const std::vector<LabelStr>& parameterNames,
						   const std::vector<LabelStr>& parameterTypes,
						   const std::vector<Expr*>& parameterValues,
						   const std::vector<LabelStr>& assignVars,
						   const std::vector<Expr*>& assignValues,
						   const std::vector<ExprConstraint*>& constraints)
    : TokenFactory(predicateName)
    , m_parameterNames(parameterNames)
    , m_parameterTypes(parameterTypes)
    , m_parameterValues(parameterValues)
    , m_assignVars(assignVars)
    , m_assignValues(assignValues)
    , m_constraints(constraints)
  {
  }

	TokenId InterpretedTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
	{
	    TokenId token = (new InterpretedToken(
	        planDb,
	        name,
	        m_parameterNames,
	        m_parameterTypes,
	        m_parameterValues,
	        m_assignVars,
	        m_assignValues,
	        m_constraints,
	        rejectable,
	        isFact,
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
					  m_parameterValues,
					  m_assignVars,
					  m_assignValues,
					  m_constraints,
					  true))->getId();
    return token;
  }


  /*
   * RuleInstanceEvalContext
   * Puts RuleInstance variables like duration, start, end, in context
   */
  RuleInstanceEvalContext::RuleInstanceEvalContext(EvalContext* parent, const InterpretedRuleInstanceId& ruleInstance)
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
      debugMsg("XMLInterpreter:EvalContext:RuleInstance","Found var in rule instance:" << name);
      return var;
    }
    else {
      debugMsg("XMLInterpreter:EvalContext:RuleInstance","Didn't find var in rule instance:" << name);
      return EvalContext::getVar(name);
    }
  }

  TokenId RuleInstanceEvalContext::getToken(const char* name)
  {
	  LabelStr ls_name(name);
      TokenId tok = m_ruleInstance->getSlave(ls_name);
      if (!tok.isNoId()) {
          debugMsg("XMLInterpreter:EvalContext:RuleInstance","Found token in rule instance:" << name);
          return tok;
      }
      else {
        debugMsg("XMLInterpreter:EvalContext:RuleInstance","Didn't find token in rule instance:" << name);
        return EvalContext::getToken(name);
      }
  }

  bool RuleInstanceEvalContext::isClass(const LabelStr& className) const
  {
     return m_ruleInstance->getPlanDatabase()->getSchema()->isObjectType(className);
  }

  std::string RuleInstanceEvalContext::toString() const
  {
    std::ostringstream os;

    os << EvalContext::toString();

    os << "Token variables {";
    const std::vector<ConstrainedVariableId>& vars = m_ruleInstance->getToken()->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end(); ++it){
      ConstrainedVariableId var = *it;
      os << var->getName().toString() << "," ;
    }
    os << "}" << std::endl;

    return os.str();
  }

  /*
   * RuleInstanceEvalContext
   * Puts Token variables like duration, start, end, in context
   */
  TokenEvalContext::TokenEvalContext(EvalContext* parent, const TokenId& token)
    : EvalContext(parent)
    , m_token(token)
  {
  }

  TokenEvalContext::~TokenEvalContext()
  {
  }

  ConstrainedVariableId TokenEvalContext::getVar(const char* name)
  {
    ConstrainedVariableId var = m_token->getVariable(LabelStr(name));

    if (!var.isNoId()) {
      debugMsg("XMLInterpreter:EvalContext:Token","Found var in token :" << name);
      return var;
    }
    else
      return EvalContext::getVar(name);
  }

  bool TokenEvalContext::isClass(const LabelStr& className) const
  {
     return m_token->getPlanDatabase()->getSchema()->isObjectType(className);
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

  InterpretedRuleInstance::InterpretedRuleInstance(
						   const RuleInstanceId& parent,
						   const ConstrainedVariableId& var,
						   const AbstractDomain& domain,
						   const bool positive,
						   const std::vector<RuleExpr*>& body)
    : RuleInstance(parent,var,domain,positive)
    , m_body(body)
  {
  }

  InterpretedRuleInstance::InterpretedRuleInstance(
						   const RuleInstanceId& parent,
						   const std::vector<ConstrainedVariableId>& vars,
						   const bool positive,
						   const std::vector<RuleExpr*>& body)
    : RuleInstance(parent,vars,positive)
    , m_body(body)
  {
  }

  InterpretedRuleInstance::~InterpretedRuleInstance()
  {
  }

  void InterpretedRuleInstance::handleExecute()
  {
    // TODO: should pass in eval context from outside
    RuleInstanceEvalContext evalContext(NULL,getId());

    debugMsg("XMLInterpreter:InterpretedRule","Executing interpreted rule:" << getRule()->getName().toString() << " token:" << m_token->toString());
    for (unsigned int i=0; i < m_body.size(); i++)
      m_body[i]->eval(evalContext);
    debugMsg("XMLInterpreter:InterpretedRule","Executed  interpreted rule:" << getRule()->getName().toString() << " token:" << m_token->toString());
  }

  void InterpretedRuleInstance::createConstraint(const LabelStr& name, std::vector<ConstrainedVariableId>& vars)
  {
    addConstraint(name,vars);
  }

  TokenId InterpretedRuleInstance::createSubgoal(
						 const LabelStr& name,
						 const LabelStr& predicateType,
						 const LabelStr& predicateInstance,
						 const LabelStr& relation,
						 bool isConstrained,
						 ConstrainedVariableId& owner)
  {
    TokenId slave;

    unsigned int tokenCnt = predicateInstance.countElements(".");
    bool isOnSameObject = (
        tokenCnt == 1 ||
        (tokenCnt==2 && (predicateInstance.getElement(0,".").toString() == "object"))
    );

    if (isOnSameObject) {
        // TODO: this is to support predicate inheritance
      	// currently doing the same as the compiler, it'll probably be surprising to the user that
      	// predicate inheritance will work only if the predicates are on the same object that the rule belongs to
      	LabelStr suffix = predicateInstance.getElement(tokenCnt-1,".");
        slave = NDDL::allocateOnSameObject(m_token,suffix,relation);
    }
    else {
        slave = m_token->getPlanDatabase()->createSlaveToken(m_token,predicateType,relation);
    }
    addSlave(slave,name);

    // For qualified names like "object.helloWorld" must add constraint to the object variable on the slave token
    // See RuleWriter.allocateSlave in Nddl compiler
    if (isConstrained) {
      std::vector<ConstrainedVariableId> vars;

      if (tokenCnt <= 2) {
	vars.push_back(owner);
      }
      else {  // equivalent of constrainObject() in NddlRules.hh
	// TODO: this can be done more efficiently
	int cnt = predicateInstance.countElements(".");
	std::string ownerName(predicateInstance.getElement(0,".").toString());
	std::string tokenName(predicateInstance.getElement(cnt-1,".").toString());
	std::string fullName = predicateInstance.toString();
	std::string objectPath = fullName.substr(
					     ownerName.size()+1,
					     fullName.size()-(ownerName.size()+tokenName.size()+2)
					     );
	debugMsg("XMLInterpreter:InterpretedRule","Subgoal slave object constraint. fullName=" << fullName << " owner=" << ownerName << " objPath=" << objectPath << " tokenName=" << tokenName);
	vars.push_back(varFromObject(owner,objectPath,fullName));
      }

      vars.push_back(slave->getObject());
      addConstraint(LabelStr("eq"),vars);
    }
    else {
      debugMsg("XMLInterpreter:InterpretedRule",predicateInstance.toString() << " NotConstrained");
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
    else if (strcmp(relationName,"contained_by") == 0) {
      contained_by("this",name);
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
    else if (strcmp(relationName, "paralleled_by") == 0) {
      relation(precedes, "this", start, name, start);
      relation(precedes, "this", end, name, end);
    }
    else if (strcmp(relationName, "parallels") == 0) {
      relation(precedes, "this", start, name, start);
      relation(precedes, "this", end, name, end);
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
    else {
      check_runtime_error(strcmp(relationName,"any") == 0,std::string("Unrecognized relation:")+relationName);
    }

    debugMsg("XMLInterpreter:InterpretedRule","Created relation " << relationName << " " << predicateInstance.c_str());
    return slave;
  }

  ConstrainedVariableId InterpretedRuleInstance::addLocalVariable(
								  const AbstractDomain& baseDomain,
								  bool canBeSpecified,
								  const LabelStr& name)
  {
    return addVariable(baseDomain,canBeSpecified,name);
  }

  ConstrainedVariableId InterpretedRuleInstance::addObjectVariable(
								   const LabelStr& type,
								   const ObjectDomain& baseDomain,
								   bool canBeSpecified,
								   const LabelStr& name)
  {
    ConstrainedVariableId localVariable = addVariable(baseDomain,canBeSpecified,name);
    getPlanDatabase()->makeObjectVariableFromType(type,localVariable,canBeSpecified);

    return localVariable;
  }

  void InterpretedRuleInstance::executeLoop(EvalContext& evalContext,
					    const LabelStr& loopVarName,
					    const LabelStr& loopVarType,
					    const LabelStr& valueSet,
					    const std::vector<RuleExpr*>& loopBody)
  {
    // Create a local domain based on the objects included in the valueSet
    ConstrainedVariableId setVar = evalContext.getVar(valueSet.c_str());
    check_error(!setVar.isNoId(),"Loop var can't be NULL");
    const AbstractDomain& loopVarDomain = setVar->derivedDomain();
    debugMsg("XMLInterpreter:InterpretedRule","set var for loop :" << setVar->toString());
    debugMsg("XMLInterpreter:InterpretedRule","set var domain for loop:" << loopVarDomain.toString());
    const ObjectDomain& loopObjectSet = dynamic_cast<const ObjectDomain&>(loopVarDomain);

    if (loopObjectSet.isEmpty())
    	return; // we're done

    // Post a locking constraint on the set
    {
    	std::vector<ConstrainedVariableId> loop_vars;
    	loop_vars.push_back(setVar);
    	loop_vars.push_back(ruleVariable(loopObjectSet));
    	rule_constraint(Lock, loop_vars);
    }

    std::list<double> loopObjectSet_values;
    loopObjectSet.getValues(loopObjectSet_values);

    // Translate into a set ordered by key to ensure reliable ordering across runs
    ObjectSet loopObjectSet_valuesByKey;
    for(std::list<double>::iterator it=loopObjectSet_values.begin();
    it!=loopObjectSet_values.end(); ++it) {
    	ObjectId t = *it;
    	loopObjectSet_valuesByKey.insert(t);
    }

    // iterate over loop collection
    for(ObjectSet::const_iterator it=loopObjectSet_valuesByKey.begin()
    		;it!=loopObjectSet_valuesByKey.end(); ++it) {
    	ObjectId loop_var = *it;
    	check_error(loop_var.isValid());

    	// Allocate a local variable for this singleton object
    	// see loopVar(Allocation, a);
    	{
    		ObjectDomain loopVarDomain(loopVarType.c_str());
    		loopVarDomain.insert(loop_var);
    		loopVarDomain.close();
    		// This will automatically put it in the evalContext, since all RuleInstance vars are reachable there
    		addVariable(loopVarDomain, false, loopVarName);
    	}

        // execute loop body
    	for (unsigned int i=0; i < loopBody.size(); i++)
    		loopBody[i]->eval(evalContext);

    	clearLoopVar(loopVarName);
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
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString());

    return instance;
  }
}

