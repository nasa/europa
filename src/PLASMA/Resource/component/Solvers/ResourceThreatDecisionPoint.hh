#ifndef H_ResourceThreatDecisionPoint
#define H_ResourceThreatDecisionPoint

#include "SolverDecisionPoint.hh"
#include "ResourceDefs.hh"
#include "Instant.hh"
#include "ConstraintEngineDefs.hh"

#include <list>

namespace EUROPA {

    class ChoiceOrder;
    class ChoiceFilters;

    class ResourceThreatDecisionPoint : public SOLVERS::DecisionPoint {
    public:
      ResourceThreatDecisionPoint(const DbClientId client, const InstantId inst, const TiXmlElement& configData, const std::string& explanation = "unknown");
      virtual ~ResourceThreatDecisionPoint();
      virtual std::string toString() const;
      virtual std::string toShortString() const;
      void execute() {DecisionPoint::execute();}
      void undo() {DecisionPoint::undo();}
      const std::vector<std::pair<TransactionId, TransactionId> >& getChoices() {return m_choices;}
      virtual void handleInitialize();
      virtual bool hasNext() const;
      virtual bool canUndo() const;
      virtual void handleExecute();
      virtual void handleUndo();
      static bool test(const EntityId entity);

    private:
      std::string toString(const std::pair<TransactionId, TransactionId>& choice) const;
      void createFilter(ChoiceFilters& filters, const std::string& filter, ProfileId profile);
      void createOrder(ChoiceOrder& order);
    protected:
      InstantId m_flawedInstant;
      std::vector<std::pair<TransactionId, TransactionId> > m_choices;
      unsigned long m_choiceCount;
      unsigned long m_index;
      ConstraintId m_constr;
      eint m_instTime;
      std::string m_resName;
      std::string m_order;
      std::string m_filter;
      std::string m_constraintOrder;
      std::vector<std::string> m_constraintNames;
      std::vector<std::string>::const_iterator m_constraintIt;
    };

}

#endif
