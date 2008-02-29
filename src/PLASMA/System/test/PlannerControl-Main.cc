#include "MasterController.hh"
#include "PlanDatabase.hh"
#include "ConstraintEngine.hh"
#include "PlanDatabaseWriter.hh"
#include "Debug.hh"
#include <string>

using namespace EUROPA;

/**
 * @brief Uses the planner to solve a planning problem
 */
int main(int argc, const char ** argv){
  if (argc < 4) {
    std::cerr << "Invalid arguments:" << std::endl;
    for(int i = 0; i < argc; i++)
      std::cerr << "  " << i << ":" << argv[i] << std::endl;

    return -1;
  }

  const char* modelSource = argv[1];
  const char* txSource = argv[2];
  const char* configSource = argv[3];
  std::string fullPath;
  if(argc == 5)
    fullPath += std::string(argv[4]);

  fullPath += std::string(modelSource);

  int result = initModel(fullPath.c_str(), txSource, "./plans", configSource, NULL, 0);

  checkError(result == MasterController::IN_PROGRESS, "Invalid initial state.");

  result = completeRun();

  terminateRun();
  return 0;
}
