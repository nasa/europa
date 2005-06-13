#ifndef _H_AbstractVar
#define _H_AbstractVar

#include "ConstrainedVariable.hh"
#include "ConstraintEngine.hh"

/**
 * @file Variable.hh
 * @author Conor McGann
 * @date August, 2003
 * @brief Provides the Users (Planners) perspective for Variable Definition.
 *
 * This file introduces classes derived from ConstrainedVariable which provide an external view
 * of a variable and also implement the contract required by the ConstrainedVariable class. Handy for testing.
 * @see ConstrainedVariable
 */
namespace EUROPA {
  /**
   * @class Variable
   * @brief Template class to provide concrete variables of specific domain types.
   */
  template<class DomainType>
  class Variable : public ConstrainedVariable {
  public:

    /**
     * @brief Type specific constructor.
     * @param constraintEngine - the engine required by parent constructors.
     * @param baseDomain - the initial value for the domain and the maximum possible value.
     * @param parent owner if appropriate.
     * @param index position in parent collection.
     */
    Variable(const ConstraintEngineId& constraintEngine, 
             const AbstractDomain& baseDomain,
             bool canBeSpecified = true,
             const LabelStr& name = ConstrainedVariable::NO_NAME(),
             const EntityId& parent = EntityId::noId(),
             int index = ConstrainedVariable::NO_INDEX);

    /* Do we want to allow the compiler-created copy constructor?
       -- wedgingt 2004 Mar 3
    */

    /**
     * Destructor.
     */
    virtual ~Variable();

    /**
     * @brief Return the domain first used in initialization.
     */
    const DomainType& getBaseDomain() const;

    /**
     * @brief Return the last domain used in a call to specify.
     * @see specify()
     */
    const DomainType& getSpecifiedDomain() const;

    /**
     * @brief Return the derived domain.
     * Return the domain resulting from constraint propagtion.
     * values.
     * @see getCurrentDomain()
     */
    const DomainType& getDerivedDomain();

    /**
     * @brief Return the current domain.
     * Return the last computed derived domain.
     * @see getCurrentDomain()
     */
    const DomainType& getLastDomain() const;


    /**
     * @brief Return the last computed derived domain.
     * @see ConstrainedVariable::lastDomain()
     */
    const AbstractDomain& lastDomain() const;

    /**
     * @brief Returns the derived domain.
     * @note Causes any needed propagation.
     * @see ConstrainedVariable::derivedDomain()
     */
    const AbstractDomain& derivedDomain();

    /**
     * @brief Retrieve the specified domain.
     */
    const AbstractDomain& specifiedDomain() const;

    /**
     * @brief Retrieve the specified domain.
     */
    const AbstractDomain& baseDomain() const;

    /**
     * @brief Restrict the base domain. This cannot be reversed. It is a special update operation
     */
    virtual void restrictBaseDomain(const AbstractDomain& baseDomain);

    virtual void handleSpecified(const AbstractDomain& specDomain) {}
    
    virtual void handleReset() {}

  protected:
    AbstractDomain& internal_specifiedDomain();
    AbstractDomain& internal_baseDomain();
  private:

    /**
     * @brief returns the current domain without checking for pending propagation first.
     * This method implements the required function for constraints to access the domain during
     * propagation.
     * @see lastDomain(), Constraint
     */
    AbstractDomain& getCurrentDomain();

  protected:
    DomainType m_baseDomain; /**< The initial (and maximal, unless dynamic) set for the domain of this variable. */
    DomainType m_specifiedDomain; /**< May contain a user specified restriction on the maximum set of the domain. */
    DomainType m_derivedDomain; /**< The current domain of the variable based on user specifications and derived from
                                   constraint propagation. */
  };

  template<class DomainType>
  Variable<DomainType>::Variable(const ConstraintEngineId& constraintEngine, 
                                 const AbstractDomain& baseDomain,
                                 bool canBeSpecified,
                                 const LabelStr& name,
                                 const EntityId& parent,
                                 int index) 
    : ConstrainedVariable(constraintEngine, canBeSpecified, name, parent, index), 
    m_baseDomain(baseDomain),
    m_specifiedDomain(baseDomain),
    m_derivedDomain(baseDomain) {
    // Note that we permit the domain to be empty initially
    m_derivedDomain.setListener(m_listener);

    // Don't propagate set operations on dynamic or empty domains.
    if (baseDomain.isOpen() || baseDomain.isEmpty())
      return;

    if (baseDomain.isSingleton())
      m_derivedDomain.set(m_specifiedDomain.getSingletonValue());
    else
      m_derivedDomain.set(m_specifiedDomain);
  }
  
  template<class DomainType>
  Variable<DomainType>::~Variable() {
  }

  template<class DomainType>
  const DomainType& Variable<DomainType>::getBaseDomain() const {
    return(m_baseDomain);
  }

  template<class DomainType>
  const DomainType& Variable<DomainType>::getSpecifiedDomain() const {
    return(m_specifiedDomain);
  }

  template<class DomainType>
  const DomainType& Variable<DomainType>::getDerivedDomain() {
    if (!m_constraintEngine->isPropagating() && pending())
      update();

    if (!provenInconsistent())
      return(m_derivedDomain);

    static bool sl_initialized = false;
    static DomainType* sl_emptyDomain = 0;
    if (!sl_initialized) {
      sl_emptyDomain = dynamic_cast<DomainType*>(m_derivedDomain.copy());
      if (sl_emptyDomain->isOpen())
        sl_emptyDomain->close();
      sl_emptyDomain->empty();
      sl_initialized = true;
    }
    return(*sl_emptyDomain);
  }

  template<class DomainType>
  AbstractDomain& Variable<DomainType>::getCurrentDomain() {
    check_error(validate());
    return(m_derivedDomain);
  }

  template<class DomainType>
  const DomainType& Variable<DomainType>::getLastDomain() const {
    check_error(validate());
    return(m_derivedDomain);
  }

  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::lastDomain() const {
    check_error(validate());
    return(m_derivedDomain);
  }
  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::derivedDomain() {
    check_error(validate());
    return(getDerivedDomain());
  }

  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::specifiedDomain() const {
    check_error(validate());
    return(m_specifiedDomain);
  }

  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::baseDomain() const {
    check_error(validate());
    return(m_baseDomain);
  }

  template<class DomainType>
  void Variable<DomainType>::restrictBaseDomain(const AbstractDomain& newBaseDomain) {
    check_error(validate());
    if(m_baseDomain.intersect(newBaseDomain))
      internalSpecify(newBaseDomain);
  }
 
  template<class DomainType>
  AbstractDomain& Variable<DomainType>::internal_specifiedDomain() {
    return(m_specifiedDomain);
  }

  template<class DomainType>
  AbstractDomain& Variable<DomainType>::internal_baseDomain() {
    return(m_baseDomain);
  }
}
#endif
