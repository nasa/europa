#ifndef H_ThreatDecisionPoint
#define H_ThreatDecisionPoint

#include "SolverDefs.hh"
#include "SolverDecisionPoint.hh"
#include <vector>

/**
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Defines a class for formulation, execution and retraction of token ordering
     * decisions as a means to resolve object flaws.
     */
    class ThreatDecisionPoint: public DecisionPoint {
    public:

      ThreatDecisionPoint(const DbClientId& client, const TokenId& tokenToOrder, const TiXmlElement& configData);

      /**
       * @brief Used to prune entities out which are not active tokens
       */
      static bool test(const EntityId& entity);

      const TokenId& getToken() const {return m_tokenToOrder;}

    protected:
      virtual void handleInitialize();

      void extractParts(unsigned int index, ObjectId& object, TokenId& predecessor, TokenId& successor) const;

      /** Main Interface for the solver **/
      bool hasNext() const;

      const TokenId m_tokenToOrder; /*!< The token that must be ordered */
      std::vector< std::pair<ObjectId, std::pair<TokenId, TokenId> > > m_choices; /*!< Choices across all objects */
      unsigned int m_choiceCount; /*!< Stored choice count - size of m_orderingChoices */
      unsigned int m_index; /*!< Current choice position in m_orderingChoices */

    private:
      virtual void handleExecute();
      virtual void handleUndo();

      /** HELPER METHODS **/
      std::string toString() const;
      std::string toString(unsigned int index, const std::pair<ObjectId, std::pair<TokenId, TokenId> >& choice) const;
    };

  }
}

#define REGISTER_THREAT_DECISION_FACTORY(CLASS, NAME)\
REGISTER_DECISION_FACTORY(CLASS, NAME);
#endif
