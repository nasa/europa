#include "Debug.hh"
#include "Utils.hh"
#include "EnumeratedDomain.hh"
#include "BoolDomain.hh"
#include "StringDomain.hh"
#include "SymbolDomain.hh"
#include "tinyxml.h"
#include "Object.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "DbClientTransactionLog.hh"

namespace EUROPA {

  DbClientTransactionLog::DbClientTransactionLog(const DbClientId& client, bool chronologicalBacktracking)
    : DbClientListener(client)
    , m_client(client)
  {
    m_chronologicalBacktracking = chronologicalBacktracking;
    m_tokensCreated = 0;
  }

  DbClientTransactionLog::~DbClientTransactionLog(){
    cleanup(m_bufferedTransactions);
  }

  const std::list<TiXmlElement*>& DbClientTransactionLog::getBufferedTransactions() const {return m_bufferedTransactions;}

  const bool DbClientTransactionLog::isBool(const std::string& typeName)  {
    return (strcmp(typeName.c_str(),"bool") == 0 || 
	strcmp(typeName.c_str(), "BOOL" ) == 0 || 
              strcmp(typeName.c_str(),BoolDomain::getDefaultTypeName().c_str()) == 0);
  }

  const bool DbClientTransactionLog::isInt(const std::string& typeName)  {
    return (strcmp(typeName.c_str(),"int") == 0 || 
	strcmp(typeName.c_str(), "INT_INTERVAL" ) == 0 || 
              strcmp(typeName.c_str(),BoolDomain::getDefaultTypeName().c_str()) == 0);
  }

  void DbClientTransactionLog::insertBreakpoint() {
    pushTransaction(allocateXmlElement("breakpoint"));
  }

  void DbClientTransactionLog::removeBreakpoint() {
    check_error(!m_bufferedTransactions.empty());
    checkError(m_bufferedTransactions.back()->Value() == std::string("breakpoint"),
	       "Last transaction is a " << m_bufferedTransactions.back()->Value() <<
	       ", not a breakpoint.");
    popTransaction();
  }

  void DbClientTransactionLog::notifyVariableCreated(const ConstrainedVariableId& variable){
    TiXmlElement * element = allocateXmlElement("var");
    const AbstractDomain& baseDomain = variable->baseDomain();
    std::string type = baseDomain.getTypeName().toString();
    if (m_client->getSchema()->isObjectType(type)) {
      ObjectId object = baseDomain.getLowerBound();
      check_error(object.isValid());
      type = object->getType().toString();
    }
    element->SetAttribute("type", type);
    if (LabelStr::isString(variable->getName())) {
      element->SetAttribute("name", variable->getName().toString());
    }
    debugMsg("notifyVariableCreated"," variable name = " << variable->getName().c_str() << " typeName = " << type << " type = " << baseDomain.getTypeName().c_str());

    element->SetAttribute("index", m_client->getIndexByVariable(variable));

    if (!baseDomain.isEmpty()) {
      TiXmlElement * value = abstractDomainAsXml(&baseDomain);
      element->LinkEndChild(value);
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyVariableDeleted(const ConstrainedVariableId& variable) {
    TiXmlElement* element = allocateXmlElement("deletevar");
    element->SetAttribute("index", m_client->getIndexByVariable(variable));
    element->SetAttribute("name", variable->getName().toString());
    element->SetAttribute("type", variable->baseDomain().getTypeName().toString());
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object){
    const std::vector<const AbstractDomain*> noArguments;
    notifyObjectCreated(object, noArguments);
  }

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object, const std::vector<const AbstractDomain*>& arguments){
    TiXmlElement * element = allocateXmlElement("new");
    if (LabelStr::isString(object->getName())) {
      element->SetAttribute("name", object->getName().toString());
    }
    element->SetAttribute("type", object->getType().toString());
    std::vector<const AbstractDomain*>::const_iterator iter;
    for (iter = arguments.begin() ; iter != arguments.end() ; iter++) {
      element->LinkEndChild(abstractDomainAsXml(*iter));
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyObjectDeleted(const ObjectId& object) {
    TiXmlElement* element = allocateXmlElement("deleteobject");
    element->SetAttribute("name", object->getName().toString());
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
    TiXmlElement * element = (token->isFact() ? allocateXmlElement("fact") : 
			      allocateXmlElement("goal"));
    TiXmlElement * instance = allocateXmlElement("predicateinstance");
    instance->SetAttribute("name", m_tokensCreated++);
    check_error(LabelStr::isString(token->getName()));
    instance->SetAttribute("type", token->getName().toString());
    instance->SetAttribute("path", m_client->getPathAsString(token));
    element->LinkEndChild(instance);
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyTokenDeleted(const TokenId& token,
						  const std::string& name) {
    TiXmlElement* element = allocateXmlElement("deletetoken");
    element->SetAttribute("type", token->getName().toString());
    element->SetAttribute("path", m_client->getPathAsString(token));
    if(!name.empty())
      element->SetAttribute("name", name);
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyConstrained(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){
    TiXmlElement * element = allocateXmlElement("constrain");
    TiXmlElement * object_el = allocateXmlElement("object");
    object_el->SetAttribute("name", object->getName().toString());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(predecessor));
    element->LinkEndChild(tokenAsXml(successor));
    pushTransaction(element);
  }


  void DbClientTransactionLog::notifyFreed(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){
    if(m_chronologicalBacktracking) {
      check_error(strcmp(m_bufferedTransactions.back()->Value(), "constrain") == 0,
		  "Chronological backtracking assumption violated");
      popTransaction();
      return;
    }
    TiXmlElement * element = allocateXmlElement("free");
    TiXmlElement * object_el = allocateXmlElement("object");
    object_el->SetAttribute("name", object->getName().toString());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(predecessor));
    element->LinkEndChild(tokenAsXml(successor));
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

  void DbClientTransactionLog::notifyMerged(const TokenId& token){
    TiXmlElement * element = allocateXmlElement("merge");
    element->LinkEndChild(tokenAsXml(token));
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
                  "Chronological backtracking assumption violated");
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
    element->SetAttribute("index", m_client->getIndexByConstraint(constraint));
    const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
    std::vector<ConstrainedVariableId>::const_iterator iter;
    for (iter = variables.begin() ; iter != variables.end() ; iter++) {
      const ConstrainedVariableId variable = *iter;
      element->LinkEndChild(variableAsXml(variable));
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyConstraintDeleted(const ConstraintId& constraint) {
    TiXmlElement* element = allocateXmlElement("deleteconstraint");
    element->SetAttribute("name", constraint->getName().toString());
    element->SetAttribute("index", m_client->getIndexByConstraint(constraint));
    const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
    std::vector<ConstrainedVariableId>::const_iterator iter;
    for (iter = variables.begin() ; iter != variables.end() ; iter++) {
      const ConstrainedVariableId variable = *iter;
      element->LinkEndChild(variableAsXml(variable));
    }
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyVariableSpecified(const ConstrainedVariableId& variable){
    checkError(variable->lastDomain().isSingleton(), variable->toString() << " is not a singleton.");
    TiXmlElement * element = allocateXmlElement("specify");
    element->LinkEndChild(variableAsXml(variable));
    element->LinkEndChild(domainValueAsXml(&variable->lastDomain(), variable->lastDomain().getSingletonValue()));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyVariableRestricted(const ConstrainedVariableId& variable){
    TiXmlElement * element = allocateXmlElement("restrict");
    element->LinkEndChild(variableAsXml(variable));
    element->LinkEndChild(abstractDomainAsXml(&variable->baseDomain()));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyVariableReset(const ConstrainedVariableId& variable){
    if (m_chronologicalBacktracking) {
      check_error(strcmp(m_bufferedTransactions.back()->Value(), "specify") == 0,
                  "Chronological backtracking assumption violated");
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
    if (isBool(domain->getTypeName().toString())) {
      return (value ? "true" : "false");
    }
   else 
     if (domain->isNumeric()) {
       // CMG: Do not use snprintf. Not supported on DEC
       std::stringstream ss;
       if (isInt(domain->getTypeName().toString())) {
	 ss << (int) value;
       } else {
	 ss << value;
       }
       return ss.str();
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
    LabelStr typeName = domain->getTypeName();
    if (m_client->getSchema()->isObjectType(typeName)) {
      TiXmlElement * element = allocateXmlElement("object");
      element->SetAttribute("value", domainValueAsString(domain, value));
      return element;
    }

    debugMsg("domainValueAsXml"," domain type = " << domain->getTypeName().c_str()  << " domain name = " << typeName.c_str());

    if (isBool(domain->getTypeName().toString())) {
      TiXmlElement * element = allocateXmlElement("value");
      element->SetAttribute("type", "bool");
      element->SetAttribute("name", domainValueAsString(domain, value));
      return(element);
    }
    else {
    if (domain->isNumeric()) {
      TiXmlElement * element = allocateXmlElement("value");
      element->SetAttribute("type", typeName.toString());
      element->SetAttribute("name", domainValueAsString(domain, value));
      return(element);
    }
    else {
      TiXmlElement * element = allocateXmlElement("symbol");
      element->SetAttribute("type", typeName.toString());
      element->SetAttribute("value", domainValueAsString(domain, value));
      return(element);
    }
    }

  }

  TiXmlElement *
  DbClientTransactionLog::abstractDomainAsXml(const AbstractDomain * domain)
  {
    check_error(!domain->isEmpty());
    if (domain->isSingleton()) {
      return domainValueAsXml(domain, domain->getSingletonValue());
    } else if (domain->isEnumerated()) {
      TiXmlElement * element = allocateXmlElement("set");
      element->SetAttribute("type", domain->getTypeName().toString());
      std::list<double> values;
      domain->getValues(values);
      std::list<double>::const_iterator iter;
      for (iter = values.begin() ; iter != values.end() ; iter++) {
        element->LinkEndChild(domainValueAsXml(domain, *iter));
      }
      return element;
    } else if (domain->isInterval()) {
      TiXmlElement * element = allocateXmlElement("interval");
      std::string typeName = domain->getTypeName().toString();
      element->SetAttribute("type",typeName);
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
    const EntityId& parent = variable->parent();
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
