#ifndef H_HSTSDecisionPoints
#define H_HSTSDecisionPoints

#include "FlawHandler.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "OpenConditionDecisionPoint.hh"
#include "ThreatDecisionPoint.hh"


#include <functional>

/**
 * @brief Provides class declarations for handling flaws with HSTS ordering preferences.
 * @author Michael Iatauro
 */

namespace EUROPA {

  namespace SOLVERS {

    namespace HSTS {
    
      class ValueEnum : public UnboundVariableDecisionPoint {
      public:
        ValueEnum(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData, const LabelStr& explanation = "unknown");
        double getNext();
        bool hasNext() const;
      private:
        double readValue(const TiXmlElement& value) const;
        unsigned int m_choiceIndex;
      };

      class TokenComparator;
      class TokenComparatorWrapper;

      class OpenConditionDecisionPoint : public SOLVERS::OpenConditionDecisionPoint {
      public:
        OpenConditionDecisionPoint(const DbClientId& client, const TokenId& flawedToken, const TiXmlElement& configData, const LabelStr& explanation = "unknown");
        void handleInitialize();
        ~OpenConditionDecisionPoint();
        const std::vector<LabelStr>& getStateChoices(){return m_choices;}
        const std::vector<TokenId>& getCompatibleTokens(){return m_compatibleTokens;}
      private:
        enum Actions {
          activateFirst = 0,
          mergeFirst,
          activateOnly,
          mergeOnly
        };
      
        Actions m_action;
        TokenComparatorWrapper* m_comparator;
      };

      class ThreatDecisionPoint : public SOLVERS::ThreatDecisionPoint {
      public:
        ThreatDecisionPoint(const DbClientId& client, const TokenId& tokenToOrder, const TiXmlElement& configData, const LabelStr& explanation = "unknown");
        void handleInitialize();
        ~ThreatDecisionPoint();
        const std::vector<std::pair<ObjectId, std::pair<TokenId, TokenId> > >& getOrderingChoices(){return m_choices;}
      private:
        std::string choicesToString();
        class ThreatComparator {
        public:
          ThreatComparator(TokenComparator* comparator, const TokenId& tok);
          ThreatComparator(const ThreatComparator& other);
          ~ThreatComparator();
          bool operator() (const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
			   const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2);
        private:
          TokenComparator* m_comparator;
        };

        ThreatComparator* m_comparator;
      };

      class TokenComparator : public Component {
      public:
        TokenComparator(const TiXmlElement& configData) : Component(configData) {}
        TokenComparator(TokenId tok) : m_flawedTok(tok) {}
        virtual bool compare(const TokenId x, const TokenId y) = 0;
        virtual bool compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                             const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2);
        virtual TokenComparator* copy() = 0;
        void extractTokens(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                           const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2,
                           TokenId& t1, TokenId& t2);
        TokenId m_flawedTok;
      };

      //have to stick this in because STL wants to copy the comparator object.
      class TokenComparatorWrapper {
      public:
        TokenComparatorWrapper(TokenComparator* cmp, TokenId flawedToken);
        TokenComparatorWrapper(const TokenComparatorWrapper& other);
        ~TokenComparatorWrapper();
        bool operator() (TokenId x, TokenId y) {return m_comparator->compare(x, y);}
      private:
        TokenComparator* m_comparator;
      };

      class EarlyTokenComparator : public TokenComparator {
      public:
        EarlyTokenComparator(const TiXmlElement& configData) : TokenComparator(configData) {};
        EarlyTokenComparator(TokenId tok) : TokenComparator(tok) {}
        bool compare(const TokenId x, const TokenId y);
        bool compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                     const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2);
        TokenComparator* copy();
      };

      class LateTokenComparator : public TokenComparator {
      public:
        LateTokenComparator(const TiXmlElement& configData) : TokenComparator(configData) {};
        LateTokenComparator(TokenId tok) : TokenComparator(tok) {}
        bool compare(const TokenId x, const TokenId y);
        bool compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                     const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2);
        TokenComparator* copy();
      };

      class NearTokenComparator : public TokenComparator {
      public:
        NearTokenComparator(const TiXmlElement& configData) : TokenComparator(configData) {};
        NearTokenComparator(TokenId tok) : TokenComparator(tok) {}
        virtual bool compare(const TokenId x, const TokenId y);
        virtual TokenComparator* copy();
        static int absoluteDistance(const TokenId& a, const TokenId& b);
        static int midpoint(const TokenId& token);
      };

      class FarTokenComparator : public NearTokenComparator {
      public:
        FarTokenComparator(const TiXmlElement& configData) : NearTokenComparator(configData) {};
        FarTokenComparator(TokenId tok) : NearTokenComparator(tok) {}
        bool compare(const TokenId x, const TokenId y);
        TokenComparator* copy();
      };

      class AscendingKeyTokenComparator : public TokenComparator {
      public:
        AscendingKeyTokenComparator(const TiXmlElement& configData) : TokenComparator(configData){};
        AscendingKeyTokenComparator(TokenId tok) : TokenComparator(tok) {}
        bool compare(const TokenId x, const TokenId y);
	bool compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
		     const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2);
        TokenComparator* copy();
      };

#define REGISTER_TOKEN_SORTER(CLASS, NAME) REGISTER_COMPONENT_FACTORY(CLASS, NAME);

    }
  }
}
#endif
