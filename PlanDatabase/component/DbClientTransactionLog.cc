#include "DbClientTransactionLog.hh"
#include "Object.hh"
#include "Token.hh"
#include "Utils.hh"
#include "tinyxml.h"

namespace PLASMA {

  DbClientTransactionLog::DbClientTransactionLog(const DbClientId& client, bool chronologicalBacktracking)
    : DbClientListener(client)
  {
    m_chronologicalBacktracking = chronologicalBacktracking;
    m_tokensCreated = 0;
  }

  DbClientTransactionLog::~DbClientTransactionLog(){
    cleanup(m_bufferedTransactions);
  }

  const std::list<TiXmlElement*>& DbClientTransactionLog::getBufferedTransactions() const {return m_bufferedTransactions;}

  void DbClientTransactionLog::notifyVariableCreated(const ConstrainedVariableId& variable){
    TiXmlElement * element = allocateXmlElement("var");
    const AbstractDomain& baseDomain = variable->baseDomain();
    std::string type = baseDomain.getTypeName().toString();
    if (type == "object") {
      ObjectId object = baseDomain.getLowerBound();
      check_error(object.isValid());
      type = object->getType().toString();
    }
    element->SetAttribute("type", type);
    if (LabelStr::isString(variable->getName())) {
      element->SetAttribute("name", variable->getName().toString());
    }
    if (!baseDomain.isEmpty()) {
      TiXmlElement * value = abstractDomainAsXml(&baseDomain);
      element->LinkEndChild(value);
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object){
    const std::vector<ConstructorArgument> noArguments;
    notifyObjectCreated(object, noArguments);
  }

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object, const std::vector<ConstructorArgument>& arguments){
    TiXmlElement * element = allocateXmlElement("new");
    if (LabelStr::isString(object->getName())) {
      element->SetAttribute("name", object->getName().toString());
    }
    element->SetAttribute("type", object->getType().toString());
    std::vector<ConstructorArgument>::const_iterator iter;
    for (iter = arguments.begin() ; iter != arguments.end() ; iter++) {
      element->LinkEndChild(abstractDomainAsXml(iter->second));
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyClosed(){
    TiXmlElement * element = allocateXmlElement("invoke");
    element->SetAttribute("name", "close");
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyClosed(const LabelStr& objectType){
    TiXmlElement * element = allocateXmlElement("invoke");
    element->SetAttribute("name", "close");
    element->SetAttribute("identifier", objectType.toString());
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyTokenCreated(const TokenId& token){
    TiXmlElement * element = allocateXmlElement("goal");
    TiXmlElement * instance = allocateXmlElement("predicateinstance");
    instance->SetAttribute("name", m_tokensCreated++);
    check_error(LabelStr::isString(token->getName()));
    instance->SetAttribute("type", token->getName().toString());
    element->LinkEndChild(instance);
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor){
    TiXmlElement * element = allocateXmlElement("constrain");
    TiXmlElement * object_el = allocateXmlElement("object");
    object_el->SetAttribute("name", object->getName().toString());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(token));
    if (!successor.isNoId()) {
      element->LinkEndChild(tokenAsXml(successor));
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyFreed(const ObjectId& object, const TokenId& token){
    if (m_chronologicalBacktracking) {
      check_error(strcmp(m_bufferedTransactions.back()->Value(), "constrain") == 0, 
                  "chronological backtracking assumption violated");
      popTransaction();
      return;
    }
    TiXmlElement * element = allocateXmlElement("free");
    TiXmlElement * object_el = allocateXmlElement("object");
    object_el->SetAttribute("name", object->getName().toString());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyActivated(const TokenId& token){
    TiXmlElement * element = allocateXmlElement("activate");
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyMerged(const TokenId& token, const TokenId& activeToken){
    TiXmlElement * element = allocateXmlElement("merge");
    element->LinkEndChild(tokenAsXml(token));
    element->LinkEndChild(tokenAsXml(activeToken));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyRejected(const TokenId& token){
    TiXmlElement * element = allocateXmlElement("reject");
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyCancelled(const TokenId& token){
    if (m_chronologicalBacktracking) {
      check_error((strcmp(m_bufferedTransactions.back()->Value(), "activate") == 0) ||
                  (strcmp(m_bufferedTransactions.back()->Value(), "reject") == 0) ||
                  (strcmp(m_bufferedTransactions.back()->Value(), "merge") == 0),
                  "chronological backtracking assumption violated");
      popTransaction();
      return;
    }
    TiXmlElement * element = allocateXmlElement("cancel");
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId& constraint){
    TiXmlElement * element = allocateXmlElement("invoke");
    element->SetAttribute("name", constraint->getName().toString());    
    const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
    std::vector<ConstrainedVariableId>::const_iterator iter;
    for (iter = variables.begin() ; iter != variables.end() ; iter++) {
      const ConstrainedVariableId variable = *iter;
      element->LinkEndChild(variableAsXml(variable));
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyVariableSpecified(const ConstrainedVariableId& variable){
    TiXmlElement * element = allocateXmlElement("specify");
    element->LinkEndChild(variableAsXml(variable));
    element->LinkEndChild(abstractDomainAsXml(&variable->specifiedDomain()));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyVariableReset(const ConstrainedVariableId& variable){
    if (m_chronologicalBacktracking) {
      check_error(strcmp(m_bufferedTransactions.back()->Value(), "specify") == 0,
                  "chronological backtracking assumption violated");
      popTransaction();
      return;
    }
    TiXmlElement * element = allocateXmlElement("reset");
    element->LinkEndChild(variableAsXml(variable));
    pushTransaction(element);
  }

  void DbClientTransactionLog::flush(std::ostream& os){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      os << **iter << std::endl;
    }
    cleanup(m_bufferedTransactions);
  }

  std::string
  DbClientTransactionLog::domainValueAsString(const AbstractDomain * domain, double value)
  {
    if (domain->getType() == AbstractDomain::BOOL) {
      return (value ? "true" : "false");
    } else if (domain->isNumeric()) {
      if (domain->getType() == AbstractDomain::INT_INTERVAL) {
        char s[64];
        snprintf(s, sizeof(s), "%d", (int)value);
        return s;
      } else {
        char s[64];
        snprintf(s, sizeof(s), "%16f", (double)value);
        return s;
      }
    } else if (LabelStr::isString(domain->getUpperBound())) {
      const LabelStr& label = value;
      return label.toString();
    } else {
      ObjectId object = value;
      check_error(object.isValid());
      return object->getName().toString();
    }
  }

  TiXmlElement *
  DbClientTransactionLog::domainValueAsXml(const AbstractDomain * domain, double value)
  {
    TiXmlElement * element = NULL;
    if (domain->isNumeric()) {
      element = allocateXmlElement(domain->getTypeName().toString());
    } else if (LabelStr::isString(value)) {
      if (domain->getTypeName() == LabelStr("string")) {
        element = allocateXmlElement("string");
      } else {
        element = allocateXmlElement("symbol");
        element->SetAttribute("type", domain->getTypeName().toString());
      }
    } else {
      element = allocateXmlElement("object");
    }
    element->SetAttribute("value", domainValueAsString(domain, value));
    return element;
  }

  TiXmlElement *
  DbClientTransactionLog::abstractDomainAsXml(const AbstractDomain * domain)
  {
    check_error(!domain->isEmpty());
    if (domain->isSingleton()) {
      return domainValueAsXml(domain, domain->getSingletonValue());
    } else if (domain->isEnumerated()) {
      TiXmlElement * element = allocateXmlElement("set");
      std::list<double> values;
      domain->getValues(values);
      std::list<double>::const_iterator iter;
      for (iter = values.begin() ; iter != values.end() ; iter++) {
        element->LinkEndChild(domainValueAsXml(domain, *iter));
      }
      return element;
    } else if (domain->isInterval()) {
      TiXmlElement * element = allocateXmlElement("interval");
      element->SetAttribute("type", domain->getTypeName().toString());
      element->SetAttribute("min", domainValueAsString(domain, domain->getLowerBound()));
      element->SetAttribute("max", domainValueAsString(domain, domain->getUpperBound()));
      return element;
    }
    check_error(ALWAYS_FAILS);
    return NULL;
  }
  
  TiXmlElement *
  DbClientTransactionLog::tokenAsXml(const TokenId& token) const
  {
    TiXmlElement * token_el = allocateXmlElement("token");
    token_el->SetAttribute("path", m_client->getPathAsString(token));
    return token_el;
  }

  TiXmlElement *
  DbClientTransactionLog::variableAsXml(const ConstrainedVariableId& variable) const
  {
    TiXmlElement * var_el = allocateXmlElement("variable");
    const EntityId& parent = variable->getParent();
    if (parent != EntityId::noId()) {
      if (TokenId::convertable(parent)) {
        TokenId token = parent;
        check_error(token.isValid());
        var_el->SetAttribute("token", m_client->getPathAsString(token));
      } else if (ObjectId::convertable(parent)) {
        ObjectId object = parent;
        check_error(object.isValid());
        var_el->SetAttribute("object", object->getName().toString());
      } else {
        var_el->SetAttribute("index", m_client->getIndexByVariable(variable));
        return var_el;
      } 
    } else {
      var_el->SetAttribute("index", m_client->getIndexByVariable(variable));
      return var_el;
    }
    if (variable->getIndex() != ConstrainedVariable::NO_INDEX) {
      var_el->SetAttribute("index", variable->getIndex());
    } else {
      var_el->SetAttribute("index", m_client->getIndexByVariable(variable));
      return var_el;
    }
    return var_el;
  }

  TiXmlElement * DbClientTransactionLog::allocateXmlElement(const std::string& name) const {
    TiXmlElement * element = new TiXmlElement(name);
    return element;
  }

  void DbClientTransactionLog::pushTransaction(TiXmlElement * tx){
    m_bufferedTransactions.push_back(tx);
  }

  void DbClientTransactionLog::popTransaction(){
    TiXmlElement* tx = m_bufferedTransactions.back();
    m_bufferedTransactions.pop_back();
    delete tx;
  }

}
