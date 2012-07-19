#ifndef _H_ResourceConstraint
#define _H_ResourceConstraint

/**
 * @file ResourceConstraint.hh
 * @author Conor McGann
 * @date 2005
 * @brief Declares the class for propagating changes on transaction variables to updates in the resource.
 */

#include "Constraint.hh"
#include "Variable.hh"

namespace EUROPA {

  /**
   * @brief Responsible for synchronizing updates of variables with the resources in question.
   */
  class ResourceConstraint: public Constraint
  {
  public:
    ResourceConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

  private:
    void handleExecute();

    bool m_assigned; /*!< True if we have notified the Resource the transaction is assaigned,
		       otherwise false. */

    /**
     * @brief Accessor required for ResourcePropagator so that it can
     * set a domain to empty when there are resource violations.
     */
    static AbstractDomain& getCurrentDomain(const ConstrainedVariableId& var);
    friend class ResourcePropagator;

    static const int OBJECT = 0; /*!< Array position of Resource Variable. */
    static const int TIME = 1; /*!< Array position of Time Variable. */
    static const int USAGE = 2; /*!< Array position of Usage/Quantity Variable. */
    static const int ARG_COUNT = 3;
  };

} //namespace EUROPA

#endif
