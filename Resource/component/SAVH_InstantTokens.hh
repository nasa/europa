#ifndef _H_SAVH_InstantTokens
#define _H_SAVH_InstantTokens

#include "SAVH_ResourceDefs.hh"
#include "EventToken.hh"

namespace EUROPA {
  namespace SAVH {
    class ReservoirToken : public EventToken {
    public:
      ReservoirToken(const PlanDatabaseId& planDatabase,
		     const LabelStr& predicateName,
		     const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
		     const IntervalDomain& quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		     bool isConsumer = false,
		     bool closed = true);

      ReservoirToken(const PlanDatabaseId& planDatabase,
		     const LabelStr& predicateName,
		     bool rejectable,
		     const IntervalIntDomain& timeBaseDomain,
		     const LabelStr& objectName,
		     bool closed);

      ReservoirToken(const TokenId& parent,
		     const LabelStr& relation,
		     const LabelStr& predicateName,
		     const IntervalIntDomain& timeBaseDomain,
		     const LabelStr& objectName,
		     bool closed);

      const ConstrainedVariableId& getQuantity() const;
      bool isConsumer() const;
      void print(std::ostream& os);
      virtual void close();
    protected:
      void commonInit(bool closed, const IntervalDomain& quantityBaseDomain);
      ConstrainedVariableId m_quantity;
      bool m_isConsumer;
    private:
    };

    class ConsumerToken : public ReservoirToken {
    public:
      ConsumerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
		    const IntervalDomain quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    bool closed = true)
	: ReservoirToken(planDatabase, predicateName, timeBaseDomain, quantityBaseDomain, true, closed) {}
      ConsumerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    bool rejectable,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed)
	: ReservoirToken(planDatabase, predicateName, rejectable, timeBaseDomain, objectName, closed) {
	m_isConsumer = true;
      }
      
      ConsumerToken(const TokenId& parent,
		    const LabelStr& relation,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed)
	: ReservoirToken(parent, relation, predicateName, timeBaseDomain, objectName, closed) {
	m_isConsumer = true;
      }
    };

    class ProducerToken : public ReservoirToken {
    public:
      ProducerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
		    const IntervalDomain quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    bool closed = true)
	: ReservoirToken(planDatabase, predicateName, timeBaseDomain, quantityBaseDomain, false, closed) {}
      ProducerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    bool rejectable,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed)
	: ReservoirToken(planDatabase, predicateName, rejectable, timeBaseDomain, objectName, closed) {
	m_isConsumer = false;
      }
      
      ProducerToken(const TokenId& parent,
		    const LabelStr& relation,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed)
	: ReservoirToken(parent, relation, predicateName, timeBaseDomain, objectName, closed) {
	m_isConsumer = false;
      }
    };

  }
}

#endif
