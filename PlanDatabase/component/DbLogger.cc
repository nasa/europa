#include "DbLogger.hh"
#include "Token.hh"
#include "Object.hh"

namespace Prototype {
  DbLogger::DbLogger(ostream& os, const PlanDatabaseId& planDatabase): PlanDatabaseListener(planDatabase), m_os(os){}

  DbLogger::~DbLogger(){}

  void DbLogger::notifyAdded(const ObjectId& object){m_os << "DbLogger: Object Added (" << object->getKey() << ")" << endl;}

  void DbLogger::notifyRemoved(const ObjectId& object){m_os << "DbLogger: Object Removed (" << object->getKey() << ")" << endl;}

  void DbLogger::notifyAdded(const TokenId& token){m_os << "DbLogger: Token Added (" << token->getKey() << ")" << endl;}

  void DbLogger::notifyRemoved(const TokenId& token){m_os << "DbLogger: Token Removed (" << token->getKey() << ")" << endl;}

  void DbLogger::notifyClosed(const TokenId& token){m_os << "DbLogger: Token Closed (" << token->getKey() << ")" << endl;}

  void DbLogger::notifyActivated(const TokenId& token){m_os << "DbLogger: Token Activated (" << token->getKey() << ")" << endl;}

  void DbLogger::notifyDeactivated(const TokenId& token){m_os << "DbLogger: Token Deactivated (" << token->getKey() << ")" << endl;}

  void DbLogger::notifyMerged(const TokenId& token){m_os << "DbLogger: Token Merged (" << token->getKey() << ")" << endl;}

  void DbLogger::notifySplit(const TokenId& token){m_os << "DbLogger: Token Split (" << token->getKey() << ")" << endl;}

  void DbLogger::notifyRejected(const TokenId& token){m_os << "DbLogger: Token Rejected (" << token->getKey() << ")" << endl;}

  void DbLogger::notifyReinstated(const TokenId& token){m_os << "DbLogger: Token Reinstated(" << token->getKey() << ")" << endl;}

  void DbLogger::notifyAdded(const ObjectId& object, const TokenId& token){
    m_os << "DbLogger: Token (" << token->getKey() << ") Added to Object(" << object->getKey() << ")" << endl;
  }
  void DbLogger::notifyRemoved(const ObjectId& object, const TokenId& token){
    m_os << "DbLogger: Token (" << token->getKey() << ") Removed from Object(" << object->getKey() << ")" << endl;
  }
}
