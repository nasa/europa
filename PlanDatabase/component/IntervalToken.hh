#ifndef _H_IntervalToken
#define _H_IntervalToken

#include "PlanDatabaseDefs.hh"
#include "Token.hh"

namespace Prototype {

  class IntervalToken: public Token {
  public:

    IntervalToken(const PlanDatabaseId& planDatabase, 
		  const LabelStr& predicateName, 
		  const BooleanDomain& rejectabilityBaseDomain,
		  const IntervalIntDomain& startBaseDomain,
		  const IntervalIntDomain& endBaseDomain,
		  const IntervalIntDomain& durationBaseDomain,
		  const std::vector<ConstrainedVariableId> parameters = std::vector<ConstrainedVariableId>(),
		  const LabelStr& objectName = Token::noObject());

    virtual ~IntervalToken();

    /*!< Accessors */
    const BoolVarId& getRejectability() const;
    const TempVarId& getStart() const;
    const TempVarId& getEnd() const;
    const TempVarId& getDuration() const;
    const std::vector<ConstrainedVariableId>& getParameters() const;

  protected:

    /**
     * @brief Default constructor - just a pass through.
     */
    IntervalToken(const PlanDatabaseId& planDatabase, 
		  const LabelStr& predicateName, 
		  const std::vector<ConstrainedVariableId> parameters,
		  const LabelStr& objectName);

    BoolVarId m_rejectability;
    TempVarId m_start;
    TempVarId m_end;
    TempVarId m_duration;
    std::vector<ConstrainedVariableId> m_parameters;
  };
}

#endif
