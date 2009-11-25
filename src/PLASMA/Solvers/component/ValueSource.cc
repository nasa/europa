#include "ValueSource.hh"
#include "ConstrainedVariable.hh"
#include "AbstractDomain.hh"
#include "Debug.hh"
#include "Schema.hh"
#include "Entity.hh"

#include <algorithm>
/**
 * @author Michael Iatauro
 * @file ValueSource.cc
 * @brief Implements ValueSource, IntervalValueSource, EnumValueSource
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    ValueSource* ValueSource::getSource(const SchemaId& schema, const ConstrainedVariableId& var, bool externalOrder) {
      if(externalOrder)
	return new OrderedValueSource(var->lastDomain());
      if(var->lastDomain().isEnumerated())
	  return new EnumValueSource(schema, var->lastDomain());
      else
	return new IntervalValueSource(var->lastDomain());
    }

    ValueSource::ValueSource(AbstractDomain::size_type count) : m_count(count) {
      debugMsg("ValueSource:ValueSource", "Allocating for " << m_count << " choices.");
    }

    ValueSource::~ValueSource(){}

    AbstractDomain::size_type ValueSource::getCount() const { return m_count;}

    EnumValueSource::EnumValueSource(const SchemaId& schema, const AbstractDomain& dom)
      : ValueSource(dom.getSize()) {
      std::list<edouble> values;
      dom.getValues(values);
      //this isn't necessary anymore (I think), since object domains are now entity keys
//       if(schema->isObjectType(dom.getTypeName())) {
// 	EntityComparator<EntityId> foo;
// 	values.sort<EntityComparator<EntityId> >(foo);
//       }

      for(std::list<edouble>::const_iterator it = values.begin(); it != values.end(); ++it)
        m_values.push_back(*it);
    }

    edouble EnumValueSource::getValue(AbstractDomain::size_type index) const { return m_values[index];}

    OrderedValueSource::OrderedValueSource(const AbstractDomain& dom) : ValueSource(0), m_dom(dom) {
      checkError(!m_dom.isEmpty(), "Cannot create a value ordering for empty domain " << m_dom);
    }
    
    void OrderedValueSource::addValue(const edouble value) {
      if(m_dom.isMember(value)) {
	debugMsg("OrderedValueSource:addValue", "Adding value " << value << " from domain " << m_dom);
	m_values.push_back(value);
	m_count++;
      }
      condDebugMsg(!m_dom.isMember(value), "OrderedValueSource:addValue", "Value " << value << " not in " << m_dom);
    }

    edouble OrderedValueSource::getValue(AbstractDomain::size_type index) const {
      checkError(!m_values.empty(), "Cannot get an ordered value from an empty set!");
      return m_values[index];
    }

    IntervalValueSource::IntervalValueSource(const AbstractDomain& dom)
      : ValueSource(calculateSize(dom)),
	m_lb(dom.getLowerBound()), m_ub(dom.getUpperBound()), m_step(dom.minDelta()){
    }

    edouble IntervalValueSource::getValue(AbstractDomain::size_type index) const {return m_lb + (m_step * index);}

    AbstractDomain::size_type IntervalValueSource::calculateSize(const AbstractDomain& dom){
      return cast_int(((dom.getUpperBound() - dom.getLowerBound())/dom.minDelta()) + 1);
    }
  }
}
