/*
 * DataTypes.cc
 *
 *  Created on: Mar 31, 2009
 *      Author: javier
 */

#include "DataTypes.hh"

#include "Domains.hh"
#include "Variable.hh"

#define DT_STATIC_MEMBERS(dataType,dtName) \
const std::string& dataType::NAME() { static std::string sl_name(#dtName); return sl_name; } \
const DataTypeId& dataType::instance() { static dataType sl_instance; return sl_instance.getId(); }


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

double VoidDT::createValue(const std::string& value) const
{
  check_error(ALWAYS_FAILS, "can't create void value");
  return -1;
}

ConstrainedVariableId
VoidDT::createVariable(const ConstraintEngineId& constraintEngine,
                                const AbstractDomain& baseDomain,
                                const bool internal,
                                bool canBeSpecified,
                                const char* name,
                                const EntityId& parent,
                                int index) const
{
    check_error(ALWAYS_FAILS, "can't create void variable");
    return ConstrainedVariableId::noId();
}

FloatDT::FloatDT()
    : DataType(NAME().c_str())
{
    m_baseDomain = new IntervalDomain(getId());
}

FloatDT::~FloatDT()
{
}

bool FloatDT::isNumeric() const { return true; }
bool FloatDT::isBool() const  { return false; }
bool FloatDT::isString() const  { return false; }

double FloatDT::createValue(const std::string& value) const
{
  // TODO: simplify this
  if (value == "-inf" || value=="-inff") {
    return MINUS_INFINITY;
  }
  if (value == "inf" || value=="+inf" || value=="inff" || value=="+inff") {
    return PLUS_INFINITY;
  }
  return atof(value.c_str());
}

IntDT::IntDT()
    : DataType(NAME().c_str())
{
    m_baseDomain = new IntervalIntDomain(getId());
}

IntDT::~IntDT()
{
}

bool IntDT::isNumeric() const { return true; }
bool IntDT::isBool() const  { return false; }
bool IntDT::isString() const  { return false; }

double IntDT::createValue(const std::string& value) const
{
  if (value == "-inf") {
    return MINUS_INFINITY;
  }
  if (value == "+inf" || value == "inf") {
    return PLUS_INFINITY;
  }
  return atoi(value.c_str());
}

BoolDT::BoolDT()
    : DataType(NAME().c_str())
{
    m_baseDomain = new BoolDomain(getId());
}

BoolDT::~BoolDT()
{
}

/*
 * To maintain compatibility with the original definition of the Bool domain in EUROPA :
 * BoolDT represents the integer interval [0,1]
 */
bool BoolDT::isNumeric() const { return true; }
bool BoolDT::isBool() const  { return true; }
bool BoolDT::isString() const  { return false; }

double BoolDT::createValue(const std::string& value) const
{
  if (value == "true") {
    return true;
  }
  if (value == "false") {
    return false;
  }
  check_error(ALWAYS_FAILS, "string value for boolean should be 'true' or 'false', not '" + value + "'");
  return -1;
}

StringDT::StringDT()
    : DataType(NAME().c_str())
{
    m_baseDomain = new StringDomain(getId());
}

StringDT::~StringDT()
{
}

bool StringDT::isNumeric() const { return false; }
bool StringDT::isBool() const  { return false; }
bool StringDT::isString() const  { return true; }

double StringDT::createValue(const std::string& value) const
{
  return LabelStr(value);
}

SymbolDT::SymbolDT()
    : DataType(NAME().c_str())
{
    m_baseDomain = new SymbolDomain(getId());
}

SymbolDT::~SymbolDT()
{
}

bool SymbolDT::isNumeric() const { return false; }
bool SymbolDT::isBool() const  { return false; }
bool SymbolDT::isString() const  { return false; }

double SymbolDT::createValue(const std::string& value) const
{
  return LabelStr(value);
}

RestrictedDT::RestrictedDT(const char* name, const DataTypeId& baseType, const AbstractDomain& baseDomain)
    : DataType(name)
    , m_baseType(baseType)
{
    m_baseDomain = baseDomain.copy();
    m_baseDomain->setDataType(getId());
    setIsRestricted(true);
}

RestrictedDT::~RestrictedDT()
{
}

bool RestrictedDT::isNumeric() const { return m_baseType->isNumeric(); }
bool RestrictedDT::isBool() const  { return m_baseType->isBool(); }
bool RestrictedDT::isString() const  { return m_baseType->isString(); }
bool RestrictedDT::isEntity() const  { return m_baseType->isEntity(); }

double RestrictedDT::createValue(const std::string& value) const
{
    if (isNumeric())
        return(atof(value.c_str()));

    return(LabelStr(value));
}


}
