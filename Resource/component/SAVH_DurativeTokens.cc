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
				 bool closed)
      : IntervalToken(planDatabase, predicateName, false, startBaseDomain, endBaseDomain, durationBaseDomain,
		      objectName, false) {
      check_error(quantityBaseDomain.getLowerBound() >= 0 && quantityBaseDomain.getUpperBound() <= PLUS_INFINITY);
      commonInit(closed, quantityBaseDomain);
    }

    ReusableToken::ReusableToken(const PlanDatabaseId& planDatabase,
				 const LabelStr& predicateName,
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const LabelStr& objectName,
				 bool closed) 
      : IntervalToken(planDatabase, predicateName, false, startBaseDomain, endBaseDomain, durationBaseDomain, objectName, false) {
      commonInit(closed, IntervalDomain(0, PLUS_INFINITY));
    }

    ReusableToken::ReusableToken(const TokenId& master, 
				 const LabelStr& relation,
				 const LabelStr& predicateName, 
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const LabelStr& objectName,
				 bool closed)
      : IntervalToken(master, relation, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, objectName, false) {
      commonInit(closed, IntervalDomain(0, PLUS_INFINITY));
    }

  ReusableToken::ReusableToken(const TokenId& master, 
			       const LabelStr& relation,
			       const LabelStr& predicateName, 
			       const IntervalIntDomain& startBaseDomain,
			       const IntervalIntDomain& endBaseDomain,
			       const IntervalIntDomain& durationBaseDomain,
			       const IntervalDomain& quantityBaseDomain,
			       const LabelStr& objectName,
			       bool closed)
    : IntervalToken(master, relation, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, objectName, false) {
    commonInit(closed, quantityBaseDomain);
  }

    void ReusableToken::commonInit(bool closed, const IntervalDomain& quantityBaseDomain) {
      StateDomain restrictDomain;
      restrictDomain.insert(Token::ACTIVE);
      m_state->restrictBaseDomain(restrictDomain);
      m_quantity = (new TokenVariable<IntervalDomain>(m_id, m_allVariables.size(),
						      m_planDatabase->getConstraintEngine(),
						      quantityBaseDomain,
						      true, LabelStr("quantity")))->getId();
      m_allVariables.push_back(m_quantity);
      ConstraintId relation = (new ResourceTokenRelation(m_planDatabase->getConstraintEngine(),
							 makeScope(m_object),
							 getId()))->getId();
      m_standardConstraints.insert(relation);
      if(closed)
	close();
    }

    void ReusableToken::close() {
      IntervalToken::close();
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
	       bool closed)
      : ReusableToken(planDatabase, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, IntervalDomain(1), objectName, closed) {}

    UnaryToken::UnaryToken(const TokenId& master, 
	       const LabelStr& relation,
	       const LabelStr& predicateName, 
	       const IntervalIntDomain& startBaseDomain,
	       const IntervalIntDomain& endBaseDomain,
	       const IntervalIntDomain& durationBaseDomain,
	       const LabelStr& objectName,
	       bool closed)
      : ReusableToken(master, relation, predicateName, startBaseDomain, endBaseDomain, durationBaseDomain, IntervalDomain(1), objectName, closed) {}
  }
}
