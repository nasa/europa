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
    NddlToken(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, const bool& rejectable = false, const bool& close = false);

    /**
     * @brief Constructor for subgoal tokens.
     * @see PLASMA::IntervalToken
     */
    NddlToken(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, const bool& close = false);

    StateVarId state; /**<Tracks token's state: active, merged, rejected, etc. */
    ObjectVarId object; /**<Tracks the objects the token could be associated with. */
    TempVarId start; /**<Tracks the token's possible start times. */
    TempVarId end; /**<Tracks the token's possible end times. */
    TempVarId duration; /**<Tracks the token's possible durations. */

  protected:
    virtual void handleDefaults(const bool&);
  private:
    void commonInit(const bool& autoClose);
  };

} // namespace NDDL

#endif // NDDL_TOKEN_HH
