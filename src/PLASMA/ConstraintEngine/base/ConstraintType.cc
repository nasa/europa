/*
 * ConstraintType.cc
 *
 *  Created on: Apr 24, 2009
 *      Author: javier
 */

#include "ConstraintType.hh"

namespace EUROPA {

ConstraintType::ConstraintType(const LabelStr& name,
               const LabelStr& propagatorName,
               bool systemDefined)
    : m_id(this)
    , m_name(name)
    , m_propagatorName(propagatorName)
    , m_systemDefined(systemDefined)
{
}

ConstraintType::~ConstraintType()
{
}

const ConstraintTypeId& ConstraintType::getId() const {return m_id;}

const LabelStr& ConstraintType::getName() const { return m_name; }

bool ConstraintType::isSystemDefined() const { return m_systemDefined;  }

}
