#ifndef _H_PSVarValue
#define _H_PSVarValue

#include "PSConstraintEngine.hh"
#include "PSUtils.hh"
#include "LabelStr.hh"

namespace EUROPA {
  class PSVarValue
  {
  public:
    PSVarValue(const double val, const PSVarType type);
    PSVarValue(const edouble val, const PSVarType type);
    PSVarType getType() const;
    
    PSEntity*           asObject() const;
    int                 asInt() const;
    double              asDouble() const;
    bool                asBoolean() const;
    const std::string&  asString() const; 

    std::string toString() const;

    static PSVarValue getInstance(const std::string& val) {return PSVarValue(cast_double((edouble)LabelStr(val)), STRING);}
    static PSVarValue getInstance(int val) {return PSVarValue((double)val, INTEGER);}
    static PSVarValue getInstance(double val) {return PSVarValue(val, DOUBLE);}
    static PSVarValue getInstance(edouble val) {return PSVarValue(val, DOUBLE);}
    static PSVarValue getInstance(bool val) {return PSVarValue((double)val, BOOLEAN);}
    static PSVarValue getObjectInstance(double obj) {return PSVarValue(obj, OBJECT);} // cast an EntityId to double to call this

  private:
    double m_val;
    PSVarType m_type;
  };                
}

#endif

