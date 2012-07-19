#include "SAVH_DurativeTokens.hh"
#include "TokenVariable.hh"
#include "SAVH_ResourceTokenRelation.hh"

namespace EUROPA {
  namespace SAVH {
    ReusableToken::ReusableToken(const PlanDatabaseId& planDatabase,
				 const LabelStr& predicateName,
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const IntervalDomain& quantityBaseDomain,
				 const LabelStr& objectName,
				 bool closed,
				 bool activate)
      : IntervalToken(planDatabase, predicateName, false, false, startBaseDomain, endBaseDomain, durationBaseDomain,
		      objectName, false) {
      check_error(quantityBaseDomain.getLowerBound() >= 0 && quantityBaseDomain.getUpperBound() <= PLUS_INFINITY);
      commonInit(closed, activate, quantityBaseDomain);
    }

    ReusableToken::ReusableToken(const PlanDatabaseId& planDatabase,
				 const LabelStr& predicateName,
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const LabelStr& objectName,
				 bool closed,
				 bool activate) 
      : IntervalToken(planDatabase, predicateName, false, false, startBaseDomain, endBaseDomain, durationBaseDomain, objectName, false) {
      commonInit(closed, activate, IntervalDomain(0, PLUS_INFINITY));
    }

    ReusableToken::ReusableToken(const PlanDatabaseId& planDatabase,
				 const LabelStr& predicateName,
				 bool rejectable,
				 bool isFact,
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const IntervalDomain& quantityBaseDomain,
				 const LabelStr& objectName,
				 bool closed,
				 bool activate)
      : IntervalToken(planDatabase, predicateName, rejectable, isFact, startBaseDomain, endBaseDomain, durationBaseDomain,
		      objectName, false) {
      check_error(quantityBaseDomain.getLowerBound() >= 0 && quantityBaseDomain.getUpperBound() <= PLUS_INFINITY);
      commonInit(closed, activate, quantityBaseDomain);
    }

    ReusableToken::ReusableToken(const PlanDatabaseId& planDatabase,
				 const LabelStr& predicateName,
				 bool rejectable,
				 bool isFact,
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const LabelStr& objectName,
				 bool closed,
				 bool activate) 
      : IntervalToken(planDatabase, predicateName, rejectable, isFact, startBaseDomain, endBaseDomain, durationBaseDomain, objectName, false) {
      commonInit(closed, activate, IntervalDomain(0, PLUS_INFINITY));
    }

    ReusableToken::ReusableToken(const TokenId& master, 
				 const LabelStr& relation,
				 const LabelStr& predicateName, 
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const LabelStr& objectName,
				 bool closed,
				 bool activate)
      : IntervalToken(master, relation, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, objectName, false) {
      commonInit(closed, activate, IntervalDomain(0, PLUS_INFINITY));
    }

  ReusableToken::ReusableToken(const TokenId& master, 
			       const LabelStr& relation,
			       const LabelStr& predicateName, 
			       const IntervalIntDomain& startBaseDomain,
			       const IntervalIntDomain& endBaseDomain,
			       const IntervalIntDomain& durationBaseDomain,
			       const IntervalDomain& quantityBaseDomain,
			       const LabelStr& objectName,
			       bool closed,
			       bool activate)
    : IntervalToken(master, relation, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, objectName, false) {
    commonInit(closed, activate, quantityBaseDomain);
  }

    void ReusableToken::commonInit(bool closed, bool activate, const IntervalDomain& quantityBaseDomain) {
      m_activate = activate;
      if(activate) {
	StateDomain restrictDomain;
	restrictDomain.insert(Token::ACTIVE);
	m_state->restrictBaseDomain(restrictDomain);
      }
      m_quantity = (new TokenVariable<IntervalDomain>(m_id, m_allVariables.size(),
						      m_planDatabase->getConstraintEngine(),
						      quantityBaseDomain,
						      false, true, LabelStr("quantity")))->getId();
      m_allVariables.push_back(m_quantity);
      ConstraintId relation = (new ResourceTokenRelation(m_planDatabase->getConstraintEngine(),
							 makeScope(m_state, m_object),
							 getId()))->getId();
      m_standardConstraints.insert(relation);
      if(closed)
	close();
    }

    void ReusableToken::close() {
      IntervalToken::close();
      if(m_activate)
	activateInternal();
    }

    const ConstrainedVariableId& ReusableToken::getQuantity() const {
      return m_quantity;
    }

    UnaryToken::UnaryToken(const PlanDatabaseId& planDatabase,
			   const LabelStr& predicateName,
			   const IntervalIntDomain& startBaseDomain,
			   const IntervalIntDomain& endBaseDomain,
			   const IntervalIntDomain& durationBaseDomain,
			   const LabelStr& objectName,
			   bool closed,
			   bool activate)
      : ReusableToken(planDatabase, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, IntervalDomain(1), objectName, closed, activate) {}

    UnaryToken::UnaryToken(const PlanDatabaseId& planDatabase,
			   const LabelStr& predicateName,
			   bool rejectable,
			   bool isFact,
			   const IntervalIntDomain& startBaseDomain,
			   const IntervalIntDomain& endBaseDomain,
			   const IntervalIntDomain& durationBaseDomain,
			   const LabelStr& objectName,
			   bool closed,
			   bool activate)
      : ReusableToken(planDatabase, predicateName, rejectable, isFact, startBaseDomain, endBaseDomain, durationBaseDomain, IntervalDomain(1), objectName, closed, activate) {}

    UnaryToken::UnaryToken(const TokenId& master, 
			   const LabelStr& relation,
			   const LabelStr& predicateName, 
			   const IntervalIntDomain& startBaseDomain,
			   const IntervalIntDomain& endBaseDomain,
			   const IntervalIntDomain& durationBaseDomain,
			   const LabelStr& objectName,
			   bool closed,
			   bool activate)
      : ReusableToken(master, relation, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, IntervalDomain(1), objectName, closed, activate) {}
  }
}
