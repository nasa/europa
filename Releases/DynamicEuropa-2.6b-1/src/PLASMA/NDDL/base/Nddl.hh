#ifndef H_Nddl
#define H_Nddl

/**
 * @file   Nddl.hh
 * @author Andrew Bachmann
 * @date   Mon Dec 27 17:07:52 2004
 * @brief  
 * @ingroup NDDL
 */

#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"

namespace EUROPA {
  namespace NDDL {
    /**
     * @brief Allocator function to be implemented by code generated from the model.
     */
    extern "C" SchemaId loadSchema(const SchemaId& schema, const RuleSchemaId& ruleSchema);
  }
}

using namespace EUROPA;
#endif
