#ifndef _H_DbClientTransactionPlayer
#define _H_DbClientTransactionPlayer

#include "PlanDatabaseDefs.hh"
#include <iostream>

class TiXmlElement;

/**
 * @file DbClientTransactionPlayer
 * @brief Main interface for playing transactions. Necessary for copy. replay, and possibly recovery.
 */

namespace Prototype {

  class DbClientTransactionPlayer {
  public:
    DbClientTransactionPlayer(const DbClientId & client);
    ~DbClientTransactionPlayer();

    /**
     * @brief Play all transactions from an input stream
     * @param is a stream of xml-based transactions.
     */
    void play(std::istream& is);

    /**
     * @brief Play all transactions from a given TransactionLog
     * @param txLog the source log which has all transactions in memory
     */
    void play(const DbClientTransactionLogId& txLog);

  protected:
    void processTransaction(const TiXmlElement & element);
    void playNamedObjectCreated(const TiXmlElement & element);
    void playObjectCreated(const TiXmlElement & element);
    void playTokenCreated(const TiXmlElement & element);
    void playConstrained(const TiXmlElement & element);
    void playFreed(const TiXmlElement & element);
    void playActivated(const TiXmlElement & element);
    void playMerged(const TiXmlElement & element);
    void playRejected(const TiXmlElement & element);
    void playCancelled(const TiXmlElement & element);
    void playVariableSpecified(const TiXmlElement & element);
    void playVariableReset(const TiXmlElement & element);
    void playInvokeConstraint(const TiXmlElement & element);

  private:
    DbClientId m_client;
    int m_objectCount;

    AbstractDomain * getAbstractDomain(const TiXmlElement & abstractDomain);
    double getValue(const TiXmlElement & value);
    ConstrainedVariableId getVariable(const TiXmlElement & variable);
  };

}

#endif // _H_DbClientTransactionPlayer
