#include "Utils.hh"
#include "CBPlanner.hh"
#include "HorizonCondition.hh"
#include "TemporalVariableFilter.hh"
#include "DynamicInfiniteRealCondition.hh"
#include "PlanDatabase.hh"
#include "DecisionManager.hh"
#include "OpenDecisionManager.hh"
#include "PlanDatabaseWriter.hh"
#include "PartialPlanWriter.hh"
#include "Debug.hh"

#include <sstream>

/**
 * Migration Notes:
 * - Constructors should assume a specific configuration file name, and be extended to specialize that.
 * - Constructors should allocate a xml config document and base it to base class Solver
 * - Instead of allocating a decision manager, it will be created in the solver configuration
 * - getClosedDecisions will delegate to getDecisionStack
 * - Horizon setters will update underlying horizon policy. They should be deprecated in favour of configuration.
 * - Dynamic Exclusion will be handled as a configuration option
 */
namespace EUROPA {

  std::string toString(const PlanDatabaseId& db){
    std::stringstream sstr;
    PlanDatabaseWriter::write(db, sstr);
    return sstr.str();
  }

  CBPlanner::CBPlanner(const PlanDatabaseId& database, const HorizonId& hor) 
    : m_db(database), m_id(this), m_allocatedOpenDecisionManager(true) {
    commonInit(hor, (new OpenDecisionManager(m_db))->getId());
  }

  CBPlanner::CBPlanner(const PlanDatabaseId& database, const HorizonId& hor, const OpenDecisionManagerId& odm) 
    : m_db(database), m_id(this), m_allocatedOpenDecisionManager(false) {
    commonInit(hor, odm);
  }

  CBPlanner::~CBPlanner() { 
    check_error(m_horizonCondition.isValid());
    delete (HorizonCondition*) m_horizonCondition;
    check_error(m_dynamicInfiniteRealCondition.isValid());
    delete (DynamicInfiniteRealCondition*) m_dynamicInfiniteRealCondition;

    // If we allocated the default then we must delete it!
    if(m_allocatedOpenDecisionManager){
      OpenDecisionManagerId odm = m_decisionManager->getOpenDecisionManager();
      check_error(odm.isValid());
      delete (OpenDecisionManager*) odm;
    }

    check_error(m_decisionManager.isValid());
    delete (DecisionManager*) m_decisionManager;

    m_id.remove();
  }

  const CBPlannerId& CBPlanner::getId() const {
    return(m_id);
  }

  void CBPlanner::setTimeout(const unsigned int timeout) {
    m_timeout = timeout;
  }

  const unsigned int CBPlanner::getTimeout() const {
    return(m_timeout);
  }

  const unsigned int CBPlanner::getTime() const {
    return(m_time);
  }


  const unsigned int CBPlanner::getDepth() const {
    return(m_decisionManager->getClosedDecisions().size());
  }

  const unsigned int CBPlanner::getStep() const {
    return(m_step);
  }

  const DecisionStack& CBPlanner::getClosedDecisions() const {
    return(m_decisionManager->getClosedDecisions());
  }

  DecisionManagerId& CBPlanner::getDecisionManager() {
    return(m_decisionManager);
  }

  void CBPlanner::setNecessarilyOutsideHorizon() {
    m_horizonCondition->setNecessarilyOutsideHorizon();
  }
  
  void CBPlanner::setPossiblyOutsideHorizon() {
    m_horizonCondition->setPossiblyOutsideHorizon();
  }

  bool CBPlanner::isNecessarilyOutsideHorizon() {
    return m_horizonCondition->isPossiblyOutsideHorizon();
  }

  bool CBPlanner::isPossiblyOutsideHorizon() {
    return m_horizonCondition->isNecessarilyOutsideHorizon();
  }

  void CBPlanner::disableDynamicExclusion() {
    m_dynamicInfiniteRealCondition->disableDynamicExclusion();
  }

  bool CBPlanner::isDynamicExclusionEnabled() {
    return m_dynamicInfiniteRealCondition->isDynamicExclusionEnabled(); 
  }


  void CBPlanner::reset() {
    m_time = 0;
    m_decisionManager->reset();
  }

  void CBPlanner::retract() {
    m_time = 0;
    m_decisionManager->retract();
  }

  CBPlanner::Status CBPlanner::step() {
    /*
      Make sure that we haven't exceeded the node limit or timeout
      imposed.  Only makes sense if we're not retracting.
    */
    if (timeoutReached())
      return(TIMEOUT_REACHED);

    stepBreakpoint(); //provides a known place for gdb to break on a step

    /* We should never be in this part of the control loop if we are retracting decisions */
    check_error(!m_decisionManager->isRetracting());

    m_db->getClient()->propagate();

    /* We should always be consistent since we have no way to repair a problem that
       is initially inconsistent. If some other change occurs on the database and leaves it in an 
       inconsistent state, then it is an error. That other client should back out the change or nuke this planner.
       We can test this here since synchronization always causes propagation, allowing a const check.
    */
    check_error(m_db->getConstraintEngine()->constraintConsistent());


    // Make next move and process results
    debugMsg("CBPlanner:step", '[' << getDepth() << ',' << m_time << ']' << std::endl << 
             getDecisionManager()->getOpenDecisionManager()->printOpenDecisions());

    // Try to make a move. If it fails and we are retracting, then handle the retraction 
    bool stepSucceeded = makeMove();
    condDebugMsg(stepSucceeded, "CBPlanner:printPlan:infrequent", std::endl << PlanDatabaseWriter::toString(m_db));

    if (!stepSucceeded){
      if(m_decisionManager->isRetracting()) {
        debugMsg("CBPlanner:step:retract:start",
                 "[" << getDepth() << "," << m_time << "] " <<
                 "Before Retracting" << m_decisionManager->getCurrentDecision() << std::endl);

        /*
          If we're here, it means that we've reached a leaf node that is
          not a solution. 
          If we're here, it means that we can proceed to retract the last
          decision and choice made.
        */
        while (m_decisionManager->isRetracting() && m_decisionManager->hasDecisionToRetract()) {
          retractMove();
          // If still have to retract, log
          condDebugMsg(m_decisionManager->isRetracting(),
                       "CBPlanner:step:retract:each", 
                       "DecisionPoint exhausted; backtracking past decision");
          //debugMsg("CBPlanner:printPlan", std::endl << PlanDatabaseWriter::toString(m_db));
        }
        //debugMsg("CBPlanner:printPlan:infrequent", std::endl << PlanDatabaseWriter::toString(m_db));
      }
      else {
        m_decisionManager->plannerSearchFinished();
        debugMsg("CBPlanner:step:found", 
                 "Found a plan at depth " << getDepth() << " after " << m_time << " nodes.");
        return(PLAN_FOUND);
      }
    }

    if (m_decisionManager->isRetracting()) {
      // If still retracting, it means we have backed out everything there is without success
      debugMsg("CBPlanner:step:exhausted", "Search Exhausted. No plan found.");
      return(SEARCH_EXHAUSTED);
    }

    return(IN_PROGRESS);
  }

  CBPlanner::Status CBPlanner::run(const unsigned int timeout) {

    if (!m_db->isClosed())
      std::cerr << "WARNING: PlanDatabase is not closed and behavior may be unexpected." << std::endl;

    if (!m_db->getClient()->propagate()) {
      debugMsg("CBPlanner:run","Initial plan is inconsistent; building plan failed.");
      return(INITIALLY_INCONSISTENT);
    }

    // Reset flags
    m_time = 0;
    m_step = 0;
    getDecisionManager()->reset();

    m_timeout = timeout;

    CBPlanner::Status result = step();
    while (result == IN_PROGRESS)
      result = step();
    return(result);
  }

  bool CBPlanner::timeoutReached() {
    check_error(m_time <= m_timeout);
    if (m_timeout < 0 || m_time < m_timeout)
      return(false);
    m_decisionManager->plannerTimeoutReached();
    debugMsg("CBPlanner:timeoutReached", '[' << getDepth() << ',' << m_time << "]\n" << EUROPA::toString(m_db));
    return(true);
  }
  
  /* for gdb symbolic breakpoint tag only */
  void CBPlanner::stepBreakpoint() {
    m_step++;
  }

  bool CBPlanner::makeMove() {
    bool success = m_decisionManager->assignDecision();
    // If we succeded, or we failed due to a backtrack, then increment the time.
    if(success || m_decisionManager->isRetracting())
      m_time++;

    return(success);
  }

  bool CBPlanner::retractMove() {
    unsigned int retractionCount(0);
    bool retVal = m_decisionManager->retractDecision(retractionCount);
    return(retVal);
  }

  /*
   * Entry points for Planner Control Client
   */
  CBPlanner::Status CBPlanner::initRun(const unsigned int timeout) {
    m_timeout = timeout;
    PlanWriter::PartialPlanWriter::noFullWrite = 1;
    PlanWriter::PartialPlanWriter::writeStep = 1;
    if (!m_db->getClient()->propagate()) {
      debugMsg("CBPlanner:initRun", "Initial plan is inconsistent; building plan failed.");
      return(m_status = INITIALLY_INCONSISTENT);
    }

    return(m_status = IN_PROGRESS);
  }

  CBPlanner::Status CBPlanner::getStatus() {
    return(m_status);
  }

  unsigned int CBPlanner::writeStep(unsigned int step_num) {
    if (m_step || step_num) {
      //step 'til one before the requested step
      while (m_step < step_num) {
        m_status = step();
        if (m_status != IN_PROGRESS)
          return(m_step);
      }
    }
    // Now write the requested step.
    PlanWriter::PartialPlanWriter::noFullWrite = 0;
    m_status = step();
    PlanWriter::PartialPlanWriter::noFullWrite = 1;
    return(m_step);
  }

  unsigned int CBPlanner::writeNext(unsigned int num_steps) {
    unsigned int count = num_steps;
    PlanWriter::PartialPlanWriter::noFullWrite = 0;
    while (count) {
      m_status = step();
      if (m_status != IN_PROGRESS)
        return(m_step);
      count--;
    }
    PlanWriter::PartialPlanWriter::noFullWrite = 1;
    return(m_step);
  }

  unsigned int CBPlanner::completeRun() {
    PlanWriter::PartialPlanWriter::noFullWrite = 1;
    for (;;) { /* Forever: only way out is to return */
      m_status = step();
      if (m_status != IN_PROGRESS)
        return(m_step);
    }
    check_error(false);
    // must return something... but in theory we never get here
    return (m_step);
  }

  void CBPlanner::commonInit(const HorizonId& hor, const OpenDecisionManagerId& dm){
    m_timeout = 100000;
    m_time = 0;
    m_step = 0;
    m_decisionManager = (new DecisionManager(m_db, dm))->getId();
    m_horizonCondition = (new HorizonCondition(hor, m_decisionManager))->getId();  
    m_dynamicInfiniteRealCondition = (new DynamicInfiniteRealCondition(m_decisionManager))->getId();
  }
}
