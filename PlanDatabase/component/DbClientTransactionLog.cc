#include "DbClientTransactionLog.hh"
#include "Object.hh"
#include "Token.hh"
#include "TransactionXml.hh"
#include "../TinyXml/tinyxml.h"

namespace Prototype {

  DbClientTransactionLog::DbClientTransactionLog(const DbClientId& client, bool chronologicalBacktracking)
    : DbClientListener(client)
  {
    m_chronologicalBacktracking = chronologicalBacktracking;
    m_tokensCreated = 0;
  }

  DbClientTransactionLog::~DbClientTransactionLog(){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      delete *iter;
    }
    m_bufferedTransactions.clear();
  }

  const std::list<TiXmlElement*>& DbClientTransactionLog::getBufferedTransactions() const {return m_bufferedTransactions;}

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object){
    const std::vector<ConstructorArgument> noArguments;
    notifyObjectCreated(object, noArguments);
  }

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object, const std::vector<ConstructorArgument>& arguments){
    TiXmlElement * element = new TiXmlElement("new");
    if (LabelStr::isString(object->getName())) {
      element->SetAttribute("name", object->getName().toString());
      element->SetAttribute("type", object->getType().toString());
    }

    std::vector<ConstructorArgument>::const_iterator iter;
    for (iter = arguments.begin() ; iter != arguments.end() ; iter++) {
      element->LinkEndChild(TransactionXml::abstractDomainAsXml(iter->second));
    }

    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyClosed(){
    TiXmlElement * element = new TiXmlElement("invoke");
    element->SetAttribute("name", "close");
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyClosed(const LabelStr& objectType){
    TiXmlElement * element = new TiXmlElement("invoke");
    element->SetAttribute("name", "close");
    TiXmlElement * id_el = new TiXmlElement("id");
    id_el->SetAttribute("name", objectType.toString());
    element->LinkEndChild(id_el);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyTokenCreated(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("goal");
    TiXmlElement * instance = new TiXmlElement("predicateinstance");
    instance->SetAttribute("name", m_tokensCreated++);
    check_error(LabelStr::isString(token->getName()));
    instance->SetAttribute("type", token->getName().toString());
    element->LinkEndChild(instance);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor){
    TiXmlElement * element = new TiXmlElement("constrain");
    TiXmlElement * object_el = new TiXmlElement("object");
    object_el->SetAttribute("name", object->getName().toString());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(token));
    if (!successor.isNoId()) {
      element->LinkEndChild(tokenAsXml(successor));
    }
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyFreed(const ObjectId& object, const TokenId& token){
    if (m_chronologicalBacktracking) {
      m_bufferedTransactions.pop_back();
      return;
    }
    TiXmlElement * element = new TiXmlElement("free");
    TiXmlElement * object_el = new TiXmlElement("object");
    object_el->SetAttribute("name", object->getName().toString());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(token));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyActivated(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("activate");
    element->LinkEndChild(tokenAsXml(token));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyMerged(const TokenId& token, const TokenId& activeToken){
    TiXmlElement * element = new TiXmlElement("merge");
    element->LinkEndChild(tokenAsXml(token));
    element->LinkEndChild(tokenAsXml(activeToken));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyRejected(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("reject");
    element->LinkEndChild(tokenAsXml(token));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyCancelled(const TokenId& token){
    if (m_chronologicalBacktracking) {
      m_bufferedTransactions.pop_back();
      return;
    }
    TiXmlElement * element = new TiXmlElement("cancel");
    element->LinkEndChild(tokenAsXml(token));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId& constraint){
    TiXmlElement * element = new TiXmlElement("invoke");
    element->SetAttribute("name", constraint->getName().toString());    
    const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
    std::vector<ConstrainedVariableId>::const_iterator iter;
    for (iter = variables.begin() ; iter != variables.end() ; iter++) {
      const ConstrainedVariableId variable = *iter;
      element->LinkEndChild(variableAsXml(variable));
    }
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId& constraint, const AbstractDomain& domain){
    TiXmlElement * element = new TiXmlElement("invoke");
    element->SetAttribute("name", constraint->getName().toString());    
    const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
    check_error(variables.size() == 1);
    element->LinkEndChild(variableAsXml(variables[0]));
    element->LinkEndChild(TransactionXml::abstractDomainAsXml(&domain));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyVariableSpecified(const ConstrainedVariableId& variable){
    TiXmlElement * element = new TiXmlElement("specify");
    element->LinkEndChild(variableAsXml(variable));
    element->LinkEndChild(TransactionXml::abstractDomainAsXml(&variable->specifiedDomain()));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyVariableReset(const ConstrainedVariableId& variable){
    if (m_chronologicalBacktracking) {
      m_bufferedTransactions.pop_back();
      return;
    }
    TiXmlElement * element = new TiXmlElement("reset");
    element->LinkEndChild(variableAsXml(variable));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::flush(std::ostream& os){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      os << **iter << std::endl;
      delete *iter;
    }
    m_bufferedTransactions.clear();
  }

  TiXmlElement *
  DbClientTransactionLog::tokenAsXml(const TokenId& token) const
  {
    TiXmlElement * token_el = new TiXmlElement("token");
    token_el->SetAttribute("path", m_client->getPathAsString(token));
    return token_el;
  }

  TiXmlElement *
  DbClientTransactionLog::variableAsXml(const ConstrainedVariableId& variable) const
  {
    TiXmlElement * var_el = new TiXmlElement("variable");
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

}
