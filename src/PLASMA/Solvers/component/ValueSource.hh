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
      static ValueSource* getSource(const SchemaId& schema, const ConstrainedVariableId& var, bool externalOrder = false);

      /**
       * @brief Accessor for total number of values available
       */
      virtual AbstractDomain::size_type getCount() const;

      /**
       * @brief Accessor for the value at position given by index.
       */
      virtual edouble getValue(AbstractDomain::size_type index) const = 0;

      /**
       * @brief Required to allow correct cleanup
       */
      virtual ~ValueSource();

    protected:
      ValueSource(AbstractDomain::size_type count);

      AbstractDomain::size_type m_count;
    };

    class EnumValueSource : public ValueSource {
    public:
      EnumValueSource(const SchemaId& schema, const AbstractDomain& dom);
      edouble getValue(AbstractDomain::size_type index) const;
    private:
      std::vector<edouble> m_values;
    };

    class OrderedValueSource : public ValueSource {
    public:
      OrderedValueSource(const AbstractDomain& dom);
      edouble getValue(AbstractDomain::size_type index) const;
      void addValue(const edouble value);
    private:
      std::vector<edouble> m_values;
      const AbstractDomain& m_dom;
    };

    class IntervalValueSource : public ValueSource {
    public:
      IntervalValueSource(const AbstractDomain& dom);
      edouble getValue(AbstractDomain::size_type index) const;
      static AbstractDomain::size_type calculateSize(const AbstractDomain & dom);
    private:
      edouble m_lb;
      edouble m_ub;
      edouble m_step;
    };
  }
}
#endif
