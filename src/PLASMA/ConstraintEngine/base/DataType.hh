/*
 * DataType.hh
 *
 *  Created on: Jan 28, 2009
 *      Author: javier
 */

#ifndef DATATYPE_HH_
#define DATATYPE_HH_

#include "LabelStr.hh"

namespace EUROPA {

class DataType {

public:
    DataType(const char* name);

    virtual ~DataType();

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

protected:
    LabelStr m_name;
    bool m_isRestricted;
};

}


#endif /* DATATYPE_HH_ */
