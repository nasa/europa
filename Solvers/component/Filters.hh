#ifndef H_Filters
#define H_Filters

#include "MatchingRule.hh"

/**
 * @file Provides Declarations for useful flaw filter components
 * @author Conor McGann
 * @date May, 2005
 */
namespace EUROPA {
  namespace SOLVERS {
    /**
     * @brief Will filter a variable out until its derived domain becomes a singleton.
     */
    class SingletonFilter: public VariableMatchingRule {
    public:
      SingletonFilter(const TiXmlElement& configData);
      bool matches(const ConstrainedVariableId& var, const LabelStr& objectType, const LabelStr& predicate) const;
    };
  }
}
#endif
