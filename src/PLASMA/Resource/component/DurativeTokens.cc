#include "DurativeTokens.hh"
#include "TokenVariable.hh"
#include "ResourceTokenRelation.hh"

namespace EUROPA {
    ReusableToken::ReusableToken(const PlanDatabaseId planDatabase,
				 const std::string& predicateName,
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const IntervalDomain& quantityBaseDomain,
				 const std::string& objectName,
				 bool closed,
				 bool _activate)
      : IntervalToken(planDatabase, predicateName, false, false, startBaseDomain,
                      endBaseDomain, durationBaseDomain, objectName, false),
        m_quantity(), m_activate(_activate) {
      check_error(quantityBaseDomain.getLowerBound() >= 0 &&
                  quantityBaseDomain.getUpperBound() <= PLUS_INFINITY);
      commonInit(closed, _activate, quantityBaseDomain);
    }

    ReusableToken::ReusableToken(const PlanDatabaseId planDatabase,
				 const std::string& predicateName,
				 const IntervalIntDomain& startBaseDomain,
				 const IntervalIntDomain& endBaseDomain,
				 const IntervalIntDomain& durationBaseDomain,
				 const std::string& objectName,
				 bool closed,
				 bool _activate)
      : IntervalToken(planDatabase, predicateName, false, false, startBaseDomain,
                      endBaseDomain, durationBaseDomain, objectName, false),
        m_quantity(), m_activate(_activate) {
      commonInit(closed, _activate, IntervalDomain(0, PLUS_INFINITY));
    }

ReusableToken::ReusableToken(const PlanDatabaseId planDatabase,
                             const std::string& predicateName,
                             bool rejectable,
                             bool _isFact,
                             const IntervalIntDomain& startBaseDomain,
                             const IntervalIntDomain& endBaseDomain,
                             const IntervalIntDomain& durationBaseDomain,
                             const IntervalDomain& quantityBaseDomain,
                             const std::string& objectName,
                             bool closed,
                             bool _activate)
: IntervalToken(planDatabase, predicateName, rejectable, _isFact, startBaseDomain,
                endBaseDomain, durationBaseDomain, objectName, false),
      m_quantity(), m_activate(_activate) {
  check_error(quantityBaseDomain.getLowerBound() >= 0 &&
              quantityBaseDomain.getUpperBound() <= PLUS_INFINITY);
  commonInit(closed, _activate, quantityBaseDomain);
}

ReusableToken::ReusableToken(const PlanDatabaseId planDatabase,
                             const std::string& predicateName,
                             bool rejectable,
                             bool _isFact,
                             const IntervalIntDomain& startBaseDomain,
                             const IntervalIntDomain& endBaseDomain,
                             const IntervalIntDomain& durationBaseDomain,
                             const std::string& objectName,
                             bool closed,
                             bool _activate)
: IntervalToken(planDatabase, predicateName, rejectable, _isFact, startBaseDomain,
                endBaseDomain, durationBaseDomain, objectName, false), 
  m_quantity(), m_activate(_activate) {
  commonInit(closed, _activate, IntervalDomain(0, PLUS_INFINITY));
}

ReusableToken::ReusableToken(const TokenId _master,
                             const std::string& relation,
                             const std::string& predicateName,
                             const IntervalIntDomain& startBaseDomain,
                             const IntervalIntDomain& endBaseDomain,
                             const IntervalIntDomain& durationBaseDomain,
                             const std::string& objectName,
                             bool closed,
                             bool _activate)
    : IntervalToken(_master, relation, predicateName, startBaseDomain, endBaseDomain,
                    durationBaseDomain, objectName, false),
  m_quantity(), m_activate(_activate) {
  commonInit(closed, _activate, IntervalDomain(0, PLUS_INFINITY));
}

ReusableToken::ReusableToken(const TokenId _master,
                             const std::string& relation,
                             const std::string& predicateName,
                             const IntervalIntDomain& startBaseDomain,
                             const IntervalIntDomain& endBaseDomain,
                             const IntervalIntDomain& durationBaseDomain,
                             const IntervalDomain& quantityBaseDomain,
                             const std::string& objectName,
                             bool closed,
                             bool _activate)
: IntervalToken(_master, relation, predicateName, startBaseDomain, endBaseDomain,
                durationBaseDomain, objectName, false),
  m_quantity(), m_activate(_activate) {
  commonInit(closed, _activate, quantityBaseDomain);
}

void ReusableToken::commonInit(bool closed, bool _activate,
                               const IntervalDomain& quantityBaseDomain) {
  m_activate = _activate;
  if(_activate) {
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

    void ReusableToken::close() {
      IntervalToken::close();
      if(m_activate)
	activateInternal();
    }

    const ConstrainedVariableId ReusableToken::getQuantity() const {
      return m_quantity;
    }

UnaryToken::UnaryToken(const PlanDatabaseId planDatabase,
                       const std::string& predicateName,
                       const IntervalIntDomain& startBaseDomain,
                       const IntervalIntDomain& endBaseDomain,
                       const IntervalIntDomain& durationBaseDomain,
                       const std::string& objectName,
                       bool closed,
                       bool _activate)
    : ReusableToken(planDatabase, predicateName, startBaseDomain, endBaseDomain,
                    durationBaseDomain, IntervalDomain(1), objectName, closed,
                    _activate) {}

UnaryToken::UnaryToken(const PlanDatabaseId planDatabase,
                       const std::string& predicateName,
                       bool rejectable,
                       bool _isFact,
                       const IntervalIntDomain& startBaseDomain,
                       const IntervalIntDomain& endBaseDomain,
                       const IntervalIntDomain& durationBaseDomain,
                       const std::string& objectName,
                       bool closed,
                       bool _activate)
    : ReusableToken(planDatabase, predicateName, rejectable, _isFact, startBaseDomain,
                    endBaseDomain, durationBaseDomain, IntervalDomain(1), objectName,
                    closed, _activate) {}

UnaryToken::UnaryToken(const TokenId _master,
                       const std::string& relation,
                       const std::string& predicateName,
                       const IntervalIntDomain& startBaseDomain,
                       const IntervalIntDomain& endBaseDomain,
                       const IntervalIntDomain& durationBaseDomain,
                       const std::string& objectName,
                       bool closed,
                       bool _activate)
    : ReusableToken(_master, relation, predicateName, startBaseDomain, endBaseDomain,
                    durationBaseDomain, IntervalDomain(1), objectName, closed,
                    _activate) {}
}
