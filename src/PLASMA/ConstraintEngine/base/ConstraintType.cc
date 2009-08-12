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

ConstraintId ConstraintType::createConstraint(const ConstraintEngineId constraintEngine,
                  const std::vector<ConstrainedVariableId>& scope)
{
  check_error(ALWAYS_FAILS);
  return ConstraintId::noId();
}

const std::vector<DataTypeId>& ConstraintType::getArgTypes() const
{
    return m_argTypes;
}

void ConstraintType::checkArgTypes(const std::vector<DataTypeId>& types) const
{
    // TODO: constraints with variable number of args need a special notation to be supported
    // here, or need to sub class and override this method for now.
    if (types.size() != m_argTypes.size()) {
        std::ostringstream os;
        os << "Constraint "<< m_name.toString()
           << " can't take " << types.size() << " parameters."
           << " It expects " << m_argTypes.size() << ".";
        // TODO: enable this
        //throw os.str();
    }

    for (unsigned int i=0;i<m_argTypes.size();i++) {
        // TODO: need some convention or a data type to represent "any"
        if (!m_argTypes[i]->isAssignableFrom(types[i])) {
            std::ostringstream os;
            os << "Constraint "<< m_name.toString()
               << " can't take a " << types[i]->getName().toString()
               << " as parameter number " << i << "."
               << " It expects " << m_argTypes[i]->getName().toString() << ".";
            throw os.str();
        }
    }
}


}
