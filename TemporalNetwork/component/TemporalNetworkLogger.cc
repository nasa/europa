#include "TemporalNetworkLogger.hh"
#include "Constraint.hh"
#include "TokenVariable.hh"
#include "IntervalIntDomain.hh"

namespace EUROPA{

  std::string TemporalNetworkLogger::s_loggername = std::string("TNLogger :");  

  TemporalNetworkLogger::TemporalNetworkLogger(const TemporalPropagatorId& prop, std::ostream& os ) : TemporalNetworkListener(prop), m_os(os) {}

  TemporalNetworkLogger::~TemporalNetworkLogger() {}

  void TemporalNetworkLogger::notifyTimepointAdded(const TempVarId& var, const TimepointId& timepoint) { 
    m_os << s_loggername 
	 << "TIMEPOINT " << timepoint->getKey() << " ADDED for variable " << var->getKey() << std::endl;
  }
  void TemporalNetworkLogger::notifyTimepointDeleted(const TimepointId& timepoint) { 
    m_os << s_loggername 
	 << "TIMEPOINT " << timepoint->getKey() << " DELETED " << std::endl;
  }
  void TemporalNetworkLogger::notifyConstraintAdded(const ConstraintId constraint, const TemporalConstraintId& c, Time lb, Time ub) {
    m_os << s_loggername 
	 << "Constraint ADDED " << constraint->getName().toString() << "(" <<  constraint->getKey() << ") - [" << c << "] " 
	 << " --[" << lb << "," << ub << "]--> " << std::endl;
  }
						    
  void TemporalNetworkLogger::notifyBaseDomainConstraintAdded(const TempVarId& var, const TemporalConstraintId& constraint, Time lb, Time ub) {
    m_os << s_loggername 
	 << "Constraint ADDED Base Domain for Variable " << var->getKey() <<  "(" <<  constraint << ") "
	 << " -[" << lb << "," << ub << "]-" << std::endl; 
  }
  void TemporalNetworkLogger::notifyConstraintDeleted(int key, const TemporalConstraintId& constraint ) {
        m_os << s_loggername 
	     << "Constraint " << key <<  " DELETED " << std::endl;
  }
  void TemporalNetworkLogger::notifyBoundsRestricted(const TempVarId& var, Time lb, Time ub) {
        m_os << s_loggername 
	     <<"Bounds of "  << var->getKey() << " Restricted to -[" << lb << "," << ub << "]-" << std::endl; 
  }

  void TemporalNetworkLogger::notifyBoundsSame(const TempVarId& var,  const TimepointId& timepoint) {
            m_os << s_loggername 
		 << " Bounds SAME " << timepoint->getKey() << " == " << var->getKey() << std::endl;
  }

}

