#ifndef _H_SAVH_Reusable
#define _H_SAVH_Reusable

#include "SAVH_ResourceDefs.hh"
#include "SAVH_Resource.hh"
#include <map>
#include <vector>

namespace EUROPA {
  namespace SAVH {
    class Reusable : public Resource {
    public:
      //initial capacity is the upper limit, maxInstConsumption == maxInstProduction, maxConsumption == maxProduction
      Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, const LabelStr& detectorName, const LabelStr& profileName,
	       double initCapacityLb = 0, double initCapacityUb = 0, double lowerLimit = MINUS_INFINITY,
	       double maxInstConsumption = PLUS_INFINITY,
	       double maxConsumption = PLUS_INFINITY);
      Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open);
      Reusable(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open);
      //virtual ~Reusable();

      void getOrderingChoices(const TokenId& token,
			      std::vector<std::pair<TokenId, TokenId> >& results,
			      unsigned int limit = PLUS_INFINITY);
      //void getTokensToOrder(std::vector<TokenId>& results);
    protected:
    private:
      //void notifyViolated(const InstantId inst);
      //void notifyFlawed(const InstantId inst);
      //void notifyNoLongerFlawed(const InstantId inst);
      void addToProfile(const TokenId& tok);
      void removeFromProfile(const TokenId& tok);

      std::map<TokenId, std::pair<TransactionId, TransactionId> > m_tokensToTransactions;
    };
  }
}

#endif
