#include "DbClientTransactionLog.hh"
#include "Object.hh"
#include "Token.hh"
#include "XmlUtils.hh"

namespace Prototype {

  DbClientTransactionLog::DbClientTransactionLog(const DbClientId& client): DbClientListener(client) {}

  DbClientTransactionLog::~DbClientTransactionLog(){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      delete *iter;
    }
    m_bufferedTransactions.empty();
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
    m_keysOfTokensCreated.push_back(token->getKey());
    TiXmlElement * element = new TiXmlElement("goal");
    TiXmlElement instance("predicateinstance");
    instance.SetAttribute("name", m_keysOfTokensCreated.size());
    check_error(LabelStr::isString(token->getName()));
    instance.SetAttribute("type", token->getName().toString());
    element->InsertEndChild(instance);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor){
    TiXmlElement * element = new TiXmlElement("constrain");
    TiXmlElement path("token");
    path.SetAttribute("path", pathAsString(getPathByToken(token, m_keysOfTokensCreated)));
    element->InsertEndChild(path);
    if (!successor.isNoId()) {
      TiXmlElement successorPath("token");
      successorPath.SetAttribute("path", pathAsString(getPathByToken(successor, m_keysOfTokensCreated)));
      element->InsertEndChild(successorPath);
    }
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyActivated(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("activate");
    TiXmlElement path("token");
    path.SetAttribute("path", pathAsString(getPathByToken(token, m_keysOfTokensCreated)));
    element->InsertEndChild(path);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyMerged(const TokenId& token, const TokenId& activeToken){
    TiXmlElement * element = new TiXmlElement("merge");
    TiXmlElement path("token");
    path.SetAttribute("path", pathAsString(getPathByToken(token, m_keysOfTokensCreated)));
    element->InsertEndChild(path);
    TiXmlElement activePath("token");
    activePath.SetAttribute("path", pathAsString(getPathByToken(activeToken, m_keysOfTokensCreated)));
    element->InsertEndChild(activePath);
    m_bufferedTransactions.push_back(element);
  }

  void DbClientTransactionLog::notifyRejected(const TokenId& token){
    TiXmlElement * element = new TiXmlElement("reject");
    TiXmlElement path("token");
    path.SetAttribute("path", pathAsString(getPathByToken(token, m_keysOfTokensCreated)));
    element->InsertEndChild(path);
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
        var.SetAttribute("token", pathAsString(getPathByToken(parent, m_keysOfTokensCreated)));
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
    m_bufferedTransactions.empty();
  }

  void DbClientTransactionLog::play(std::istream& is){}

  /**
   * @brief Traverse the path and obtain the right token
   */
  TokenId DbClientTransactionLog::getTokenByPath(const std::vector<int>& relativePath, const std::vector<int>& tokenKeysByIndex){
    check_error(!relativePath.empty());
    check_error(!tokenKeysByIndex.empty());
    check_error(relativePath[0] >= 0); // Can never be a valid path

    // Quick check for the root of the path
    if((unsigned)relativePath[0] >= tokenKeysByIndex.size()) // Cannot be a path for a token with this key set
      return TokenId::noId();

    // Obtain the root token key using the first element in the path to index the tokenKeys.
    int rootTokenKey = tokenKeysByIndex[relativePath[0]];

    // Now source the token as an enityt lookup by key. This works because we have a shared pool
    // of entities per process
    EntityId entity = Entity::getEntity(rootTokenKey);

    if (entity.isNoId())
      return(TokenId::noId());

    TokenId rootToken = entity;
    for (unsigned int i = 1;
         !rootToken.isNoId() && i < relativePath.size();
         i++)
      rootToken = rootToken->getSlave(relativePath[i]);

    return(rootToken);
  }


  /**
   * @brief Build the path from the bottom up, and return it from the top down.
   */
  std::vector<int> DbClientTransactionLog::getPathByToken(const TokenId& targetToken, const std::vector<int>& tokenKeysByIndex){
    std::list<int> path; // Used to build up the path from the bottom up.

    TokenId master = targetToken->getMaster();
    TokenId slave = targetToken;

    while(!master.isNoId()){
      int slavePosition = master->getSlavePosition(slave);
      check_error(slavePosition >= 0);
      path.push_front(slavePosition);
      slave = slave->getMaster();
      master = master->getMaster();
    }

    // Master must be a noId, in which case slave is the root token, so dump the key
    int keyOfMaster = slave->getKey();
    int indexOfMaster = -1;
    for(unsigned int i=0; i< tokenKeysByIndex.size(); i++)
      if(tokenKeysByIndex[i] == keyOfMaster){
	indexOfMaster = i;
	break;
      }

    check_error(indexOfMaster >= 0);

    // Now push the key for the root and generate path going from top down
    path.push_front(indexOfMaster);

    std::vector<int> pathAsVector;
    for(std::list<int>::const_iterator it = path.begin(); it != path.end(); ++it)
      pathAsVector.push_back(*it);

    return pathAsVector;
  }
}
