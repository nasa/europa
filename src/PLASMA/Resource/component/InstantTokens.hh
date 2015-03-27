#ifndef H_InstantTokens
#define H_InstantTokens

#include "ResourceDefs.hh"
#include "EventToken.hh"

namespace EUROPA {
    class ReservoirToken : public EventToken {
    public:
      ReservoirToken(const PlanDatabaseId planDatabase,
		     const std::string& predicateName,
		     const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
		     const IntervalDomain& quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		     bool isConsumer = false,
		     bool closed = true,
		     bool activate = true);

      ReservoirToken(const PlanDatabaseId planDatabase,
		     const std::string& predicateName,
		     bool rejectable,
		     bool isFact,
		     const IntervalIntDomain& timeBaseDomain,
		     const std::string& objectName,
		     bool isConsumer,
		     bool closed,
		     bool activate = true);

      ReservoirToken(const TokenId parent,
		     const std::string& relation,
		     const std::string& predicateName,
		     const IntervalIntDomain& timeBaseDomain,
		     const std::string& objectName,
		     bool isConsumer,
		     bool closed,
		     bool activate = true);

      const ConstrainedVariableId getQuantity() const;
      bool isConsumer() const;
      void print(std::ostream& os);
      virtual void close();
    protected:
      void commonInit(bool closed, bool activate, const IntervalDomain& quantityBaseDomain);
      ConstrainedVariableId m_quantity;
      bool m_isConsumer;
    private:
      bool m_activate;
    };

class ConsumerToken : public ReservoirToken {
 public:
  ConsumerToken(const PlanDatabaseId planDatabase,
                const std::string& predicateName,
                const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
                const IntervalDomain quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
                bool closed = true,
                bool _activate = true)
      : ReservoirToken(planDatabase, predicateName, timeBaseDomain,
                       quantityBaseDomain, true, closed, _activate) {
  }
  ConsumerToken(const PlanDatabaseId planDatabase,
                const std::string& predicateName,
                bool rejectable,
                bool _isFact,
                const IntervalIntDomain& timeBaseDomain,
                const std::string& objectName,
                bool closed,
                bool _activate = true)
      : ReservoirToken(planDatabase, predicateName, rejectable, _isFact,
                       timeBaseDomain, objectName, true, closed, _activate) {
  }

  ConsumerToken(const TokenId parent,
                const std::string& relation,
                const std::string& predicateName,
                const IntervalIntDomain& timeBaseDomain,
                const std::string& objectName,
                bool closed,
                bool _activate = true)
      : ReservoirToken(parent, relation, predicateName, timeBaseDomain, objectName, true, closed, _activate) {
  }
};

class ProducerToken : public ReservoirToken {
 public:
  ProducerToken(const PlanDatabaseId planDatabase,
                const std::string& predicateName,
                const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
                const IntervalDomain quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
                bool closed = true,
                bool _activate = true)
      : ReservoirToken(planDatabase, predicateName, timeBaseDomain, quantityBaseDomain,
                       false, closed, _activate) {
  }
  ProducerToken(const PlanDatabaseId planDatabase,
                const std::string& predicateName,
                bool rejectable,
                bool _isFact,
                const IntervalIntDomain& timeBaseDomain,
                const std::string& objectName,
                bool closed,
                bool _activate = true)
      : ReservoirToken(planDatabase, predicateName, rejectable, _isFact,
                       timeBaseDomain, objectName, false, closed, _activate) {
  }

  ProducerToken(const TokenId parent,
                const std::string& relation,
                const std::string& predicateName,
                const IntervalIntDomain& timeBaseDomain,
                const std::string& objectName,
                bool closed,
                bool _activate = true)
      : ReservoirToken(parent, relation, predicateName, timeBaseDomain, objectName,
                       false, closed, _activate) {
  }
};

}

#endif
