#ifndef _H_PlannerControlAssembly
#define _H_PlannerControlAssembly

/**
 * @file   PlannerControlAssembly.hh
 * @author Patrick Daley
 * @date   
 * @brief  
 * @ingroup System
 */

#include "PlanDatabaseDefs.hh"
#include "CBPlanner.hh"
#include "StandardAssembly.hh"
#include "PartialPlanWriter.hh"
#include "DbClientTransactionLog.hh"

namespace EUROPA {

#define PPW_WITH_PLANNER

  class PlannerControlAssembly : public StandardAssembly {
  public:
    PlannerControlAssembly(const SchemaId& schema);
    virtual ~PlannerControlAssembly();

    /**
     * @brief Invoke the planner initialization. Calls playTransactions(txSource).
     * This does not run the planner to completion. Planning is run by Planner Control.
     * Cleenup of planner and ppw is also performed by Planner Control.
     *
     * @param txSource The source from which we get the initial state
     * @return The result of planner initialization
     * @see CBPlanner::Status
     */
    CBPlanner::Status initPlan(const char* txSource);


    const CBPlannerId& getPlanner() const { return m_planner; }

    PlanWriter::PartialPlanWriter* getWriter() const { return m_ppw; }

    /**
     * The following method is used by the PlannerControl interface for PlanWorks.
     */
    const HorizonId& getHorizon() const { return m_horizon; }

  protected:
    CBPlannerId m_planner;	/*!< A planner to complete the plan. */
    PlanWriter::PartialPlanWriter* m_ppw; /*!< A plan writer to output data for PlanWorks */
    HorizonId m_horizon;      /*!< A horizon for the plan. */
  };
}
#endif
