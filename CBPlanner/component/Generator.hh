/**
 * @file   Generator.hh
 * @author Tania Bedrax-Weiss
 * @date   Thu Jul  8 17:39:41 2004
 * @verbinclude 
 * 
 * @brief  
 * 
 * 
 */

#include "Entity.hh"
#include "LabelStr.hh"

namespace Prototype {

  class Generator;
  typedef Id<Generator> GeneratorId;

  class Generator : public Entity {
  public:
    Generator(const LabelStr& name);
    virtual ~Generator();
    const GeneratorId& getId() const;
    const LabelStr& getName() const;
  protected:
    virtual void getAllValues(std::list<double>& values);
    virtual void getRemainingValues(std::list<double>& values);
    virtual void getNextValue(double next, const double last);
  private:
    GeneratorId m_id;
  };

#define REGISTER_VARIABLE_GENERATOR(GeneratorType, GeneratorName) \
  (HSTSHeuristics::addVariableGenerator((new GeneratorType(LabelStr(GeneratorName)))->getId()))

#define REGISTER_SUCC_TOKEN_GENERATOR(GeneratorType, GeneratorName) \
  (HSTSHeuristics::addSuccTokenGenerator((new GeneratorType(LabelStr(GeneratorName)))->getId()))

}
