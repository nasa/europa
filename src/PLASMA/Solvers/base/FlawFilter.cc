/**
 * @author Conor McGann
 */

#include "FlawFilter.hh"

namespace EUROPA {
namespace SOLVERS {

FlawFilter::FlawFilter(const TiXmlElement& configData, bool _isDynamic)
    : MatchingRule(configData), m_isDynamic(_isDynamic), m_context() {}

bool FlawFilter::isDynamic() const {return m_isDynamic;}

bool FlawFilter::test(const EntityId) {return true;}
}
}
