#ifndef _H_TimetableProfile
#define _H_TimetableProfile

#include "ResourceDefs.hh"
#include "Profile.hh"
#include "DomainListener.hh"

namespace EUROPA {

    class TimetableProfile : public Profile {
    public:
      TimetableProfile(const PlanDatabaseId db, const FVDetectorId flawDetector);

      void getTransactionsToOrder(const InstantId& inst, std::vector<TransactionId>& results);
    protected:

    	/**
    	 * @brief Compute level changes when transaction starts at the current instant.
    	 */
    	virtual void handleTransactionStart(bool isConsumer, const edouble & lb, const edouble & ub);

    	/**
    	 * @brief Compute level changes when transaction ends at the current instant.
    	 */
    	virtual void handleTransactionEnd(bool isConsumer, const edouble & lb, const edouble & ub);


      edouble m_lowerLevelMin, m_lowerLevelMax, m_upperLevelMin, m_upperLevelMax;
      edouble m_minPrevConsumption, m_maxPrevConsumption;
      edouble m_minPrevProduction, m_maxPrevProduction;

    private:
      void initRecompute(InstantId inst);
      void initRecompute();

    protected:
      virtual void recomputeLevels( InstantId prev, InstantId inst);
    };
}

#endif
