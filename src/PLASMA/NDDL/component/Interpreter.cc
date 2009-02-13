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
#include "IntervalTypeFactory.hh"
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
    vars.push_back(context.getToken(origin.c_str())->originvar());	\
    vars.push_back(context.getToken(target.c_str())->targetvar());	\
    InterpretedRuleInstance* rule = (InterpretedRuleInstance*)(context.getElement("RuleInstance")); \
    rule->createConstraint(LabelStr(#relationname), vars); \
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

  const ConstrainedVariableId& DataRef::getValue() { return m_value; }

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
    debugMsg("Interpreter:EvalContext","Added var:" << name << " to EvalContext");
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

  std::string ExprList::toString() const
  {
      std::ostringstream os;

      for (unsigned int i=0;i<m_children.size();i++)
          os << m_children[i]->toString() << std::endl;

      return os.str();
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
    debugMsg("Interpreter:InterpretedObject","Initialized variable:" << object->getName().toString() << "." << m_lhs.toString() << " to " << rhsValue->derivedDomain().toString() << " in constructor");

    return DataRef::null;
  }

  /*
   * ExprConstant
   */
  ExprConstant::ExprConstant(const DbClientId& dbClient, const char* type, const AbstractDomain* domain)
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
    // TODO: need to create a new variable every time this is evaluated, since propagation
    // will affect the variable, should provide immutable variables so that a single var
    // can be created for a constant.
    ConstrainedVariableId var;

    // TODO: this isn't pretty, have the EvalConstext create the new var, deal with it gracefully for all the possibilities
    RuleInstanceEvalContext *riec = dynamic_cast<RuleInstanceEvalContext*>(&context);
    bool canBeSpecified = false;
    static int nameCnt=0;
    std::stringstream sstr;
    sstr << "ExprConstant_PSEUDO_VARIABLE_" << nameCnt++;
    const char* name = sstr.str().c_str();

    if (riec != NULL) {
        var = riec->getRuleInstance()->addLocalVariable(*m_domain,canBeSpecified,name);
    }
    else {
        // TODO: Who destroys this variable?
        var = m_dbClient->createVariable(
                m_type.c_str(),
                *m_domain,
                name,
                true, // isTmp
                canBeSpecified
        );
    }

    return DataRef(var);
  }

  std::string ExprConstant::toString() const
  {
      std::ostringstream os;

      os << "{CONSTANT " << m_type.c_str() << " " << m_domain->toString() << "}";
      return os.str();
  }



  /*
   * ExprVarRef
   */
  ExprVarRef::ExprVarRef(const char* varName)
    : m_varName(varName)
  {
  }

  ExprVarRef::~ExprVarRef()
  {
  }

  DataRef ExprVarRef::eval(EvalContext& context) const
  {
    // TODO: do this only once
    std::vector<std::string> vars;
    tokenize(m_varName.toString(),vars,".");
    std::string varName=vars[0];

    ConstrainedVariableId rhs = context.getVar(vars[0].c_str());

    if (rhs.isNoId())
      check_runtime_error(ALWAYS_FAILS,std::string("Couldn't find variable ")+varName+" in Evaluation Context");

    for (unsigned int idx = 1;idx<vars.size();idx++) {
      // TODO: parser must do type checking instead
      //check_error(m_schema->isObjectType(rhs->baseDomain().getTypeName()), std::string("Can't apply dot operator to:")+rhs->baseDomain().getTypeName().toString());
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

  std::string ExprVarRef::toString() const
  {
      return m_varName.toString();
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

    ConstrainedVariableId thisVar = context.getVar("this");
    ObjectId thisObject = (thisVar.isId() ? ObjectId(thisVar->derivedDomain().getSingletonValue()) : ObjectId::noId());
    std::string prefix = (thisObject.isId() ? thisObject->getName().toString() + "." : "");

    LabelStr name(prefix+m_objectName.toString());
    ObjectId newObject = m_dbClient->createObject(
						  m_objectType.c_str(),
						  name.c_str(),
						  arguments
						  );

    if (thisObject.isId())
        newObject->setParent(thisObject);

    return DataRef(newObject->getThis());
  }

  std::string ExprNewObject::toString() const
  {
      std::ostringstream os;

      os << "{NEW_OBJECT " << m_objectName.toString() << "(" << m_objectType.toString() << ") ";

      for (unsigned int i =0; i < m_argExprs.size();i++) {
          if (i>0)
              os << " , ";
          os << m_argExprs[i]->toString();
      }
      os << " }";

      return os.str();
  }


  /*
   * ExprRuleVarRef
   */
  ExprRuleVarRef::ExprRuleVarRef(const char* varName)
    : m_varName(varName)
  {
    std::vector<std::string> vars;
    tokenize(m_varName,vars,".");

    if (vars.size() > 1) {
      m_parentName = vars[0];
      m_varName = m_varName.substr(m_parentName.length()+1);
      debugMsg("Interpreter:InterpretedRule","Split " << varName << " into " << m_parentName << " and " << m_varName);
    }
    else {
      m_parentName = "";
      debugMsg("Interpreter:InterpretedRule","Didn't split " << varName);
    }
  }

  ExprRuleVarRef::~ExprRuleVarRef()
  {
  }

  DataRef ExprRuleVarRef::doEval(RuleInstanceEvalContext& context) const
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

  ExprConstraint::ExprConstraint(const char* name,const std::vector<Expr*>& args)
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

  DataRef ExprConstraint::eval(EvalContext& context) const
  {
    std::vector<ConstrainedVariableId> vars;
    for (unsigned int i=0; i < m_args.size(); i++) {
      DataRef arg = m_args[i]->eval(context);
      vars.push_back(arg.getValue());
    }

    InterpretedRuleInstance* rule = (InterpretedRuleInstance*)(context.getElement("RuleInstance"));
    if (rule != NULL) {
        rule->createConstraint(m_name,vars);
        debugMsg("Interpreter:InterpretedRule","Added Constraint : " << m_name.toString() << " - " << varsToString(vars));
    }
    else { // The constraint is being added in the global context
        PlanDatabase* pdb = (PlanDatabase*)(context.getElement("PlanDatabase"));
        pdb->getClient()->createConstraint(m_name.c_str(), vars);
        debugMsg("Interpreter","Added Constraint : " << m_name.toString() << " - " << varsToString(vars));
    }

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
    debugMsg("Interpreter:InterpretedRule","Creating subgoal " << m_predicateType.toString() << ":" << m_name.toString());

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
    debugMsg("Interpreter:InterpretedRule","Created  subgoal " << m_predicateType.toString() << ":" << m_name.toString());
    return DataRef::null;
  }

  ExprRelation::ExprRelation(const char* relation,
                             const char* origin,
                             const char* target)
    : m_relation(relation)
    , m_origin(origin)
    , m_target(target)
  {
  }

  ExprRelation::~ExprRelation()
  {
  }

  DataRef ExprRelation::doEval(RuleInstanceEvalContext& context) const
  {
    const char* relationName = m_relation.c_str();

    // Allen Relations according to EUROPA
    // See ConstraintLibraryReference on the wiki
    if (strcmp(relationName,"meets") == 0) {
      relation(concurrent, m_origin, end, m_target, start);
    }
    else if (strcmp(relationName,"met_by") == 0) {
      relation(concurrent, m_target, end, m_origin, start);
    }
    else if (strcmp(relationName,"contains") == 0) {
      relation(precedes, m_origin, start, m_target, start);
      relation(precedes, m_target, end, m_origin, end);
    }
    else if (strcmp(relationName,"contained_by") == 0) {
      relation(precedes, m_target, start, m_origin, start);
      relation(precedes, m_origin, end, m_target, end);
    }
    else if (strcmp(relationName,"before") == 0) {
      relation(precedes, m_origin, end, m_target, start);
    }
    else if (strcmp(relationName,"after") == 0) {
      relation(precedes, m_target, end, m_origin, start);
    }
    else if (strcmp(relationName,"starts") == 0) {
      relation(concurrent, m_origin, start, m_target, start);
    }
    else if (strcmp(relationName,"ends") == 0) {
      relation(concurrent, m_target, end, m_origin, end);
    }
    else if (strcmp(relationName, "parallels") == 0) {
      relation(precedes, m_origin, start, m_target, start);
      relation(precedes, m_origin, end, m_target, end);
    }
    else if (strcmp(relationName, "paralleled_by") == 0) {
      relation(precedes, m_target, start, m_origin, start);
      relation(precedes, m_target, end, m_origin, end);
    }
    else if (strcmp(relationName,"ends_after") == 0) {
      relation(precedes, m_target, end, m_origin, end);
    }
    else if (strcmp(relationName,"ends_before") == 0) {
      relation(precedes, m_origin, end, m_target, end);
    }
    else if (strcmp(relationName,"ends_after_start") == 0) {
      relation(precedes, m_target, start, m_origin, end);
    }
    else if (strcmp(relationName,"starts_before_end") == 0) {
      relation(precedes, m_origin, start, m_target, end);
    }
    else if (strcmp(relationName,"starts_during") == 0) {
      relation(precedes, m_target, start, m_origin, start);
      relation(precedes, m_target, start, m_origin, end);
    }
    else if (strcmp(relationName,"ends_during") == 0) {
      relation(precedes, m_target, end, m_origin, start);
      relation(precedes, m_target, end, m_origin, end);
    }
    else if (strcmp(relationName,"contains_start") == 0) {
      relation(precedes, m_origin, start, m_target, start);
      relation(precedes, m_origin, start, m_target, end);
    }
    else if (strcmp(relationName,"contains_end") == 0) {
      relation(precedes, m_origin, end, m_target, start);
      relation(precedes, m_origin, end, m_target, end);
    }
    else if (strcmp(relationName,"starts_after") == 0) {
      relation(precedes, m_target, start, m_origin, start);
    }
    else if (strcmp(relationName,"starts_before") == 0) {
      relation(precedes, m_origin, start, m_target, start);
    }
    else if (strcmp(relationName,"equals") == 0) {
      relation(concurrent, m_origin, start, m_target, start);
      relation(concurrent, m_target, end, m_origin, end);
    }
    else {
      check_runtime_error(strcmp(relationName,"any") == 0,std::string("Unrecognized relation:")+relationName);
    }

    debugMsg("Interpreter:InterpretedRule","Created relation " << relationName);
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
    debugMsg("Interpreter:InterpretedRule","Added RuleInstance local var:" << localVar->toString());
    return DataRef::null;
  }

  ExprIfGuard::ExprIfGuard(const char* op, Expr* lhs,Expr* rhs)
    : m_op(op)
    , m_lhs(lhs)
    , m_rhs(rhs)
  {
  }

  ExprIfGuard::~ExprIfGuard()
  {
  }

  const std::string& ExprIfGuard::getOperator() { return m_op; }
  Expr* ExprIfGuard::getLhs() { return m_lhs; }
  Expr* ExprIfGuard::getRhs() { return m_rhs; }

  DataRef ExprIfGuard::eval(EvalContext& context) const
  {
      // TODO: rework ExprIf implementation so that this can be evaluated
      check_runtime_error(ALWAYS_FAILS,"ExprIfGuard can't be evaluated");
      return DataRef::null;
  }

  std::string ExprIfGuard::toString() const
  {
      std::stringstream os;

      os << "{IfGuard:" << m_op << " LHS=" << m_lhs->toString() << " RHS=" << (m_rhs != NULL ? m_rhs->toString() : "NULL") << "}";

      return os.str();
  }



  ExprIf::ExprIf(ExprIfGuard* guard,const std::vector<Expr*>& ifBody, const std::vector<Expr*>& elseBody)
    : m_guard(guard)
    , m_ifBody(ifBody)
    , m_elseBody(elseBody)
  {
  }

  ExprIf::~ExprIf()
  {
  }

  DataRef ExprIf::doEval(RuleInstanceEvalContext& context) const
  {
    bool isOpEquals = (m_guard->getOperator() == "equals" || m_guard->getOperator()=="==");

    DataRef lhs = m_guard->getLhs()->eval(context);

    // TODO: this assumes that the variable is always on the lhs and the value on the rhs
    // is this enforced by the parser? underlying Rule implementation should be made more generic

    if (m_guard->getRhs() != NULL) {
      DataRef rhs = m_guard->getRhs()->eval(context);
      context.getRuleInstance()->addChildRule(
					      new InterpretedRuleInstance(
									  context.getRuleInstance()->getId(),
									  lhs.getValue(),
									  rhs.getValue()->lastDomain(),
									  isOpEquals,
									  m_ifBody
									  )
					      );

      if (m_elseBody.size() > 0) {
          context.getRuleInstance()->addChildRule(
                              new InterpretedRuleInstance(
                                          context.getRuleInstance()->getId(),
                                          lhs.getValue(),
                                          rhs.getValue()->lastDomain(),
                                          !isOpEquals,
                                          m_ifBody
                                          )
                              );
      }

      debugMsg("Interpreter:InterpretedRule","Evaluated IF " << m_guard->toString());
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

      check_runtime_error(m_elseBody.size()==0, "Can't have else body for singleton guard");
      debugMsg("Interpreter:InterpretedRule","Evaluated IF " << m_guard->toString());
    }

    return DataRef::null;
  }

  std::string ExprIf::toString() const
  {
      std::stringstream os;

      os << "{If " << m_guard->toString() << " body(" << m_ifBody.size() << ") }";

      return os.str();
  }

  ExprLoop::ExprLoop(const char* varName, const char* varValue,const std::vector<Expr*>& loopBody)
    : m_varName(varName)
    , m_varValue(varValue)
    , m_loopBody(loopBody)
  {
  }

  ExprLoop::~ExprLoop()
  {
  }

  DataRef ExprLoop::doEval(RuleInstanceEvalContext& context) const
  {
    context.getRuleInstance()->executeLoop(context,m_varName,m_varValue,m_loopBody);
    debugMsg("Interpreter:InterpretedRule","Evaluated LOOP " << m_varName.toString() << "," << m_varValue.toString());
    return DataRef::null;
  }

    /*
     * InterpretedToken
     */
    InterpretedToken::InterpretedToken(const PlanDatabaseId& planDatabase,
                     const LabelStr& predicateName,
                     const std::vector<ExprVarDeclaration*>& parameters,
                     const std::vector<ExprAssignment*>& varAssignments,
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
    	commonInit(parameters, varAssignments, constraints, close);
    	debugMsg("Interpreter:InterpretedToken","Created token(" << getKey() << ") of type:" << predicateName.toString() << " objectVar=" << getVariable("object")->toString());
    }

  InterpretedToken::InterpretedToken(const TokenId& master,
				     const LabelStr& predicateName,
				     const LabelStr& relation,
                     const std::vector<ExprVarDeclaration*>& parameters,
                     const std::vector<ExprAssignment*>& varAssignments,
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
    commonInit(parameters, varAssignments, constraints, close);
    debugMsg("Interpreter:InterpretedToken","Created slave token(" << getKey() << ") of type:" << predicateName.toString() << " objectVar=" << getVariable("object")->toString());
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
                    const std::vector<ExprVarDeclaration*>& parameters,
                    const std::vector<ExprAssignment*>& varAssignments,
				    const std::vector<ExprConstraint*>& constraints,
				    const bool& autoClose)
  {
    debugMsg("XMLInterpreter","Token " << getName().toString() << " has " << parameters.size() << " parameters");

    // TODO: Pass in EvalContext
    TokenEvalContext context(NULL,getId()); // TODO: give access to class or global context?

    // TODO: just do straight evaluation of Exprs for parameters,varAssignments and constraints
    for (unsigned int i=0; i < parameters.size(); i++) {
        const LabelStr& parameterName = parameters[i]->getName();
        const LabelStr& parameterType = parameters[i]->getType();
        const Expr* initValue = parameters[i]->getInitValue();

        check_runtime_error(getVariable(parameterName,false) == ConstrainedVariableId::noId(),
                            "Token parameter "+parameterName.toString()+ " already exists!");

        // This is a hack needed because TokenVariable is parametrized by the domain arg to addParameter
        ConstrainedVariableId parameter;

        // same as completeObjectParam in NddlRules.hh
        if(initValue != NULL) {
            parameter = addParameter(
                    initValue->eval(context).getValue()->baseDomain(),
                    parameterName
            );
            if (context.isClass(parameterName))
                getPlanDatabase()->makeObjectVariableFromType(parameterType, parameter);
        }
        else {
            if (context.isClass(parameterType)) {
                parameter = addParameter(
                        ObjectDomain(parameterType.c_str()),
                        parameterName
                );
                getPlanDatabase()->makeObjectVariableFromType(parameterType, parameter);
            }
            else {
                parameter = addParameter(
                        getPlanDatabase()->getConstraintEngine()->getCESchema()->baseDomain(parameterType.c_str()),
                        parameterName
                );
            }
        }

        debugMsg("Interpreter:InterpretedToken","Token " << getName().toString() << " added Parameter "
                << parameter->toString() << " " << parameterName.toString());
    }

    if (autoClose)
      close();

    // Take care of initializations that were part of the predicate declaration
    for (unsigned int i=0; i < varAssignments.size(); i++)
      varAssignments[i]->eval(context).getValue()->baseDomain();

    // Post parameter constraints
    // TODO: just call eval on ExprConstraints
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
            const ObjectTypeId& objType)
        : TokenFactory(predicateName)
        , m_objType(objType)
    {
    }

    void InterpretedTokenFactory::addParameter(ExprVarDeclaration* parameterDecl)
    {
        m_parameters.push_back(parameterDecl);
        addArg(parameterDecl->getType(),parameterDecl->getName());
    }

    ExprVarDeclaration* InterpretedTokenFactory::getParameter(const LabelStr& name)
    {
        for (unsigned int i=0;i<m_parameters.size();i++) {
            if ((double)(m_parameters[i]->getName()) == (double)name)
                return m_parameters[i];
        }

        return NULL;
    }

    void InterpretedTokenFactory::addConstraint(ExprConstraint* c)
    {
        m_constraints.push_back(c);
    }

    void InterpretedTokenFactory::addVarAssignment(ExprAssignment* va)
    {
        m_varAssignments.push_back(va);
    }

    TokenFactoryId InterpretedTokenFactory::getParentFactory(const PlanDatabaseId& planDb) const
    {
        // TODO: cache this?
        // TODO: drop planDb parameter, ObjectType must be able to answer this without reference to schema
        TokenFactoryId parentFactory = planDb->getSchema()->getParentTokenFactory(getSignature(), m_objType->getParent());
        return parentFactory;
    }

	TokenId InterpretedTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
	{
	    TokenFactoryId parentFactory = getParentFactory(planDb);

        TokenId token;
	    if (parentFactory.isNoId()) {
	        token = (new InterpretedToken(
	                planDb,
	                name,
	                m_parameters,
	                m_varAssignments,
	                m_constraints,
	                rejectable,
	                isFact,
	                false))->getId();
	    }
	    else {
	        token = parentFactory->createInstance(planDb,name,rejectable,isFact);
	        // TODO: Hack! this makes it impossible to extend native tokens
	        // class hierarchy needs to be fixed to avoid this cast
            InterpretedToken* it = dynamic_cast<InterpretedToken*>((Token*)token);
            it->commonInit(
                    m_parameters,
                    m_varAssignments,
                    m_constraints,
                    false);
	    }

	    return token;
	}

  TokenId InterpretedTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
      TokenFactoryId parentFactory = getParentFactory(master->getPlanDatabase());

      TokenId token;
      if (parentFactory.isNoId()) {
          token = (new InterpretedToken(
                  master,
                  name,
                  relation,
                  m_parameters,
                  m_varAssignments,
                  m_constraints,
                  false))->getId();
      }
      else {
          token = parentFactory->createInstance(master,name,relation);
          // TODO: Hack! this makes it impossible to extend native tokens
          // class hierarchy needs to be fixed to avoid this cast
          InterpretedToken* it = dynamic_cast<InterpretedToken*>((Token*)token);
          it->commonInit(
                  m_parameters,
                  m_varAssignments,
                  m_constraints,
                  false);
      }

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

  void* RuleInstanceEvalContext::getElement(const char* name) const
  {
      std::string str(name);
      if (str == "RuleInstance")
          return (InterpretedRuleInstance*)m_ruleInstance;

      return EvalContext::getElement(name);
  }

  ConstrainedVariableId RuleInstanceEvalContext::getVar(const char* name)
  {
    ConstrainedVariableId var = m_ruleInstance->getVariable(LabelStr(name));

    if (!var.isNoId()) {
      debugMsg("Interpreter:EvalContext:RuleInstance","Found var in rule instance:" << name);
      return var;
    }
    else {
      debugMsg("Interpreter:EvalContext:RuleInstance","Didn't find var in rule instance:" << name);
      return EvalContext::getVar(name);
    }
  }

  TokenId RuleInstanceEvalContext::getToken(const char* name)
  {
	  LabelStr ls_name(name);
      TokenId tok = m_ruleInstance->getSlave(ls_name);
      if (!tok.isNoId()) {
          debugMsg("Interpreter:EvalContext:RuleInstance","Found token in rule instance:" << name);
          return tok;
      }
      else {
        debugMsg("Interpreter:EvalContext:RuleInstance","Didn't find token in rule instance:" << name);
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
      debugMsg("Interpreter:EvalContext:Token","Found var in token :" << name);
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
						   const std::vector<Expr*>& body)
    : RuleInstance(rule, token, planDb)
    , m_body(body)
  {
  }

  InterpretedRuleInstance::InterpretedRuleInstance(
						   const RuleInstanceId& parent,
						   const ConstrainedVariableId& var,
						   const AbstractDomain& domain,
						   const bool positive,
						   const std::vector<Expr*>& body)
    : RuleInstance(parent,var,domain,positive)
    , m_body(body)
  {
  }

  InterpretedRuleInstance::InterpretedRuleInstance(
						   const RuleInstanceId& parent,
						   const std::vector<ConstrainedVariableId>& vars,
						   const bool positive,
						   const std::vector<Expr*>& body)
    : RuleInstance(parent,vars,positive)
    , m_body(body)
  {
  }

  InterpretedRuleInstance::~InterpretedRuleInstance()
  {
  }

  void InterpretedRuleInstance::handleExecute()
  {
    // Inheritance is handled by RulesEngine, see RuleSchema::getRules, where rules for the entire hierarchy are gathered

    // TODO: should pass in eval context from outside
    RuleInstanceEvalContext evalContext(NULL,getId());

    debugMsg("Interpreter:InterpretedRule","Executing interpreted rule:" << getRule()->getName().toString() << " token:" << m_token->toString());
    for (unsigned int i=0; i < m_body.size(); i++)
      m_body[i]->eval(evalContext);
    debugMsg("Interpreter:InterpretedRule","Executed  interpreted rule:" << getRule()->getName().toString() << " token:" << m_token->toString());
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
          debugMsg("Interpreter:InterpretedRule","Subgoal slave object constraint. fullName=" << fullName << " owner=" << ownerName << " objPath=" << objectPath << " tokenName=" << tokenName);
          vars.push_back(varFromObject(owner,objectPath,fullName));
      }

      vars.push_back(slave->getObject());
      addConstraint(LabelStr("eq"),vars);
    }
    else {
      debugMsg("Interpreter:InterpretedRule",predicateInstance.toString() << " NotConstrained");
    }

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
					    const LabelStr& valueSet,
					    const std::vector<Expr*>& loopBody)
  {
    // Create a local domain based on the objects included in the valueSet
    ConstrainedVariableId setVar = evalContext.getVar(valueSet.c_str());
    check_error(!setVar.isNoId(),"Loop var can't be NULL");
    const AbstractDomain& loopVarDomain = setVar->derivedDomain();
    LabelStr loopVarType = loopVarDomain.getTypeName();
    debugMsg("Interpreter:InterpretedRule","set var for loop :" << setVar->toString());
    debugMsg("Interpreter:InterpretedRule","set var domain for loop:" << loopVarDomain.toString());
    debugMsg("Interpreter:InterpretedRule","loop var type:" << loopVarType.toString());
    const ObjectDomain& loopObjectSet = dynamic_cast<const ObjectDomain&>(loopVarDomain);

    if (loopObjectSet.isEmpty())
    	return; // we're done

    // The lock is an assert check really
    {
    	std::vector<ConstrainedVariableId> loop_vars;
    	loop_vars.push_back(setVar);
    	loop_vars.push_back(ruleVariable(loopObjectSet));
    	rule_constraint(filterLock, loop_vars);
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
						 const std::vector<Expr*>& body)
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


  // TODO: keep using pdbClient?
  const DbClientId& getPDB(EvalContext& context)
  {
      // TODO: Add this behavior to EvalContext instead?
      DbClient* dbClient = (DbClient*)context.getElement("DbClient");
      if (dbClient != NULL)
          return dbClient->getId();

      PlanDatabase* pdb = (PlanDatabase*)context.getElement("PlanDatabase");
      check_error(pdb != NULL,"Could not find Plan Database in eval context");
      return pdb->getClient();
  }

  const SchemaId& getSchema(EvalContext& context)
  {
      /* TODO why doesn't this work???
      Schema* schema = (Schema*)context.getElement("Schema");
      check_error(schema != NULL,"Could not find Schema in eval context");
      return schema->getId();
      */
      return getPDB(context)->getSchema();
  }


  ExprTypedef::ExprTypedef(const char* name, AbstractDomain* type)
      : m_name(name)
      , m_type(type)
  {
  }

  ExprTypedef::~ExprTypedef()
  {
  }

  DataRef ExprTypedef::eval(EvalContext& context) const
  {
      const char* name = m_name.c_str();
      const AbstractDomain& domain = *m_type;

      debugMsg("Interpreter:typedef","Defining type:" << name);

      TypeFactory* factory = NULL;
      if (m_type->isEnumerated())
        factory = new EnumeratedTypeFactory(name,name,domain);
      else
        factory = new IntervalTypeFactory(name,domain);

      // TODO: this is what the code generator does for every typedef, it doesn't seem right for interval types though
      SchemaId schema = getSchema(context);
      schema->addEnum(name);
      schema->getCESchema()->registerFactory(factory->getId());

      debugMsg("Interpreter:typedef"
              , "Created type factory " << name <<
                " with base domain " << domain.toString());

      return DataRef::null;
  }

  std::string ExprTypedef::toString() const
  {
      std::ostringstream os;

      os << "TYPEDEF:" << m_type->toString() << " -> " << m_name.toString();

      return os.str();
  }

  ExprVarDeclaration::ExprVarDeclaration(const char* name, const char* type, Expr* initValue)
      : m_name(name)
      , m_type(type)
      , m_initValue(initValue)
  {
  }

  ExprVarDeclaration::~ExprVarDeclaration()
  {
  }

  const LabelStr& ExprVarDeclaration::getName() const { return m_name; }
  const LabelStr& ExprVarDeclaration::getType() const { return m_type; }
  const Expr* ExprVarDeclaration::getInitValue() const { return m_initValue; }
  void ExprVarDeclaration::setInitValue(Expr* iv) { m_initValue = iv; }

  DataRef ExprVarDeclaration::eval(EvalContext& context) const
  {
      const DbClientId& pdb = getPDB(context);

      ConstrainedVariableId v = pdb->createVariable(
              m_type.c_str(),
              m_name.c_str()
      );

      return DataRef(v);
  }

  std::string ExprVarDeclaration::toString() const
  {
      std::ostringstream os;

      os << m_type.c_str() << " " << m_name.toString();
      if (m_initValue != NULL)
          os << " " << m_initValue->toString();

      return os.str();
  }

  ExprAssignment::ExprAssignment(Expr* lhs, Expr* rhs)
      : m_lhs(lhs)
      , m_rhs(rhs)
  {
  }

  ExprAssignment::~ExprAssignment()
  {
  }

  DataRef ExprAssignment::eval(EvalContext& context) const
  {
      DataRef lhs = m_lhs->eval(context);

      if (m_rhs != NULL) {
          DataRef rhs = m_rhs->eval(context);
          const DbClientId& pdb = getPDB(context);

          if (rhs.getValue()->lastDomain().isSingleton()) {
              pdb->specify(lhs.getValue(),rhs.getValue()->lastDomain().getSingletonValue());
          }
          else {
              pdb->restrict(lhs.getValue(),rhs.getValue()->lastDomain());
              // TODO: this behavior seems more reasonable, specially to support violation reporting
              // lhs.getValue()->getCurrentDomain().equate(rhs.getValue()->getCurrentDomain());
          }
      }

      return lhs;
  }

  std::string ExprAssignment::toString() const
  {
      std::ostringstream os;

      os << "{ " << m_lhs->toString() << " = " << (m_rhs != NULL ? m_rhs->toString() : "NULL") << "}";

      return os.str();
  }


  ExprObjectTypeDeclaration::ExprObjectTypeDeclaration(const ObjectTypeId& objType)
      : m_objType(objType)
  {
  }

  ExprObjectTypeDeclaration::~ExprObjectTypeDeclaration()
  {
  }

  DataRef ExprObjectTypeDeclaration::eval(EvalContext& context) const
  {
      SchemaId schema = getSchema(context);

      schema->declareObjectType(m_objType->getName());

      return DataRef::null;
  }

  std::string ExprObjectTypeDeclaration::toString() const
  {
      std::ostringstream os;

      os << "{class " << m_objType->getName().c_str() << "}";

      return os.str();
  }


  ExprObjectTypeDefinition::ExprObjectTypeDefinition(const ObjectTypeId& objType)
      : m_objType(objType)
  {
  }

  ExprObjectTypeDefinition::~ExprObjectTypeDefinition()
  {
  }

  DataRef ExprObjectTypeDefinition::eval(EvalContext& context) const
  {
      SchemaId schema = getSchema(context);

      schema->registerObjectType(m_objType);

      return DataRef::null;
  }

  std::string ExprObjectTypeDefinition::toString() const
  {
      std::ostringstream os;

      os << m_objType->toString();

      return os.str();
  }

  ExprRuleTypeDefinition::ExprRuleTypeDefinition(const RuleId& rf)
      : m_ruleFactory(rf)
  {
  }

  ExprRuleTypeDefinition::~ExprRuleTypeDefinition()
  {

  }

  DataRef ExprRuleTypeDefinition::eval(EvalContext& context) const
  {
      RuleSchema* rs = (RuleSchema*)context.getElement("RuleSchema");
      rs->registerRule(m_ruleFactory);
      return DataRef::null;
  }

  std::string ExprRuleTypeDefinition::toString() const
  {
      std::ostringstream os;

      os << m_ruleFactory->toString();

      return os.str();
  }

}

