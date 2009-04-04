/*
 * DataType.cc
 *
 *  Created on: Jan 28, 2009
 *      Author: javier
 */

#include "DataType.hh"
#include "Variable.hh"

namespace EUROPA {

DataType::DataType(const char* name)
    : m_id(this)
    , m_name(name)
    , m_isRestricted(false)
    , m_baseDomain(NULL)
{
}

DataType::~DataType()
{
    m_id.remove();

    if (m_baseDomain != NULL)
        delete m_baseDomain;
}

const DataTypeId& DataType::getId() const { return m_id; }

const LabelStr& DataType::getName() const { return m_name; }
void DataType::setName(const LabelStr& name) { m_name = name; }

bool DataType::isEntity() const {return false;}

bool DataType::isSymbolic() const {return !isNumeric();}

bool DataType::getIsRestricted() const { return m_isRestricted; }
void DataType::setIsRestricted(bool b) { m_isRestricted = b; }

bool DataType::canBeCompared(const DataType& rhs)
{
    // If either is numeric, both must be numeric.
    if(isNumeric())
        return(rhs.isNumeric());

    // If either is string, both must be string.
    if(isString())
        return(rhs.isString());

    // Now we know that neither is numeric. We also can infer that
    // they must be enumerated since neither is numeric. Thus, the
    // types must match
    // TODO: this can be more sophisticated
    return (getName() == rhs.getName());
}

const AbstractDomain & DataType::baseDomain() const
{
    check_error(m_baseDomain != NULL);
    return *m_baseDomain;
}


ConstrainedVariableId
DataType::createVariable(const ConstraintEngineId& constraintEngine,
                                  const AbstractDomain& baseDomain,
                                  const bool internal,
                                  bool canBeSpecified,
                                  const char* name,
                                  const EntityId& parent,
                                  int index) const
{
    // TODO: perform stronger checks here
    check_error(AbstractDomain::canBeCompared(*m_baseDomain,baseDomain),
            std::string("Tried to create a ") + getName().c_str()
            + " variable with a different kind of base domain:"
            + baseDomain.getTypeName().c_str());

    Variable<AbstractDomain>* variable = new Variable<AbstractDomain>(
            constraintEngine,
            baseDomain,
            internal,
            canBeSpecified,
            name,
            parent,
            index
    );

    check_error(variable != NULL,
            std::string("Failed to create Variable of type ")+ getName().c_str() +" with name '" + std::string(name) + "'");

    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());

    return id;
}

}
