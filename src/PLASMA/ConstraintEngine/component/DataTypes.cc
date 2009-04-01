/*
 * DataTypes.cc
 *
 *  Created on: Mar 31, 2009
 *      Author: javier
 */

#include "DataTypes.hh"

#define DT_STATIC_MEMBERS(dataType,dtName) \
const std::string& dataType::NAME() { static std::string sl_name(#dtName); return sl_name; } \
dataType& dataType::instance() { static dataType sl_instance; return sl_instance; }


namespace EUROPA
{

DT_STATIC_MEMBERS(VoidDT,void)
DT_STATIC_MEMBERS(FloatDT,float)
DT_STATIC_MEMBERS(IntDT,int)
DT_STATIC_MEMBERS(BoolDT,bool)
DT_STATIC_MEMBERS(StringDT,string)
DT_STATIC_MEMBERS(SymbolDT,symbol)


VoidDT::VoidDT()
    : DataType(NAME().c_str())
{
}

VoidDT::~VoidDT()
{
}

bool VoidDT::isNumeric() const { return false; }
bool VoidDT::isBool() const  { return false; }
bool VoidDT::isString() const  { return false; }


FloatDT::FloatDT()
    : DataType(NAME().c_str())
{
}

FloatDT::~FloatDT()
{
}

bool FloatDT::isNumeric() const { return true; }
bool FloatDT::isBool() const  { return false; }
bool FloatDT::isString() const  { return false; }

IntDT::IntDT()
    : DataType(NAME().c_str())
{
}

IntDT::~IntDT()
{
}

bool IntDT::isNumeric() const { return true; }
bool IntDT::isBool() const  { return false; }
bool IntDT::isString() const  { return false; }

BoolDT::BoolDT()
    : DataType(NAME().c_str())
{
}

BoolDT::~BoolDT()
{
}

/*
 * To maintain compatibiloity with the original definition of the Bool domain in EUROPA :
 * BoolDT represents the integer interval [0,1]
 */
bool BoolDT::isNumeric() const { return true; }
bool BoolDT::isBool() const  { return true; }
bool BoolDT::isString() const  { return false; }

StringDT::StringDT()
    : DataType(NAME().c_str())
{
}

StringDT::~StringDT()
{
}

bool StringDT::isNumeric() const { return false; }
bool StringDT::isBool() const  { return false; }
bool StringDT::isString() const  { return true; }

SymbolDT::SymbolDT()
    : DataType(NAME().c_str())
{
}

SymbolDT::~SymbolDT()
{
}

bool SymbolDT::isNumeric() const { return false; }
bool SymbolDT::isBool() const  { return false; }
bool SymbolDT::isString() const  { return true; }

}
