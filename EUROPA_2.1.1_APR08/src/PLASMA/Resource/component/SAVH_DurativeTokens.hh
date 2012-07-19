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
		    bool closed = true,
		    bool activate = true);

      ReusableToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true);

      ReusableToken(const TokenId& master, 
		    const LabelStr& relation,
		    const LabelStr& predicateName, 
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true);

      ReusableToken(const TokenId& master, 
		    const LabelStr& relation,
		    const LabelStr& predicateName, 
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const IntervalDomain& quantityBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true);

      ReusableToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    bool rejectable,
		    bool isFact,
		    const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		    const IntervalDomain& quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    const LabelStr& objectName = Token::noObject(),
		    bool closed = true,
		    bool activate = true);

      ReusableToken(const PlanDatabaseId& planDatabase,
		    const LabelStr& predicateName,
		    bool rejectable,
		    bool isFact,
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const LabelStr& objectName,
		    bool closed,
		    bool activate = true);

      const ConstrainedVariableId& getQuantity() const;
      void print(std::ostream& os);
      virtual void close();
    protected:
      void commonInit(bool close, bool activate, const IntervalDomain& quantityBaseDomain);
      ConstrainedVariableId m_quantity;
      bool m_activate;
    };

    class UnaryToken : public ReusableToken {
    public:
      UnaryToken(const PlanDatabaseId& planDatabase,
		 const LabelStr& predicateName,
		 const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		 const LabelStr& objectName = Token::noObject(),
		 bool closed = true,
		 bool activate = true);

      UnaryToken(const PlanDatabaseId& planDatabase,
		 const LabelStr& predicateName,
		 bool rejectable,
		 bool isFact,
		 const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		 const LabelStr& objectName = Token::noObject(),
		 bool closed = true,
		 bool activate = true);

      UnaryToken(const TokenId& master, 
		 const LabelStr& relation,
		 const LabelStr& predicateName, 
		 const IntervalIntDomain& startBaseDomain,
		 const IntervalIntDomain& endBaseDomain,
		 const IntervalIntDomain& durationBaseDomain,
		 const LabelStr& objectName,
		 bool closed,
		 bool activate = true);
    };
  }
}

#endif
