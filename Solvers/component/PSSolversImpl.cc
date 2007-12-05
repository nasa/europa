
#include "PSSolversImpl.hh"
#include "Filters.hh"
#include "Solver.hh"
#include "SolverPartialPlanWriter.hh"

namespace EUROPA 
{
  PSSolverManagerImpl::PSSolverManagerImpl(ConstraintEngineId ce,PlanDatabaseId pdb,RulesEngineId re)
    : m_planDatabase(pdb)
  {
    m_ppw = new SOLVERS::PlanWriter::PartialPlanWriter(pdb,ce,re);	  
  }
  
  PSSolverManagerImpl::~PSSolverManagerImpl()
  {
	delete m_ppw;	  
  }
  
  PSSolver* PSSolverManagerImpl::createSolver(const std::string& configurationFile) 
  {
    TiXmlDocument* doc = new TiXmlDocument(configurationFile.c_str());
    doc->LoadFile();

    SOLVERS::SolverId solver =
    	(new SOLVERS::Solver(m_planDatabase, *(doc->RootElement())))->getId();
    return new PSSolverImpl(solver,configurationFile, m_ppw);
  }

  PSSolverImpl::PSSolverImpl(const SOLVERS::SolverId& solver, const std::string& configFilename,
		     SOLVERS::PlanWriter::PartialPlanWriter* ppw) 
      : m_solver(solver) 
      , m_configFile(configFilename),
	m_ppw(ppw)
  {
    m_ppw->setSolver(m_solver);
  }

  PSSolverImpl::~PSSolverImpl() {
    if(m_solver.isValid())
      destroy();
  }

  void PSSolverImpl::step() {
    m_solver->step();
  }

  void PSSolverImpl::solve(int maxSteps, int maxDepth) {
    m_solver->solve(maxSteps, maxDepth);
  }

  void PSSolverImpl::reset() {
    m_solver->reset();
  }

  void PSSolverImpl::destroy() {
    m_ppw->clearSolver();
    delete (SOLVERS::Solver*) m_solver;
    m_solver = SOLVERS::SolverId::noId();
  }

  int PSSolverImpl::getStepCount() {
    return (int) m_solver->getStepCount();
  }

  int PSSolverImpl::getDepth() {
    return (int) m_solver->getDepth();
  }

  bool PSSolverImpl::isExhausted() {
    return m_solver->isExhausted();
  }

  bool PSSolverImpl::isTimedOut() {
    return m_solver->isTimedOut();
  }

  bool PSSolverImpl::isConstraintConsistent() {
    return m_solver->isConstraintConsistent();
  }

  bool PSSolverImpl::hasFlaws() {
    return !m_solver->noMoreFlaws();
  }

  int PSSolverImpl::getOpenDecisionCnt() {
    int count = 0;
    IteratorId flawIt = m_solver->createIterator();
    while(!flawIt->done()) {
      count++;
      flawIt->next();
    }
    delete (Iterator*) flawIt;
    return count;
  }

  PSList<std::string> PSSolverImpl::getFlaws() {
    PSList<std::string> retval;

    /*    
    IteratorId flawIt = m_solver->createIterator();
    while(!flawIt->done()) {
      EntityId entity = flawIt->next();
      std::string flaw = entity->toString();
      retval.push_back(flaw);
    }
    delete (Iterator*) flawIt;
    */  

    std::multimap<SOLVERS::Priority, std::string> priorityQueue = m_solver->getOpenDecisions();
    for(std::multimap<SOLVERS::Priority, std::string>::const_iterator it=priorityQueue.begin();it!=priorityQueue.end(); ++it) {
        std::stringstream os;      
        os << it->second << " PRIORITY==" << it->first; 
        retval.push_back(os.str());
    }
    
    return retval;
  }

  std::string PSSolverImpl::getLastExecutedDecision() {
    return m_solver->getLastExecutedDecision();
  }

  const std::string& PSSolverImpl::getConfigFilename() {return m_configFile;}

  int PSSolverImpl::getHorizonStart() {
    return (int) SOLVERS::HorizonFilter::getHorizon().getLowerBound();
  }

  int PSSolverImpl::getHorizonEnd() {
    return (int) SOLVERS::HorizonFilter::getHorizon().getUpperBound();
  }

  void PSSolverImpl::configure(int horizonStart, int horizonEnd) {
    check_runtime_error(horizonStart <= horizonEnd);
    SOLVERS::HorizonFilter::getHorizon().reset(IntervalIntDomain());
    SOLVERS::HorizonFilter::getHorizon().intersect(horizonStart, horizonEnd);
  }

}
