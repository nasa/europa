#ifndef _H_StringTypeFactory
#define _H_StringTypeFactory

#include "TypeFactory.hh"
#include "StringDomain.hh"

namespace Prototype {

  class StringTypeFactory : public ConcreteTypeFactory {
  public:
    StringTypeFactory(const LabelStr& name = StringDomain::getDefaultTypeName());

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
    StringDomain m_baseDomain;
  };

} // namespace Prototype

#endif // _H_StringTypeFactory
