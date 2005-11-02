#ifndef _H_PlannerControlAssembly
#define _H_PlannerControlAssembly

/**
 * @file   PlannerControlAssembly.hh
 * @author Patrick Daley & Michael Iatauro
 * @date   
 * @brief  
 * @ingroup System
 */

#include "PlanDatabaseDefs.hh"
#include "Solver.hh"
#include "SearchListener.hh"
#include "SolverPartialPlanWriter.hh"
#include "StandardAssembly.hh"
#include "DbClientTransactionLog.hh"
#include <fstream>

namespace EUROPA {

#define PPW_WITH_PLANNER

  enum PlannerStatus { IN_PROGRESS=0,
                       TIMEOUT_REACHED,
                       PLAN_FOUND,
                       SEARCH_EXHAUSTED,
                       INITIALLY_INCONSISTENT
  };

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
    PlannerStatus initPlan(const char* txSource, const char* plannerConfig, const char* destPath);


    const SOLVERS::SolverId& getPlanner() const { return m_planner; }

    SOLVERS::PlanWriter::PartialPlanWriter* getWriter() const { return m_ppw; }

    const int getPlannerStatus() const;

    int writeStep(int step);

    int writeNext(int n);
    
    int completeRun();
    
    int terminateRun();

    void enableDebugMessage(const char* file, const char* pattern);
    void disableDebugMessage(const char* file, const char* pattern);

  protected:
    SOLVERS::SolverId m_planner;	/*!< A planner to complete the plan. */
    SOLVERS::SearchListenerId m_listener;
    SOLVERS::PlanWriter::PartialPlanWriter* m_ppw; /*!< A plan writer to output data for PlanWorks */
    int m_step;
    std::ofstream* m_debugStream;
    

    class StatusListener : public SOLVERS::SearchListener {
    public:
      StatusListener(SOLVERS::SolverId solver) : SearchListener(), m_solver(solver), m_status(IN_PROGRESS) {solver->addListener(getId());}
      ~StatusListener() {m_solver->removeListener(getId());}
      const PlannerStatus getStatus() const {return m_status;}
      void notifyStepSucceeded() {m_status = IN_PROGRESS;}
      void notifyCompleted() {m_status = PLAN_FOUND;}
      void notifyExhausted() {m_status = SEARCH_EXHAUSTED;}
      void notifyTimedOut() {m_status = TIMEOUT_REACHED;}
    protected:
    private:
      SOLVERS::SolverId m_solver;
      PlannerStatus m_status;
    };
  };
}
#endif
