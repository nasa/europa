#ifndef _H_DbClientTransactionPlayer
#define _H_DbClientTransactionPlayer

#include "PlanDatabaseDefs.hh"

/**
 * @file DbClientTransactionLog
 * @brief Main interface for playing transactions. Necessary for copy. replay, and possibly recovery.
 */

namespace Prototype {

  class DbClientTransactionPlayer {
  public:
    DbClientTransactionPlayer(const DbClientId & client, const DbClientTransactionTokenMapperId & tokenMapper);
    ~DbClientTransactionPlayer();

    /**
     * @brief Play all transactions from an input stream
     */
    void play(std::istream& is);

  private:
    DbClientId m_client;
    DbClientTransactionTokenMapperId m_tokenMapper;
  };

}

#endif // _H_DbClientTransactionPlayer
