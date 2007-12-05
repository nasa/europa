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
      PSSolverManagerImpl(ConstraintEngineId ce,PlanDatabaseId pdb,RulesEngineId re);
      virtual ~PSSolverManagerImpl();
      
      virtual PSSolver* createSolver(const std::string& configurationFile); 
      
    protected:
      PlanDatabaseId m_planDatabase;	
      SOLVERS::PlanWriter::PartialPlanWriter* m_ppw;          
  };

  class PSSolverImpl : public PSSolver
  {
    public:
      PSSolverImpl(const SOLVERS::SolverId& solver, 
    		       const std::string& configFilename, 
    		       SOLVERS::PlanWriter::PartialPlanWriter* ppw);
      virtual ~PSSolverImpl();
  
      virtual void step();
      virtual void solve(int maxSteps,int maxDepth);
      virtual void reset();
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
      virtual int getHorizonStart();
      virtual int getHorizonEnd();

      virtual void configure(int horizonStart, int horizonEnd);

    protected:
    	SOLVERS::SolverId m_solver;
    	std::string m_configFile;
    	SOLVERS::PlanWriter::PartialPlanWriter* m_ppw;
  };

}

#endif

