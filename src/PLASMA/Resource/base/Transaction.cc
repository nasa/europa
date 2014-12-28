/*
 * Transaction.cc
 *
 *  Created on: Dec 23, 2008
 *      Author: javier
 */
#include "Transaction.hh"
#include "Debug.hh"
#include "ConstrainedVariable.hh"
#include "Domain.hh"

namespace EUROPA {
Transaction::Transaction(ConstrainedVariableId _time, ConstrainedVariableId _quantity, 
                         bool _isConsumer, EntityId owner)
    : m_id(this)
    , m_time(_time)
    , m_quantity(_quantity)
    , m_isConsumer(_isConsumer)
    , m_owner(owner) {
  checkRuntimeError(_quantity->lastDomain().getLowerBound() >= 0.0,
                    "All transactions require positive quantity variables.");
}

      Transaction::~Transaction()
      {
          m_id.remove();
      }

      std::string Transaction::toString() const {
        check_error(m_time.isValid());
        check_error(m_quantity.isValid());
        EntityId parent = m_time->parent();
        eint parentKey = (parent.isId() ? parent->getKey() : -1);
        std::stringstream os;
        os << "{"
           << "(token=" << parentKey << ")"
           << " time(" << m_time->getKey() << ")=[" << m_time->lastDomain().getLowerBound() << " " << m_time->lastDomain().getUpperBound() << "]"
           << ",quantity(" << m_quantity->getKey() << ")=" << (m_isConsumer ? "-" : "+") << "[" << m_quantity->lastDomain().getLowerBound() << " " << m_quantity->lastDomain().getUpperBound() << "]"
           << "}";
        return os.str();
      }
}
