#include "STNTemporalAdvisor.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "TemporalNetworkDefs.hh"
#include "TemporalPropagator.hh"

namespace Prototype {

  STNTemporalAdvisor::STNTemporalAdvisor(const TemporalPropagatorId& propagator): m_propagator(propagator) {
}

  STNTemporalAdvisor::~STNTemporalAdvisor(){
  }

  bool STNTemporalAdvisor::canPrecede(const TokenId& first, const TokenId& second){    
    if (!DefaultTemporalAdvisor::canPrecede(first, second))
      return false;

    bool retval = m_propagator->canPrecede(first->getEnd(), second->getStart());
    return (retval);
  }

  bool STNTemporalAdvisor::canFitBetween(const TokenId& token, const TokenId& predecessor, const TokenId& successor){
    if (!DefaultTemporalAdvisor::canFitBetween(token, predecessor, successor))
      return false;
    return m_propagator->canFitBetween(token->getStart(), token->getEnd(), predecessor->getEnd(), successor->getStart());
  }

  bool STNTemporalAdvisor::canBeConcurrent(const TokenId& first, const TokenId& second){
    if(m_propagator->canBeConcurrent(first->getStart(), second->getStart()) &&
       m_propagator->canBeConcurrent(first->getEnd(), second->getEnd()))
      return true;
    else
      return false;
  }
}
