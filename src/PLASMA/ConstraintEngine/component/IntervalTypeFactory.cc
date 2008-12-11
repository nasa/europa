#include "IntervalTypeFactory.hh"
#include "IntervalDomain.hh"
#include "Variable.hh"

namespace EUROPA {
  
  //
  // IntervalTypeFactory
  //

  IntervalTypeFactory::IntervalTypeFactory(const std::string& name)
   : TypeFactory(name), m_baseDomain(name)
  {
  }

  IntervalTypeFactory::IntervalTypeFactory(const std::string& name, const IntervalDomain& baseDomain)
   : TypeFactory(name), m_baseDomain(baseDomain)
  {
  }

  ConstrainedVariableId
  IntervalTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                      const AbstractDomain& baseDomain,
                                      const bool internal,
                                      bool canBeSpecified,
                                      const std::string& name,
                                      const EntityId& parent,
                                      int index) const
  {
    const IntervalDomain * intervalDomain = dynamic_cast<const IntervalDomain*>(&baseDomain);
    check_error(intervalDomain != NULL, "tried to create an IntervalDomain variable with a different kind of base domain");
    Variable<IntervalDomain> * variable
      = new Variable<IntervalDomain>(constraintEngine, *intervalDomain, internal, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for IntervalDomain with name '" + std::string(name) + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  const AbstractDomain &
  IntervalTypeFactory::baseDomain() const
  {
    return m_baseDomain;
  }

  edouble IntervalTypeFactory::createValue(const std::string& value) const
  {
    return atof(value.c_str());
  }

} // namespace EUROPA
