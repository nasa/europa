#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_Profile.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {
  namespace SAVH {
    
    Instant::Instant(const int time, const ProfileId prof) 
      : Entity(), m_id(this), m_time(time), m_profile(prof), m_lowerLevel(0), m_lowerLevelMax(0), m_upperLevelMin(0), m_upperLevel(0),
        m_maxInstProduction(0), m_maxInstConsumption(0), m_minInstProduction(0), m_minInstConsumption(0),
        m_maxCumulativeProduction(0), m_maxCumulativeConsumption(0), m_minCumulativeProduction(0), m_minCumulativeConsumption(0),
        m_maxPrevProduction(0), m_maxPrevConsumption(0), m_minPrevProduction(0), m_minPrevConsumption(0),
        m_upperFlawMagnitude(0), m_lowerFlawMagnitude(0),
        m_violated(false), m_flawed(false), m_upperFlaw(false), m_lowerFlaw(false) {}

    Instant::~Instant() {
      m_id.remove();
    }

    int Instant::getTime() const {return m_time;}

    const std::set<TransactionId>& Instant::getTransactions() const {return m_transactions;}
    const std::set<TransactionId>& Instant::getEndingTransactions() const {return m_endingTransactions;}
    const std::set<TransactionId>& Instant::getStartingTransactions() const {return m_startingTransactions;}
    const std::set<TransactionId>& Instant::getOverlappingTransactions() const {return m_overlappingTransactions;}

    void Instant::addTransaction(const TransactionId t) {
      checkError(m_transactions.find(t) == m_transactions.end(), "Instant for time " << m_time << " already has transaction " << t);
      checkError(t->time()->lastDomain().isMember(m_time), "Attempted to add a transaction spanning time " << 
                 t->time()->toString() << " to instant for " << m_time);

      debugMsg("Instant:addTransaction", "Adding transaction to instant (" << getId() << ") for time " << t->time()->toString() << " with quantity " << t->quantity()->toString());
      m_transactions.insert(t);

      if(t->time()->lastDomain().getLowerBound() == m_time ) 
        m_startingTransactions.insert(t);

      if(t->time()->lastDomain().getUpperBound() == m_time)
        m_endingTransactions.insert(t);
      else
        {
          m_overlappingTransactions.insert(t);
        }

    }

    void Instant::updateTransaction(const TransactionId t) {      
      checkError(t->time()->lastDomain().isMember(m_time), "Attempted to update a transaction spanning time " << 
                 t->time()->toString() << " to instant for " << m_time);
      
      debugMsg("Instant:updateTransaction", "Updating transaction to instant (" << getId() << ") for time " << t->time()->toString() << " with quantity " << t->quantity()->toString());

      if(t->time()->lastDomain().getLowerBound() == m_time ) 
        m_startingTransactions.insert(t);
      else
        m_startingTransactions.erase(t);

      if(t->time()->lastDomain().getUpperBound() == m_time)
        {
          m_endingTransactions.insert(t);
          m_overlappingTransactions.erase(t);
        }
      else
        {
          m_endingTransactions.erase(t);
          m_overlappingTransactions.insert(t);
        }
    }

    void Instant::removeTransaction(const TransactionId t) {
      checkError(m_transactions.find(t) != m_transactions.end(), "Instant for time " << m_time << " has no transaction " << t);
      m_transactions.erase(t);
      m_endingTransactions.erase(t);
      m_overlappingTransactions.erase(t);
      m_startingTransactions.erase(t);
    }

    double Instant::getLowerLevel() {return m_lowerLevel;}
    double Instant::getLowerLevelMax() {return m_lowerLevelMax;}
    double Instant::getUpperLevelMin() {return m_upperLevelMin;}
    double Instant::getUpperLevel() {return m_upperLevel;}
    double Instant::getMaxInstantProduction() {return m_maxInstProduction;}
    double Instant::getMinInstantProduction() {return m_minInstProduction;}
    double Instant::getMaxInstantConsumption() {return m_maxInstConsumption;}
    double Instant::getMinInstantConsumption() {return m_minInstConsumption;}
    double Instant::getMaxCumulativeConsumption() {return m_maxCumulativeConsumption;}
    double Instant::getMaxCumulativeProduction() {return m_maxCumulativeProduction;}
    double Instant::getMinCumulativeConsumption() {return m_minCumulativeConsumption;}
    double Instant::getMinCumulativeProduction() {return m_minCumulativeProduction;}
    double Instant::getMaxPrevConsumption() {return m_maxPrevConsumption;}
    double Instant::getMaxPrevProduction() {return m_maxPrevProduction;}
    double Instant::getMinPrevConsumption() {return m_minPrevConsumption;}
    double Instant::getMinPrevProduction() {return m_minPrevProduction;}

    void Instant::update(double lowerLevelMin, double lowerLevelMax, double upperLevelMin, double upperLevelMax,
                         double minInstConsumption, double maxInstConsumption, double minInstProduction, double maxInstProduction,
                         double minCumulativeConsumption, double maxCumulativeConsumption, double minCumulativeProduction, double maxCumulativeProduction,
                         double minPrevConsumption, double maxPrevConsumption, double minPrevProduction, double maxPrevProduction) {
      m_lowerLevel = lowerLevelMin;
      m_lowerLevelMax = lowerLevelMax;
      m_upperLevelMin = upperLevelMin;
      m_upperLevel = upperLevelMax;
      m_minInstConsumption = minInstConsumption;
      m_maxInstConsumption = maxInstConsumption;
      m_minInstProduction = minInstProduction;
      m_maxInstProduction = maxInstProduction;
      m_minCumulativeConsumption = minCumulativeConsumption;
      m_maxCumulativeConsumption = maxCumulativeConsumption;
      m_minCumulativeProduction = minCumulativeProduction;
      m_maxCumulativeProduction = maxCumulativeProduction;
      m_minPrevConsumption = minPrevConsumption;
      m_maxPrevConsumption = maxPrevConsumption;
      m_minPrevProduction = minPrevProduction;
      m_maxPrevProduction = maxPrevProduction;
    }

    bool Instant::containsStartOrEnd() {
      bool retval = false;
      for(std::set<TransactionId>::const_iterator it = m_transactions.begin(); it != m_transactions.end(); ++it) {
        TransactionId trans = *it;
        if(trans->time()->lastDomain().getLowerBound() == m_time || trans->time()->lastDomain().getUpperBound() == m_time) {
          retval = true;
          break;
        }
      }
      return retval;
    }

    std::string Instant::toString() const {
      std::stringstream sstr;
      for(std::set<TransactionId>::const_iterator it = m_transactions.begin(); it != m_transactions.end(); ++it)
        sstr << " " << m_time << ": " << (*it) << " " << (*it)->time()->toString() << " " << (*it)->quantity()->toString() <<
          ((*it)->isConsumer() ? " (C)" : " (P)") << std::endl;
      return sstr.str();
    }
  }
}
