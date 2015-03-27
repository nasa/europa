#ifndef H_DurativeTokens
#define H_DurativeTokens

#include "ResourceDefs.hh"
#include "EventToken.hh"

namespace EUROPA {
    class ReusableToken : public IntervalToken {
    public:
      ReusableToken(const PlanDatabaseId planDatabase,
		    const std::string& predicateName,
		    const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		    const IntervalDomain& quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    const std::string& objectName = Token::noObject(),
		    bool closed = true,
		    bool activate = true);

      ReusableToken(const PlanDatabaseId planDatabase,
		    const std::string& predicateName,
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const std::string& objectName,
		    bool closed,
		    bool activate = true);

      ReusableToken(const TokenId master,
		    const std::string& relation,
		    const std::string& predicateName,
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const std::string& objectName,
		    bool closed,
		    bool activate = true);

      ReusableToken(const TokenId master,
		    const std::string& relation,
		    const std::string& predicateName,
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const IntervalDomain& quantityBaseDomain,
		    const std::string& objectName,
		    bool closed,
		    bool activate = true);

      ReusableToken(const PlanDatabaseId planDatabase,
		    const std::string& predicateName,
		    bool rejectable,
		    bool isFact,
		    const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		    const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		    const IntervalDomain& quantityBaseDomain = IntervalDomain(0, PLUS_INFINITY),
		    const std::string& objectName = Token::noObject(),
		    bool closed = true,
		    bool activate = true);

      ReusableToken(const PlanDatabaseId planDatabase,
		    const std::string& predicateName,
		    bool rejectable,
		    bool isFact,
		    const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    const IntervalIntDomain& durationBaseDomain,
		    const std::string& objectName,
		    bool closed,
		    bool activate = true);

      const ConstrainedVariableId getQuantity() const;
      void print(std::ostream& os);
      virtual void close();
    protected:
      void commonInit(bool close, bool activate, const IntervalDomain& quantityBaseDomain);
      ConstrainedVariableId m_quantity;
      bool m_activate;
    };

    class UnaryToken : public ReusableToken {
    public:
      UnaryToken(const PlanDatabaseId planDatabase,
		 const std::string& predicateName,
		 const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		 const std::string& objectName = Token::noObject(),
		 bool closed = true,
		 bool activate = true);

      UnaryToken(const PlanDatabaseId planDatabase,
		 const std::string& predicateName,
		 bool rejectable,
		 bool isFact,
		 const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		 const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		 const std::string& objectName = Token::noObject(),
		 bool closed = true,
		 bool activate = true);

      UnaryToken(const TokenId master,
		 const std::string& relation,
		 const std::string& predicateName,
		 const IntervalIntDomain& startBaseDomain,
		 const IntervalIntDomain& endBaseDomain,
		 const IntervalIntDomain& durationBaseDomain,
		 const std::string& objectName,
		 bool closed,
		 bool activate = true);
    };
}

#endif
