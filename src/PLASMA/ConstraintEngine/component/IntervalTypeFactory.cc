#include "IntervalTypeFactory.hh"
#include "IntervalDomain.hh"
#include "Variable.hh"

namespace EUROPA {

  //
  // IntervalTypeFactory
  //

  IntervalTypeFactory::IntervalTypeFactory(const char* name)
   : TypeFactory(name), m_baseDomain(name)
  {
  }

  IntervalTypeFactory::IntervalTypeFactory(const char* name, const IntervalDomain& baseDomain)
   : TypeFactory(name), m_baseDomain(baseDomain)
  {
  }

  ConstrainedVariableId
  IntervalTypeFactory::createVariable(const ConstraintEngineId& constraintEngine,
                                      const AbstractDomain& baseDomain,
                                      const bool internal,
                                      bool canBeSpecified,
                                      const char* name,
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

  double IntervalTypeFactory::createValue(const std::string& value) const
  {
    return atof(value.c_str());
  }

  //
  // floatTypeFactory
  //

  floatTypeFactory::floatTypeFactory()
    : IntervalTypeFactory(getDefaultTypeName().c_str(), getDefaultTypeName().c_str()){}

  double floatTypeFactory::createValue(const std::string& value) const
  {
    if (value == "-inf") {
      return MINUS_INFINITY;
    }
    if (value == "+inf") {
      return PLUS_INFINITY;
    }
    return atof(value.c_str());
  }

  const LabelStr& floatTypeFactory::getDefaultTypeName() {
    static const LabelStr sl_typeName("float");
    return(sl_typeName);
  }

} // namespace EUROPA
