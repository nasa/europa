#include "STNTemporalAdvisor.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "TokenTemporalVariable.hh"
#include "PlanDatabase.hh"
#include "../TemporalNetwork/TemporalNetworkDefs.hh"
#include "../TemporalNetwork/TemporalPropagator.hh"

namespace Prototype {

  STNTemporalAdvisor::STNTemporalAdvisor(const PropagatorId& propagator): m_propagator(propagator), m_id(this){
    check_error (TemporalPropagatorId::convertable(propagator));
    //    std::cout << "STN Advisor being constructed" << m_id << std::endl;
}

  STNTemporalAdvisor::~STNTemporalAdvisor(){
    //    std::cout << "STN Advisor being destroyed" << m_id << std::endl;
    m_id.remove();
  }

  bool STNTemporalAdvisor::canPrecede(const TokenId& first, const TokenId& second){    
    return ((TemporalPropagatorId)m_propagator)->canPrecede(first->getEnd(), second->getStart());
  }

  bool STNTemporalAdvisor::canFitBetween(const TokenId& token, const TokenId& predecessor, const TokenId& successor){
    return ((TemporalPropagatorId)m_propagator)->canFitBetween(token->getStart(), token->getEnd(), predecessor->getEnd(), successor->getStart());
  }

  const TemporalAdvisorId& STNTemporalAdvisor::getId() const {return m_id;}

}
