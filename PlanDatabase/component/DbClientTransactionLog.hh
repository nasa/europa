#ifndef _H_DbClientTransactionLog
#define _H_DbClientTransactionLog

#include "DbClientListener.hh"
#include <vector>
#include <string>
#include <iostream>

/**
 * @file DbClientTransactionLog
 * @brief Main interface for logging and playing transactions. Necessary for copy. replay, and possibly recovery.
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
    void notifyActivated(const TokenId& token);
    void notifyMerged(const TokenId& token, const TokenId& activeToken);
    void notifyRejected(const TokenId& token);
    void notifyConstraintCreated(const ConstraintId& constraint);
    void notifyConstraintCreated(const ConstraintId& constraint, const AbstractDomain& domain);
    void notifyVariableSpecified(const ConstrainedVariableId& variable);

    /**
     * @brief Flush all buffered transactions to an output stream and clear the buffer. Handy for checkpointing.
     */
    void flush(std::ostream& os);

    /**
     * @brief Play all transactions from an input stream
     */
    void play(std::istream& is);

    /**
     * @brief Retrieve the vector of token keys. Essential for path based retrieval
     */
    const std::vector<int>& getKeysOfTokensCreated() const;

    static TokenId getTokenByPath(const std::vector<int>& relativePath, const std::vector<int>& tokenKeysByIndex);

    static std::vector<int> getPathByToken(const TokenId& targetToken, const std::vector<int>& tokenKeysByIndex);

  private:
    std::vector<int> m_keysOfTokensCreated;
    std::string m_bufferedTransactions;
  };
}
#endif
