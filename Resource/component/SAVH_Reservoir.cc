#include "SAVH_Instant.hh"
#include "SAVH_InstantTokens.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Reservoir.hh"
#include "SAVH_Transaction.hh"
#include "ConstraintEngine.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"

namespace EUROPA {
  namespace SAVH {
    Reservoir::Reservoir(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, const LabelStr& detectorName, const LabelStr& profileName,
			 double initCapacityLb, double initCapacityUb, double lowerLimit, double upperLimit, double maxInstProduction, double maxInstConsumption,
			 double maxProduction, double maxConsumption) :
      Resource(planDatabase, type, name, detectorName, profileName, initCapacityLb, initCapacityUb, lowerLimit, upperLimit, maxInstProduction,
	       maxInstConsumption, maxProduction, maxConsumption) {}

    Reservoir::Reservoir(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open) :
      Resource(planDatabase, type, name, open) {}

    Reservoir::Reservoir(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open) :
      Resource(parent, type, localName, open) {}

    Reservoir::~Reservoir() {
      for(std::map<TransactionId, TokenId>::const_iterator it = m_transactionsToTokens.begin(); it != m_transactionsToTokens.end();
	  ++it) {
	delete (Transaction*) it->first;
      }
    }
    
    //can cache these results or perhaps compute the all-pairs incrementally in the instant
    void Reservoir::getOrderingChoices(const TokenId& token,
				       std::vector<std::pair<TokenId, TokenId> >& results,
				       unsigned int limit) {
      check_error(results.empty());
      check_error(token.isValid());
      checkError(m_tokensToTransactions.find(token) != m_tokensToTransactions.end(), "Token " << token->getPredicateName().toString() << 
		 "(" << token->getKey() << ") not in profile.");
      if(!getPlanDatabase()->getConstraintEngine()->propagate())
	return;
      
      std::multimap<TokenId, TokenId> pairs;
      std::map<int, InstantId>::iterator first = m_flawedInstants.lower_bound((int)token->getStart()->lastDomain().getLowerBound());
      checkError(first != m_flawedInstants.end(), 
		 "No flawed instants within token " << token->getPredicateName().toString() << "(" << token->getKey() << ")");
      std::map<int, InstantId>::iterator last = m_flawedInstants.lower_bound((int)token->getEnd()->lastDomain().getUpperBound());
      
      if(last != m_flawedInstants.end() && last->second->getTime() == token->getEnd()->lastDomain().getUpperBound())
	++last;
      
      unsigned int count = 0;
      for(; first != last && count < limit; ++first) {
	const std::set<TransactionId>& transactions = first->second->getTransactions();
	for(std::set<TransactionId>::const_iterator it = transactions.begin(); it != transactions.end() && count < limit; ++it) {
	  TransactionId predecessor = *it;
	  check_error(predecessor.isValid());
	  check_error(m_transactionsToTokens.find(predecessor) != m_transactionsToTokens.end());
	  
	  TokenId predecessorToken = m_transactionsToTokens.find(predecessor)->second;
	  check_error(predecessorToken.isValid());
	  
	  for(std::set<TransactionId>::const_iterator subIt = transactions.begin(); subIt != transactions.end() && count < limit; ++subIt) {
	    if(subIt == it)
	      continue;
	    
	    TransactionId successor = *subIt;
	    check_error(successor.isValid());
	    check_error(m_transactionsToTokens.find(successor) != m_transactionsToTokens.end());
	    
	    TokenId successorToken = m_transactionsToTokens.find(successor)->second;
	    check_error(successorToken.isValid());
	    
	    pairs.insert(std::pair<TokenId, TokenId>(predecessorToken, successorToken));
	    ++count;
	  }
	}
      }
      if(pairs.empty()) {
	results.push_back(std::pair<TokenId, TokenId>(token, token));
	return;
      }
      for(std::multimap<TokenId, TokenId>::iterator it = pairs.begin(); it != pairs.end(); ++it)
	results.push_back(*it);
    }

    void Reservoir::getTokensToOrder(std::vector<TokenId>& results) {
      check_error(results.empty());
      getPlanDatabase()->getConstraintEngine()->propagate();
      checkError(getPlanDatabase()->getConstraintEngine()->constraintConsistent(),
		 "Should be consistent to continue here. Should have checked before you called the method in the first place.");
      for(std::map<TokenId, int>::const_iterator it = m_flawedTokens.begin(); it != m_flawedTokens.end(); ++it)
	results.push_back(it->first);
    }

    void Reservoir::notifyViolated(const InstantId inst) {
      check_error(inst.isValid());
      check_error(inst->isViolated());
      check_error(!inst->getTransactions().empty());

      TransactionId trans = *(inst->getTransactions().begin());
      const_cast<AbstractDomain&>(trans->quantity()->lastDomain()).empty();
    }

    //all ordering choices apply to all tokens in all flaws
    void Reservoir::notifyFlawed(const InstantId inst) {
      check_error(inst.isValid());
      check_error(inst->isFlawed());
      check_error(!inst->getTransactions().empty());

      std::map<int, InstantId>::iterator instIt = m_flawedInstants.find(inst->getTime());

      //even if we've already recorded a flaw for this instant, there may be new transactions
      for(std::set<TransactionId>::const_iterator it = inst->getTransactions().begin();
	  it != inst->getTransactions().end(); ++it) {
	TransactionId trans = *it;
	check_error(trans.isValid());
	std::map<TransactionId, TokenId>::iterator transIt = m_transactionsToTokens.find(trans);
	check_error(transIt != m_transactionsToTokens.end());
	TokenId tok = transIt->second;
	check_error(tok.isValid());

	//if we haven't already recorded this token, insert it and notify that an ordering is required
	if(m_flawedTokens.find(tok) == m_flawedTokens.end()) {
	  notifyOrderingRequired(tok);
	  m_flawedTokens.insert(std::pair<TokenId, int>(tok, 1));
	}
	//it's possible that we're getting a redundant notification, so only increment the flaw count
	//if we don't have the instant recorded
	else if(instIt == m_flawedInstants.end())
	  m_flawedTokens[tok] = m_flawedTokens[tok] + 1;
      }
      if(instIt == m_flawedInstants.end())
	m_flawedInstants.insert(std::pair<int, InstantId>(inst->getTime(), inst));
    }

    void Reservoir::notifyNoLongerFlawed(const InstantId inst) {
      check_error(inst.isValid());
      check_error(!inst->isFlawed());
      check_error(!inst->getTransactions().empty());
      check_error(m_flawedInstants.find(inst->getTime()) != m_flawedInstants.end());

      m_flawedInstants.erase(inst->getTime());
      for(std::map<TokenId, int>::iterator it = m_flawedTokens.begin(); it != m_flawedTokens.end();
	  ++it) {
	int count = it->second - 1;
	if(count > 0)
	  it->second = count;
	else {
	  std::map<TokenId, int>::iterator temp = it;
	  --it;
	  notifyOrderingNoLongerRequired(getId());
	  m_flawedTokens.erase(temp);
	}
      }
    }

    void Reservoir::notifyDeleted(const InstantId inst) {
      m_flawedInstants.erase(inst->getTime());
      //does anything else need to be done here?
    }

    void Reservoir::addToProfile(const TokenId& tok) {
      checkError(m_tokensToTransactions.find(tok) == m_tokensToTransactions.end(),
		 "Token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ") is already in the profile.");
      ReservoirTokenId t(tok);
      debugMsg("Reservoir:addToProfile", "Adding " << (t->isConsumer() ? "consumer " : "producer ") << "token " << 
	       tok->getPredicateName().toString() << "(" << tok->getKey() << ")");
      TransactionId trans = (new Transaction(t->getTime(), t->getQuantity(), t->isConsumer()))->getId();
      m_transactionsToTokens.insert(std::pair<TransactionId, TokenId>(trans, tok));
      m_tokensToTransactions.insert(std::pair<TokenId, TransactionId>(tok, trans));
      m_profile->addTransaction(trans);
    }

    void Reservoir::removeFromProfile(const TokenId& tok) {
//       checkError(m_tokensToTransactions.find(tok) != m_tokensToTransactions.end(),
// 		 "Token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ") isn't in the profile.");
      if(m_tokensToTransactions.find(tok) == m_tokensToTransactions.end())
	return;
      debugMsg("Reservoir:removeFromProfile", "Removing token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ")");
      ReservoirTokenId t(tok); //just for error checking
      TransactionId trans = m_tokensToTransactions.find(tok)->second;
      m_profile->removeTransaction(trans);
      m_tokensToTransactions.erase(tok);
      m_transactionsToTokens.erase(trans);
      delete (Transaction*) trans;
    }
  }
}
