#ifndef _H_SAVH_DurativeTokens
#define _H_SAVH_DurativeTokens

#include "SAVH_ResourceDefs.hh"
#include "EventToken.hh"

namespace EUROPA {
  namespace SAVH {
    class ReusableToken : public IntervalToken {
    public:
      ReusableToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		    const IntervalDomain& quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    const LabelStr& objectName = Token::noObject(),
		    bool closed = true);

      ReusableToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const LabelStr& objectName,
		    bool closed);

      ReusableToken(const TokenId& master, 
		    const LabelStr& relation,
		    const LabelStr& predicateName, 
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const LabelStr& objectName,
		    bool closed);

      ReusableToken(const TokenId& master, 
		    const LabelStr& relation,
		    const LabelStr& predicateName, 
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const IntervalDomain& quantityBaseDomain,
		    const LabelStr& objectName,
		    bool closed);

      const ConstrainedVariableId& getQuantity() const;
      void print(std::ostream& os);
      virtual void close();
    protected:
      void commonInit(bool close, const IntervalDomain& quantityBaseDomain);
      ConstrainedVariableId m_quantity;
    };

    class UnaryToken : public ReusableToken {
    public:
      UnaryToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		    const LabelStr& objectName = Token::noObject(),
		    bool closed = true);

      UnaryToken(const TokenId& master, 
		    const LabelStr& relation,
		    const LabelStr& predicateName, 
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const LabelStr& objectName,
		    bool closed);
    };
  }
}

#endif
