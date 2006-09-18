#include "HSTSDecisionPoints.hh"
#include "Schema.hh"
#include "XMLUtils.hh"
#include "ValueSource.hh"
#include "DbClient.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "ValueSource.hh"

#include <algorithm>
#include <sstream>

namespace EUROPA {
  namespace SOLVERS {
    namespace HSTS {

      ValueEnum::ValueEnum(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData, const LabelStr& explanation)
        : UnboundVariableDecisionPoint(client, flawedVariable, configData, explanation), m_choiceIndex(0) {
 
        // have to get rid of the old choice set that UnboundVaraibleDecisionPoint came up with
        delete m_choices;

        debugMsg("Solver:ValueEnum", "Instantiating value source for " << flawedVariable->toString());
        m_choices = ValueSource::getSource(flawedVariable, true);
        checkError(m_choices != NULL, "Failed to correctly allocate a value source for " << flawedVariable->toString());

        bool convertToObject = Schema::instance()->isObjectType(flawedVariable->baseDomain().getTypeName());

        if(flawedVariable->lastDomain().isSingleton()) {
          debugMsg("Solver:ValueEnum", "Ignoring ordering heuristic because '" << flawedVariable->toString() << "' is singleton.");
          ((OrderedValueSource*) m_choices)->addValue(flawedVariable->lastDomain().getSingletonValue());
        }
        else {
          TiXmlElement* value = configData.FirstChildElement();      
          checkError(value != NULL, "Attempted to define an empty value enumeration.");
	  
          while(value != NULL) {
            if(strcmp(value->Value(), "Value") == 0) {
              double v = readValue(*value);
              debugMsg("Solver:ValueEnum", "Read value " << v << " from " << *value << " convertToObject " << convertToObject);
              if(convertToObject) {
                ObjectId obj = client->getObject((LabelStr(v)).c_str());
                checkError(obj.isValid(), "No object named '" << LabelStr(v) << "' exists, which is listed in choice ordering for " 
                           << flawedVariable->toString());
                v = obj;
              }
              ((OrderedValueSource*)m_choices)->addValue(v);
            }
            value = value->NextSiblingElement();
          }
        }
      }
    
      double ValueEnum::readValue(const TiXmlElement& element) const {
        const char* data = element.Attribute("val");
        checkError(data != NULL && strcmp(data, "") != 0, "Value element with missing or empty val attribute.");
      
        double retval;

        if(!isNumber(data, retval)) {
          if(strcmp(data, "true") == 0 || strcmp(data, "TRUE") == 0 || strcmp(data, "True") == 0)
            retval = 1;
          else if(strcmp(data, "false") == 0 || strcmp(data, "FALSE") == 0 || strcmp(data, "False") == 0)
            retval = 0;
          else
            retval = LabelStr::getKey(data);
        }

        return retval;
      }

      double ValueEnum::getNext(){
        debugMsg("Solver:ValueEnum:getNext", "Getting value " << m_choices->getValue(m_choiceIndex) << " for variable " << 
                 getFlawedVariable()->toString() << " (index " << m_choiceIndex << ")");
        return m_choices->getValue(m_choiceIndex++);
      }

      bool ValueEnum::hasNext() const {
        debugMsg("Solver:ValueEnum:hasNext", "Current index " << m_choiceIndex << " total count " << m_choices->getCount());
        return m_choiceIndex < m_choices->getCount();
      }

      //accepted choice options: mergeFirst, activateFirst, mergeOnly, activateOnly
      //accepted order options (only for merge): early, late, near, far
      OpenConditionDecisionPoint::OpenConditionDecisionPoint(const DbClientId& client, const TokenId& flawedToken, const TiXmlElement& configData, const LabelStr& explanation) 
        : SOLVERS::OpenConditionDecisionPoint(client, flawedToken, configData, explanation) {
        std::string choice(configData.Attribute("choice") == NULL ? "mergeFirst" : configData.Attribute("choice"));
        std::string order(configData.Attribute("order") == NULL ? "early" : configData.Attribute("order"));

        debugMsg("OpenConditionDecisionPoint:constructor", "Constructing for " << m_flawedToken->getKey() << " with choice order " << choice << " and merge order " << order);

        if(choice == "activateFirst")
          m_action = activateFirst;
        else if(choice == "mergeFirst")
          m_action = mergeFirst;
        else if(choice == "activateOnly")
          m_action = activateOnly;
        else if(choice == "mergeOnly")
          m_action = mergeOnly;
        else
          checkError(ALWAYS_FAIL, "No such open condition resolution order '" << choice << "'");

        TiXmlElement orderElem("order");
        orderElem.SetAttribute("component", order);

        m_comparator = new TokenComparatorWrapper((TokenComparator*) Component::AbstractFactory::allocate(orderElem), flawedToken);

        checkError(m_comparator != NULL, "Failed to allocate comparator.");
      }

      void OpenConditionDecisionPoint::handleInitialize() {
        const StateDomain stateDomain(m_flawedToken->getState()->lastDomain());

        if(stateDomain.isMember(Token::ACTIVE) && (m_action == activateFirst || m_action == activateOnly) &&
           m_flawedToken->getPlanDatabase()->hasOrderingChoice(m_flawedToken)) {
          debugMsg("OpenConditionDecisionPoint:handleInitialize", "Adding choice '" << Token::ACTIVE.toString() << "' for token " << m_flawedToken->getKey());
          m_choices.push_back(Token::ACTIVE);
        }

        if(stateDomain.isMember(Token::MERGED) && (m_action == mergeFirst || m_action == mergeOnly || m_action == activateFirst)) {

          m_flawedToken->getPlanDatabase()->getCompatibleTokens(m_flawedToken, m_compatibleTokens, PLUS_INFINITY, true);
          m_mergeCount = m_compatibleTokens.size();

          if(m_mergeCount > 0) {
            debugMsg("OpenConditionDecisionPoint:handleInitialize", "Adding choice '" << Token::MERGED.toString() << "' for token " << m_flawedToken->getKey());
            m_choices.push_back(Token::MERGED);
            std::sort<std::vector<TokenId>::iterator, TokenComparatorWrapper& >(m_compatibleTokens.begin(), m_compatibleTokens.end(), *m_comparator);
          }
          else {
            debugMsg("OpenConditionDecisionPoint:handleInitialize", "Skipping choice '" << Token::MERGED.toString() << "' for token " << m_flawedToken->getKey() 
                     << " because there were no compatible tokens.");
          }

          if(m_action == mergeFirst && m_flawedToken->getPlanDatabase()->hasOrderingChoice(m_flawedToken)) {
            debugMsg("OpenConditionDecisionPoint:handleInitialize", "Adding choice '" << Token::ACTIVE.toString() << "' for token " << m_flawedToken->getKey());
            m_choices.push_back(Token::ACTIVE);
          }
        }

        if(stateDomain.isMember(Token::REJECTED)) {
          debugMsg("OpenConditionDecisionPoint:handleInitialize", "Adding choice '" << Token::REJECTED.toString() << "' for token " << m_flawedToken->getKey());
          m_choices.push_back(Token::REJECTED);
        }
      
        m_choiceCount = m_choices.size();
      }
      
      OpenConditionDecisionPoint::~OpenConditionDecisionPoint() {
        delete m_comparator;
      }

      void TokenComparator::extractTokens(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                                          const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2,
                                          TokenId& t1, TokenId& t2) {
        t1 = (p1.second.first == m_flawedTok ? p1.second.second : p1.second.first);
        t2 = (p2.second.first == m_flawedTok ? p2.second.second : p2.second.first);
      }

      bool TokenComparator::compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                                    const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2) {
        TokenId x, y;
        extractTokens(p1, p2, x, y);
        return compare(x, y);
      }

      bool EarlyTokenComparator::compare(const TokenId x, const TokenId y) {
        return x->getStart()->lastDomain().getLowerBound() < y->getStart()->lastDomain().getLowerBound();
      }

      bool EarlyTokenComparator::compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                                         const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2) {
        check_error(m_flawedTok.isValid());
        if(m_flawedTok == p1.second.second) {
          if(m_flawedTok == p2.second.second) {
            return p1.second.second->getEnd()->lastDomain().getLowerBound() <
              p2.second.second->getEnd()->lastDomain().getLowerBound();
          }
          else
            return false;
        }
        else if(m_flawedTok == p2.second.second)
          return true;
        else
          return compare(p1.second.second, p2.second.second);
        check_error(ALWAYS_FAIL);
        return true;
      }


      TokenComparator* EarlyTokenComparator::copy() {
        return new EarlyTokenComparator(m_flawedTok);
      }

      bool LateTokenComparator::compare(const TokenId x, const TokenId y) {
        return x->getStart()->lastDomain().getLowerBound() > y->getStart()->lastDomain().getLowerBound();
      }

      bool LateTokenComparator::compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                                        const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2) {
        check_error(m_flawedTok.isValid());
        if(m_flawedTok == p1.second.second) {
          if(m_flawedTok == p2.second.second) {
            return p1.second.second->getEnd()->lastDomain().getLowerBound() >
              p2.second.second->getEnd()->lastDomain().getLowerBound();
          }
          else
            return true;
        }
        else if(m_flawedTok == p2.second.second)
          return false;
        else
          return compare(p1.second.second, p2.second.second);
        check_error(ALWAYS_FAIL);
        return true;
      }

      TokenComparator* LateTokenComparator::copy() {
        return new LateTokenComparator(m_flawedTok);
      }

      bool NearTokenComparator::compare(const TokenId x, const TokenId y) {
        bool retval = absoluteDistance(m_flawedTok, x) < absoluteDistance(m_flawedTok, y);
        debugMsg("NearTokenComparator:compare", "" << x->getKey() << " before " << y->getKey() << " ? "
                 << (retval ? "true" : "false"));
        return retval;
      }

      TokenComparator* NearTokenComparator::copy() {
        return new NearTokenComparator(m_flawedTok);
      }

      int NearTokenComparator::absoluteDistance(const TokenId& a, const TokenId& b) {
        return abs(midpoint(a) - midpoint(b));
      }

      int NearTokenComparator::midpoint(const TokenId& token) {
        int maxTemporalExtent = (int) (token->getEnd()->lastDomain().getUpperBound() -
                                       token->getStart()->lastDomain().getLowerBound());
        return (int)(token->getStart()->lastDomain().getLowerBound() + maxTemporalExtent / 2);
      }

      bool FarTokenComparator::compare(const TokenId x, const TokenId y) {
        bool retval = absoluteDistance(m_flawedTok, x) > absoluteDistance(m_flawedTok, y);
        debugMsg("FarTokenComparator:compare", "" << x->getKey() << " before " << y->getKey() << " ? "
                 << (retval ? "true" : "false"));
        return retval;
      }

      TokenComparator* FarTokenComparator::copy() {
        return new FarTokenComparator(m_flawedTok);
      }

      bool AscendingKeyTokenComparator::compare(const TokenId x, const TokenId y) {
        return x->getKey() < y->getKey();
      }

      bool AscendingKeyTokenComparator::compare(const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
						const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2) {
	if(p1.first->getKey() < p2.first->getKey())
	  return true;
	if(p1.first->getKey() > p2.first->getKey())
	  return false;
	return TokenComparator::compare(p1, p2);
      }


      TokenComparator* AscendingKeyTokenComparator::copy() {
        return new AscendingKeyTokenComparator(m_flawedTok);
      }

      TokenComparatorWrapper::TokenComparatorWrapper(TokenComparator* cmp, TokenId flawedToken) : m_comparator(cmp) {
        checkError(m_comparator != NULL, "Invalid comparator object.");
        checkError(flawedToken.isValid(), "Cannot base a comparator on an invalid token.");

        m_comparator->m_flawedTok = flawedToken;
      }

      TokenComparatorWrapper::TokenComparatorWrapper(const TokenComparatorWrapper& other) {
        m_comparator = other.m_comparator->copy();
        checkError(m_comparator != NULL, "Invalid comparator object.");
      }

      TokenComparatorWrapper::~TokenComparatorWrapper() {
        delete m_comparator;
      }

      ThreatDecisionPoint::ThreatDecisionPoint(const DbClientId& client, const TokenId& tokenToOrder, const TiXmlElement& configData, const LabelStr& explanation)
        : SOLVERS::ThreatDecisionPoint(client, tokenToOrder, configData, explanation) {
        std::string order((configData.Attribute("order") == NULL ? "early" : configData.Attribute("order")));
      
        debugMsg("ThreatDecisionPoint:constructor", "Constructing for " << tokenToOrder->getKey() << " with choice order " << order);
        TiXmlElement orderElem("order");
        orderElem.SetAttribute("component", order);
        m_comparator = new ThreatComparator((TokenComparator*) Component::AbstractFactory::allocate(orderElem), tokenToOrder);
      }

      //       class ObjectComparator {
      //       public:
      // 	bool operator() (const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
      // 			 const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2) const {
      // 	  ObjectId o1 = p1.first;
      // 	  ObjectId o2 = p2.first;
      // 	  return o1->getKey() < o2->getKey();
      // 	}
      // 	bool operator==(const ObjectComparator& c){return true;}
      //       };

      void ThreatDecisionPoint::handleInitialize() {
        SOLVERS::ThreatDecisionPoint::handleInitialize();
        //first order choices by object key
        // 	ObjectComparator cmp;
        // 	std::sort<std::vector<std::pair<ObjectId, std::pair<TokenId, TokenId> > >::iterator, ObjectComparator&>(m_choices.begin(), m_choices.end(), cmp);
        //then order them by the heuristic
        std::sort<std::vector<std::pair<ObjectId, std::pair<TokenId, TokenId> > >::iterator, ThreatComparator&>(m_choices.begin(), m_choices.end(), *m_comparator);
        debugMsg("ThreatDecisionPoint:handleInitialize", "Final choice order for " << m_tokenToOrder->getKey() << ": " << choicesToString());
      }
  
      ThreatDecisionPoint::~ThreatDecisionPoint() {
        delete m_comparator;
      }


      std::string ThreatDecisionPoint::choicesToString() {
        std::stringstream retval;
        for(std::vector<std::pair<ObjectId, std::pair<TokenId, TokenId> > >::const_iterator it = m_choices.begin();
            it != m_choices.end(); ++it) {
          ObjectId o = (*it).first;
          TokenId t1 = (*it).second.first;
          TokenId t2 = (*it).second.second;
          retval << o->toString() << "(" << o->getKey() << "):" << t1->getKey() << "<" << t2->getKey() << ",";
        }
        return retval.str();
      }

      ThreatDecisionPoint::ThreatComparator::ThreatComparator(TokenComparator* comparator, const TokenId& tok) : m_comparator(comparator) {
        m_comparator->m_flawedTok = tok;
      }

      ThreatDecisionPoint::ThreatComparator::ThreatComparator(const ThreatComparator& other) {
        m_comparator = other.m_comparator->copy();
      }

      ThreatDecisionPoint::ThreatComparator::~ThreatComparator() {
        delete m_comparator;
      }
      
      bool ThreatDecisionPoint::ThreatComparator::operator() (const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p1,
                                                              const std::pair<ObjectId, std::pair<TokenId, TokenId> >& p2) {
        return m_comparator->compare(p1, p2);
      }

      class HSTSDecisionPointLocalStatic {
      public:
        HSTSDecisionPointLocalStatic() {
          static bool sl_bool = false;
          checkError(sl_bool == false, "Should only be called once");
          if(sl_bool == false) {
            REGISTER_TOKEN_SORTER(EarlyTokenComparator, early);
            REGISTER_TOKEN_SORTER(LateTokenComparator, late);
            REGISTER_TOKEN_SORTER(NearTokenComparator, near);
            REGISTER_TOKEN_SORTER(FarTokenComparator, far);
            REGISTER_TOKEN_SORTER(AscendingKeyTokenComparator, ascendingKey);
            sl_bool = true;
          }
        }
      };

      HSTSDecisionPointLocalStatic s_hstsDecisionPointLocalStatic;
    }
  }
}
