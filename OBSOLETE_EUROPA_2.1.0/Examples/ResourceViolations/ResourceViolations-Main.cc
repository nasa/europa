/**
 * @file Main.cc
 *
 * @brief Provides an executable for your project which will use a
 * standard Chronological backtracking planner and a Test Assembly of
 * EUROPA
 */

#include "Nddl.hh" /*!< Includes protypes required to load a model */
#include "SolverAssembly.hh" /*!< For using a test EUROPA Assembly */
#include "FlawHandler.hh"
#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "ResourceThreatDecisionPoint.hh"
#include "SAVH_ProfilePropagator.hh"
#include "ResourcePropagator.hh"
#include "PSEngine.hh" 
#include "PSResources.hh" 
#include "Debug.hh"

using namespace EUROPA;

void executeWithAssembly(const char* plannerConfig, const char* txSource);
bool executeWithPSEngine(const char* plannerConfig, const char* txSource, int startHorizon, int endHorizon, int maxSteps);
void printFlaws(int it, PSList<std::string>& flaws);

/**
 * @brief Uses the planner to solve a planning problem
 */
int main(int argc, const char ** argv){
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  const char* txSource = argv[1];
  const char* plannerConfig = argv[2];
  int startHorizon = 0;
  int endHorizon   = 100;
  int maxSteps     = 1000;

  if (!executeWithPSEngine(plannerConfig,txSource,startHorizon,endHorizon,maxSteps)) 
      return -1;

  // executeWithAssembly(plannerConfig,txSource);
   
  return 0;
}

void testViolations(PSEngine& psengine)
{
	/*
	PSObject* res = psengine.getObjectsByType("CapacityResource").get(0);
	PSList<PSToken*> toks = res->getTokens();
	PSToken* t1 = toks.get(0);
	PSToken* t2 = toks.get(1);
    */
	
	PSObject* act_obj1 = psengine.getObjectsByType("Activity").get(0);
	PSObject* act_obj2 = psengine.getObjectsByType("Activity").get(1);
	PSObject* act_obj3 = psengine.getObjectsByType("Activity").get(2);

	PSToken* act1 = act_obj1->getTokens().get(0);
	PSToken* act2 = act_obj2->getTokens().get(0);	
	PSToken* act3 = act_obj3->getTokens().get(0);	

	PSVariable* s1 = act1->getParameter("start");
	PSVariable* s2 = act2->getParameter("start");
	PSVariable* s3 = act3->getParameter("start");

	PSVarValue vv5 = PSVarValue::getInstance(5);
	PSVarValue vv11 = PSVarValue::getInstance(11);
	PSVarValue vv18 = PSVarValue::getInstance(18);
	PSVarValue vv20 = PSVarValue::getInstance(20);

	// Cause Violation
	s1->specifyValue(vv5);
	s2->specifyValue(vv11);
	s2->specifyValue(vv20);
	debugMsg("testViolations",psengine.getViolation());
	debugMsg("testViolations",psengine.getViolationExpl());

	// Remove Violation
	s3->specifyValue(vv18);
	debugMsg("testViolations",psengine.getViolation());
	debugMsg("testViolations",psengine.getViolationExpl());
}

bool executeWithPSEngine(const char* plannerConfig, const char* txSource, int startHorizon, int endHorizon, int maxSteps)
{
    try {
	  PSEngineWithResources engine;
	  
	  engine.start();
	  engine.setAllowViolations(true);
	  engine.executeTxns(txSource,true,true);
	
	  PSSolver* solver = engine.createSolver(plannerConfig);
	  solver->configure(startHorizon,endHorizon);
	  int i;
      for (i = 0; 
           solver->hasFlaws() &&
           !solver->isExhausted() &&
           !solver->isTimedOut() &&
           i<maxSteps; i = solver->getStepCount()) {
		  solver->step();
		  PSList<std::string> flaws;
		  if (solver->isConstraintConsistent()) {
	          flaws = solver->getFlaws();
        	  printFlaws(i,flaws);
			  if (flaws.size() == 0)
			      break;
		  }
		  else
			debugMsg("Main","Iteration " << i << " Solver is not constraint consistent");
	  }
	  
	  if (solver->isExhausted()) {
	      debugMsg("Main","Solver was exhausted after " << i << " steps");	  
	  }
	  else if (solver->isTimedOut()) {
	      debugMsg("Main","Solver timed out after " << i << " steps");
	  }
	  else {    
	      debugMsg("Main","Solver finished after " << i << " steps");
	  }	      
	      
	  testViolations(engine);
	  
	  delete solver;	  
	  engine.shutdown();

	  return true;
	}
	catch (Error& e) {
		std::cerr << "PSEngine failed:" << e.getMsg() << std::endl;
		return false;
	}	
}

void printFlaws(int it, PSList<std::string>& flaws)
{
	debugMsg("Main","Iteration:" << it << " " << flaws.size() << " flaws");
	
	for (int i=0; i<flaws.size(); i++) {
		std::cout << "    " << (i+1) << " - " << flaws.get(i) << std::endl;
	}
}


void executeWithAssembly(const char* plannerConfig, const char* txSource)
{
	  REGISTER_FVDETECTOR( EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector );
	  REGISTER_PROFILE( EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
	  REGISTER_FLAW_HANDLER( EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat);
	  
	  // Initialize Library  
	  SolverAssembly::initialize();

	  // Allocate the schema with a call to the linked in model function - eventually
	  // make this called via dlopen
	  SchemaId schema = NDDL::loadSchema();

	  //new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_constraintEngine);
	  //new ResourcePropagator(LabelStr("Resource"), m_constraintEngine, m_planDatabase);

	  
	  // Enacpsualte allocation so that they go out of scope before calling terminate
	  {  
	    // Allocate the test assembly.
	    SolverAssembly assembly(schema);

	    // Run the planner
	    assembly.plan(txSource, plannerConfig);

	    // Dump the results
	    assembly.write(std::cout);
	  }

	  // Terminate the library
	  SolverAssembly::terminate();

	  std::cout << "Finished\n";
}

