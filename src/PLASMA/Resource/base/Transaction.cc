/*
 * Transaction.cc
 *
 *  Created on: Dec 23, 2008
 *      Author: javier
 */
#include "Transaction.hh"
#include "Debug.hh"

namespace EUROPA {
      Transaction::Transaction(ConstrainedVariableId time, ConstrainedVariableId quantity, bool isConsumer, EntityId owner)
          : m_id(this)
          , m_time(time)
          , m_quantity(quantity)
          , m_isConsumer(isConsumer)
          , m_owner(owner)
      {
      }

      Transaction::~Transaction()
      {
          m_id.remove();
      }

      std::string Transaction::toString() const {
        check_error(m_time.isValid());
        check_error(m_quantity.isValid());
        EntityId parent = m_time->parent();
        int parentKey = (parent.isId() ? parent->getKey() : -1);
        std::stringstream os;
        os << "{"
           << "(token=" << parentKey << ")"
           << " time(" << m_time->getKey() << ")=[" << m_time->lastDomain().getLowerBound() << " " << m_time->lastDomain().getUpperBound() << "]"
           << ",quantity(" << m_quantity->getKey() << ")=" << (m_isConsumer ? "-" : "+") << "[" << m_quantity->lastDomain().getLowerBound() << " " << m_quantity->lastDomain().getUpperBound() << "]"
           << "}";
        return os.str();
      }
}
