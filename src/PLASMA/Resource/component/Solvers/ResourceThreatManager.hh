#ifndef H_ResourceThreatManager
#define H_ResourceThreatManager

#include "FlawManager.hh"
#include "Instant.hh"

namespace EUROPA {
    class InstantComparator;

    class DecisionOrder {
    public:
      DecisionOrder() : m_cmps() {}
      DecisionOrder(const DecisionOrder& other);
      ~DecisionOrder();
      bool operator()(const InstantId a, const InstantId b, std::string& explanation) const;
      void addOrder(InstantComparator* cmp);
    private:
      std::list<InstantComparator*> m_cmps;
    };

    class ResourceThreatManager : public SOLVERS::FlawManager {
    public:
      ResourceThreatManager(const TiXmlElement& configData);
      virtual ~ResourceThreatManager();
      virtual std::string toString(const EntityId entity) const;
      virtual bool staticMatch(const EntityId entity);
      virtual bool dynamicMatch(const EntityId entity);
      virtual void handleInitialize();
      virtual bool betterThan(const EntityId a, const EntityId b, std::string& explanation);
      virtual IteratorId createIterator();

      virtual void notifyAdded(const ConstraintId){}
      virtual void notifyRemoved(const ConstraintId) {}
      virtual void notifyRemoved(const ConstrainedVariableId) {}
      virtual void notifyChanged(const ConstrainedVariableId, const DomainListener::ChangeType&){}
      virtual void notifyAdded(const TokenId) {}
      virtual void notifyRemoved(const TokenId) {}
      bool noMoreFlaws();

    protected:
    private:
      bool m_preferUpper, m_preferLower;
      DecisionOrder m_order;
    };
}

#endif
