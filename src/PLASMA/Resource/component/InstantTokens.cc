#include "InstantTokens.hh"
#include "TokenVariable.hh"
#include "ResourceTokenRelation.hh"

namespace EUROPA {
    ReservoirToken::ReservoirToken(const PlanDatabaseId planDatabase,
				   const std::string& predicateName,
				   const IntervalIntDomain& timeBaseDomain,
				   const IntervalDomain& quantityBaseDomain,
				   bool _isConsumer,
				   bool closed,
				   bool _activate)
      : EventToken(planDatabase, predicateName,
		   false, false, timeBaseDomain,
		   Token::noObject(), false),
        m_quantity(), m_isConsumer(_isConsumer), m_activate(_activate) {
      check_error(quantityBaseDomain.getLowerBound() >= 0 &&
		  quantityBaseDomain.getUpperBound() <= PLUS_INFINITY);
      commonInit(closed, _activate, quantityBaseDomain);
      //m_quantity->restrictBaseDomain(quantityBaseDomain);
    }

    ReservoirToken::ReservoirToken(const PlanDatabaseId planDatabase,
				   const std::string& predicateName,
				   bool rejectable,
				   bool _isFact,
				   const IntervalIntDomain& timeBaseDomain,
				   const std::string& objectName,
				   bool _isConsumer,
				   bool closed,
				   bool _activate)
      : EventToken(planDatabase, predicateName,
		   rejectable, _isFact, timeBaseDomain, objectName, false),
        m_quantity(), m_isConsumer(_isConsumer), m_activate(_activate) {
      commonInit(closed, _activate, IntervalDomain( 0, PLUS_INFINITY ) );
    }

    ReservoirToken::ReservoirToken(const TokenId parent,
				   const std::string& relation,
				   const std::string& predicateName,
				   const IntervalIntDomain& timeBaseDomain,
				   const std::string& objectName,
				   bool _isConsumer,
				   bool closed,
				   bool _activate)
      : EventToken(parent, relation, predicateName, timeBaseDomain, objectName, false),
        m_quantity(), m_isConsumer(_isConsumer), m_activate(_activate) {
      commonInit(closed, _activate, IntervalDomain( 0, PLUS_INFINITY ));
    }

void ReservoirToken::commonInit(bool closed, bool _activate, const IntervalDomain& quantityBaseDomain) {
  m_activate = _activate;
  if(m_activate) {
    StateDomain restrictDomain;
    restrictDomain.insert(Token::ACTIVE);
    m_state->restrictBaseDomain(restrictDomain);
  }
  m_quantity = (new TokenVariable<IntervalDomain>(m_id, m_allVariables.size(),
                                                  m_planDatabase->getConstraintEngine(),
                                                  quantityBaseDomain,
                                                  false, true, "quantity"))->getId();
  m_allVariables.push_back(m_quantity);
  ConstraintId relation = (new ResourceTokenRelation(m_planDatabase->getConstraintEngine(),
                                                     makeScope(m_state, m_object),
                                                     getId()))->getId();
  m_standardConstraints.insert(relation);
  if(closed)
    close();
}

    void ReservoirToken::close() {
      EventToken::close();
      if(m_activate)
	activateInternal();
    }

    const ConstrainedVariableId ReservoirToken::getQuantity() const {
      return m_quantity;
    }

    bool ReservoirToken::isConsumer() const {return m_isConsumer;}
}
