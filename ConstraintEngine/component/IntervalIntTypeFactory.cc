#include "IntervalIntTypeFactory.hh"
#include "IntervalIntDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // IntervalIntTypeFactory
  //

  IntervalIntTypeFactory::IntervalIntTypeFactory() : ConcreteTypeFactory(IntervalIntDomain().getTypeName()) {}

  ConstrainedVariableId
  IntervalIntTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                         const AbstractDomain& baseDomain,
                                         bool canBeSpecified = true,
                                         const LabelStr& name = ConstrainedVariable::NO_NAME(),
                                         const EntityId& parent = EntityId::noId(),
                                         int index = ConstrainedVariable::NO_INDEX) const
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

  AbstractDomain *
  IntervalIntTypeFactory::createDomain() const
  {
    IntervalIntDomain * domain = new IntervalIntDomain();
    check_error(domain != NULL, "failed to create IntervalIntDomain");
    return domain;
  }

  double IntervalIntTypeFactory::createValue(std::string value) const
  {
    return atoi(value.c_str());
  }

} // namespace Prototype
