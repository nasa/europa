#include "StringTypeFactory.hh"
#include "StringDomain.hh"
#include "Variable.hh"

namespace EUROPA {
  
  //
  // StringTypeFactory
  //

  StringTypeFactory::StringTypeFactory(const char* name)
   : TypeFactory(name), m_baseDomain(name) {}

  ConstrainedVariableId
  StringTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                    const AbstractDomain& baseDomain,
                                    const bool internal,
                                    bool canBeSpecified,
                                    const char* name,
                                    const EntityId& parent,
                                    int index) const
  {
    Variable<StringDomain> * variable
      = new Variable<StringDomain>(constraintEngine, baseDomain, internal, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for StringDomain with name '" + std::string(name) + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  const AbstractDomain &
  StringTypeFactory::baseDomain() const
  {
    return m_baseDomain;
  }

  double StringTypeFactory::createValue(const std::string& value) const
  {
    return LabelStr(value);
  }

} // namespace EUROPA
