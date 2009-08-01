/*
 * Methods.cc
 *
 *  Created on: Jul 31, 2009
 *      Author: javier
 */

#include "Methods.hh"
#include "DataTypes.hh"

namespace EUROPA {

PDBClose::PDBClose()
    : Method("close")
{
}

PDBClose::~PDBClose()
{
}

DataRef PDBClose::eval(const std::vector<ConstrainedVariableId>& args) const
{
    // TODO: implement this
    return DataRef::null;
}

const std::vector<DataTypeId>& PDBClose::getSignature()
{
    static std::vector<DataTypeId> signature;

    return signature;
}

const DataTypeId& PDBClose::getReturnType()
{
    return VoidDT::instance();
}

}
