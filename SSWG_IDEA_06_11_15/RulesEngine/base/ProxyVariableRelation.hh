#ifndef H_ProxyVariableRelation
#define H_ProxyVariableRelation

#include "Constraint.hh"
#include "Object.hh"
#include "ConstrainedVariable.hh"

/**
 * @author Conor McGann
 */

namespace EUROPA {

  class ProxyVariableRelation : public Constraint {
  public:
    /**
     * @brief Constructor used for initial allocation
     */
    ProxyVariableRelation(const ConstrainedVariableId& objectVar,
			  const ConstrainedVariableId& proxyVar,
			  const std::vector<unsigned int>& path);

    /**
     * @brief Standard constructor to support copying
     */
    ProxyVariableRelation(const LabelStr& name,
			  const LabelStr& propagatorName,
			  const ConstraintEngineId& constraintEngine,
			  const std::vector<ConstrainedVariableId>& variables);

  private:

    void handleExecute();

    /**
     * @brief Used to only track set and reset events. Will not be subject to propagation
     */
    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex, 
		   const DomainListener::ChangeType& changeType);

    /**
     * @brief Required to support copying
     */
    void setSource(const ConstraintId& sourceConstraint);

    ObjectDomain& m_objectDomain; /*!< The set of objects that we source from */
    EnumeratedDomain& m_proxyDomain; /*!< The set of field values stored in the proxy variable */
    std::vector<unsigned int> m_path; /*!< The index based path from the root object to the field value */
    bool m_autoSpecified; /*!< Tracks if we have propagated a specification from the source object 
			    variable to the proxy */
    static const unsigned int ARG_COUNT = 2;
  };
}

#endif
