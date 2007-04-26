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
		     bool closed = true,
		     bool activate = true);

      ReservoirToken(const PlanDatabaseId& planDatabase,
		     const LabelStr& predicateName,
		     bool rejectable,
		     bool isFact,
		     const IntervalIntDomain& timeBaseDomain,
		     const LabelStr& objectName,
		     bool isConsumer,
		     bool closed,
		     bool activate = true);

      ReservoirToken(const TokenId& parent,
		     const LabelStr& relation,
		     const LabelStr& predicateName,
		     const IntervalIntDomain& timeBaseDomain,
		     const LabelStr& objectName,
		     bool isConsumer,
		     bool closed,
		     bool activate = true);

      const ConstrainedVariableId& getQuantity() const;
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
      ConsumerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
		    const IntervalDomain quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    bool closed = true,
		    bool activate = true)
	: ReservoirToken(planDatabase, predicateName, timeBaseDomain, quantityBaseDomain, true, closed, activate) {
      }
      ConsumerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    bool rejectable,
		    bool isFact,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true)
	: ReservoirToken(planDatabase, predicateName, rejectable, isFact, timeBaseDomain, objectName, true, closed, activate) {
      }
      
      ConsumerToken(const TokenId& parent,
		    const LabelStr& relation,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true)
	: ReservoirToken(parent, relation, predicateName, timeBaseDomain, objectName, true, closed, activate) {
      }
    };

    class ProducerToken : public ReservoirToken {
    public:
      ProducerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
		    const IntervalDomain quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    bool closed = true,
		    bool activate = true)
	: ReservoirToken(planDatabase, predicateName, timeBaseDomain, quantityBaseDomain, false, closed, activate) {
      }
      ProducerToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    bool rejectable,
		    bool isFact,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true)
	: ReservoirToken(planDatabase, predicateName, rejectable, isFact, timeBaseDomain, objectName, false, closed, activate) {
      }
      
      ProducerToken(const TokenId& parent,
		    const LabelStr& relation,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& timeBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true)
	: ReservoirToken(parent, relation, predicateName, timeBaseDomain, objectName, false, closed, activate) {
      }
    };

  }
}

#endif
