#include "DbLogger.hh"
#include "Token.hh"
#include "Object.hh"

namespace Prototype {
  const std::string CLASS_DELIMITER(":");

  DbLogger::DbLogger(std::ostream& os, const PlanDatabaseId& planDatabase): PlanDatabaseListener(planDatabase), m_os(os){}

  DbLogger::~DbLogger(){}

  void DbLogger::notifyAdded(const ObjectId& object){
    m_os << "DbLogger: Object Added " << object->getType().toString() <<
      CLASS_DELIMITER << object->getName().toString() << " (" << object->getKey() << ")" << std::endl;
  }

  void DbLogger::notifyRemoved(const ObjectId& object){
    m_os << "DbLogger: Object Removed " << object->getType().toString() << 
      CLASS_DELIMITER << object->getName().toString() << " (" << object->getKey() << ")" << std::endl;
  }

  void DbLogger::notifyAdded(const TokenId& token){m_os << "DbLogger: Token Added " << token->getPredicateName().toString()  << " (" << token->getKey() << ")" << std::endl;}

  void DbLogger::notifyRemoved(const TokenId& token){m_os << "DbLogger: Token Removed " << token->getPredicateName().toString()  << " (" << token->getKey() << ")" << std::endl;}

  void DbLogger::notifyActivated(const TokenId& token){m_os << "DbLogger: Token Activated " << token->getPredicateName().toString()  << " (" << token->getKey() << ")" << std::endl;}

  void DbLogger::notifyDeactivated(const TokenId& token){m_os << "DbLogger: Token Deactivated " << token->getPredicateName().toString()  << " (" << token->getKey() << ")" << std::endl;}

  void DbLogger::notifyMerged(const TokenId& token){m_os << "DbLogger: Token Merged " << token->getPredicateName().toString()  << " (" << token->getKey() << ") with active token (" << token->getActiveToken()->getKey() << ")" << std::endl;}

  void DbLogger::notifySplit(const TokenId& token){m_os << "DbLogger: Token Split " << token->getPredicateName().toString()  << " (" << token->getKey() << ")" << std::endl;}

  void DbLogger::notifyRejected(const TokenId& token){m_os << "DbLogger: Token Rejected " << token->getPredicateName().toString()  << " (" << token->getKey() << ")" << std::endl;}

  void DbLogger::notifyReinstated(const TokenId& token){m_os << "DbLogger: Token Reinstated" << token->getPredicateName().toString()  << " (" << token->getKey() << ")" << std::endl;}

  void DbLogger::notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor)
  {
    if (successor.isNoId()){
      m_os << "DbLogger: Token Constrained " << token->getPredicateName().toString()  << " (" << token->getKey() << ") On Object " << 
	object->getType().toString() << CLASS_DELIMITER << object->getName().toString() << 
	" (" << object->getKey() << ") Before Token (noId)" << std::endl;
    }
    else {
      m_os << "DbLogger: Token Constrained " << token->getPredicateName().toString()  << " (" << token->getKey() << ") On Object " << 
	object->getType().toString() << CLASS_DELIMITER << object->getName().toString() << " (" 
	   << object->getKey() << ") Before Token (" << successor->getKey() << ")" << std::endl;
    }
  }

  void DbLogger::notifyFreed(const ObjectId& object, const TokenId& token){
    m_os << "DbLogger: Token Freed " << token->getPredicateName().toString()  << " (" << token->getKey() << ") On Object " << 
      object->getType().toString() << CLASS_DELIMITER << object->getName().toString() 
	 << " (" << object->getKey() << ")" << std::endl;}

  void DbLogger::notifyAdded(const ObjectId& object, const TokenId& token){
    m_os << "DbLogger: Token " << token->getPredicateName().toString()  << " (" << token->getKey() << ") Added to Object " 
	 << object->getType().toString() << CLASS_DELIMITER << object->getName().toString() 
	 << " (" << object->getKey() << ")" << std::endl;
  }
  void DbLogger::notifyRemoved(const ObjectId& object, const TokenId& token){
    m_os << "DbLogger: Token " << token->getPredicateName().toString()  << " (" << token->getKey() << ") Removed from Object " 
	 << object->getType().toString() << CLASS_DELIMITER << object->getName().toString() << " (" 
	 << object->getKey() << ")" << std::endl;
  }
}
