#ifndef _H_TokenVariable
#define _H_TokenVariable

#include "Token.hh"
#include "PlanDatabaseDefs.hh"
#include "Variable.hh"
//#include "Token.hh"


/**
 * @file Provides implementation and interface for TokenVariables.
 * @author Conor McGann
 * @date December, 2003
 */
namespace EUROPA{

  /**
   * @brief Specializes Variables for a Token.
   *
   * Key things are:
   * - Include a parent token id
   * - Handle computation and synchronization of induced specified domains
   * - Handle special requirements for dealing with constraints under conditions of merging.
   */
  template <class DomainType>
  class TokenVariable : public Variable<DomainType> {
  public:
    TokenVariable(const TokenId& parent,
		  int index,
		  const ConstraintEngineId& constraintEngine,
		  const AbstractDomain& baseDomain,
		  const bool internal = false,
		  bool canBeSpecified = true,
		  const LabelStr& name = ConstrainedVariable::NO_NAME());

    virtual ~TokenVariable();

    void insert(edouble value);

    void remove(edouble value);

    void close();

    void specify(edouble singletonValue);

    void relax();

    void reset();

    void handleRestrictBaseDomain(const AbstractDomain& baseDomain);

    void handleBase(const AbstractDomain& domain);

    void handleSpecified(edouble value);

    void handleReset();

    bool isCompatible(const ConstrainedVariableId& var) const;

    const TokenId& getParentToken() const;

  private:
    // Internal methods for specification that circumvent test for canBeSpeciifed()
    friend class Token;
    void setSpecified(edouble singletonValue);
    void resetSpecified();

    bool computeBaseDomain();

    void handleConstraintAdded(const ConstraintId& constraint);

    void handleConstraintRemoved(const ConstraintId& constraint);

    DomainType* m_integratedBaseDomain; /**< The integrated base domain over this and all supported tokens. */
    bool m_isLocallySpecified;
    edouble m_localSpecifiedValue;
    const TokenId m_parentToken;
  };

  template <class DomainType>
  const TokenId& TokenVariable<DomainType>::getParentToken() const { return m_parentToken; }

  template <class DomainType>
  TokenVariable<DomainType>::TokenVariable(const TokenId& parent,
					   int index,
					   const ConstraintEngineId& constraintEngine,
					   const AbstractDomain& baseDomain,
					   const bool internal,
					   bool canBeSpecified,
					   const LabelStr& name)
    : Variable<DomainType>(constraintEngine, baseDomain, internal, canBeSpecified, name, parent, index),
      m_integratedBaseDomain(static_cast<DomainType*>(baseDomain.copy())), m_isLocallySpecified(false), m_localSpecifiedValue(0),
      m_parentToken(parent){
    check_error(m_parentToken.isValid());
    check_error(this->getIndex() >= 0);
  }

  template <class DomainType>
  TokenVariable<DomainType>::~TokenVariable()
  {
  	delete m_integratedBaseDomain;
  }

  template<class DomainType>
  void TokenVariable<DomainType>::insert(edouble value) {
    Variable<DomainType>::insert(value);
    this->m_integratedBaseDomain->insert(value);
  }

  template<class DomainType>
  void TokenVariable<DomainType>::remove(edouble value) {
    Variable<DomainType>::remove(value);
    if(this->m_integratedBaseDomain->isMember(value))
      this->m_integratedBaseDomain->remove(value);
  }

  template <class DomainType>
  void TokenVariable<DomainType>::close(){
    Variable<DomainType>::close();
    this->m_integratedBaseDomain->close();
  }

  template <class DomainType>
  void TokenVariable<DomainType>::specify(edouble singletonValue){
    check_error(this->canBeSpecified());
    setSpecified(singletonValue);
  }

  template <class DomainType>
  void TokenVariable<DomainType>::reset(){
    check_error(this->canBeSpecified());
    resetSpecified();
  }

  template <class DomainType>
  void TokenVariable<DomainType>::handleRestrictBaseDomain(const AbstractDomain& domain){
    Variable<DomainType>::handleRestrictBaseDomain(domain);

    if(this->m_integratedBaseDomain->isOpen() && domain.isClosed())
      this->m_integratedBaseDomain->close();

    this->m_integratedBaseDomain->intersect(domain);
  }

  template <class DomainType>
  void TokenVariable<DomainType>::setSpecified(edouble singletonValue){
    check_error(this->m_parentToken.isValid());
    checkError(!this->m_parentToken->isMerged(),
	       "Attempted to specify " << this->toString() <<
	       " of merged token " << this->m_parentToken->toString() << ". ");

    this->internalSpecify(singletonValue);

    m_isLocallySpecified = true;
    m_localSpecifiedValue = singletonValue;
  }


  template <class DomainType>
  void TokenVariable<DomainType>::resetSpecified(){
    // Clear flag indicating locally specified
    m_isLocallySpecified = false;

    bool shouldBeSpecified = computeBaseDomain();

    // If it should no longer be specified, relax it to the integrated base domain
    if(!shouldBeSpecified)
      Variable<DomainType>::reset(*(this->m_integratedBaseDomain));

    // Notify active token variable to recompute specified domain if necessary
    if(this->m_parentToken->isMerged()){
      check_error(this->m_parentToken->getActiveToken().isValid());
      this->m_parentToken->getActiveToken()->getVariables()[this->getIndex()]->handleReset();
    }
  }

  template <class DomainType>
  void TokenVariable<DomainType>::handleBase(const AbstractDomain& domain){
    this->m_integratedBaseDomain->intersect(domain);
    this->m_derivedDomain->intersect(domain);
  }

  template <class DomainType>
  void TokenVariable<DomainType>::handleSpecified(edouble value){
    check_error(this->m_parentToken->isActive());
    checkError(this->lastDomain().isMember(value), value << " is not in " << this->toString());

    // If not already specified, we can specify it as an implication of the supported token
    // being specified
    if(!this->isSpecified())
      Variable<DomainType>::specify(value);
  }

  template <class DomainType>
  void TokenVariable<DomainType>::handleReset(){
    // Recompute the base domain
    bool shouldBeSpecified = computeBaseDomain();

    // If it should be specified or it is locally specified then do nothing
    if(shouldBeSpecified || m_isLocallySpecified)
      return;

    // If it is already specified, reset it, otherwsie just relax it.
    if(this->isSpecified())
      Variable<DomainType>::reset(*(this->m_integratedBaseDomain));
    //this->m_derivedDomain->reset(*(this->m_integratedBaseDomain));
    else{
      // The integrated base domain refelxts the updated domain which includes the original base domain and this there is
      // no reason to relax twice.
      //this->m_derivedDomain->relax(*(this->m_baseDomain));
      this->m_derivedDomain->relax(*(this->m_integratedBaseDomain));
    }
  }

  template <class DomainType>
  bool TokenVariable<DomainType>::computeBaseDomain(){
    this->m_integratedBaseDomain->relax(*(this->m_baseDomain));
    bool shouldBeSpecified(false);
    edouble specifiedValue(0);

    const TokenSet& mergedTokens = this->m_parentToken->getMergedTokens();
    for(TokenSet::const_iterator it = mergedTokens.begin(); it != mergedTokens.end(); ++it){
      TokenId mergedToken = *it;
      check_error( mergedToken->isMerged());
      check_error(mergedToken->getActiveToken() == this->m_parentToken);
      check_error(mergedToken->getVariables().size() == this->m_parentToken->getVariables().size());
      const Id<TokenVariable<DomainType> >& var = mergedToken->getVariables()[this->getIndex()];

      // Update the base domain to include any restrictions on merged token. Note that if the base domain is open then intersection
      // will not restrict, However, that should be handled in the basic implementation of intersection
      this->m_integratedBaseDomain->intersect(var->baseDomain());

      checkError(var->baseDomain().isOpen() || !this->m_integratedBaseDomain->isEmpty(), var->toString() << " cannot merge.");

      // if not specified, ignore it
      if(!var->isSpecified())
	continue;

      checkError(!shouldBeSpecified || specifiedValue == var->lastDomain().getSingletonValue(), var->toString());

      // If any are specified than all are specified.
      if(!shouldBeSpecified){
	shouldBeSpecified = true;
	specifiedValue = var->lastDomain().getSingletonValue();
      }
    }

    return shouldBeSpecified;
  }

  template <class DomainType>
  void TokenVariable<DomainType>::handleConstraintAdded(const ConstraintId& constraint){
    // Not valid to add a constraint if token is rejected
    //check_error(!this->m_parentToken->isRejected());

    // If the token has been merged, the new constraint should be migrated
    if(this->m_parentToken->isMerged()){
      checkError(!constraint->isActive(), "Constraint should be inactive if it is on a merged token.");
      this->m_parentToken->handleAdditionOfInactiveConstraint(constraint);
    }
  }

  template <class DomainType>
  void TokenVariable<DomainType>::handleConstraintRemoved(const ConstraintId& constraint){
    // If the Token has been merged, then have to handle the migrated constraint also. For
    // Example, suppose a constraint was posted on the token.
    if(this->m_parentToken->isMerged() && !constraint->isActive())
      this->m_parentToken->handleRemovalOfInactiveConstraint(constraint);
  }

  template<class DomainType>
  bool TokenVariable<DomainType>::isCompatible(const ConstrainedVariableId& var) const {
    Id<TokenVariable <DomainType> > id(var);

    if(id.isNoId() || this->m_index != id->getIndex() || TokenVariable<DomainType>::getBaseDomain() != id->getBaseDomain())
      return false;
    else
      return true;
  }

  template<class DomainType>
  void TokenVariable<DomainType>::relax() {
    Variable<DomainType>::relax();

    // Construct the relaxation domain based on the integrated base domain and the specified value. We do this to avoid introducing a restriction after
    // first relaxing. There is code in the constraint engine that assumes that we do not impose restrictiosn during relaxation. Moreover, we shouldn't as
    // we open up the possibility that restriction and relaxation are interleaved. Thus, we structure this to compute the target domain to relax to and then
    // relax to that in one go.
    AbstractDomain* dom = m_integratedBaseDomain->copy();
    if(this->isSpecified())
      dom->set(this->getSpecifiedValue());

    this->m_derivedDomain->relax(*dom);
    delete dom;
  }

}
#endif
