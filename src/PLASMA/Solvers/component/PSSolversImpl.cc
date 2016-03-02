
#include "PSSolversImpl.hh"
#include "Filters.hh"
#include "Solver.hh"
#include "Context.hh"
#include "tinyxml.h"

namespace EUROPA
{
  PSSolverManagerImpl::PSSolverManagerImpl(PlanDatabaseId pdb)
    : m_pdb(pdb)
  {
  }

  PSSolver* PSSolverManagerImpl::createSolver(const std::string& configurationFile)
  {
    TiXmlDocument* doc = new TiXmlDocument(configurationFile.c_str());
    doc->LoadFile();
    checkRuntimeError(!doc->Error(), doc->ErrorDesc());

    SOLVERS::SolverId solver =
    	(new SOLVERS::Solver(m_pdb, *(doc->RootElement())))->getId();
    return new PSSolverImpl(solver,configurationFile);
  }

  PSSolverImpl::PSSolverImpl(const SOLVERS::SolverId solver, const std::string& configFilename)
      : m_solver(solver)
      , m_configFile(configFilename)
  {
  }

  PSSolverImpl::~PSSolverImpl() {
    if(m_solver.isValid())
      destroy();
  }

  void PSSolverImpl::step() {
    m_solver->step();
  }

bool PSSolverImpl::solve(int maxSteps, int maxDepth) {
  return m_solver->solve(static_cast<unsigned int>(maxSteps),
                         static_cast<unsigned int>(maxDepth));
}

  bool PSSolverImpl::backjump(unsigned int stepCount) {
	return m_solver->backjump(stepCount);
  }

  void PSSolverImpl::reset() {
    m_solver->reset();
  }

  void PSSolverImpl::reset(unsigned int depth) {
     m_solver->reset(depth);
   }

  void PSSolverImpl::destroy() {
    delete static_cast<SOLVERS::Solver*>(m_solver);
    m_solver = SOLVERS::SolverId::noId();
  }

  int PSSolverImpl::getStepCount() {
    return static_cast<int>(m_solver->getStepCount());
  }

  int PSSolverImpl::getDepth() {
    return static_cast<int>(m_solver->getDepth());
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
    delete static_cast<Iterator*>(flawIt);
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

  eint::basis_type PSSolverImpl::getHorizonStart() {
    return static_cast<eint::basis_type>(m_solver->getContext()->get("horizonStart"));
  }

  eint::basis_type PSSolverImpl::getHorizonEnd() {
    return static_cast<eint::basis_type>(m_solver->getContext()->get("horizonEnd"));
  }

  void PSSolverImpl::configure(eint::basis_type horizonStart, eint::basis_type horizonEnd) {
    check_runtime_error(horizonStart <= horizonEnd);
    m_solver->getContext()->put("horizonStart", static_cast<double>(horizonStart));
    m_solver->getContext()->put("horizonEnd", static_cast<double>(horizonEnd));
  }

}
