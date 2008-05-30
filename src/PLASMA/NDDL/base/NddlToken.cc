#include "NddlToken.hh"
#include "Utils.hh"
#include "Object.hh"
namespace NDDL {

  NddlToken::NddlToken(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, const bool& rejectable, const bool& isFact, const bool& close)
    : EUROPA::IntervalToken(planDatabase, 
                            predicateName,
                            rejectable,
                            isFact,
                            IntervalIntDomain(),
                            IntervalIntDomain(),
                            IntervalIntDomain(1, PLUS_INFINITY),
                            EUROPA::Token::noObject(),
                            false) {
    commonInit(close);
  }

  NddlToken::NddlToken(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, const bool& close)
    : EUROPA::IntervalToken(master, 
                            relation,
                            predicateName,
                            IntervalIntDomain(),
                            IntervalIntDomain(),
                            IntervalIntDomain(1, PLUS_INFINITY),
                            EUROPA::Token::noObject(),
                            false) {
    commonInit(close);
  }

  void NddlToken::handleDefaults(const bool&) {
  }

  void NddlToken::commonInit(const bool& autoClose) {
    state = getState();
    object = getObject();
    tStart = start();
    tEnd = end();
    tDuration = duration();
    if (autoClose)
      close();
  }

//   ConstrainedVariableId NddlToken::var(const std::string& name) const {
//     return getVariable(LabelStr(name));
//   }

} // namespace NDDL
