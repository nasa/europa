#ifndef _H_EnumeratedTypeFactory
#define _H_EnumeratedTypeFactory

#include "TypeFactory.hh"
#include "EnumeratedDomain.hh"

namespace EUROPA {

  class EnumeratedTypeFactory : public ConcreteTypeFactory {
  public:
    EnumeratedTypeFactory(const char* typeName, const char* elementName);
    EnumeratedTypeFactory(const char* typeName, const char* elementName, const EnumeratedDomain& baseDomain);

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const AbstractDomain& baseDomain,
                                                 bool canBeSpecified = true,
                                                 const char* name = NO_VAR_NAME,
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

} // namespace EUROPA

#endif // _H_EnumeratedTypeFactory
