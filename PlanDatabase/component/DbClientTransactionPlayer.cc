#include "DbClientTransactionPlayer.hh"

namespace Prototype {

  DbClientTransactionPlayer::DbClientTransactionPlayer(const DbClientId& client, const DbClientTransactionTokenMapperId & tokenMapper)
  {
    m_client = client;
    m_tokenMapper = tokenMapper;
  }

  DbClientTransactionPlayer::~DbClientTransactionPlayer()
  {
  }

  void DbClientTransactionPlayer::play(std::istream& is)
  {
  }

}
