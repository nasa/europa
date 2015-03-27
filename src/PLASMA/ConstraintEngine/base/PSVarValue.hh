#ifndef H_PSVarValue
#define H_PSVarValue

#include "PSConstraintEngine.hh"
#include "LabelStr.hh"

namespace EUROPA {
class PSEntity;

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

    static PSVarValue getInstance(const std::string& val) {
      return PSVarValue(cast_double(static_cast<edouble>(LabelStr(val))), STRING);
    }
    static PSVarValue getInstance(int val) {
      return PSVarValue(static_cast<double>(val), INTEGER);
    }
    static PSVarValue getInstance(double val) {return PSVarValue(val, DOUBLE);}
    static PSVarValue getInstance(edouble val) {return PSVarValue(val, DOUBLE);}
    static PSVarValue getInstance(bool val) {
      return PSVarValue(static_cast<double>(val), BOOLEAN);
    }
    static PSVarValue getObjectInstance(double obj) {return PSVarValue(obj, OBJECT);} 

  private:
    double m_val;
    PSVarType m_type;
  };                
}

#endif

