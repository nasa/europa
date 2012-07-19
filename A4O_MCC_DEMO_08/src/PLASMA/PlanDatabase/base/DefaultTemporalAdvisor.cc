#include "DefaultTemporalAdvisor.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "Utils.hh"

/**
 * @author Sailesh Ramakrishnan
 * @date 2004
 */
namespace EUROPA {

  DefaultTemporalAdvisor::DefaultTemporalAdvisor(const ConstraintEngineId& ce): m_id(this), m_ce(ce){
    //    std::cout << "Default Advisor being constructed" << m_id << std::endl;
  }

  DefaultTemporalAdvisor::~DefaultTemporalAdvisor(){
    //    std::cout << "Default Advisor being destroyed" << m_id << std::endl;
    m_id.remove();
  }

  bool DefaultTemporalAdvisor::canPrecede(const TokenId& first, const TokenId& second){    
    //    std::cout << "DefaultTemporalAdvisor canPrecede (" << first->getKey() << ") and (" << second->getKey() << ")" << std::endl;

    int earliest_end = (int) first->end()->getDerivedDomain().getLowerBound();
    int latest_start = (int) second->start()->getDerivedDomain().getUpperBound();

    return (earliest_end <= latest_start);
  }

  bool DefaultTemporalAdvisor::canPrecede(const TimeVarId& first, const TimeVarId& second) {
    return (int) first->getDerivedDomain().getLowerBound() <= (int) second->getDerivedDomain().getUpperBound();
  }

  bool DefaultTemporalAdvisor::canFitBetween(const TokenId& token, const TokenId& predecessor, const TokenId& successor){
    check_error(token.isValid());
    check_error(predecessor.isValid());
    check_error(successor.isValid());
    check_error(successor != predecessor);
    check_error(token != successor);
    check_error(token != predecessor);

    int latest_start = (int) successor->start()->getDerivedDomain().getUpperBound();
    int earliest_end = (int) predecessor->end()->getDerivedDomain().getLowerBound();
    int min_duration = latest_start - earliest_end;

    if(min_duration >= token->duration()->getDerivedDomain().getLowerBound())
      return true;
    else 
      return false;
  }


  /**
   * @brief Trivially return true since basic domain intersection tests have been done in
   * the plan database already
   */
  bool DefaultTemporalAdvisor::canBeConcurrent(const TokenId& first, const TokenId& second){
    return true;
  }

  const TemporalAdvisorId& DefaultTemporalAdvisor::getId() const {return m_id;}

  /**
   * @brief Default return that the distance is infinite
   */
  const IntervalIntDomain DefaultTemporalAdvisor::getTemporalDistanceDomain(const TimeVarId& first, 
									    const TimeVarId& second,
									    const bool exact){ 
    if( first->getExternalEntity().isNoId() 
	||
	second->getExternalEntity().isNoId() )
      {
	int f_lb = (int) first->getDerivedDomain().getLowerBound();
	int f_ub = (int) first->getDerivedDomain().getUpperBound();
	
	int s_lb = (int) second->getDerivedDomain().getLowerBound();
	int s_ub = (int) second->getDerivedDomain().getUpperBound();
	
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

    return(IntervalIntDomain(-g_infiniteTime(), g_infiniteTime()));
  }

  unsigned int DefaultTemporalAdvisor::mostRecentRepropagation() const {
    return m_ce->mostRecentRepropagation();
  }
}
