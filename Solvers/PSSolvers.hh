#ifndef _H_PSSolvers
#define _H_PSSolvers

#include "Engine.hh"
#include "PSUtils.hh"
#include <string>

namespace EUROPA 
{
  class PSSolver;
  
  class PSSolverManager : public EngineComponent
  {
    public:
      virtual ~PSSolverManager() {}
    	
      virtual PSSolver* createSolver(const std::string& configurationFile) = 0;            	
  };
  
  class PSSolver
  {
    public:
	  virtual ~PSSolver() {}

	  virtual void step() = 0;
	  virtual void solve(int maxSteps,int maxDepth) = 0;
	  virtual void reset() = 0;

	  virtual int getStepCount() = 0;
	  virtual int getDepth() = 0;		
	  virtual int getOpenDecisionCnt() = 0;	

	  virtual bool isExhausted() = 0;
	  virtual bool isTimedOut() = 0;	
	  virtual bool isConstraintConsistent() = 0;
	  virtual bool hasFlaws() = 0;	

	  virtual PSList<std::string> getFlaws() = 0;	
	  virtual std::string getLastExecutedDecision() = 0;	

	  // TODO: should horizon start and end be part of configuration?
	  virtual const std::string& getConfigFilename() = 0;	
	  virtual int getHorizonStart() = 0;
	  virtual int getHorizonEnd() = 0;

	  virtual void configure(int horizonStart, int horizonEnd) = 0;
  };

}

#endif

