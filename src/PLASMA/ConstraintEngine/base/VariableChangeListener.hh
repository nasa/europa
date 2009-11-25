#ifndef _H_VariableChangeListener
#define _H_VariableChangeListener

/**
 * @file VariableChangeListener.hh
 * @author Conor McGann
 * @date August, 2003
 * @brief Provides base class for customizing event propagation to the ConstraintEngine from a variables Domain
 * @see ConstraintEngine, ConstrainedVariable
 */
#include "ConstraintEngineDefs.hh"
#include "DomainListener.hh"

namespace EUROPA {

  /**
   * @class VariableChangeListener
   * @brief Base class for Listeners which allow the ConstraintEngine to observe domain changes, and thus trigger propagation event handling.
   *
   * @see ConstraintEngine::allocateVariableListener()
   */
  class VariableChangeListener: public DomainListener
  {
  public:
    /**
     * @brief Constructor has all the data to hook up publisher and possible subscribers.
     * @param variable The variable to be listened to
     * @param constraintEngine The ConstraintEngine to whcih the listener belongs.
     */
    VariableChangeListener(const ConstrainedVariableId& variable,
			   const ConstraintEngineId& constraintEngine);

    /**
     * @brief Implements handler for a domaing change event. Just a pass through to the ConstraintEngine
     * @param changeType The type of change on the variable domain.
     * @see ConstraintEngine::notify
     */
    void notifyChange(const ChangeType& changeType);

  protected:
    friend class ConstrainedVariable; /**< Grant access for destructor */

    ~VariableChangeListener(){} /**< Make this protected so only the ConstrainedVariable can delete it */

    const ConstrainedVariableId m_variable; /**< The ConstrainedVariable whose domain we are listening to */
    const ConstraintEngineId& m_constraintEngine; /**< Cache the ConstraintEngine - candidate subscriber */
  };
}

#endif 
