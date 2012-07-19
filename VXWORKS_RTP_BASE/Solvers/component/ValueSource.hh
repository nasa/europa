#ifndef H_ValueSource
#define H_ValueSource

/**
 * @author Michael Iatauro
 * @brief Encapsulates access to interval and enumerated domains in a single interface
 * suitable for indexed access.
 * @date March, 2005
 */

#include "SolverDefs.hh"

namespace EUROPA {
  namespace SOLVERS {
    class ValueSource {
    public:
      /**
       * @brief Factory method to create appropriate concrete source
       */
      static ValueSource* getSource(const ConstrainedVariableId& var, bool externalOrder = false);

      /**
       * @brief Accessor for total number of values available
       */
      virtual unsigned int getCount() const;

      /**
       * @brief Accessor for the value at position given by index.
       */
      virtual double getValue(unsigned int index) const = 0;

      /**
       * @brief Required to allow correct cleanup
       */
      virtual ~ValueSource();

    protected:
      ValueSource(unsigned int count);

      unsigned int m_count;
    };

    class EnumValueSource : public ValueSource {
    public:
      EnumValueSource(const AbstractDomain& dom);
      double getValue(unsigned int index) const;
    private:
      std::vector<double> m_values;
    };

    class OrderedValueSource : public ValueSource {
    public:
      OrderedValueSource(const AbstractDomain& dom);
      double getValue(unsigned int index) const;
      void addValue(const double value);
    private:
      std::vector<double> m_values;
      const AbstractDomain& m_dom;
    };

    class IntervalValueSource : public ValueSource {
    public:
      IntervalValueSource(const AbstractDomain& dom);
      double getValue(unsigned int index) const;
      static unsigned int calculateSize(const AbstractDomain & dom);
    private:
      double m_lb;
      double m_ub;
      double m_step;
    };
  }
}
#endif
