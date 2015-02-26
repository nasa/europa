#ifndef NDDL_TOKEN_HH
#define NDDL_TOKEN_HH

#include "NddlDefs.hh"
#include "IntervalToken.hh"
#include <string>

namespace NDDL {

  /**
   * Specialized token to allow public access to built in variables as member variables
   * rather than functions.
   */
  class NddlToken: public EUROPA::IntervalToken {
  public:
    /**
     * @brief Constructor for goal tokens.
     * @see PLASMA::IntervalToken
     */
    NddlToken(const EUROPA::PlanDatabaseId planDatabase,
              const std::string& predicateName,
              const bool& rejectable = false, const bool& isFact = false,
              const bool& close = false);

    /**
     * @brief Constructor for subgoal tokens.
     * @see PLASMA::IntervalToken
     */
    NddlToken(const EUROPA::TokenId master,
              const std::string& predicateName,
              const std::string& relation, const bool& close = false);

    EUROPA::StateVarId state; /**<Tracks token's state: active, merged, rejected, etc. */
    EUROPA::ObjectVarId object; /**<Tracks the objects the token could be associated with. */
    EUROPA::TempVarId tStart; /**<Tracks the token's possible start times. */
    EUROPA::TempVarId tEnd; /**<Tracks the token's possible end times. */
    EUROPA::TempVarId tDuration; /**<Tracks the token's possible durations. */

  protected:
    virtual void handleDefaults(const bool&);
  private:
    void commonInit(const bool& autoClose);
  };

} // namespace NDDL

#endif // NDDL_TOKEN_HH
