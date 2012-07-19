#ifndef _H_ResourcePropagator
#define _H_ResourcePropagator

/**
 * @file ResourcePropagator.hh"
 * @author Conor McGann
 * @date 2005
 */

#include "DefaultPropagator.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabaseListener.hh"
#include "ResourceDefs.hh"
#include <set>

namespace EUROPA {    

  /**
   * @brief Provides a minor extension by overriding the execute method to process all constraints
   * globally by deferring to the resources.
   *
   * The ResourcePropagator uses the capabilities of the DefaultPropagator for all agenda management.
   */
  class ResourcePropagator: public DefaultPropagator
  {
  public:
    /**
     * @brief Standard constructor
     */
    ResourcePropagator(const LabelStr& name, 
		       const ConstraintEngineId& constraintEngine, 
		       const PlanDatabaseId& planDatabase);

  private:

    /**
     * @brief Provides the propagation behaviour to delegate to relevant resources after processing
     * constraints in the agenda.
     */
    void execute();

    /**
     * @brief Check the superclass and also check the resources themselves.
     */
    bool updateRequired() const;

    static const LabelStr& resourceString();

    /**
     * @brief Helper method to obtain the list of dirty resources in the database
     */
    const std::list<ResourceId> getDirtyResources() const;

    const PlanDatabaseId m_planDb;

    const bool m_isEnabled;
  };
}
#endif
