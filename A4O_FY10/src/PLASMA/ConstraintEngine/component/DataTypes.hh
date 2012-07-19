/*
 * DataTypes.hh
 *
 *  Created on: Mar 31, 2009
 *      Author: javier
 */

#ifndef DATATYPES_HH_
#define DATATYPES_HH_

#include "DataType.hh"

namespace EUROPA {

class VoidDT : public DataType
{
public:
    VoidDT();
    virtual ~VoidDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    virtual edouble createValue(const std::string& value) const;
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine,
                                                 const Domain& baseDomain,
                                                 const bool internal = false,
                                                 bool canBeSpecified = true,
                                                 const char* name = NO_VAR_NAME,
                                                 const EntityId& parent = EntityId::noId(),
                                                 int index = ConstrainedVariable::NO_INDEX) const;

    static const std::string& NAME();
    static const DataTypeId& instance();
};

class FloatDT : public DataType
{
public:
    FloatDT();
    virtual ~FloatDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    virtual edouble createValue(const std::string& value) const;

    static const std::string& NAME();
    static const DataTypeId& instance();
};

class IntDT : public DataType
{
public:
    IntDT();
    virtual ~IntDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    virtual edouble createValue(const std::string& value) const;

    static const std::string& NAME();
    static const DataTypeId& instance();
};

class BoolDT : public DataType
{
public:
    BoolDT();
    virtual ~BoolDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    virtual edouble createValue(const std::string& value) const;
    virtual std::string toString(edouble value) const;

    static const std::string& NAME();
    static const DataTypeId& instance();
};

class StringDT : public DataType
{
public:
    StringDT();
    virtual ~StringDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    virtual edouble createValue(const std::string& value) const;

    static const std::string& NAME();
    static const DataTypeId& instance();
};

class SymbolDT : public DataType
{
public:
    SymbolDT();
    virtual ~SymbolDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    virtual edouble createValue(const std::string& value) const;

    static const std::string& NAME();
    static const DataTypeId& instance();
};

class RestrictedDT : public DataType
{
public:
    RestrictedDT(const char* name, const DataTypeId& baseType, const Domain& baseDomain);
    virtual ~RestrictedDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;
    virtual bool isEntity() const;
    virtual edouble minDelta() const;

    virtual edouble createValue(const std::string& value) const;

protected:
    DataTypeId m_baseType;
};

}

#endif /* DATATYPES_HH_ */
