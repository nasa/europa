#ifndef _H_StringTypeFactory
#define _H_StringTypeFactory

#include "TypeFactory.hh"

namespace Prototype {

  class StringTypeFactory : public ConcreteTypeFactory {
  public:
    StringTypeFactory();

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

#endif // _H_StringTypeFactory
