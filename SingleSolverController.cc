#include "SingleSolverController.hh"
#include "Solver.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "ConstrainedVariable.hh"
#include "IntervalDomain.hh"
#include "Filters.hh"
#include "Utils.hh"

namespace EUROPA {
  namespace SOLVERS {

    SingleSolverController::SingleSolverController(): MasterController() {}

    SingleSolverController::~SingleSolverController(){
      if(m_solver.isId())
	delete (Solver*) m_solver;
    }

    void SingleSolverController::handleRegistration(){
      MasterController::handleRegistration();
    }

    void SingleSolverController::configureSolvers(const char* configPath){
      logMsg(std::string("SingleSolverController:configureSolvers: Configuration source is ") + configPath);
      check_error(configPath != NULL, "Must have a planner config argument.");
      TiXmlDocument doc(configPath);
      doc.LoadFile();
      m_solver = (new EUROPA::SOLVERS::Solver(getPlanDatabase(), *(doc.RootElement())))->getId();

      // Configure the planner from data in the initial state
      std::list<ObjectId> configObjects;
      getPlanDatabase()->getObjectsByType("PlannerConfig", configObjects); // Standard configuration class

      check_error(configObjects.size() == 1,
		  "Expect exactly one instance of the class 'PlannerConfig'");

      ObjectId configSource = configObjects.front();
      check_error(configSource.isValid());

      const std::vector<ConstrainedVariableId>& variables = configSource->getVariables();
      check_error(variables.size() == 4, "Expecting exactly 4 configuration variables");

      // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
      ConstrainedVariableId horizonStart = variables[0];
      ConstrainedVariableId horizonEnd = variables[1];
      ConstrainedVariableId plannerSteps = variables[2];
      ConstrainedVariableId maxDepth = variables[3];

      int start = (int) horizonStart->baseDomain().getSingletonValue();
      int end = (int) horizonEnd->baseDomain().getSingletonValue();
      SOLVERS::HorizonFilter::getHorizon() = IntervalDomain(start, end);

      // Now get planner step max
      m_maxSteps = (int) plannerSteps->baseDomain().getSingletonValue();
      m_maxDepth = (int) maxDepth->baseDomain().getSingletonValue();

      std::string msg = std::string("SolverConfiguration: \n") +
	"    Horizon = [" + EUROPA::toString(start) + ", " + EUROPA::toString(end) + "]\n" +
	"    MaxSteps = " + EUROPA::toString(m_maxSteps) + "\n" +
	"    MaxDepth = " + EUROPA::toString(m_maxDepth);
      logMsg(msg);
    }

    MasterController::Status SingleSolverController::handleNext(){
      m_solver->step();

      if(m_solver->getDepth() > m_maxDepth || m_solver->getStepCount() > m_maxSteps)
	return MasterController::TIMEOUT_REACHED;
      else if(m_solver->noMoreFlaws())
	return MasterController::PLAN_FOUND;
      else if(m_solver->isExhausted())
	return MasterController::SEARCH_EXHAUSTED;
      else
	return MasterController::IN_PROGRESS;
    }
  }
}
