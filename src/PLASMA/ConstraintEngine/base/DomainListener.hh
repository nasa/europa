#ifndef _H_DomainListener
#define _H_DomainListener

/**
 * @file DomainListener.hh
 * @author Conor McGann
 * @date August, 2003
 * @brief Defines the listener interface for domain change events
 */

#include "Id.hh"

#include <iosfwd>


namespace EUROPA{

  class DomainListener;
  typedef Id<DomainListener> DomainListenerId;

  /**
   * @class DomainListener
   * @brief Declares an abstract class for listeners to domain change events on an Domain
   *
   * This listener interface is the means to connect propagation events to the ConstraintEngine. It allows
   * a more fine-grain event model to handle changes in variable domains. Most events are restrictions to a
   * domain. Other key changes that may have special significance are also specified.
   * @see ChangeType, Domain, VariableChangeListener
   */
  class DomainListener {
  public:

    /**
     * @enum ChangeType
     * @brief Possible ways that a Domain can change.
     * @see Domain
     */
    enum ChangeType { UPPER_BOUND_DECREASED = 0, /**< The upper bound of an interval domain is reduced. @note Must be 0. @see EVENT_COUNT. */
                      LOWER_BOUND_INCREASED, /**< The lower bound of an interval domain is increased. */
                      BOUNDS_RESTRICTED, /**< Upper and lower bound are decreased and increased respectively. */
                      VALUE_REMOVED, /**< A restriction to an enumerated domain. */
                      RESTRICT_TO_SINGLETON, /**< A restriction of the domain to a singleton value through inference. */
                      SET_TO_SINGLETON, /**< Special case restriction when the domain is set to a singleton. */
                      RESET, /**< Special case of an external relaxation. */
                      RELAXED, /**< Inferred relaxation to the domain. */
                      CLOSED, /**< When a dynamic domain is closed this event will be generated. */
                      OPENED, /**< When a closed domain is re-opened this event will be generated. */
                      EMPTIED, /**< A domain was emptied, indicating an inconsistency. */
                      LAST_CHANGE_TYPE /**< Use only for the value of EVENT_COUNT. @note Must be last in ChangeType. @see EVENT_COUNT */
    };

    /**
     * The maximum count of events supported - i.e. "useful" entries in enum ChangeType.
     * @note This implementation depends on the first member of
     * ChangeType having the int value 0 and no others being given
     * an explicit numeric value but does not otherwise need to be
     * modified when ChangeType is added to.
     * @see ChangeType
     */
    static const int EVENT_COUNT = (int)LAST_CHANGE_TYPE;

    static std::string toString(const ChangeType& changeType);

    /**
     * @brief Utility to test if an event is a restriction
     * @note Assumes that restriction events are defined earlier
     */
    inline static bool isRestriction(const ChangeType& changeType){
      return changeType <= RESTRICT_TO_SINGLETON;
    }

    /**
     * @brief Constructor sets up the Id.
     */
    DomainListener();

    /**
     * @brief Destructor cleans up the id.
     */
    virtual ~DomainListener();

    /**
     * @brief Id accessor
     */
    const DomainListenerId& getId() const;

    /**
     * @brief The critical method to implement in order to define how events are to be propagated.
     * @param changetype the type of change occuring on the domain.
     */
    virtual void notifyChange(const ChangeType& changeType) = 0;

  private:
    DomainListenerId m_id; /**< Reference to self. */
  };

  std::ostream& operator<<(std::ostream& str, const DomainListener::ChangeType ct);
}
#endif
