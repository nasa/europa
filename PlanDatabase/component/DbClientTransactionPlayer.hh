#ifndef _H_DbClientTransactionPlayer
#define _H_DbClientTransactionPlayer

#include "PlanDatabaseDefs.hh"
#include <iostream>

class TiXmlElement;

/**
 * @file DbClientTransactionLog
 * @brief Main interface for playing transactions. Necessary for copy. replay, and possibly recovery.
 */

namespace Prototype {

  class DbClientTransactionPlayer {
  public:
    DbClientTransactionPlayer(const PlanDatabaseId & db, const DbClientTransactionTokenMapperId & tokenMapper);
    ~DbClientTransactionPlayer();

    /**
     * @brief Play all transactions from an input stream
     */
    void play(std::istream& is);

  protected:
    void playNamedObjectCreated(const TiXmlElement & element);
    void playObjectCreated(const TiXmlElement & element);
    void playClosed(const TiXmlElement & element);
    void playTokenCreated(const TiXmlElement & element);
    void playConstrained(const TiXmlElement & element);
    void playActivated(const TiXmlElement & element);
    void playMerged(const TiXmlElement & element);
    void playRejected(const TiXmlElement & element);
    void playVariableSpecified(const TiXmlElement & element);

  private:
    PlanDatabaseId m_db;
    DbClientId m_client;
    DbClientTransactionTokenMapperId m_tokenMapper;
    int m_objectCount;
  };

}

#endif // _H_DbClientTransactionPlayer
