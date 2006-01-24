#include "Debug.hh"
#include "Nddl.hh"
#include "Solver.hh"
#include "HSTSSolverAssembly.hh"
#include "DNPConstraints.hh"
#include "Filters.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "HSTSDecisionPoints.hh"
#include "Token.hh"

int main(int argc, const char** argv) {
  if(argc != 5) {
    std::cerr << "Error: incorrect number of arguments.  Expecting 2: -i (initial state) and -c (solver config)." << std::endl;
    return -1;
  }

  const char* txSource = "";
  const char* configSource = "";

  for(int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    if(arg[0] != '-')
      break;
    if(strcmp(arg, "-c") == 0) {
      i++;
      if(i >= argc) {
        std::cerr << "Error: expected solver config filename" << std::endl;
        return -1;
      }
      configSource = argv[i];
      continue;
    }
    if(strcmp(arg, "-i") == 0) {
      i++;
      if(i >= argc) {
        std::cerr << "Error: expected initial transactions filename" << std::endl;
        return -1;
      }
      txSource = argv[i];
      continue;
    }
    std::cerr << argv[0] << ": unrecognized option '" << arg << "'" << std::endl;
    return -1;
  }

  EUROPA::HSTS::SolverAssembly::initialize();
  registerDNPConstraints();
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::SingletonFilter, Singleton);
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonFilter, HorizonFilter);
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDymanicFilter);
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter);
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::TokenMustBeAssignedFilter, ParentMustBeInsertedFilter);
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::MasterMustBeAssignedFilter, MasterMustBeInsertedFilter);
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::GuardFilter, GuardFilter);
  REGISTER_FLAW_FILTER(EUROPA::SOLVERS::NotGuardFilter, NotGuardFilter);

  REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, Min);
  REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MaxValue, Max);
  REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::ValueEnum, ValEnum);
  REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::OpenConditionDecisionPoint, HSTSOpenConditionDecisionPoint);
  REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::ThreatDecisionPoint, HSTSThreatDecisionPoint);
  SchemaId schema = NDDL::loadSchema();
  {
    EUROPA::HSTS::SolverAssembly assembly(schema);
    if(!assembly.plan(txSource, configSource)) {
      std::cerr << "Failed to find a plan.";
      return -1;
    }
  }
  return 0;
}
