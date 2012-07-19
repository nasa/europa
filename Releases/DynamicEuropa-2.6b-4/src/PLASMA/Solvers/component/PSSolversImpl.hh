#ifndef _H_PSSolversImpl
#define _H_PSSolversImpl

#include "PSSolvers.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"
#include "SolverDefs.hh"

namespace EUROPA
{
  class PSSolverManagerImpl : public PSSolverManager
  {
    public:
      PSSolverManagerImpl(PlanDatabaseId pdb);

      virtual PSSolver* createSolver(const std::string& configurationFile);

    protected:
      PlanDatabaseId m_pdb;
  };

  class PSSolverImpl : public PSSolver
  {
    public:
      PSSolverImpl(const SOLVERS::SolverId& solver,
    		       const std::string& configFilename);
      virtual ~PSSolverImpl();

      virtual void step();
      virtual void solve(int maxSteps,int maxDepth);
      virtual bool backjump(unsigned int stepCount);
      virtual void reset();
      virtual void reset(unsigned int depth);
      virtual void destroy();

      virtual int getStepCount();
      virtual int getDepth();
      virtual int getOpenDecisionCnt();

      virtual bool isExhausted();
      virtual bool isTimedOut();
      virtual bool isConstraintConsistent();

      virtual bool hasFlaws();
      virtual PSList<std::string> getFlaws();
      virtual std::string getLastExecutedDecision();

      virtual const std::string& getConfigFilename();
    virtual eint::basis_type getHorizonStart();
    virtual eint::basis_type getHorizonEnd();

    virtual void configure(eint::basis_type horizonStart, eint::basis_type horizonEnd);

    protected:
    	SOLVERS::SolverId m_solver;
    	std::string m_configFile;
  };

}

#endif

