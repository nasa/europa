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
    TiXmlElement * element = new TiXmlElement("new");
    check_error(LabelStr::isString(object->getName()));
    element->SetAttribute("name", object->getName().toString());
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object, const std::vector<ConstructorArgument>& arguments){
    TiXmlElement * element = new TiXmlElement("new");
    if (LabelStr::isString(object->getName())) {
      element->SetAttribute("name", object->getName().toString());
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
    TiXmlElement * path = new TiXmlElement("token");
    path->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(path);
    if (!successor.isNoId()) {
      TiXmlElement * successorPath = new TiXmlElement("token");
      successorPath->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(successor)));
      element->LinkEndChild(successorPath);
    }
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyActivated(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("activate");
    TiXmlElement * path = new TiXmlElement("token");
    path->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(path);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyMerged(const TokenId& token, const TokenId& activeToken){
    TiXmlElement * element = new TiXmlElement("merge");
    TiXmlElement * path = new TiXmlElement("token");
    path->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(path);
    TiXmlElement * activePath = new TiXmlElement("token");
    activePath->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(activeToken)));
    element->LinkEndChild(activePath);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyRejected(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("reject");
    TiXmlElement * path = new TiXmlElement("token");
    path->SetAttribute("path", pathAsString(m_tokenMapper->getPathByToken(token)));
    element->LinkEndChild(path);
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
    }
    if (variable->getIndex() != ConstrainedVariable::NO_INDEX) {
      var.SetAttribute("index", variable->getIndex());
    } else {
      var.SetAttribute("key", variable->getKey());
    }
    element->InsertEndChild(var);
    element->LinkEndChild(abstractDomainAsXml(&variable->specifiedDomain()));
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::flush(std::ostream& os){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      os << **iter << endl;
      delete *iter;
    }
    m_bufferedTransactions.clear();
  }

}
