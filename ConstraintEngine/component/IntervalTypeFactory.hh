#ifndef _H_IntervalTypeFactory
#define _H_IntervalTypeFactory

#include "TypeFactory.hh"

namespace Prototype {

  class IntervalTypeFactory : public ConcreteTypeFactory {
  public:
    IntervalTypeFactory();

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const AbstractDomain& baseDomain,
                                                 bool canBeSpecified = true,
                                                 const LabelStr& name = ConstrainedVariable::NO_NAME(),
                                                 const EntityId& parent = EntityId::noId(),
                                                 int index = ConstrainedVariable::NO_INDEX) const;

    /**
     * @brief Create a domain
     */
    virtual AbstractDomain * createDomain() const;

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(std::string value) const;

  };

} // namespace Prototype

#endif // _H_IntervalTypeFactory
