#include "BoolTypeFactory.hh"
#include "BoolDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // BoolTypeFactory
  //

  BoolTypeFactory::BoolTypeFactory(const LabelStr& name)
   : ConcreteTypeFactory(name), m_baseDomain(name)
  {
  }

  ConstrainedVariableId
  BoolTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                  const AbstractDomain& baseDomain,
                                  bool canBeSpecified,
                                  const LabelStr& name,
                                  const EntityId& parent,
                                  int index) const
  {
    const BoolDomain * boolDomain = dynamic_cast<const BoolDomain*>(&baseDomain);
    check_error(boolDomain != NULL, "tried to create a BoolDomain variable with a different kind of base domain");
    Variable<BoolDomain> * variable
      = new Variable<BoolDomain>(constraintEngine, *boolDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for BoolDomain with name '" + name.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  const AbstractDomain &
  BoolTypeFactory::baseDomain() const
  {
    return m_baseDomain;
  }

  double BoolTypeFactory::createValue(std::string value) const
  {
    if (value == "true") {
      return true;
    }
    if (value == "false") {
      return false;
    }
    check_error(ALWAYS_FAILS, "string value for boolean should be 'true' or 'false', not '" + value + "'");
    return -1;
  }

} // namespace Prototype
