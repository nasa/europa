#ifndef _H_EnumeratedTypeFactory
#define _H_EnumeratedTypeFactory

#include "TypeFactory.hh"
#include "EnumeratedDomain.hh"

namespace Prototype {

  class EnumeratedTypeFactory : public ConcreteTypeFactory {
  public:
    EnumeratedTypeFactory(const LabelStr& typeName, const LabelStr& elementName);
    EnumeratedTypeFactory(const LabelStr& typeName, const LabelStr& elementName, const EnumeratedDomain& baseDomain);

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
     * @brief Return the base domain
     */
    virtual const AbstractDomain & baseDomain() const;

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(std::string value) const;

  private:
    const LabelStr m_elementName;
    const EnumeratedDomain m_baseDomain;
  };

} // namespace Prototype

#endif // _H_EnumeratedTypeFactory
