#ifndef _H_SAVH_ThreatDecisionPoint
#define _H_SAVH_ThreatDecisionPoint

#include "SolverDecisionPoint.hh"
#include "SAVH_ResourceDefs.hh"
#include "SAVH_Instant.hh"
#include "ConstraintEngineDefs.hh"

#include <list>

namespace EUROPA {
  namespace SAVH {

    class ChoiceOrder;
    class ChoiceFilters;

    class ThreatDecisionPoint : public SOLVERS::DecisionPoint {
    public:
      ThreatDecisionPoint(const DbClientId& client, const InstantId& inst, const TiXmlElement& configData, const LabelStr& explanation = "unknown");
      virtual ~ThreatDecisionPoint();
      virtual std::string toString() const;
      void execute() {DecisionPoint::execute();}
      void undo() {DecisionPoint::undo();}
      const std::vector<std::pair<TransactionId, TransactionId> >& getChoices() {return m_choices;}
      virtual void handleInitialize();
      virtual bool hasNext() const;
      virtual bool canUndo() const;
      virtual void handleExecute();
      virtual void handleUndo();
      static bool test(const EntityId& entity);
    protected:
    private:
      std::string toString(const std::pair<TransactionId, TransactionId>& choice) const;
      void createFilter(ChoiceFilters& filters, const std::string& filter, ProfileId& profile);
      void createOrder(ChoiceOrder& order);

      InstantId m_flawedInstant;
      std::vector<std::pair<TransactionId, TransactionId> > m_choices;
      unsigned int m_choiceCount;
      unsigned int m_index;
      ConstraintId m_constr;
      int m_instTime;
      LabelStr m_resName;
      std::string m_order;
      std::string m_filter;
      std::string m_constraintOrder;
      std::vector<std::string> m_constraintNames;
      std::vector<std::string>::const_iterator m_constraintIt;
    };

  }
}

#endif
