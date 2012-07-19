#include "AverDefs.hh"
#include "AverHelper.hh"
#include "Schema.hh"
//#include "StringDomain.hh"
#include "SymbolDomain.hh"
#include "NumericDomain.hh"
#include "Id.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include <algorithm>

//#include <execinfo.h>

namespace EUROPA {

  int DomainContainer::s_lastId = 0;
  PlanDatabaseId AverHelper::s_db = PlanDatabaseId::noId();
  //std::stack<AbstractDomain*> AverHelper::s_domStack = std::stack<AbstractDomain*>();
  std::stack<DomainContainer*> AverHelper::s_domStack = std::stack<DomainContainer*>();
  std::stack<std::string> AverHelper::s_opStack = std::stack<std::string>();

//   void show_stackframe() {
//     void* trace[16];
//     char** messages = (char**) NULL;
//     int trace_size = 0;
    
//     trace_size = backtrace(trace, 16);
//     messages = backtrace_symbols(trace, trace_size);
//     std::cerr << "[bt] Stack trace: " << std::endl;
//     for(int i = 0; i < trace_size; i++)
//       std::cerr << "[bt] " << messages[i] << std::endl;
//   }

  DomainContainer::DomainContainer(AbstractDomain* dom, bool remove, bool dieEarly) : m_dom(dom), m_remove(remove), m_dieEarly(dieEarly),
                                                                                      m_id(s_lastId++) {
    //std::cerr << "Allocated " << m_id << std::endl;
    //show_stackframe();
  }

  DomainContainer::~DomainContainer() {
    //std::cerr << "Deleting " << m_id << std::endl;
    //show_stackframe();
    if(m_remove && m_dom != NULL) {
      delete(m_dom);
      m_dom = NULL;
    }
  }

  const std::string AverExecutionPath::s_actNames[NUMACTIONS] = 
    {"COUNT", "QUERY_TOKENS", "QUERY_OBJECTS", "PROPERTY", "ENTITY", "TRANSACTIONS",
     "TRIM_TOKENS_BY_PRED", "TRIM_TOKENS_BY_VAR", "TRIM_TOKENS_BY_START",
     "TRIM_TOKENS_BY_END", "TRIM_TOKENS_BY_STATUS", "TRIM_TOKENS_BY_PATH",
     "TRIM_OBJECTS_BY_NAME", "TRIM_OBJECTS_BY_VAR", "BOOL_OP", "NO_OP",
     "PUSH_DOMAIN", "PUSH_OP"};

 
  AverExecutionPath::AverExecutionPath(const Actions action) : m_id(this), m_action(action) {
  }
  
  AverExecutionPath::AverExecutionPath(const Actions action, DomainContainer* dom) :
    m_id(this), m_action(action) {
    check_error(m_action == PUSH_DOMAIN);
    m_dom = dom;
  }

  AverExecutionPath::AverExecutionPath(const Actions action, const std::string& op) :
    m_id(this), m_action(action), m_op(op) {
    check_error(m_action == PUSH_OP);
  }

  AverExecutionPath::~AverExecutionPath() {
    m_id.remove();
    if(m_action == PUSH_DOMAIN)
      delete m_dom;
    if(!m_next.isNoId())
      delete (AverExecutionPath*) m_next;
  }

  AverExecutionPathId AverExecutionPath::end() const {
    AverExecutionPathId path = m_id;
    while(!path->next().isNoId())
      path = path->next();
    return path;
  }

  void AverExecutionPath::operator >> (std::ostream& os) const {
    os << s_actNames[m_action] << "\t";
    if(m_action == PUSH_DOMAIN)
      (AbstractDomain&)m_dom >> os;
    else if(m_action == PUSH_OP)
      os << m_op;
    os << std::endl;
    if(!m_next.isNoId())
      (*m_next) >> os;
  }

  DomainContainer* AverHelper::evaluateDomain(const TiXmlElement* dom, bool dieEarly) {
    check_error(("EnumeratedDomain" == std::string(dom->Value()) || 
                 "IntervalDomain" == std::string(dom->Value())),
                "Attempted to evaluate non-standard domain.", AverErr::FlowError());
    if("EnumeratedDomain" == std::string(dom->Value()))
      return evaluateEnumDomain(dom, dieEarly);
    else if("IntervalDomain" == std::string(dom->Value()))
      return evaluateIntDomain(dom, dieEarly);
    check_error(ALWAYS_FAIL, "Attempted to evaluate a domain with non-domain value.",
                 AverErr::FlowError());
    return (DomainContainer*) NULL;
  }

  DomainContainer* AverHelper::evaluateEnumDomain(const TiXmlElement* dom, bool dieEarly) {
    TiXmlElement* value = dom->FirstChildElement("Value");
    if(value == NULL)
      return new DomainContainer(new EnumeratedDomain(true, EnumeratedDomain::getDefaultTypeName().c_str()), true, dieEarly);

    EnumeratedDomain* enumDom = NULL;
    std::string type(value->Attribute("type"));
    if("number" == type)
      enumDom = new EnumeratedDomain(true, EnumeratedDomain::getDefaultTypeName().c_str());
    else if("string" == type)
      enumDom = new SymbolDomain(SymbolDomain::getDefaultTypeName().c_str());
    else
      check_error(ALWAYS_FAIL, "Attempted to evaluate non-string and non-numeric value!",
                   AverErr::XmlError());
    while(value != NULL) {
      check_error(value->FirstChild() != NULL, AverErr::XmlError());
      check_error(value->FirstChild()->Value() != NULL, AverErr::XmlError());
      double x;
      if("number" == std::string(value->Attribute("type"))) {
        std::istringstream i(value->FirstChild()->Value());
        bool conv = i >> x;
        check_error(conv, "Attempted to convert non-numerical value.",
                    AverErr::XmlError());
      }
      else
        x = (double) LabelStr(value->FirstChild()->Value());
      enumDom->insert(x);
      value = value->NextSiblingElement();
    }
    enumDom->close();
    return new DomainContainer(enumDom, true, dieEarly);
  }

  DomainContainer* AverHelper::evaluateIntDomain(const TiXmlElement* dom, bool dieEarly) {
    check_error(dom->FirstChildElement("UpperBound") != NULL, 
                "Interval domain without upper bound.", AverErr::XmlError());
    check_error(dom->FirstChildElement("UpperBound")->FirstChildElement("Value") != NULL,
                AverErr::XmlError());
    check_error(dom->FirstChildElement("UpperBound")->FirstChildElement("Value")->
                FirstChild() != NULL, AverErr::XmlError());
    check_error(dom->FirstChildElement("LowerBound") != NULL, 
                "Interval domain without upper bound.", AverErr::XmlError());
    check_error(dom->FirstChildElement("LowerBound")->FirstChildElement("Value") != NULL,
                AverErr::XmlError());
    check_error(dom->FirstChildElement("LowerBound")->FirstChildElement("Value")->
                FirstChild() != NULL, AverErr::XmlError());
    
    TiXmlNode* ub = 
      dom->FirstChildElement("UpperBound")->FirstChildElement("Value")->FirstChild();
    TiXmlNode* lb =
      dom->FirstChildElement("LowerBound")->FirstChildElement("Value")->FirstChild();
    double upperVal, lowerVal;
    std::istringstream u(ub->Value());
    bool conv = u >> upperVal;
    check_error(conv, "Attempted to convert a non-numerical value.",
                AverErr::XmlError());
    std::istringstream l(lb->Value());
    conv = l >> lowerVal;
    check_error(conv, "Attempted to convert a non-numerical value.",
                AverErr::XmlError());
    return new DomainContainer(new IntervalDomain(lowerVal, upperVal, 
                                                  IntervalDomain::getDefaultTypeName().c_str()), true, dieEarly);
  }

  bool AverHelper::evaluateBoolean(const std::string& op, const AbstractDomain& d1,
                                   const AbstractDomain& d2) {
    if(op == "=" || op == "==" || op == "eq") {
      if(d1.getTypeName().getKey() == SymbolDomain::getDefaultTypeName().getKey() &&
         d2.getTypeName().getKey() == SymbolDomain::getDefaultTypeName().getKey()) {
        return ((const SymbolDomain&)d1) == ((const SymbolDomain&)d2);
      }
      if(!Schema::instance()->isEnum(d1.getTypeName()) &&
         !Schema::instance()->isEnum(d2.getTypeName()) &&
         !d1.isEnumerated() && !d2.isEnumerated())
        return ((const IntervalDomain&)d1).getUpperBound() ==
          ((const IntervalDomain&)d2).getUpperBound() &&
          ((const IntervalDomain&)d1).getLowerBound() ==
          ((const IntervalDomain&)d2).getLowerBound();
      return d1 == d2;
    }
    else if(op == "<" || op == "lt")
      return getGreatest(d1) < getLeast(d2);
    else if(op == ">" || op == "gt")
      return getLeast(d1) > getGreatest(d2);
    else if(op == ">=" || op == "ge")
      return (evaluateBoolean("=", d1, d2) || evaluateBoolean(">", d1, d2));
    else if(op == "<=" || op == "le")
      return (evaluateBoolean("=", d1, d2) || evaluateBoolean("<", d1, d2));
    else if(op == "!=" || op == "ne")
      return !evaluateBoolean("=", d1, d2);
    else if(op == "in")
      return d1.isSubsetOf(d2);
    else if(op == "out")
      return !evaluateBoolean("in", d1, d2);
    else if(op == "intersects")
      return d1.intersects(d2);
    else
      check_error(ALWAYS_FAIL, "Attempted to evaluate non-boolean.",
                   AverErr::FlowError());
    return false;
  }

  double AverHelper::getGreatest(const AbstractDomain& d) {
    if(d.isInterval())
      return d.getUpperBound();
    std::list<double> vals;
    d.getValues(vals);
    return *(max_element(vals.begin(), vals.end()));
  }
  
  double AverHelper::getLeast(const AbstractDomain& d) {
    if(d.isInterval())
      return d.getLowerBound();
    std::list<double> vals;
    d.getValues(vals);
    return *(min_element(vals.begin(), vals.end()));
  }

  AverExecutionPathId AverHelper::buildExecutionPath(const TiXmlElement* assn) {
    check_error(assn != NULL);

    AverExecutionPathId end;
    std::string type(assn->Value());
    if(type == "=" || type == "==" || type == "eq" || type == "<" || type == "lt" ||
       type == ">" || type == "gt" || type == "<=" || type == "le" || type == ">=" ||
       type == "ge" || type == "!=" || type == "ne" || type == "in" || type == "out" ||
       type == "intersects") {
      end = (new AverExecutionPath(AverExecutionPath::PUSH_OP, type))->getId();
      end->setNext((new AverExecutionPath(AverExecutionPath::BOOL_OP))->getId());
    }
    else
      check_error(ALWAYS_FAIL, "Attempted to compile non-boolean.",
                   AverErr::FlowError());
    AverExecutionPathId retval;
    TiXmlElement* child = NULL;
    while((child = dynamic_cast<TiXmlElement*>(assn->IterateChildren(child))) != NULL) {
      AverExecutionPathId intermediate = _buildExecutionPath(child);
      if(retval.isNoId())
        retval = intermediate;
      else
        retval->end()->setNext(intermediate);
    }
    retval->end()->setNext(end);
    return retval;
  }

  AverExecutionPathId AverHelper::_buildExecutionPath(const TiXmlElement* xml) {
    check_error(xml != NULL);
    check_error(xml->Value() != NULL);
    
    std::string type = xml->Value();
    if(type == "Count") //count goes at end
      return buildPathAtEnd(AverExecutionPath::COUNT, xml);
    else if(type == "Tokens") //tokens comes before trim statements
      return buildPathAtFront(AverExecutionPath::QUERY_TOKENS, xml);
    else if(type == "Objects") //objects comes before trim statements
      return buildPathAtFront(AverExecutionPath::QUERY_OBJECTS, xml);
    else if(type == "Property") //property goes at end
      return buildPathAtEnd(AverExecutionPath::PROPERTY, xml);
    else if(type == "Entity") //entity goes at end
      return buildEntityPath(xml);
      //return buildPathAtEnd(AverExecutionPath::ENTITY, xml);
    else if(type == "Transactions") {
      warn("Transactions are not currently implemented.");
      return (new AverExecutionPath(AverExecutionPath::NO_OP))->getId();
    }
    else if(type == "start")
      return buildTrimPath(xml, AverExecutionPath::TRIM_TOKENS_BY_START);
    else if(type == "end")
      return buildTrimPath(xml, AverExecutionPath::TRIM_TOKENS_BY_END);
    else if(type == "status")
      return buildTrimPath(xml, AverExecutionPath::TRIM_TOKENS_BY_STATUS);
    else if(type == "predicate")
      return buildTrimPath(xml, AverExecutionPath::TRIM_TOKENS_BY_PRED);
    else if(type == "variable") {
      TiXmlNode* parent = xml->Parent();
      if("Tokens" == std::string(parent->Value()))
        return buildTrimPath(xml, AverExecutionPath::TRIM_TOKENS_BY_VAR);
      else if("Objects" == std::string(parent->Value()))
        return buildTrimPath(xml, AverExecutionPath::TRIM_OBJECTS_BY_VAR);
      else
        check_error(ALWAYS_FAIL, 
                     "Found a variable trim element not part of a Tokens or Objects query!",
                     AverErr::XmlError());
    }
    else if(type == "name") {
      TiXmlNode* parent = xml->Parent();
      if("Objects" == std::string(parent->Value()))
        return buildTrimPath(xml, AverExecutionPath::TRIM_OBJECTS_BY_NAME);
      else if("Transactions" == std::string(parent->Value())) {
        check_error(ALWAYS_FAIL, "Transactions are not currently implemented.",
                     AverErr::ExecutionError());
      }
      else {
        check_error(ALWAYS_FAIL, 
                     "Found a name trim element not part of an Objects or Transactions query!",
                     AverErr::XmlError());
      }
    }
    else if(type == "type") {
      check_error(ALWAYS_FAIL,
                   "Transactions are not currently implemented.", 
                   AverErr::ExecutionError());
    }
    else if(type == "EnumeratedDomain")
      return (new AverExecutionPath(AverExecutionPath::PUSH_DOMAIN, 
                                    evaluateEnumDomain(xml, false)))->getId();
    else if(type == "IntervalDomain")
      return (new AverExecutionPath(AverExecutionPath::PUSH_DOMAIN,
                                    evaluateIntDomain(xml, false)))->getId();
    else {
      check_error(ALWAYS_FAIL, "Don't know how to compile xml.", 
                   AverErr::XmlError());
    }
    return AverExecutionPathId::noId();
  }

  AverExecutionPathId AverHelper::buildPathAtEnd(const AverExecutionPath::Actions action,
                                                 const TiXmlElement* xml) {
    AverExecutionPathId retval;
    TiXmlElement* child = NULL;
    AverExecutionPathId slider;
    while((child = dynamic_cast<TiXmlElement*>(xml->IterateChildren(child))) != NULL) {
      AverExecutionPathId intermediate = _buildExecutionPath(child);
      if(retval.isNoId()) {
        retval = intermediate;
        slider = retval->end();
      }
      else {
        slider->setNext(intermediate);
        slider = slider->end();
      }
    }
    
    retval->end()->setNext((new AverExecutionPath(action))->getId());
    return retval;
  }

  AverExecutionPathId AverHelper::buildPathAtFront(const AverExecutionPath::Actions action,
                                                   const TiXmlElement* xml) {
    AverExecutionPathId retval = (new AverExecutionPath(action))->getId();
    AverExecutionPathId slider = retval->end();
    TiXmlElement* child = NULL;
    while((child = dynamic_cast<TiXmlElement*>(xml->IterateChildren(child))) != NULL) {
      slider->setNext(_buildExecutionPath(child));
      slider = slider->end();
    }
    return retval;
  }

  //stack order: index, domain

  AverExecutionPathId AverHelper::buildEntityPath(const TiXmlElement* xml) {
    check_error(xml != NULL);
    check_error(std::string(xml->Value()) == "Entity");
    
    TiXmlElement* index = xml->FirstChildElement("Index");
    check_error(index != NULL);
    
    TiXmlElement* domain = xml->FirstChildElement("Domain");
    check_error(domain != NULL);

    AverExecutionPathId retval = _buildExecutionPath(domain->FirstChildElement());
    retval->end()->setNext(_buildExecutionPath(index->FirstChildElement()));
    retval->end()->setNext((new AverExecutionPath(AverExecutionPath::ENTITY))->getId());
    return retval;
  }

  AverExecutionPathId AverHelper::buildTrimPath(const TiXmlElement* xml,
                                                const AverExecutionPath::Actions action) {
    check_error(xml->FirstChildElement() != NULL, AverErr::XmlError());
    AverExecutionPathId retval;
    if(action != AverExecutionPath::TRIM_TOKENS_BY_VAR &&
       action != AverExecutionPath::TRIM_OBJECTS_BY_VAR) {
      check_error(xml->Attribute("operator") != NULL, AverErr::XmlError());

      retval = 
        (new AverExecutionPath(AverExecutionPath::PUSH_OP, 
                                 xml->Attribute("operator")))->getId();
      AverExecutionPathId dom = _buildExecutionPath(xml->FirstChildElement());
      retval->setNext(dom);
      dom->setNext((new AverExecutionPath(action))->getId());
    }
    else {
      check_error(xml->FirstChildElement("name") != NULL, AverErr::XmlError());
      check_error(xml->FirstChildElement("value") != NULL, AverErr::XmlError());

      TiXmlElement* name = xml->FirstChildElement("name");
      TiXmlElement* value = xml->FirstChildElement("value");
      
      check_error(name->Attribute("operator") != NULL, AverErr::XmlError());
      check_error(name->FirstChildElement() != NULL, AverErr::XmlError());
      check_error(value->Attribute("operator") != NULL, AverErr::XmlError());
      check_error(value->FirstChildElement() != NULL, AverErr::XmlError());

      retval = 
        (new AverExecutionPath(AverExecutionPath::PUSH_OP, 
                               value->Attribute("operator")))->getId();
      AverExecutionPathId slider = retval;
      slider->setNext(_buildExecutionPath(value->FirstChildElement()));
      slider = slider->end();
      slider->setNext((new AverExecutionPath(AverExecutionPath::PUSH_OP,
                                            name->Attribute("operator")))->getId());
      slider = slider->end();
      slider->setNext(_buildExecutionPath(name->FirstChildElement()));
      slider = slider->end();
      slider->setNext((new AverExecutionPath(action))->getId());
    }
    return retval;
  }
  
  bool AverHelper::execute(const AverExecutionPathId& exec) {
    AverExecutionPathId currAction = exec;
    while(!currAction.isNoId()) {
      switch(currAction->action()) {
      case AverExecutionPath::COUNT:
        count();
        break;
      case AverExecutionPath::QUERY_TOKENS:
        check_error(s_db.isValid(), "Attempted to execute tests without a database.",
                    AverErr::FlowError());
        queryTokens();
        break;
      case AverExecutionPath::QUERY_OBJECTS:
        check_error(s_db.isValid(), "Attempted to execute tests without a database.",
                    AverErr::FlowError());
        queryObjects();
        break;
      case AverExecutionPath::PROPERTY:
        property();
        break;
      case AverExecutionPath::ENTITY:
        entity();
        break;
      case AverExecutionPath::TRIM_TOKENS_BY_PRED:
        trimTokensByPredicate();
        break;
      case AverExecutionPath::TRIM_TOKENS_BY_VAR:
        trimTokensByVariable();
        break;
      case AverExecutionPath::TRIM_TOKENS_BY_START:
        trimTokensByStart();
        break;
      case AverExecutionPath::TRIM_TOKENS_BY_END:
        trimTokensByEnd();
        break;
      case AverExecutionPath::TRIM_TOKENS_BY_STATUS:
        trimTokensByStatus();
        break;
      case AverExecutionPath::TRIM_TOKENS_BY_PATH:
        trimTokensByPath();
        break;
      case AverExecutionPath::TRIM_OBJECTS_BY_NAME:
        trimObjectsByName();
        break;
      case AverExecutionPath::TRIM_OBJECTS_BY_VAR:
        trimObjectsByVariable();
        break;
      case AverExecutionPath::PUSH_DOMAIN:
        s_domStack.push(currAction->domain());
        break;
      case AverExecutionPath::PUSH_OP:
        s_opStack.push(currAction->op());
        break;
      case AverExecutionPath::BOOL_OP:
        return boolOp();
      case AverExecutionPath::NO_OP:
        break;
      default:
        check_error(ALWAYS_FAIL, "Attempted to execute invalid operation.", 
                     AverErr::ExecutionError());
      }
      currAction = currAction->next();
    }
    check_error(ALWAYS_FAIL, "Reached end of execution path without a boolean statement.",
                 AverErr::ExecutionError());
    return false;
  }

  void AverHelper::count() {
    unsigned int stackSize = s_domStack.size();
    DomainContainer* temp = s_domStack.top();
    s_domStack.pop();
    s_domStack.push(new DomainContainer(new NumericDomain((*temp)->getSize()), true, true));
    if(debug)
      std::cout << "===>count " << *(*temp) << " => " << (*temp)->getSize() << std::endl;
    check_error(stackSize == s_domStack.size());
    if(temp->earlyDeath()) delete temp;
  }

  void AverHelper::queryTokens() {
    unsigned int stackSize = s_domStack.size();
    s_domStack.push(tokenSetToDomain(s_db->getTokens()));
    if(debug)
      std::cout << "===>queryTokens => " << *tokenSetToDomain(s_db->getTokens()) << std::endl;
    check_error(stackSize == s_domStack.size() - 1);
  }

  void AverHelper::queryObjects() {
    unsigned int stackSize = s_domStack.size();
    s_domStack.push(objectSetToDomain(s_db->getObjects()));
    if(debug)
      std::cout << "===>queryObjects => " << *objectSetToDomain(s_db->getObjects()) << std::endl;
    check_error(stackSize == s_domStack.size() - 1);
  }

  void AverHelper::property() {
    unsigned int stackSize = s_domStack.size();
    DomainContainer* nameDom = s_domStack.top();
    s_domStack.pop();
    
    check_error((*nameDom)->isSingleton(), AverErr::ExecutionError());

    DomainContainer* dom = s_domStack.top();
    s_domStack.pop();

    check_error((*dom)->isSingleton(), AverErr::ExecutionError());
    
    LabelStr name((*nameDom)->getSingletonValue());
    double idNum = (*dom)->getSingletonValue();
    
    std::vector<ConstrainedVariableId> vars;
    EntityId id(idNum);
    if(TokenId::convertable(id))
      vars = (TokenId(id))->getVariables();
    else if(ObjectId::convertable(id))
      vars = (ObjectId(id))->getVariables();
    else
      check_error(ALWAYS_FAIL, AverErr::ExecutionError());
    
    if(debug)
      std::cout << "===>property " << id << " " << (*nameDom) << " " << (*dom) << " => ";

    bool foundVar = false;
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin();
        it != vars.end(); ++it) {
      ConstrainedVariableId var = *it;
      check_error(var.isValid());

      std::string sname(var->getName().c_str());
      sname = sname.substr(sname.rfind(".") + 1);

      if((foundVar = (LabelStr(sname) == name))) {
        if(debug)
          std::cout << *(const_cast<AbstractDomain*>(&(var->lastDomain()))) << std::endl;

        s_domStack.push(new DomainContainer(const_cast<AbstractDomain*>(&(var->lastDomain())), false, true));
        check_error(stackSize == s_domStack.size() + 1);
        return;
      }
    }
    check_error(foundVar, "Couldn't find property.", 
                 AverErr::ExecutionError());
    if(dom->earlyDeath()) delete dom;
    if(nameDom->earlyDeath()) delete nameDom;
  }

  void AverHelper::entity() {
    unsigned int stackSize = s_domStack.size();
    DomainContainer* indexDom = s_domStack.top();
    s_domStack.pop();
    DomainContainer* dom = s_domStack.top();
    s_domStack.pop();
    
    double index = (*indexDom)->getSingletonValue();
    std::list<double> values;
    (*dom)->getValues(values);
    check_error(index <= values.size());
    std::list<double>::const_iterator it = values.begin();
    for(; it != values.end() && index != 0; ++it, --index) {}
    if(debug)
      std::cout << "===>entity " << index << " " << *(dom) << " => " << *it << std::endl;
    s_domStack.push(new DomainContainer(new EnumeratedDomain(*it, (*dom)->isNumeric(), (*dom)->getTypeName().c_str()), true, true));
    check_error(stackSize == s_domStack.size() + 1);
    if(indexDom->earlyDeath()) delete indexDom;
    if(indexDom->earlyDeath()) delete dom;
  }

  void AverHelper::trimTokensByPredicate() {
    unsigned int stackSize = s_domStack.size();

    std::string op = s_opStack.top();
    s_opStack.pop();
    DomainContainer* predDom = s_domStack.top();
    s_domStack.pop();

    if(debug)
      std::cout << "===>trimTokensByPredicate " << *(*predDom) << "  "  << op << " " << *s_domStack.top() << " => ";
   
    DomainContainer* t = s_domStack.top();
    TokenSet tokens = domainToTokenSet(t);
    s_domStack.pop();

    TokenSet trimmedTokens;
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      check_error(tok.isValid());
      SymbolDomain pred((double)tok->getPredicateName());
      if(debug)
        std::cout << "@comparing " << pred << " " << *(*predDom) << " => " << evaluateBoolean(op, pred, *(*predDom)) << std::endl;
      if(evaluateBoolean(op, pred, *(*predDom)))
        trimmedTokens.insert(tok);
    }

    s_domStack.push(tokenSetToDomain(trimmedTokens));

    if(debug)
      std::cout << *tokenSetToDomain(trimmedTokens) << std::endl;
    check_error(stackSize == s_domStack.size() + 1);
    if(predDom->earlyDeath()) delete predDom;
    if(t->earlyDeath()) delete t;
  }

  void AverHelper::trimTokensByVariable() {
    unsigned int stackSize = s_domStack.size();
    std::string nameOp = s_opStack.top();
    s_opStack.pop();
    
    DomainContainer* nameDom = s_domStack.top();
    s_domStack.pop();

    std::string domOp = s_opStack.top();
    s_opStack.pop();
    
    DomainContainer* domDom = s_domStack.top();
    s_domStack.pop();

    if(debug)
      std::cout << "===>trimTokensByVariable " << *nameDom << " " << nameOp << " " << *domDom << " " << domOp << " " << *s_domStack.top() << " => ";

    DomainContainer* t = s_domStack.top();
    TokenSet tokens = domainToTokenSet(t);
    s_domStack.pop();

    TokenSet trimmedTokens;

    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      check_error(tok.isValid());
      const std::vector<ConstrainedVariableId>& vars = tok->getVariables();

      for(std::vector<ConstrainedVariableId>::const_iterator vit = vars.begin();
          vit != vars.end(); ++vit) {
        ConstrainedVariableId var = *vit;
        check_error(var.isValid());
        SymbolDomain varName(var->getName());
        if(evaluateBoolean(nameOp, varName, *(*nameDom))) {
          if(evaluateBoolean(domOp, var->lastDomain(), *(*domDom)))
            trimmedTokens.insert(tok);
        }
      }
    }
    s_domStack.push(tokenSetToDomain(trimmedTokens));
    if(debug)
      std::cout << *tokenSetToDomain(trimmedTokens) << std::endl;
    check_error(stackSize == s_domStack.size() + 2);
    if(t->earlyDeath()) delete t;
    if(domDom->earlyDeath()) delete domDom;
    if(nameDom->earlyDeath())delete nameDom;
  }

  void AverHelper::trimTokensByStart() {
    unsigned int stackSize = s_domStack.size();
    std::string op = s_opStack.top();
    s_opStack.pop();
    
    DomainContainer* startDom = s_domStack.top();
    s_domStack.pop();
    
    if(debug)
      std::cout << "===>trimTokensByStart " << *startDom << " " << op << " " << *s_domStack.top() << " => ";

    DomainContainer* t = s_domStack.top();
    TokenSet tokens = domainToTokenSet(t);
    s_domStack.pop();
    
    TokenSet trimmedTokens;

    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      check_error(tok.isValid());
      if(evaluateBoolean(op, tok->getStart()->lastDomain(), *(*startDom)))
        trimmedTokens.insert(tok);
    }
    
    if(debug)
      std::cout << *tokenSetToDomain(trimmedTokens) << std::endl;

    s_domStack.push(tokenSetToDomain(trimmedTokens));
    check_error(stackSize == s_domStack.size() + 1);
    if(t->earlyDeath()) delete t;
    if(startDom->earlyDeath()) delete startDom;
  }

  void AverHelper::trimTokensByEnd() {
    unsigned int stackSize = s_domStack.size();
    std::string op = s_opStack.top();
    s_opStack.pop();
    
    DomainContainer* endDom = s_domStack.top();
    s_domStack.pop();

    if(debug)
      std::cout << "===>trimTokensByEnd " << *endDom << " " << op << " " << *s_domStack.top() << " => ";

    DomainContainer* t = s_domStack.top();
    TokenSet tokens = domainToTokenSet(t);
    s_domStack.pop();


    TokenSet trimmedTokens;

    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      check_error(tok.isValid());
      if(evaluateBoolean(op, tok->getEnd()->lastDomain(), *(*endDom)))
        trimmedTokens.insert(tok);
    }
    if(debug)
      std::cout << *tokenSetToDomain(trimmedTokens) << std::endl;
    s_domStack.push(tokenSetToDomain(trimmedTokens));
    check_error(stackSize == s_domStack.size() + 1);
    if(t->earlyDeath()) delete t;
    if(endDom->earlyDeath()) delete endDom;
  }

  void AverHelper::trimTokensByStatus() {
    unsigned int stackSize = s_domStack.size();
    std::string op = s_opStack.top();
    s_opStack.pop();
    
    DomainContainer* statusDom = s_domStack.top();
    s_domStack.pop();

    if(debug)
      std::cout << "===>trimTokensByStatus " << *statusDom << " " << op << " " << *s_domStack.top() << " => ";
    
    DomainContainer* t = s_domStack.top();
    TokenSet tokens = domainToTokenSet(t);
    s_domStack.pop();

    TokenSet trimmedTokens;
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      check_error(tok.isValid());
      if(evaluateBoolean(op, tok->getState()->lastDomain(), *(*statusDom)))
        trimmedTokens.insert(tok);
    }
    
    if(debug)
      std::cout << *tokenSetToDomain(trimmedTokens) << std::endl;

    s_domStack.push(tokenSetToDomain(trimmedTokens));
    check_error(stackSize == s_domStack.size() + 1);
    if(statusDom->earlyDeath()) delete statusDom;
    if(t->earlyDeath()) delete t;
  }

  void AverHelper::trimTokensByPath() {
    check_error(ALWAYS_FAIL, "Selecting tokens by path is not implemented.",
                 AverErr::ExecutionError());
  }

  //gnats: the trim[Tokens|Objects]by[Name|Predicate] functions need to accept
  //some boolean ops
  void AverHelper::trimObjectsByName() {
    unsigned int stackSize = s_domStack.size();
    std::string op = s_opStack.top();
    s_opStack.pop();

    DomainContainer* nameDom = s_domStack.top();
    s_domStack.pop();

    if(debug)
      std::cout << "===>trimObjectsByName " << *nameDom << " " << op << " " << *s_domStack.top() << " => ";

    DomainContainer* o = s_domStack.top();
    ObjectSet objs = domainToObjectSet(o);
    s_domStack.pop();

    ObjectSet trimmedObjs;
    for(ObjectSet::iterator it = objs.begin(); it != objs.end(); ++it) {
      ObjectId obj = *it;
      check_error(obj.isValid());
      SymbolDomain name((double)obj->getName());
      if(evaluateBoolean(op, name, *(*nameDom)))
        trimmedObjs.insert(obj);
    }
    if(debug)
      std::cout << *objectSetToDomain(trimmedObjs) << std::endl;
    s_domStack.push(objectSetToDomain(trimmedObjs));
    check_error(stackSize == s_domStack.size() + 1);
    if(o->earlyDeath()) delete o;
    if(nameDom->earlyDeath()) delete nameDom;
  }
  
  void AverHelper::trimObjectsByVariable() {
    unsigned int stackSize = s_domStack.size();
    std::string nameOp = s_opStack.top();
    s_opStack.pop();
    
    DomainContainer* nameDom = s_domStack.top();
    s_domStack.pop();

    std::string domOp = s_opStack.top();
    s_opStack.pop();

    DomainContainer* domDom = s_domStack.top();
    s_domStack.pop();

    if(debug)
      std::cout << "===>trimObjectsByVariable " << *nameDom << " " << nameOp << " " << *domDom << " " << domOp << " " << *s_domStack.top() << " => ";

    DomainContainer* o = s_domStack.top();
    ObjectSet objs = domainToObjectSet(o);
    s_domStack.pop();

    ObjectSet trimmedObjs;
    for(ObjectSet::iterator it = objs.begin(); it != objs.end(); ++it) {
      ObjectId obj = *it;
      check_error(obj.isValid());
      
      const std::vector<ConstrainedVariableId>& vars = obj->getVariables();
      for(std::vector<ConstrainedVariableId>::const_iterator vit = vars.begin();
          vit != vars.end(); ++vit) {
        ConstrainedVariableId var = *vit;
        check_error(var.isValid());
        std::string sname(var->getName().c_str());
        sname = sname.substr(sname.rfind(".") + 1);

        SymbolDomain name((double)LabelStr(sname));
        if(evaluateBoolean(nameOp, name, *(*nameDom)))
          if(evaluateBoolean(domOp, var->lastDomain(), *(*domDom)))
            trimmedObjs.insert(obj);
      }
    }

    if(debug)
      std::cout << *objectSetToDomain(trimmedObjs) << std::endl;

    s_domStack.push(objectSetToDomain(trimmedObjs));
    check_error(stackSize == s_domStack.size() + 2);
    if(o->earlyDeath()) delete o;
    if(nameDom->earlyDeath()) delete nameDom;
    if(domDom->earlyDeath()) delete domDom;
  }

  bool AverHelper::boolOp() {
    unsigned int stackSize = s_domStack.size();
    std::string op = s_opStack.top();
    s_opStack.pop();
    
    DomainContainer* d1 = s_domStack.top();
    s_domStack.pop();
    
    DomainContainer* d2 = s_domStack.top();
    s_domStack.pop();

    //return evaluateBoolean(op, *d1, *d2);
    check_error(stackSize == s_domStack.size() + 2);

    bool retval = evaluateBoolean(op, *(*d2), *(*d1));
    if(debug)
      std::cout << "===>boolOp " << *d1 << " " << op << " " << *d2 << " => " << retval << std::endl;
    if(d1->earlyDeath()) delete d1;
    if(d2->earlyDeath()) delete d2;
    return retval;
  }  

  DomainContainer* AverHelper::tokenSetToDomain(const TokenSet& tokens) {
    EnumeratedDomain* retval = new EnumeratedDomain(false, 
                                                    EnumeratedDomain::getDefaultTypeName().
                                                    c_str());
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      check_error(tok.isValid());
//      std::cout << " in tokenSetToDomain " << (double) tok << tok << tok->getKey() << std::endl;
      retval->insert((double) tok);
    }
    retval->close();
    return new DomainContainer(retval, true, true);
  }

  TokenSet AverHelper::domainToTokenSet(const DomainContainer* dom) {
    TokenSet retval;
    std::list<double> ids;
    (*dom)->getValues(ids);
    for(std::list<double>::iterator it = ids.begin(); it != ids.end(); ++it) {
      EntityId id(*it);
      check_error(id.isValid());
      check_error(TokenId::convertable(id));
      retval.insert(TokenId(id));
    }
    return retval;
  }

  DomainContainer* AverHelper::objectSetToDomain(const ObjectSet& objs) {
    EnumeratedDomain* retval = new EnumeratedDomain(false,
                                                    EnumeratedDomain::getDefaultTypeName().
                                                    c_str());
    for(ObjectSet::iterator it = objs.begin(); it != objs.end(); ++it) {
      ObjectId obj = *it;
      check_error(obj.isValid());
      retval->insert((double) obj);
    }
    retval->close();
    return new DomainContainer(retval, true, true);
  }

  ObjectSet AverHelper::domainToObjectSet(const DomainContainer* dom) {
    ObjectSet retval;
    std::list<double> ids;
    (*dom)->getValues(ids);
    for(std::list<double>::iterator it = ids.begin(); it != ids.end(); ++it) {
      EntityId id(*it);
      check_error(id.isValid());
      check_error(ObjectId::convertable(id));
      retval.insert(ObjectId(id));
    }
    return retval;
  }
}
