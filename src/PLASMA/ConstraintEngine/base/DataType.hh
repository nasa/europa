/*
 * DataType.hh
 *
 *  Created on: Jan 28, 2009
 *      Author: javier
 */

#ifndef DATATYPE_HH_
#define DATATYPE_HH_

#include "ConstraintEngineDefs.hh"
#include "LabelStr.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {

class DataType {

public:
    DataType(const char* name);

    virtual ~DataType();

    const DataTypeId& getId() const;

    /**
     * @brief Get the data type's name.
     */
    virtual const LabelStr& getName() const;

    /**
     * @brief Set the data ype's name.
     */
    virtual void setName(const LabelStr& name);

    /**
     * @brief Check if the domain is numeric.
     */
    virtual bool isNumeric() const = 0;

    /**
     * @brief Check if the domain is Boolean.
     */
    virtual bool isBool() const = 0;

    /**
     * @brief Check if the domain is String.
     */
    virtual bool isString() const = 0;

    /**
      * @brief Check if the domain contains entities
      */
    virtual bool isEntity() const;

    /**
     * @brief Check if the domain is symbolic.
     */
    virtual bool isSymbolic() const;

    /**
     * @brief Tests if two domains can be compared. For example, one cannot compare a symbolic
     * enumerated domain with a numeric domain. This is useful to enforce type checking
     * in constraints in particular.
     */
    virtual bool canBeCompared(const DataType& rhs);

    /**
     * is the original definition for this domain restricted?, for instance : int [3 10], float {3.0,4.0}, etc
     */
    virtual bool getIsRestricted() const;

    /**
     * is the original definition for this domain restricted?, for instance : int [3 10], float {3.0,4.0}, etc
     */
    virtual void setIsRestricted(bool b);

    /**
     * @brief Return the base domain
     */
    virtual const AbstractDomain& baseDomain() const;

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(const std::string& value) const = 0;

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine,
                                                 const AbstractDomain& baseDomain,
                                                 const bool internal = false,
                                                 bool canBeSpecified = true,
                                                 const char* name = NO_VAR_NAME,
                                                 const EntityId& parent = EntityId::noId(),
                                                 int index = ConstrainedVariable::NO_INDEX) const;

protected:
    DataTypeId m_id;
    LabelStr m_name;
    bool m_isRestricted;
    AbstractDomain* m_baseDomain;
};

}


#endif /* DATATYPE_HH_ */
