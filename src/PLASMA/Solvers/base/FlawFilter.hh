#ifndef H_FlawFilter
#define H_FlawFilter

/**
 * @author Conor McGann
 */

#include "MatchingRule.hh"

namespace EUROPA {
  namespace SOLVERS {

    class FlawFilter: public MatchingRule {
    public:
      FlawFilter(const TiXmlElement& configData, bool isDynamic = false);

      /**
       * @brief Tests if the filter is dynamically changing and may need to be re-evaluated.
       */
      bool isDynamic() const;

      virtual bool test(const EntityId& entity);

      ContextId getContext() const {return m_context;}
      void setContext(ContextId& ctx) {m_context = ctx;}

    private:
      const bool m_isDynamic;
      ContextId m_context;
    };

  }
}

#define REGISTER_FLAW_FILTER(CLASS, NAME) REGISTER_COMPONENT_FACTORY(CLASS, NAME)

#endif
