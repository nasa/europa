#ifndef _H_ObjectTokenRelation
#define _H_ObjectTokenRelation

#include "PlanDatabaseDefs.hh"
#include "DomainListener.hh"
#include "Constraint.hh"

namespace EUROPA
{
  /**
   * @brief Responsible for managing the relationship between objects and tokens through propagation of changes
   * in Token.state and Token.object.
   *
   * In a fully grounded plan database, there will exist a one to one maping between an object instance and an active token
   * instance. However, in the general case, there is a many to many relation between objects and tokens which can be impacted
   * through decisions by a planner directly, such as specifying the value of the token object variable, or
   * indirectly, through propagation. Objects are only related to active tokens. Thus, the ObjectTokenRelation is a constraint
   * over the object variable of the token, the state variable of the token, and the set of tokens active tokens on each object.
   */
  class ObjectTokenRelation : public Constraint {
  public:

    ObjectTokenRelation(const LabelStr& name,
			const LabelStr& propagatorName,
			const ConstraintEngineId& constraintEngine,
			const std::vector<ConstrainedVariableId>& variables);

    ~ObjectTokenRelation();

    void handleExecute();

    void handleExecute(const ConstrainedVariableId& variable, 
		       int argIndex, 
		       const DomainListener::ChangeType& changeType);

    bool canIgnore(const ConstrainedVariableId& variable, 
		   int argIndex, 
		   const DomainListener::ChangeType& changeType);

  private:
    void handleDiscard();
    void notifyAdditions();
    void notifyRemovals();

    bool isValid() const;

    const TokenId m_token;
    std::set<ObjectId> m_notifiedObjects; /**< Keeps track of notified objects (of additions). Updated on each execution. */
    const ObjectDomain& m_currentDomain; /**< Holds a direct reference to the propagated domain of the objectVariable */

    static const int STATE_VAR = 0;
    static const int OBJECT_VAR = 1;
  };
}
#endif
