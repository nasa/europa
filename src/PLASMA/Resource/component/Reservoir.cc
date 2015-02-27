#include "Instant.hh"
#include "InstantTokens.hh"
#include "Profile.hh"
#include "Reservoir.hh"
#include "Transaction.hh"
#include "ConstraintEngine.hh"
#include "Domains.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"

namespace EUROPA {
Reservoir::Reservoir(const PlanDatabaseId planDatabase, const std::string& type,
                     const std::string& name, const std::string& detectorName,
                     const std::string& profileName, 
                     edouble initCapacityLb, edouble initCapacityUb,
                     edouble lowerLimit, edouble upperLimit,
                     edouble maxInstProduction, edouble maxInstConsumption,
                     edouble maxProduction, edouble maxConsumption) :
    Resource(planDatabase, type, name, detectorName, profileName, initCapacityLb, 
             initCapacityUb, lowerLimit, upperLimit,
             maxInstProduction, maxInstConsumption, maxProduction, maxConsumption),
  m_tokensToTransactions() {}

Reservoir::Reservoir(const PlanDatabaseId planDatabase, const std::string& type,
                     const std::string& name, bool open) :
    Resource(planDatabase, type, name, open), m_tokensToTransactions() {}

Reservoir::Reservoir(const ObjectId parent, const std::string& type, 
                     const std::string& localName, bool open) :
    Resource(parent, type, localName, open), m_tokensToTransactions() {}

  void Reservoir::createTransactions(const TokenId tok) {
    if(m_tokensToTransactions.find(tok) != m_tokensToTransactions.end()) {
      debugMsg("Reservoir:createTransactions",
               "Token " << tok->getPredicateName() << "(" << tok->getKey() << ") already has transactions.");
      return;
    }
    ReservoirTokenId t(tok);
    debugMsg("Reservoir:createTransactions",
             "Creating transactions for " << (t->isConsumer() ? "consumer " : "producer ") << "token " <<
             tok->getPredicateName() << "(" << tok->getKey() << ")");
    TransactionId trans = 
        (new Transaction(t->getTime(), t->getQuantity(), t->isConsumer(), getId()))->getId();
    m_transactionsToTokens.insert(std::pair<TransactionId, TokenId>(trans, tok));
    m_tokensToTransactions.insert(std::pair<TokenId, TransactionId>(tok, trans));
  }

  void Reservoir::addToProfile(const TokenId tok) {
    checkError(m_tokensToTransactions.find(tok) != m_tokensToTransactions.end(),
               "No transaction for " << tok->getPredicateName() << "(" << tok->getKey() << ")");
    TransactionId trans = m_tokensToTransactions.find(tok)->second;
    debugMsg("Reservoir:addToProfile", "For " << tok->toString() << " adding transaction " << trans->toString() << " to profile.");
    m_profile->addTransaction(trans);
  }

  void Reservoir::removeTransactions(const TokenId tok) {
    if(m_tokensToTransactions.find(tok) == m_tokensToTransactions.end())
      return;
    TransactionId trans = m_tokensToTransactions.find(tok)->second;
    debugMsg("Reservoir:removeTransactions", "For " << tok->toString() << " deleting transaction " << trans->toString());
    m_tokensToTransactions.erase(tok);
    m_transactionsToTokens.erase(trans);
    delete static_cast<Transaction*>(trans);
  }

  void Reservoir::removeFromProfile(const TokenId tok) {
    //       checkError(m_tokensToTransactions.find(tok) != m_tokensToTransactions.end(),
    // 		 "Token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ") isn't in the profile.");
    if(m_tokensToTransactions.find(tok) == m_tokensToTransactions.end())
      return;
    debugMsg("Reservoir:removeFromProfile", toString() << " Removing token " << tok->getPredicateName() << "(" << tok->getKey() << ")");

    TransactionId trans = m_tokensToTransactions.find(tok)->second;
    m_profile->removeTransaction(trans);
    Resource::removeFromProfile(tok);
  }

  void Reservoir::getOrderingChoices(const TokenId token,
                                     std::vector<std::pair<TokenId, TokenId> >& results,
                                     unsigned long limit) {
    checkError(m_tokensToTransactions.find(token) != m_tokensToTransactions.end(), "Token " << token->getPredicateName() <<
               "(" << token->getKey() << ") not in profile.");
    Resource::getOrderingChoices(token, results, limit);
  }
}
