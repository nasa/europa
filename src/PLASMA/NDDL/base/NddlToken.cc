#include "NddlToken.hh"
#include "Utils.hh"
#include "Object.hh"

using namespace EUROPA;
namespace NDDL {

NddlToken::NddlToken(const PlanDatabaseId planDatabase, 
                     const std::string& predicateName, const bool& rejectable,
                     const bool& _isFact, const bool& _close)
    : EUROPA::IntervalToken(planDatabase, 
                            predicateName,
                            rejectable,
                            _isFact,
                            IntervalIntDomain(),
                            IntervalIntDomain(),
                            IntervalIntDomain(1, PLUS_INFINITY),
                            EUROPA::Token::noObject(),
                            false),
  state(), object(), tStart(), tEnd(), tDuration() {
  commonInit(_close);
}

NddlToken::NddlToken(const TokenId _master, const std::string& predicateName,
                     const std::string& relation, const bool& _close)
    : EUROPA::IntervalToken(_master, 
                            relation,
                            predicateName,
                            IntervalIntDomain(),
                            IntervalIntDomain(),
                            IntervalIntDomain(1, PLUS_INFINITY),
                            EUROPA::Token::noObject(),
                            false),
      state(), object(), tStart(), tEnd(), tDuration() {
  commonInit(_close);
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
//     return getVariable(std::string(name));
//   }

} // namespace NDDL
