#ifndef _H_TimePointWrapper
#define _H_TimePointWrapper

#include "TemporalNetwork.hh"
#include "TemporalNetworkDefs.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {

   /**
     * @class  TimepointWrapper
     * @brief Wraps a timepoint to manage deallocation in a staged manner that can be
     *        used to synchronize the propagator state.
     * @ingroup TemporalNetwork
    */

  class TimepointWrapper : public Entity {
  public:
    
    /**
     * @brief constructor 
     * @param prop Temporal propagator managing this timepoint
     * @param tempVar 
     * @param point
     */
    TimepointWrapper(const TemporalPropagatorId& prop, const ConstrainedVariableId& tempVar, const TimepointId& point);
 
    /**
     * @brief deconstructor
     */
    virtual ~TimepointWrapper();
    
    /**
     * @brief accessor for unique id of this TimepointWrapper
     * @return unique id
     */
    inline const EntityId& getId() { return m_id; }

    /**
     * @brief accessor for wrapped timepoint
     * @return timepoint
     */
    inline const TimepointId& getTimepoint() const {return m_timepoint;}

    /**
     * @brief accessor for constrained variable
     * @return constrained variable's id
     */
    inline const ConstrainedVariableId& getTempVar() const {return m_tempVar;}
  private:
    void handleDiscard();
    TemporalPropagatorId m_propagator;
    ConstrainedVariableId m_tempVar;
    TimepointId m_timepoint;
    EntityId m_id;
  };
}

#endif
