/*
 * DataType.cc
 *
 *  Created on: Jan 28, 2009
 *      Author: javier
 */

#include "DataType.hh"

namespace EUROPA {

DataType::DataType(const char* name)
    : m_name(name)
    , m_isRestricted(false)
{
}

DataType::~DataType()
{
}

const LabelStr& DataType::getName() const { return m_name; }
void DataType::setName(const LabelStr& name) { m_name = name; }

bool DataType::isEntity() const {return false;}

bool DataType::isSymbolic() const {return !isNumeric();}

bool DataType::getIsRestricted() const { return m_isRestricted; }
void DataType::setIsRestricted(bool b) { m_isRestricted = b; }

bool DataType::canBeCompared(const DataType& rhs)
{
    // The following numeric test now replaces this test.
    //if(isBool())
    //   return(rhs.isBool());

    // If either is numeric, both must be numeric.
    if(isNumeric())
        return(rhs.isNumeric());

    // If either is string, both must be string.
    if(isString())
        return(rhs.isString());

    // Now we know that neither is numeric. We also can infer that
    // they must be enumerated since neither is numeric. Thus, the
    // types must match
    return (getName() == rhs.getName());
}

}
