#include "IntervalIntTypeFactory.hh"
#include "IntervalIntDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // IntervalIntTypeFactory
  //

  IntervalIntTypeFactory::IntervalIntTypeFactory(const LabelStr& name)
   : ConcreteTypeFactory(name), m_baseDomain(DomainListenerId::noId(), name)
  {
  }

  IntervalIntTypeFactory::IntervalIntTypeFactory(const LabelStr& name, const IntervalIntDomain& baseDomain)
   : ConcreteTypeFactory(name), m_baseDomain(baseDomain)
  {
  }

  ConstrainedVariableId
  IntervalIntTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                         const AbstractDomain& baseDomain,
                                         bool canBeSpecified,
                                         const LabelStr& name,
                                         const EntityId& parent,
                                         int index) const
  {
    const IntervalIntDomain * intervalIntDomain = dynamic_cast<const IntervalIntDomain*>(&baseDomain);
    check_error(intervalIntDomain != NULL, "tried to create an IntervalIntDomain variable with a different kind of base domain");
    Variable<IntervalIntDomain> * variable
      = new Variable<IntervalIntDomain>(constraintEngine, *intervalIntDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for IntervalIntDomain with name '" + name.toString() + "'");
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

} // namespace Prototype
