#include "DbClientTransactionLog.hh"
#include "DbClientTransactionTokenMapper.hh"
#include "Object.hh"
#include "Token.hh"
#include "XmlUtils.hh"

namespace Prototype {

  DbClientTransactionLog::DbClientTransactionLog(const DbClientId& client, const DbClientTransactionTokenMapperId & tokenMapper)
    : DbClientListener(client)
  {
    m_tokenMapper = tokenMapper;
  }

  DbClientTransactionLog::~DbClientTransactionLog(){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      delete *iter;
    }
    m_bufferedTransactions.clear();
  }

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
      element->LinkEndChild(abstractDomainAsXml(iter->second));
    }

    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyClosed(){
    TiXmlElement * element = new TiXmlElement("close");
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyClosed(const LabelStr& objectType){
    TiXmlElement * element = new TiXmlElement("close");
    element->SetAttribute("name", objectType.toString());
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyTokenCreated(const TokenId& token){
    static int index = 0;
    TiXmlElement * element = new TiXmlElement("goal");
    TiXmlElement * instance = new TiXmlElement("predicateinstance");
    instance->SetAttribute("name", index++);
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
    TiXmlElement * token_el = new TiXmlElement("token");
    token_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(token_el);
    if (!successor.isNoId()) {
      TiXmlElement * successor_el = new TiXmlElement("token");
      successor_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(successor)));
      element->LinkEndChild(successor_el);
    }
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyFreed(const ObjectId& object, const TokenId& token){
    TiXmlElement * element = new TiXmlElement("free");
    TiXmlElement * object_el = new TiXmlElement("object");
    object_el->SetAttribute("name", object->getName().toString());
    element->LinkEndChild(object_el);
    TiXmlElement * token_el = new TiXmlElement("token");
    token_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(token_el);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyActivated(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("activate");
    TiXmlElement * token_el = new TiXmlElement("token");
    token_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(token_el);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyMerged(const TokenId& token, const TokenId& activeToken){
    TiXmlElement * element = new TiXmlElement("merge");
    TiXmlElement * token_el = new TiXmlElement("token");
    token_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(token_el);
    TiXmlElement * active_el = new TiXmlElement("token");
    active_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(activeToken)));
    element->LinkEndChild(active_el);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyRejected(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("reject");
    TiXmlElement * path_el = new TiXmlElement("token");
    path_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(path_el);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyCancelled(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("cancel");
    TiXmlElement * path_el = new TiXmlElement("token");
    path_el->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(path_el);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId& constraint){}

  void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId& constraint, const AbstractDomain& domain){}

  void DbClientTransactionLog::notifyVariableSpecified(const ConstrainedVariableId& variable){
    TiXmlElement * element = new TiXmlElement("specify");
    TiXmlElement var("variable");
    const EntityId& parent = variable->getParent();
    if (parent != EntityId::noId()) {
      if (TokenId::convertable(parent)) {
        var.SetAttribute("token", pathAsString(m_tokenMapper->getPathByToken(parent)));
      } else {
        check_error(ALWAYS_FAILS);
      }
    } else {
      check_error(ALWAYS_FAILS);
    }
    if (variable->getIndex() != ConstrainedVariable::NO_INDEX) {
      var.SetAttribute("index", variable->getIndex());
    } else {
      check_error(ALWAYS_FAILS);
    }
    element->InsertEndChild(var);
    element->LinkEndChild(abstractDomainAsXml(&variable->specifiedDomain()));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyVariableReset(const ConstrainedVariableId& variable){
    TiXmlElement * element = new TiXmlElement("reset");
    TiXmlElement var("variable");
    const EntityId& parent = variable->getParent();
    if (parent != EntityId::noId()) {
      if (TokenId::convertable(parent)) {
        var.SetAttribute("token", pathAsString(m_tokenMapper->getPathByToken(parent)));
      } else {
        check_error(ALWAYS_FAILS);
      }
    } else {
      check_error(ALWAYS_FAILS);
    }
    if (variable->getIndex() != ConstrainedVariable::NO_INDEX) {
      var.SetAttribute("index", variable->getIndex());
    } else {
      check_error(ALWAYS_FAILS);
    }
    element->InsertEndChild(var);
  }

  void DbClientTransactionLog::flush(std::ostream& os){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      os << **iter << std::endl;
      delete *iter;
    }
    m_bufferedTransactions.clear();
  }

}
