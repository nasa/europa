#include "DbClientTransactionLog.hh"
#include "Token.hh"

namespace Prototype {

  DbClientTransactionLog::DbClientTransactionLog(const DbClientId& client): DbClientListener(client) {}

  DbClientTransactionLog::~DbClientTransactionLog(){}

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object){

  }

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId& object, const std::vector<ConstructorArgument>& arguments){
    m_bufferedTransactions += "OBJECT CREATED\n";
  }

  void DbClientTransactionLog::notifyClosed(){}

  void DbClientTransactionLog::notifyClosed(const LabelStr& objectType){}

  void DbClientTransactionLog::notifyTokenCreated(const TokenId& token){
    m_keysOfTokensCreated.push_back(token->getKey());
    m_bufferedTransactions += "TOKEN CREATED\n";
  }

  void DbClientTransactionLog::notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor){}

  void DbClientTransactionLog::notifyActivated(const TokenId& token){}

  void DbClientTransactionLog::notifyMerged(const TokenId& token, const TokenId& activeToken){}

  void DbClientTransactionLog::notifyRejected(const TokenId& token){}

  void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId& constraint){}

  void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId& constraint, const AbstractDomain& domain){}

  void DbClientTransactionLog::notifyVariableSpecified(const ConstrainedVariableId& variable){}

  void DbClientTransactionLog::flush(std::ostream& os){
    os << m_bufferedTransactions;
    m_bufferedTransactions = "";
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
