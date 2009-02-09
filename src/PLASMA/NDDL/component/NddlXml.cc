
#include "NddlXml.hh"
#include "Interpreter.hh"
#include "tinyxml.h"
#include "EnumeratedTypeFactory.hh"
#include "IntervalTypeFactory.hh"

namespace EUROPA {

  class NddlXmlEvalContext : public EvalContext
  {
  public:
      NddlXmlEvalContext(NddlXmlInterpreter* i)
          : EvalContext(NULL)
          , m_interpreter(i)
      {
      }

      virtual ~NddlXmlEvalContext()
      {
      }

      virtual void* getElement(const char* name) const
      {
          std::string str(name);

          if (str=="DbClient")
              return (DbClient*)(m_interpreter->getDbClient());
          if (str=="Schema")
              return (Schema*)(m_interpreter->getSchema());
          if (str=="RuleSchema")
              return (RuleSchema*)(m_interpreter->m_ruleSchema);

          return EvalContext::getElement(name);
      }

  protected:
      NddlXmlInterpreter* m_interpreter;
  };

  /*
   *
   * NddlXmlInterpreter
   *
   */

  NddlXmlInterpreter::NddlXmlInterpreter(const DbClientId & client, const RuleSchemaId& ruleSchema)
    : DbClientTransactionPlayer(client)
    , m_ruleSchema(ruleSchema)
  {
    m_evalContext = new NddlXmlEvalContext(this);
  }

  NddlXmlInterpreter::~NddlXmlInterpreter()
  {
      delete m_evalContext;
  }

  std::string NddlXmlInterpreter::interpret(std::istream& input, const std::string& script)
  {
      play(input);
      return "";
  }

  const char* safeStr(const char* str)
  {
    return (str !=NULL ? str : "NULL");
  }

  std::ostringstream dbgout;
  const char* ident="    ";

  void NddlXmlInterpreter::playDeclareClass(const TiXmlElement& element)
  {
    const char* className = element.Attribute("name");
    getSchema()->declareObjectType(className);

    dbgout.str("");
    dbgout << "Declared class " << className << std::endl;
    debugMsg("XMLInterpreter:XML",dbgout.str());
  }

   Expr* NddlXmlInterpreter::valueToExpr(const TiXmlElement* element, bool isRule)
  {
    check_runtime_error(element != NULL,"Unexpected NULL element, expected value or id element");

    if (strcmp(element->Value(),"value") == 0 ||
        strcmp(element->Value(),"symbol") == 0 ||
        strcmp(element->Value(),"interval") == 0) {
      return new ExprConstant(m_client,element->Attribute("type"),xmlAsAbstractDomain(*element));
    }
    else if (strcmp(element->Value(),"set") == 0) {
        TiXmlElement* child = element->FirstChildElement();
        return new ExprConstant(m_client,child->Attribute("type"),xmlAsAbstractDomain(*element));
    }
    else if (strcmp(element->Value(),"id") == 0) {
      const char* varName = element->Attribute("name");
      if (isRule)
          return new ExprRuleVarRef(varName);
      else
          return new ExprVarRef(varName);
    }
    else
      check_runtime_error(ALWAYS_FAILS,std::string("Unexpected xml element:") + element->Value() + ", expected constant(value,symbol,interval) or id element");

    return NULL;
  }

  void NddlXmlInterpreter::playDefineClass(const TiXmlElement& element)
  {
      const char* className = element.Attribute("name");

      ObjectType* objType = getSchema()->getObjectType(className);

      if (objType != NULL) {
          // TODO: should always be displayed as INFO!
          // TODO: allow redefinition of non-native classes?
          std::string isNative = (objType->isNative() ? "native" : "");
          debugMsg("XMLInterpreter:XML","Skipping definition for "<< isNative << " class : " << className);
          return;
      }

      getSchema()->declareObjectType(className);

      const char* parentClassName = element.Attribute("extends");
      parentClassName = (parentClassName == NULL ? "Object" : parentClassName);

      objType = new ObjectType(className,parentClassName);

      for(const TiXmlElement* child = element.FirstChildElement(); child; child = child->NextSiblingElement() ) {
          const char * tagname = child->Value();

          if (strcmp(tagname, "var") == 0)
              defineClassMember(objType,child);
          else if (strcmp(tagname, "constructor") == 0)
              defineConstructor(objType,child);
          else if (strcmp(tagname, "predicate") == 0)
              declarePredicate(objType,child);
          else if (strcmp(tagname, "enum") == 0)
              defineEnum(getSchema(),className,child); // TODO: move to ObjectType?
          else
              check_runtime_error(ALWAYS_FAILS,std::string("Unexpected element ")+tagname+" while defining class "+className);
      }

      ExprObjectTypeDefinition otd(objType->getId());
      otd.eval(*m_evalContext);
  }

  void NddlXmlInterpreter::defineClassMember(ObjectType* objType,  const TiXmlElement* element)
  {
    const char* type = safeStr(element->Attribute("type"));
    const char* name = safeStr(element->Attribute("name"));
    objType->addMember(type,name);
  }

  int NddlXmlInterpreter::defineConstructor(ObjectType* objType, const TiXmlElement* element)
  {
      const char* className = objType->getName().c_str();
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
                  argExprs.push_back(valueToExpr(argChild,false));

              superCallExpr = new ExprConstructorSuperCall(objType->getParent(),argExprs);
          }
          else if (strcmp(child->Value(),"assign") == 0) {
              const TiXmlElement* rhsChild = child->FirstChildElement();
              const char* lhs = child->Attribute("name");

              Expr* rhs=NULL;
              const char* rhsType = rhsChild->Value();
              if (strcmp(rhsType,"new") == 0) {
                  const char* objectType = rhsChild->Attribute("type");

                  std::vector<Expr*> argExprs;
                  for(const TiXmlElement* argChild = rhsChild->FirstChildElement(); argChild; argChild = argChild->NextSiblingElement() )
                      argExprs.push_back(valueToExpr(argChild,false));

                  rhs = new ExprNewObject(
                          m_client,
                          objectType,
                          lhs,
                          argExprs
                  );
              }
              else
                  rhs = valueToExpr(rhsChild,false);

              debugMsg("NddlXmlInterpreter:defineConstructor",
                      "Adding an assignment to " << lhs);
              constructorBody.push_back(new ExprConstructorAssignment(lhs,rhs));
          }
          else
              check_runtime_error(ALWAYS_FAILS,std::string("Unexpected xml element:") + child->Value());
      }

      // If constructor for super class isn't called explicitly, call default one with no args
      if (superCallExpr == NULL)
          superCallExpr = new ExprConstructorSuperCall(objType->getParent(),std::vector<Expr*>());

      objType->addObjectFactory(
              (new InterpretedObjectFactory(
                      className,
                      signature.str(),
                      constructorArgNames,
                      constructorArgTypes,
                      superCallExpr,
                      constructorBody
                  )
              )->getId()
      );

      return constructorArgNames.size();
  }

  void NddlXmlInterpreter::declarePredicate(ObjectType* objType,  const TiXmlElement* element)
  {
      const char* className = objType->getName().c_str();
      std::string predName = std::string(className) + "." + element->Attribute("name");

      if (objType->isNative()) {
          // TODO: should always be displayed as INFO!
          debugMsg("XMLInterpreter:XML",className << " is a native class, skipping factory registration for token : " << predName);
          return;
      }

      std::vector<LabelStr> parameterNames;
      std::vector<LabelStr> parameterTypes;
      std::vector<Expr*> parameterValues;
      std::vector<LabelStr> assignVars;
      std::vector<Expr*> assignValues;
      std::vector<ExprConstraint*> constraints;

      for(const TiXmlElement* predArg = element->FirstChildElement(); predArg; predArg = predArg->NextSiblingElement() ) {
          if (strcmp(predArg->Value(),"var") == 0) {
              const char* type = safeStr(predArg->Attribute("type"));
              const char* name = safeStr(predArg->Attribute("name"));
              parameterNames.push_back(name);
              parameterTypes.push_back(type);
              if(!predArg->NoChildren())
                  parameterValues.push_back(valueToExpr(predArg->FirstChildElement()));
              else
                  parameterValues.push_back(NULL);
          }
          else if (strcmp(predArg->Value(),"assign") == 0) {
              //const char* type = safeStr(predArg->Attribute("type")); // TODO: use type?
              const char* name = safeStr(predArg->Attribute("name"));
              bool inherited = (predArg->Attribute("inherited") != NULL ? true : false);
              if (inherited) {
                  assignVars.push_back(name);
                  assignValues.push_back(valueToExpr(predArg->FirstChildElement()));
              }
              else {
                  // it *should* be a parameter, find it and tag it
                  std::vector<Expr*>::iterator vit = parameterValues.begin();
                  for(std::vector<LabelStr>::const_iterator it = parameterNames.begin(); it != parameterNames.end(); ++it) {
                      if(strcmp(it->c_str(), name) == 0)
                          break;
                      ++vit;
                  }
                  check_runtime_error(vit != parameterValues.end(), std::string("Cannot assign to undeclared parameter:") + name + " in predicate "+predName);
                  // should probably say something about redefinition, but meh
                  *vit = valueToExpr(predArg->FirstChildElement());
              }
          }
          else if (strcmp(predArg->Value(),"invoke") == 0) {
              dbgout << "constraint " << predArg->Attribute("name");
              std::vector<Expr*> constraintArgs;
              for(const TiXmlElement* arg = predArg->FirstChildElement(); arg; arg = arg->NextSiblingElement() )
                  constraintArgs.push_back(valueToExpr(arg));

              constraints.push_back(new ExprConstraint(predArg->Attribute("name"),constraintArgs));
          }
          else
              check_runtime_error(ALWAYS_FAILS,std::string("Unexpected xml element:") + predArg->Value()+ " in predicate "+predName);
      }
      dbgout << ")" << std::endl;

      TokenFactoryId parentFactory = getSchema()->getParentTokenFactory(predName, objType->getParent());

      objType->addTokenFactory(
          (new InterpretedTokenFactory(
              predName,
              parentFactory,
              parameterNames,
              parameterTypes,
              parameterValues,
              assignVars,
              assignValues,
              constraints
          ))->getId()
      );
  }

  void NddlXmlInterpreter::defineEnum(const SchemaId& schema, const char* className, const TiXmlElement* element)
  {
    // Enum is scoped within the class but in the generated code it doesn't make a difference
    playDefineEnumeration(*element);
  }

  bool NddlXmlInterpreter::isClass(const LabelStr& className) const
  {
      return getSchema()->isObjectType(className);
  }

  LabelStr NddlXmlInterpreter::getObjectVarClass(const LabelStr& className,const LabelStr& var) const
  {
    const SchemaId& schema = getSchema();
    check_runtime_error(schema->hasMember(className,var),className.toString()+" has no member called "+var.toString());
    return schema->getMemberType(className,var);
  }

  LabelStr NddlXmlInterpreter::getTokenVarClass(const LabelStr& className,const LabelStr& predName,const LabelStr& var) const
  {
    if (strcmp(var.c_str(),"object") == 0) // is it the object variable?
      return className;
    else { // look through the parameters to the token
      const SchemaId& schema = getSchema();
      if (schema->hasMember(predName,var))
    return schema->getMemberType(predName,var);
    }

    // if everything else fails, see if it's an object member
    return getObjectVarClass(className,var);
  }

  LabelStr NddlXmlInterpreter::checkPredicateType(const LabelStr& type) const
  {
    check_runtime_error(getSchema()->isPredicate(type),type.toString()+" is not a Type");
    return type;
  }

  /*
   * figures out the type of a predicate given an instance
   *
   */
  LabelStr NddlXmlInterpreter::predicateInstanceToType(const char* className,
                                   const char* predicateName,
                                   const char* predicateInstance,
                                   std::map<std::string,std::string>& localVars) const
  {
    // see ModelAccessor.getSlaveObjectType() in NDDL compiler
    LabelStr str(predicateInstance);

    unsigned int tokenCnt = str.countElements(".");

    if (tokenCnt == 1) {
      std::string retval = std::string(className)+"."+predicateInstance;
      return checkPredicateType(LabelStr(retval));
    }
    else if (tokenCnt == 2) {
      LabelStr prefix(str.getElement(0,"."));
      LabelStr suffix(str.getElement(1,"."));

      if (prefix.toString() == "object") {
    std::string retval = std::string(className)+"."+suffix.toString();
    return checkPredicateType(LabelStr(retval.c_str()));
      }
      else if (isClass(prefix)) {
    return checkPredicateType(LabelStr(predicateInstance));
      }
      else if (localVars.find(prefix.toString()) != localVars.end()) {
    std::string clazz = localVars[prefix.toString()];
    return checkPredicateType(clazz+"."+suffix.toString());
      }
      else {
    LabelStr clazz = getTokenVarClass(className,predicateName,prefix);
    std::string retval = clazz.toString()+"."+suffix.toString();
    return checkPredicateType(LabelStr(retval.c_str()));
      }
    }
    else {
      LabelStr var = str.getElement(0,".");
      LabelStr clazz;
      if (localVars.find(var.toString()) != localVars.end())
    clazz = localVars[var.toString()];
      else
    clazz = getTokenVarClass(className,predicateName,var);

      for (unsigned int i=1;i<tokenCnt-1;i++) {
    LabelStr var = str.getElement(i,".");
    clazz = getObjectVarClass(clazz,var);
      }

      LabelStr predicate = str.getElement(tokenCnt-1,".");
      std::string retval = clazz.toString() + "." + predicate.toString();
      return checkPredicateType(LabelStr(retval));
    }
  }

  bool isGuarded(const LabelStr& varName,const TiXmlElement* node)
  {
    if (node->Parent() == NULL)
      return false;

    if(strcmp(node->Value(),"id") == 0) {
      const char* name = node->Attribute("name");

      const TiXmlElement* parent = (const TiXmlElement*)node->Parent();
      if (strcmp(name,varName.c_str())==0 &&
          (
       strcmp(parent->Value(),"if")==0 ||
       strcmp(parent->Value(),"equals")==0 ||
       strcmp(parent->Value(),"nequals")==0
       )
      )
        return true; /* We are done */
    }

    for(const TiXmlElement* child = node->FirstChildElement(); child; child = child->NextSiblingElement() ) {
      if(isGuarded(varName, child))
        return true;
    }

    return false;
  }

  void NddlXmlInterpreter::buildRuleBody(
                               const char* className,
                               const std::string& predName,
                               const TiXmlElement* element,
                               std::vector<Expr*>& ruleBody,
                               std::map<std::string,std::string>& localVars)
  {
      int slave_cnt = 0;

      for(const TiXmlElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement() ) {
          if (strcmp(child->Value(),"invoke") == 0) {

              std::vector<Expr*> constraintArgs;
              for(const TiXmlElement* arg = child->FirstChildElement(); arg; arg = arg->NextSiblingElement() )
                  constraintArgs.push_back(valueToExpr(arg));

              ruleBody.push_back(new ExprConstraint(child->Attribute("name"),constraintArgs));
          }
          else if (strcmp(child->Value(),"subgoal") == 0) {
              const char* predicateInstance = NULL;
              const char* name = NULL;
              const char* relation = child->Attribute("relation");
              const char* origin = child->Attribute("origin");

              for(const TiXmlElement* arg = child->FirstChildElement(); arg; arg = arg->NextSiblingElement() ) {
                  if (strcmp(arg->Value(),"predicateinstance") == 0) {
                      predicateInstance = arg->Attribute("type");
                      name = arg->Attribute("name");
                      if (name == NULL) {
                           std::ostringstream tmpname;
                           tmpname << "slave" << (slave_cnt++);
                           name = LabelStr(tmpname.str()).c_str();
                      }
                      debugMsg("XMLInterpreter:rulebody", "new token for subgoal: " << predicateInstance << " " << name);
                  }
                  else
                      // TODO: support interval offsets
                      check_runtime_error(ALWAYS_FAILS,std::string("Unknown subgoal element:") + arg->Value());
              }

              // Create a token!
              if(predicateInstance != NULL) {
                  const char* predicateType = predicateInstanceToType(className, predName.c_str(), predicateInstance,localVars).c_str();
                  ruleBody.push_back(new ExprSubgoal(name,predicateType,predicateInstance,relation));
              }

              // create relation
              if(origin == NULL)
                  origin = "this";
              if(name == NULL)
                  name = child->Attribute("target");

              ruleBody.push_back(new ExprRelation(relation, origin, name));
          }
          else if (strcmp(child->Value(),"var") == 0) {
              LabelStr name(child->Attribute("name"));
              LabelStr type(child->Attribute("type"));
              localVars[name.toString()]=type.toString();

              Expr* domainRestriction=NULL;
              if (child->FirstChildElement() != NULL)
                  domainRestriction=valueToExpr(child->FirstChildElement());

              const AbstractDomain& baseDomain = getCESchema()->baseDomain(type.c_str());
              ruleBody.push_back(new ExprLocalVar(name,type, isGuarded(name,element), domainRestriction, baseDomain));
          }
          else if (strcmp(child->Value(),"if") == 0) {
              const TiXmlElement* opElement = child->FirstChildElement();
              const TiXmlElement* opArg = opElement->FirstChildElement();

              Expr *lhs=NULL,*rhs=NULL;
              std::string op = "equals";

              if (opArg != NULL) {
                  lhs = valueToExpr(opArg);
                  rhs = valueToExpr(opArg->NextSiblingElement());
                  op = opElement->Value();
              }
              else {
                  lhs = valueToExpr(opElement);
              }

              std::vector<Expr*> ifBody;
              buildRuleBody(className,predName,opElement->NextSiblingElement(),ifBody,localVars);
              ruleBody.push_back(new ExprIf(op.c_str(),lhs,rhs,ifBody));
          }
          else if (strcmp(child->Value(),"loop") == 0) {
              const char* varName = child->Attribute("name");
              const char* varType = child->Attribute("type");
              const char* varValue = child->Attribute("value");
              localVars[varName]=varType;
              std::vector<Expr*> loopBody;
              buildRuleBody(className,predName,child->FirstChildElement(),loopBody,localVars);
              ruleBody.push_back(new ExprLoop(varName,varValue,loopBody));
          }
          else
              check_runtime_error(ALWAYS_FAILS,std::string("Unknown Compatibility element:") + child->Value());
      }
  }

  void NddlXmlInterpreter::playDefineCompat(const TiXmlElement& element)
  {
    const char* className = element.Attribute("class");
    std::string predName = std::string(className) + "." + element.Attribute("name");
    std::string source = "\"" + std::string(element.Attribute("filename")) +
      "," + element.Attribute("line") + "\"";

    std::vector<Expr*> ruleBody;
    std::map<std::string,std::string> localVars; // name-type map for local vars
    buildRuleBody(className,predName,element.FirstChildElement(),ruleBody,localVars);

    m_ruleSchema->registerRule((new InterpretedRuleFactory(predName,source,ruleBody))->getId());
  }

  void NddlXmlInterpreter::playDefineEnumeration(const TiXmlElement &element)
  {
    const char* enumName = element.Attribute("name");
    const TiXmlElement* setElement = element.FirstChildElement();
    check_error(strcmp(setElement->Value(),"set") == 0, "Expected value set as part of Enum definition");

    Id<Schema> schema = getSchema();
    schema->addEnum(enumName);

    std::list<double> values;
    const TiXmlElement* enumValue;
    for(enumValue = setElement->FirstChildElement(); enumValue; enumValue = enumValue->NextSiblingElement() ) {
      double newValue=0;

      if (strcmp(enumValue->Value(),"symbol") == 0) {
    LabelStr symbolValue(enumValue->Attribute("value"));
    newValue = symbolValue;
      }
      else {
    check_runtime_error(ALWAYS_FAILS,std::string("Don't know how to deal with enum values of type ") + enumValue->Value());
      }

      schema->addValue(enumName, newValue);
      values.push_back(newValue);
    }

    getCESchema()->registerFactory((new EnumeratedTypeFactory(
                  enumName,
                  enumName,
                  EnumeratedDomain(values,false,enumName)
                  ))->getId());

  }

  void NddlXmlInterpreter::playDefineType(const TiXmlElement& element)
  {
    const char* name = element.Attribute("name");

    const AbstractDomain* restrictedDomain = NULL;
    if (element.FirstChildElement() != NULL)
      restrictedDomain =  xmlAsAbstractDomain(*(element.FirstChildElement()),"", name);

    const AbstractDomain& domain = (
                    restrictedDomain != NULL
                    ? *restrictedDomain
                    : getCESchema()->baseDomain(element.Attribute("basetype"))
                    );


    TypeFactory * factory = NULL;
    if (domain.isEnumerated())
      factory = new EnumeratedTypeFactory(name,name,domain);
    else
      factory = new IntervalTypeFactory(name,domain);

    std::string domainString = domain.toString();
    debugMsg("XMLInterpreter:typedef", "Created type factory " << name
         << " with base domain " << domainString);

    if (restrictedDomain != NULL)
      delete restrictedDomain;

    // TODO: this is what the code generator does for every typedef, it doesn't seem right for interval types though
    getSchema()->addEnum(name);
    getCESchema()->registerFactory(factory->getId());
  }
}


