/**
 * @file Main.cc
 *
 * @brief Provides an executable for your project which will use a
 * standard Chronological backtracking planner and a Test Assembly of
 * EUROPA
 */

#include "Nddl.hh" /*!< Includes protypes required to load a model */
#include "SolverAssembly.hh" /*!< For using a test EUROPA Assembly */
#include "PSEngine.hh" 
#include "Debug.hh"

using namespace EUROPA;

void executeWithAssembly(const char* plannerConfig, const char* txSource);
bool executeWithPSEngine(const char* plannerConfig, const char* txSource, int startHorizon, int endHorizon, int maxSteps);
void printFlaws(int it, PSList<std::string>& flaws);

int main(int argc, const char ** argv)
{
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

bool executeWithPSEngine(const char* plannerConfig, const char* txSource, int startHorizon, int endHorizon, int maxSteps)
{
    try {
	  PSEngine engine;
	
	  engine.start();
	  engine.executeTxns(txSource,true,true);
	
	  PSSolver* solver = engine.createSolver(plannerConfig);
	  solver->configure(startHorizon,endHorizon);
	  int i;
      for (i = 0; 
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
  // Initialize Library  
  SolverAssembly::initialize();

  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  SchemaId schema = NDDL::loadSchema();

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

