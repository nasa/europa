#ifndef _H_DbClientTransactionLog
#define _H_DbClientTransactionLog

#include "DbClientListener.hh"
#include <list>
#include <vector>
#include <string>
#include <iostream>

class TiXmlElement;

/**
 * @file DbClientTransactionLog
 * @brief Main interface for logging transactions. Necessary for copy. replay, and possibly recovery.
 */

namespace Prototype {

  class DbClientTransactionLog: public DbClientListener {
  public:
    DbClientTransactionLog(const DbClientId& client);
    ~DbClientTransactionLog();

    /* Declare DbClient event handlers we will over-ride */
    void notifyObjectCreated(const ObjectId& object) ;
    void notifyObjectCreated(const ObjectId& object, const std::vector<ConstructorArgument>& arguments);
    void notifyClosed();
    void notifyClosed(const LabelStr& objectType);
    void notifyTokenCreated(const TokenId& token);
    void notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor);
    void notifyFreed(const ObjectId& object, const TokenId& token);
    void notifyActivated(const TokenId& token);
    void notifyMerged(const TokenId& token, const TokenId& activeToken);
    void notifyRejected(const TokenId& token);
    void notifyCancelled(const TokenId& token);
    void notifyConstraintCreated(const ConstraintId& constraint);
    void notifyConstraintCreated(const ConstraintId& constraint, const AbstractDomain& domain);
    void notifyVariableSpecified(const ConstrainedVariableId& variable);
    void notifyVariableReset(const ConstrainedVariableId& variable);

    /**
     * @brief Flush all buffered transactions to an output stream and clear the buffer. Handy for checkpointing.
     */
    void flush(std::ostream& os);

  private:
    friend class DbClientTransactionPlayer;
    const std::list<TiXmlElement*>& getBufferedTransactions() const;

    std::list<TiXmlElement*> m_bufferedTransactions;
  };
}
#endif
