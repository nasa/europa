#include "IntervalTypeFactory.hh"
#include "IntervalDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // IntervalTypeFactory
  //

  IntervalTypeFactory::IntervalTypeFactory(const LabelStr& name)
   : ConcreteTypeFactory(name), m_baseDomain(name)
  {
  }

  ConstrainedVariableId
  IntervalTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                      const AbstractDomain& baseDomain,
                                      bool canBeSpecified,
                                      const LabelStr& name,
                                      const EntityId& parent,
                                      int index) const
  {
    const IntervalDomain * intervalDomain = dynamic_cast<const IntervalDomain*>(&baseDomain);
    check_error(intervalDomain != NULL, "tried to create an IntervalDomain variable with a different kind of base domain");
    Variable<IntervalDomain> * variable
      = new Variable<IntervalDomain>(constraintEngine, *intervalDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for IntervalDomain with name '" + name.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  const AbstractDomain &
  IntervalTypeFactory::baseDomain() const
  {
    return m_baseDomain;
  }

  double IntervalTypeFactory::createValue(std::string value) const
  {
    return atof(value.c_str());
  }

} // namespace Prototype
