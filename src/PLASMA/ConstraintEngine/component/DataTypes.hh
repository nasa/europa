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

    static const std::string& NAME();
    static VoidDT& instance();
};

class FloatDT : public DataType
{
public:
    FloatDT();
    virtual ~FloatDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    static const std::string& NAME();
    static FloatDT& instance();
};

class IntDT : public DataType
{
public:
    IntDT();
    virtual ~IntDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    static const std::string& NAME();
    static IntDT& instance();
};

class BoolDT : public DataType
{
public:
    BoolDT();
    virtual ~BoolDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    static const std::string& NAME();
    static BoolDT& instance();
};

class StringDT : public DataType
{
public:
    StringDT();
    virtual ~StringDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    static const std::string& NAME();
    static StringDT& instance();
};

// TODO: why have strings and symbols?
class SymbolDT : public DataType
{
public:
    SymbolDT();
    virtual ~SymbolDT();

    virtual bool isNumeric() const;
    virtual bool isBool() const;
    virtual bool isString() const;

    static const std::string& NAME();
    static SymbolDT& instance();
};

}

#endif /* DATATYPES_HH_ */
