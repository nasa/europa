#ifndef _H_DbLogger
#define _H_DbLogger

#include "PlanDatabaseListener.hh"
#include <iostream>

namespace EUROPA{

  class DbLogger: public PlanDatabaseListener {
  public:
    DbLogger(std::ostream& os, const PlanDatabaseId& planDatabase);
    ~DbLogger();
    void notifyAdded(const ObjectId& object);
    void notifyRemoved(const ObjectId& object);
    void notifyAdded(const TokenId& token);
    void notifyRemoved(const TokenId& token);
    void notifyActivated(const TokenId& token);
    void notifyDeactivated(const TokenId& token);
    void notifyMerged(const TokenId& token);
    void notifySplit(const TokenId& token);
    void notifyRejected(const TokenId& token);
    void notifyReinstated(const TokenId& token);
    void notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor);
    void notifyFreed(const ObjectId& object, const TokenId& token);
    void notifyAdded(const ObjectId& object, const TokenId& token);
    void notifyRemoved(const ObjectId& object, const TokenId& token);
  private:
    std::ostream& m_os;
  };

}

#endif
