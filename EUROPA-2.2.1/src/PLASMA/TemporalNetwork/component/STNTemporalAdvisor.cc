#include "STNTemporalAdvisor.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "TemporalNetworkDefs.hh"
#include "TemporalPropagator.hh"
#include "Debug.hh"
#include "Utils.hh"

namespace EUROPA {

  STNTemporalAdvisor::STNTemporalAdvisor(const TemporalPropagatorId& propagator)
    : DefaultTemporalAdvisor(propagator->getConstraintEngine()), m_propagator(propagator) {}

  STNTemporalAdvisor::~STNTemporalAdvisor(){}

  bool STNTemporalAdvisor::canPrecede(const TokenId& first, const TokenId& second){    
    if (!DefaultTemporalAdvisor::canPrecede(first, second))
      return false;

    bool retval = m_propagator->canPrecede(first->end(), second->start());
    return (retval);
  }

  bool STNTemporalAdvisor::canPrecede(const TimeVarId& first, const TimeVarId& second) {
    if(!DefaultTemporalAdvisor::canPrecede(first, second))
      return false;
    return m_propagator->canPrecede(first, second);
  }

  bool STNTemporalAdvisor::canFitBetween(const TokenId& token, const TokenId& predecessor, const TokenId& successor){
    if (!DefaultTemporalAdvisor::canFitBetween(token, predecessor, successor))
      return false;
    return m_propagator->canFitBetween(token->start(), token->end(), predecessor->end(), successor->start());
  }

  /**
   * @brief 2 tokens can be concurrent if the temporal distance between them can be 0
   */
  bool STNTemporalAdvisor::canBeConcurrent(const TokenId& first, const TokenId& second){
    debugMsg("STNTemporalAdvisor:canBeConcurrent", "first [" << first->start() << ", " << first->end() << "]");
    debugMsg("STNTemporalAdvisor:canBeConcurrent", "second[" << second->start() << ", " << second->end() << "]"); 

   return (m_propagator->canBeConcurrent(first->start(), second->start()) &&
	    m_propagator->canBeConcurrent(first->end(), second->end()));
  }

  /**
   * @brief Gets the temporal distance between two temporal variables. 
   * @param exact if set to true makes this distance calculation exact.
   */
  const IntervalIntDomain STNTemporalAdvisor::getTemporalDistanceDomain(const TimeVarId& first, const TimeVarId& second, const bool exact) {
    if( first->getExternalEntity().isNoId() 
	||
	second->getExternalEntity().isNoId() )
      {
	int f_lb = (int) first->getLastDomain().getLowerBound();
	int f_ub = (int) first->getLastDomain().getUpperBound();
	
	int s_lb = (int) second->getLastDomain().getLowerBound();
	int s_ub = (int) second->getLastDomain().getUpperBound();
	
	int min_distance = -g_infiniteTime();

	if( s_lb > -g_infiniteTime() && f_ub < g_infiniteTime() ) {
	    min_distance = std::max( min_distance, s_lb - f_ub );
	  }
	  
	int max_distance = g_infiniteTime();
	
	if( f_lb > -g_infiniteTime() && s_ub < g_infiniteTime() ) {
	  max_distance = std::min( max_distance, s_ub - f_lb );
	  }

	return(IntervalIntDomain( min_distance, max_distance ));
      }

    return (m_propagator->getTemporalDistanceDomain(first, second, exact));
  }

  unsigned int STNTemporalAdvisor::mostRecentRepropagation() const{
    return m_propagator->mostRecentRepropagation();
  }
}
