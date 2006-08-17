#include "ResourceThreatDecisionPoint.hh"
#include "Token.hh"
#include "Object.hh"
#include "DbClient.hh"
#include "Debug.hh"
#include "PlanDatabase.hh"

namespace EUROPA {
  namespace SOLVERS {

    ResourceThreatDecisionPoint::ResourceThreatDecisionPoint(const DbClientId& client,
                                                             const TokenId& tokenToOrder,
                                                             const TiXmlElement& configData,
                                                             const LabelStr& explanation) 
      : ThreatDecisionPoint(client, tokenToOrder, configData, explanation) {}


    void ResourceThreatDecisionPoint::handleExecute() {
      checkError(m_index < m_choiceCount, "Tried to execute past available choices:" << m_index << ">=" << m_choiceCount);
      ObjectId object;
      TokenId predecessor;
      TokenId successor;
      extractParts(m_index, object, predecessor, successor);
            
      debugMsg("SolverDecisionPoint:handleExecute", "For " << m_tokenToOrder->getPredicateName().toString() << "(" <<
               m_tokenToOrder->getKey() << "), assigning " << predecessor->getPredicateName().toString() << "(" <<
               predecessor->getKey() << ") to be before " << successor->getPredicateName().toString() << "(" <<
               successor->getKey() << ").");
      m_client->constrain(object, predecessor, successor);
    }

    void ResourceThreatDecisionPoint::handleUndo() {
      debugMsg("SolverDecisionPoint:handleUndo", "Retracting ordering decision on " << m_tokenToOrder->getPredicateName().toString() <<
               "(" << m_tokenToOrder->getKey() << ").");
      ObjectId object;
      TokenId predecessor;
      TokenId successor;
      extractParts(m_index, object, predecessor, successor);

      m_client->free(object, predecessor, successor);
      m_index++; // Advance to next choice
    }
  }
}
