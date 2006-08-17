#ifndef _H_CBPLANNER
#define _H_CBPLANNER

/**
 * @file   CBPlanner.hh
 * @author Tania Bedrax-Weiss <tania@email.arc.nasa.gov>
 * @date   Mon Dec 27 16:58:39 2004
 * @brief  Substantially re-written by Conor McGann
 * @ingroup CBPlanner
 */

#include "CBPlannerDefs.hh"

namespace EUROPA {

  class CBPlanner {
  public:

    enum Status { IN_PROGRESS=0,
		  TIMEOUT_REACHED,
		  PLAN_FOUND,
		  SEARCH_EXHAUSTED,
		  INITIALLY_INCONSISTENT
    };


    /**
     * @brief Constructor with Default Open Decision Manager
     * @param database The plan database
     * @param hor The planning horizon
     */
    CBPlanner(const PlanDatabaseId& database, const HorizonId& hor);

    /**
     * @brief Constructor with Custom Open Decision Manager
     * @param database The plan database
     * @param hor The planning horizon
     */
    CBPlanner(const PlanDatabaseId& database, const HorizonId& hor, const OpenDecisionManagerId& odm);

    virtual ~CBPlanner();

    const CBPlannerId& getId() const;

    virtual Status run(const unsigned int timeout = 100000);

    virtual Status step();

    void setTimeout(const unsigned int timeout);
    const unsigned int getTimeout() const;

    const unsigned int getTime() const;
    const unsigned int getDepth() const;
    const unsigned int getStep() const;
    const DecisionStack& getClosedDecisions() const;
    DecisionManagerId& getDecisionManager();

    void setNecessarilyOutsideHorizon();
    void setPossiblyOutsideHorizon();
    bool isPossiblyOutsideHorizon();
    bool isNecessarilyOutsideHorizon();

    void disableDynamicExclusion();
    bool isDynamicExclusionEnabled();


    /**
     * @brief Reset the search state data without rolling back any decisions
     * @see retract
     */
    void reset();

    /**
     * @brief Reset the search state data and roll back all commitments.
     * @see reset
     */
    void retract();

    /*
     * Entry points for JNI Adapter
     */
    virtual Status initRun(const unsigned int timeout = 100000);
    virtual Status getStatus();
    virtual unsigned int writeStep(unsigned int step_num);
    virtual unsigned int writeNext(unsigned int num_steps);
    virtual unsigned int completeRun();

  protected:
    virtual bool timeoutReached();
    virtual bool makeMove();
    virtual bool retractMove();
    virtual void stepBreakpoint();

    /**
     * @brief Helper method to factor common initialization code
     */
    void commonInit(const HorizonId& horizon, const OpenDecisionManagerId& odm);

    HorizonConditionId m_horizonCondition;
    DynamicInfiniteRealConditionId m_dynamicInfiniteRealCondition;

    PlanDatabaseId m_db;

    unsigned int m_timeout;	/**< number of nodes limit */
    unsigned int m_time;		/**< keeps track of the number of nodes */
    unsigned int m_step;		/**< keeps track of current step */
    Status m_status;                    /**< keeps track of current status */

    DecisionManagerId m_decisionManager;
    CBPlannerId m_id;
    const bool m_allocatedOpenDecisionManager; /*!< Tracks if we allocate the default so we can deallocate it */
  };

}

#endif
