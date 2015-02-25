/*
 * ConstraintType.cc
 *
 *  Created on: Apr 24, 2009
 *      Author: javier
 */

#include "ConstraintType.hh"

namespace EUROPA {

ConstraintType::ConstraintType(const std::string& name,
                               const std::string& propagatorName,
                               bool systemDefined)
    : m_id(this)
    , m_name(name)
    , m_propagatorName(propagatorName)
    , m_systemDefined(systemDefined)
{
}

ConstraintType::~ConstraintType()
{
  if(m_id.isValid())
    m_id.remove();
}

const ConstraintTypeId ConstraintType::getId() const {return m_id;}

const std::string& ConstraintType::getName() const { return m_name; }

bool ConstraintType::isSystemDefined() const { return m_systemDefined;  }

}
