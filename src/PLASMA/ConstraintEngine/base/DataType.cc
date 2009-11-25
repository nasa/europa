/*
 * DataType.cc
 *
 *  Created on: Jan 28, 2009
 *      Author: javier
 */

#include "DataType.hh"
#include "Variable.hh"
#include "Debug.hh"
#include "Utils.hh"

namespace EUROPA {

DataType::DataType(const char* name)
    : m_id(this)
    , m_name(name)
    , m_isRestricted(false)
    , m_baseDomain(NULL)
    , m_minDelta(EPSILON)
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

// TODO: this can be more sophisticated
bool DataType::canBeCompared(const DataTypeId& rhs) const
{
    // Assumes type names don't matter for now

    if (isNumeric() || rhs->isNumeric())
      return (isNumeric() && rhs->isNumeric());

    if (isString() || rhs->isString())
      return (isString() && rhs->isString());

    if (isSymbolic() || rhs->isSymbolic())
      return (isSymbolic() && rhs->isSymbolic()); // TODO?: && baseDomain().intersects(rhs->baseDomain()));

    return false;
}

// TODO: make this more sophisticated, may have to delegate to each type
bool DataType::isAssignableFrom(const DataTypeId& rhs) const
{

    if (isNumeric()) {
        // Can only assign from types with less resolution,
        // for instance  "float <- int" is ok, but not the other way around
        return rhs->isNumeric() && (minDelta() <= rhs->minDelta());
    }

    // TODO?: check baseDomain().intersects(rhs->baseDomain()));

    return canBeCompared(rhs);
}


const AbstractDomain & DataType::baseDomain() const
{
    check_error(m_baseDomain != NULL);
    return *m_baseDomain;
}

double DataType::minDelta() const
{
    return m_minDelta;
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

    debugMsg("DataType::createVariable", "Created Variable : " << id->toLongString());

    return id;
}


std::string DataType::toString(double value) const
{
    if(isNumeric())
        return EUROPA::toString(value);
    else {
        checkError(LabelStr::isString(value), "expected LabelStr");
        return LabelStr(value).toString();
    }
}

}
