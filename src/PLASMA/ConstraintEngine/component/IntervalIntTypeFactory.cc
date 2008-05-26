#include "IntervalIntTypeFactory.hh"
#include "IntervalIntDomain.hh"
#include "Variable.hh"

namespace EUROPA {
  
  //
  // IntervalIntTypeFactory
  //

  IntervalIntTypeFactory::IntervalIntTypeFactory(const char* name)
   : TypeFactory(name), m_baseDomain(name)
  {
  }

  IntervalIntTypeFactory::IntervalIntTypeFactory(const char* name, const IntervalIntDomain& baseDomain)
   : TypeFactory(name), m_baseDomain(baseDomain)
  {
  }

  ConstrainedVariableId
  IntervalIntTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                         const AbstractDomain& baseDomain,
                                         bool canBeSpecified,
                                         const char* name,
                                         const EntityId& parent,
                                         int index) const
  {
    const IntervalIntDomain * intervalIntDomain = dynamic_cast<const IntervalIntDomain*>(&baseDomain);
    check_error(intervalIntDomain != NULL, "tried to create an IntervalIntDomain variable with a different kind of base domain");
    Variable<IntervalIntDomain> * variable
      = new Variable<IntervalIntDomain>(constraintEngine, *intervalIntDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for IntervalIntDomain with name '" + std::string(name) + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  const AbstractDomain &
  IntervalIntTypeFactory::baseDomain() const
  {
    return m_baseDomain;
  }

  double IntervalIntTypeFactory::createValue(std::string value) const
  {
    return atoi(value.c_str());
  }

} // namespace EUROPA
