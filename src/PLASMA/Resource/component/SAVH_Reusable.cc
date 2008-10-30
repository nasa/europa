#include "SAVH_Reusable.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_DurativeTokens.hh"
#include "ConstraintEngine.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"

namespace EUROPA {
  namespace SAVH {
    Reusable::Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, const LabelStr& detectorName, const LabelStr& profileName,
			 double initCapacityLb, double initCapacityUb, double lowerLimit, double maxInstConsumption,
			 double maxConsumption) :
      Resource(planDatabase, type, name, detectorName, profileName, initCapacityLb, initCapacityUb, lowerLimit, initCapacityUb, maxInstConsumption,
	       maxInstConsumption, maxConsumption, maxConsumption) {}

    Reusable::Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open) :
      Resource(planDatabase, type, name, open) {}

    Reusable::Reusable(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open) :
      Resource(parent, type, localName, open) {}

    void Reusable::getOrderingChoices(const TokenId& token,
				      std::vector<std::pair<TokenId, TokenId> >& results,
				      unsigned int limit) {
      checkError(m_tokensToTransactions.find(token) != m_tokensToTransactions.end(), "Token " << token->getPredicateName().toString() <<
                 "(" << token->getKey() << ") not in profile for " << toString());
      Resource::getOrderingChoices(token, results, limit);
    }

    void Reusable::addToProfile(const TokenId& tok) {
      if(m_tokensToTransactions.find(tok) != m_tokensToTransactions.end()) {
	debugMsg("Reusable:addToProfile",
		 "Token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ") is already in the profile.");
	return;
      }
      ReusableTokenId t(tok);
      debugMsg("Reusable:addToProfile", "Adding token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ")");
      //here's the major difference between Resuable and Reservoir:  always consume the quantity at the start
      //and produce it again at the end
      TransactionId t1 = (new Transaction(t->start(), t->getQuantity(), true))->getId();
      TransactionId t2 = (new Transaction(t->end(), t->getQuantity(), false))->getId();
      m_transactionsToTokens.insert(std::make_pair(t1, tok));
      m_transactionsToTokens.insert(std::make_pair(t2, tok));
      m_tokensToTransactions.insert(std::make_pair(tok, std::make_pair(t1, t2)));
      m_profile->addTransaction(t1);
      m_profile->addTransaction(t2);
    }

    void Reusable::removeFromProfile(const TokenId& tok) {
      if(m_tokensToTransactions.find(tok) == m_tokensToTransactions.end())
        return;

      debugMsg("Reusable:removeFromProfile", "Removing token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ")");
      std::pair<TransactionId, TransactionId> trans = m_tokensToTransactions.find(tok)->second;
      debugMsg("Reusable:removeFromProfile", "Removing transaction for time " << trans.first->time()->toString() << " with quantity " << trans.first->quantity()->toString());
      m_profile->removeTransaction(trans.first);
      debugMsg("Reusable:removeFromProfile", "Removing transaction for time " << trans.second->time()->toString() << " with quantity " << trans.second->quantity()->toString());
      m_profile->removeTransaction(trans.second);
      m_tokensToTransactions.erase(tok);
      m_transactionsToTokens.erase(trans.first);
      m_transactionsToTokens.erase(trans.second);
      std::map<TokenId, std::set<InstantId> >::iterator it = m_flawedTokens.find(tok);
      if(it != m_flawedTokens.end()) {
        m_flawedTokens.erase(it);
        notifyOrderingNoLongerRequired(tok);
      }
      delete (Transaction*) trans.first;
      delete (Transaction*) trans.second;

    }

    UnaryTimeline::UnaryTimeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
      : Reusable(planDatabase, type, name, open) {init(1, 1, 0, 1, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, "ClosedWorldFVDetector", "IncrementalFlowProfile");}

    UnaryTimeline::UnaryTimeline(const ObjectId& parent, const LabelStr& type, const LabelStr& name, bool open)
      : Reusable(parent, type, name, open) {init(1, 1, 0, 1, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, "ClosedWorldFVDetector", "IncrementalFlowProfile");}
  }
}
