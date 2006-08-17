#ifndef _H_SAVH_ThreatManager
#define _H_SAVH_ThreatManager

#include "FlawManager.hh"

namespace EUROPA {
  namespace SAVH {
    class InstantComparator;

    class DecisionOrder {
    public:
      DecisionOrder() {}
      DecisionOrder(const DecisionOrder& other);
      ~DecisionOrder();
      bool operator()(const InstantId& a, const InstantId& b, LabelStr& explanation) const;
      void addOrder(InstantComparator* cmp);
    private:
      std::list<InstantComparator*> m_cmps;
    };

    class ThreatManager : public SOLVERS::FlawManager {
    public:
      ThreatManager(const TiXmlElement& configData);
      virtual ~ThreatManager();
      virtual std::string toString(const EntityId& entity) const;
      virtual bool staticMatch(const EntityId& entity);
      virtual bool dynamicMatch(const EntityId& entity);
      virtual void handleInitialize();
      virtual bool betterThan(const EntityId& a, const EntityId& b, LabelStr& explanation);
      virtual IteratorId createIterator();

      virtual void notifyAdded(const ConstraintId& constraint){}
      virtual void notifyRemoved(const ConstraintId& constraint) {}
      virtual void notifyRemoved(const ConstrainedVariableId& var) {}
      virtual void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType){}
      virtual void notifyAdded(const TokenId& token) {}
      virtual void notifyRemoved(const TokenId& token) {}
      
    protected:
    private:
      bool m_preferUpper, m_preferLower;
      DecisionOrder m_order;
    };
  }
}

#endif
