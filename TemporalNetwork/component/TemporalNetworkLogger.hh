#ifndef _H_TemporalNetworkLogger
#define _H_TemporalNetworkLogger

#include "TemporalNetworkDefs.hh"
#include "TemporalNetworkListener.hh"

namespace Prototype{

  /**
   * @class TemporalNetworkListener
   * @brief The abstract interface for events published from the temporal network
   */
  class TemporalNetworkLogger: public TemporalNetworkListener {

  public:
    TemporalNetworkLogger(const TemporalPropagatorId& prop, std::ostream& os);
    virtual ~TemporalNetworkLogger();
    virtual void notifyTimepointAdded(const TempVarId& var, const TimepointId& timepoint);
    virtual void notifyTimepointDeleted(const TimepointId& timepoint);
    virtual void notifyBaseDomainConstraintAdded(const TempVarId& c, const TemporalConstraintId& constraint, Time lb, Time ub);
    virtual void notifyConstraintAdded(const ConstraintId c, const TemporalConstraintId& constraint, Time lb, Time ub);
    virtual void notifyConstraintDeleted(int key,  const TemporalConstraintId& constraint);
    virtual void notifyBoundsRestricted(const TempVarId& v, Time newlb, Time newub);
    virtual void notifyBoundsSame(const TempVarId& v,  const TimepointId& timepoint);


  protected:
    static std::string s_loggername;

  private:
    std::ostream& m_os;

  };
}
#endif
