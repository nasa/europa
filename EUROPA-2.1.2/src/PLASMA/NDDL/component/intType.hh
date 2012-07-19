#ifndef INT_TYPE_HH
#define INT_TYPE_HH

#include "IntervalIntDomain.hh"
#include "IntervalIntTypeFactory.hh"

namespace EUROPA {

  /**
   * @class intTypeFactory
   * @brief same as IntervalIntTypeFactory, except with the NDDL-specific
   *        "int" type name, and NDDL parsing value
   */
  class intTypeFactory : public IntervalIntTypeFactory {
  public:
    intTypeFactory();

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(const std::string& value) const;

  private:
    static const LabelStr& getDefaultTypeName();
  };

} // namespace EUROPA

#endif // INT_TYPE_HH
