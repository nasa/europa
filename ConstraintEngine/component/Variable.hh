#ifndef _H_AbstractVar
#define _H_AbstractVar

#include "ConstrainedVariable.hh"

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
namespace Prototype
{
  /**
   * @class Variable
   * @brief Template class to provide concrete variables of specific domain types.
   */
  template<class DomainType>
  class Variable: public ConstrainedVariable {
  public:

    /**
     * @brief Type specific constructor
     * @param constraintEngine - the engine required by parent constructors.
     * @param baseDomain - the initial value for the domain and the maximum possible value.
     * @param parent owner if appropriate
     * @param index position in parent collection
     */
    Variable(const ConstraintEngineId& constraintEngine, 
	     const DomainType& baseDomain,
	     bool canBeSpecified = true,
	     const LabelStr& name = ConstrainedVariable::NO_NAME(),
	     const EntityId& parent = EntityId::noId(),
	     int index = ConstrainedVariable::NO_INDEX);

    virtual ~Variable();

    /**
     * @brief Adds an element to a dynamic domain. The domain must be closed.
     * @param value The value to be inserted to the various domains of the variable.
     */
    virtual void insert(double value);

    /**
     * @brief Restricts the domain of the variable to be a subset of the given domain.
     *
     * This operation is only valid if the ConstraintEngine does not have an empty domain.
     * @param domain - the domain to restrict to. Note that the domain must be a subset of the current domain.
     * @see reset()
     */
    virtual void specify(const AbstractDomain& domain);

    /**
     * @brief Special case of specify where we restrict it to a singleton
     * @param singleTonValue to specify it to
     * @see specify(const DomainType& domain), reset()
     */
    virtual void specify(const double& singletonValue);

    /**
     * @brief Retract previously specified domain restriction.
     * @see specify()
     */
    virtual void reset();
    

    /**
     * @brief Allows a client to close a variables domain. The variable must be dynamic.
     */
    virtual void close();

    /**
     * @brief return the domain first used in initialization
     */
    const DomainType& getBaseDomain() const;

    /**
     * @brief return the last domain used in a call to specify.
     * @see specify()
     */
    const DomainType& getSpecifiedDomain() const;

    /**
     * @brief return the derived domain i.e. that resulting from constraint propagtion of initial or specified
     * values.
     * @see getCurrentDomain()
     */
    const DomainType& getDerivedDomain();

    /**
     * @brief Impements the base class pure virtual function
     * @see ConstrainedVariable::lastDomain()
     */
    const AbstractDomain& lastDomain() const;

    /**
     * @brief Impements the base class pure virtual function
     * @see ConstrainedVariable::derivedDomain()
     */
    const AbstractDomain& derivedDomain();

    /**
     * @brief Retrieve the specified domain
     */
    const AbstractDomain& specifiedDomain() const;

    /**
     * @brief Retrieve the specified domain
     */
    const AbstractDomain& baseDomain() const;


    virtual void handleSpecified(const AbstractDomain& specDomain){}
    virtual void handleReset() {}

  private:

    /**
     * @brief Relaxes the derived domain to the current specified domain.
     * 
     * This method implements the required behaviour to relax variables in the ConstraintEngine.
     */
    void relax();

    /**
     * @brief returns the current domain without checking for pending propagation first.
     *
     * This method implements the required function for constraints to access the domain during
     * propagation.
     * @see lastDomain(), Constraint
     */
    AbstractDomain& getCurrentDomain();


    /**
     * @brief Called by the parent class function isValid().
     * @return true of the derived domain is a subset of the specified domain and the specified domain is a
     * subset of the base domain. Otherwise false.
     */
    bool validate() const;

  protected:
    DomainType m_baseDomain; /*!< The initial (and maximal, unless dynamic) set for the domain of this variable */
    DomainType m_specifiedDomain; /*!< May contain a user specified restriction on the maximum set of the domain */
    DomainType m_derivedDomain; /*!< The current domain of the variable based on user specifications and derived from
				  constraint propagation */
  };


  template<class DomainType>
  Variable<DomainType>::Variable(const ConstraintEngineId& constraintEngine, 
				 const DomainType& baseDomain,
				 bool canBeSpecified,
				 const LabelStr& name,
				 const EntityId& parent,
				 int index) 
    : ConstrainedVariable(constraintEngine, canBeSpecified, name, parent, index), 
    m_baseDomain(baseDomain),
    m_specifiedDomain(baseDomain),
    m_derivedDomain(baseDomain){
    check_error(m_derivedDomain.isDynamic() || !m_derivedDomain.isEmpty());
    m_derivedDomain.setListener(m_listener);

    // Don't propagate set operations on domains.
    if(baseDomain.isDynamic())
      return;

    if(baseDomain.isSingleton())
      m_derivedDomain.set(m_specifiedDomain.getSingletonValue());
    else
      m_derivedDomain.set(m_specifiedDomain);
  }
  
  template<class DomainType>
  Variable<DomainType>::~Variable(){}

  template<class DomainType>
  void Variable<DomainType>::insert(double value) {
    check_error(m_baseDomain.isDynamic());
    m_baseDomain.insert(value);
    m_specifiedDomain.insert(value);
    m_derivedDomain.insert(value);
  }

  template<class DomainType>
  bool Variable<DomainType>::validate() const{
    return (m_derivedDomain.isSubsetOf(m_specifiedDomain) &&
	    m_specifiedDomain.isSubsetOf(m_baseDomain));
  }

  template<class DomainType>
  const DomainType& Variable<DomainType>::getBaseDomain() const {return m_baseDomain;}

  template<class DomainType>
  const DomainType& Variable<DomainType>::getSpecifiedDomain() const {return m_specifiedDomain;}

  template<class DomainType>
  const DomainType& Variable<DomainType>::getDerivedDomain(){
    if(pending())
      update();

    if(provenInconsistent()){
      static bool sl_initialized = false;
      static DomainType sl_emptyDomain;
      if (sl_initialized == false) {
	if (sl_emptyDomain.isDynamic())
	  sl_emptyDomain.close();
	sl_emptyDomain.empty();
	sl_initialized = true;
      }
      return sl_emptyDomain;
    }
    else
      return m_derivedDomain;
  }

  template<class DomainType>
  AbstractDomain& Variable<DomainType>::getCurrentDomain() {return m_derivedDomain;}

  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::lastDomain() const {
    return m_derivedDomain;
  }
  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::derivedDomain() {
    return getDerivedDomain();
  }

  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::specifiedDomain() const {
    return m_specifiedDomain;
  }

  template<class DomainType>
  const AbstractDomain& Variable<DomainType>::baseDomain() const {
    return m_baseDomain;
  }

  template<class DomainType>
  void Variable<DomainType>::specify(const AbstractDomain& domain){
    check_error(!domain.isDynamic() && !domain.isEmpty());
    check_error(!m_baseDomain.isDynamic());
    check_error(domain.isSubsetOf(m_specifiedDomain));

    // If this actually changes the domain then propagate the change to the derived domain
    if(m_specifiedDomain.intersect(domain)){
      if(m_specifiedDomain.isSingleton())
	m_derivedDomain.set(m_specifiedDomain.getSingletonValue());
      else
	m_derivedDomain.set(m_specifiedDomain);
    }
    check_error(isValid());
  }

  template <class DomainType>
  void Variable<DomainType>::specify(const double& singletonValue){
    check_error(!m_baseDomain.isDynamic());
    check_error(m_specifiedDomain.isMember(singletonValue));
    if(!m_specifiedDomain.isSingleton()){
      m_specifiedDomain.set(singletonValue);
      m_derivedDomain.set(singletonValue);
    }
    check_error(isValid());
  }

  template<class DomainType>
  void Variable<DomainType>::reset(){
    m_specifiedDomain.relax(m_baseDomain);
    m_derivedDomain.reset(m_baseDomain);
  }

  template<class DomainType>
  void Variable<DomainType>::close(){
    check_error(m_baseDomain.isDynamic());
    m_baseDomain.close();
    m_specifiedDomain.close();
    m_derivedDomain.close();

    // Now we should propagate specified domain to the derived domain as a set operation
    // since we have closed the domain
    if(m_specifiedDomain.isSingleton())
      m_derivedDomain.set(m_specifiedDomain.getSingletonValue());
    else
      m_derivedDomain.set(m_specifiedDomain);
  }

  template<class DomainType>
  void Variable<DomainType>::relax(){
    m_derivedDomain.relax(m_specifiedDomain);
  }
}
#endif
