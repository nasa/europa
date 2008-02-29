#include "SAVH_InstantTokens.hh"
#include "TokenVariable.hh"
#include "SAVH_ResourceTokenRelation.hh"

namespace EUROPA {
  namespace SAVH {
    ReservoirToken::ReservoirToken(const PlanDatabaseId& planDatabase,
				   const LabelStr& predicateName,
				   const IntervalIntDomain& timeBaseDomain,
				   const IntervalDomain& quantityBaseDomain,
				   bool isConsumer,
				   bool closed,
				   bool activate) 
      : EventToken(planDatabase, predicateName,
		   false, false, timeBaseDomain,
		   Token::noObject(), false), m_isConsumer(isConsumer) {
      check_error(quantityBaseDomain.getLowerBound() >= 0 &&
		  quantityBaseDomain.getUpperBound() <= PLUS_INFINITY);
      commonInit(closed, activate, quantityBaseDomain);
      //m_quantity->restrictBaseDomain(quantityBaseDomain);
    }

    ReservoirToken::ReservoirToken(const PlanDatabaseId& planDatabase,
				   const LabelStr& predicateName,
				   bool rejectable,
				   bool isFact,
				   const IntervalIntDomain& timeBaseDomain,
				   const LabelStr& objectName,
				   bool isConsumer,
				   bool closed,
				   bool activate)
      : EventToken(planDatabase, predicateName,
		   rejectable, isFact, timeBaseDomain, objectName, false), m_isConsumer(isConsumer) {
      commonInit(closed, activate, IntervalDomain( 0, PLUS_INFINITY ) );
    }

    ReservoirToken::ReservoirToken(const TokenId& parent,
				   const LabelStr& relation,
				   const LabelStr& predicateName,
				   const IntervalIntDomain& timeBaseDomain,
				   const LabelStr& objectName,
				   bool isConsumer,
				   bool closed,
				   bool activate)
      : EventToken(parent, relation, predicateName, timeBaseDomain, objectName, false), m_isConsumer(isConsumer) {
      commonInit(closed, activate, IntervalDomain( 0, PLUS_INFINITY ));
    }

    void ReservoirToken::commonInit(bool closed, bool activate, const IntervalDomain& quantityBaseDomain) {
      m_activate = activate;
      if(activate) {
	StateDomain restrictDomain;
	restrictDomain.insert(Token::ACTIVE);
	m_state->restrictBaseDomain(restrictDomain);
      }
      m_quantity = (new TokenVariable<IntervalDomain>(m_id, m_allVariables.size(),
						      m_planDatabase->getConstraintEngine(),
						      quantityBaseDomain,
						      true, LabelStr("quantity")))->getId();
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
    
    const ConstrainedVariableId& ReservoirToken::getQuantity() const {
      return m_quantity;
    }

    bool ReservoirToken::isConsumer() const {return m_isConsumer;}
  }
}
