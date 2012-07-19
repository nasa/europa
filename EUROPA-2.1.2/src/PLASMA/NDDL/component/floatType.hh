#ifndef FLOAT_TYPE_HH
#define FLOAT_TYPE_HH

#include "IntervalDomain.hh"
#include "IntervalTypeFactory.hh"

namespace EUROPA {

  /**
   * @class floatTypeFactory
   * @brief same as IntervalTypeFactory, except with the NDDL-specific
   *        "float" type name, and NDDL parsing value
   */
  class floatTypeFactory : public IntervalTypeFactory {
  public:
    floatTypeFactory();

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(const std::string& value) const;

  private:
    static const LabelStr& getDefaultTypeName();
  };

} // namespace EUROPA

#endif // FLOAT_TYPE_HH
