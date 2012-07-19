/**
 * @author Conor McGann
 */

#include "FlawFilter.hh"

namespace EUROPA {
  namespace SOLVERS {

    FlawFilter::FlawFilter(const TiXmlElement& configData, bool isDynamic)
      : MatchingRule(configData), m_isDynamic(isDynamic) {}

    bool FlawFilter::isDynamic() const {return m_isDynamic;}

    bool FlawFilter::test(const EntityId& entity) {return true;}
  }
}
