#ifndef _H_ResourceThreatDecisionPoint
#define _H_ResourceThreatDecisionPoint

#include "ThreatDecisionPoint.hh"

namespace EUROPA {
  namespace SOLVERS {
    class ResourceThreatDecisionPoint : public ThreatDecisionPoint {
    public:
      ResourceThreatDecisionPoint(const DbClientId& client,
				  const TokenId& tokenToOrder,
				  const TiXmlElement& configData);
      void execute() {ThreatDecisionPoint::execute();}
      void undo() {ThreatDecisionPoint::undo();}
      bool hasNext() const {return ThreatDecisionPoint::hasNext();}
    private:
      void handleExecute();
      void handleUndo();
    };
  }
}
#endif
