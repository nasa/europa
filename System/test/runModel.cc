#include "PLASMAPerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "ResourceOpenDecisionManager.hh"
#include "PlanDatabaseWriter.hh"

#include "Constraints.hh"
//#include "LoadInitModel.hh"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

SchemaId schema;
const char * initialTransactions = NULL;

const char* TX_LOG = "TransactionLog.xml";
const char* TX_REPLAY_LOG = "ReplayedTransactions.xml";
bool replay = false;

bool runPlanner(){
  SamplePlanDatabase db1(schema, replay);

  // Set ResourceOpenDecisionManager
  DecisionManagerId local_dm = db1.planner->getDecisionManager();
  ResourceOpenDecisionManagerId local_rodm = (new ResourceOpenDecisionManager(local_dm))->getId();
  local_dm->setOpenDecisionManager( local_rodm );

  DbClientId client = db1.planDatabase->getClient();

  DbClientTransactionPlayer player(client);
  check_error(initialTransactions != NULL);
  std::ifstream in(initialTransactions);
  player.play(in);

  assert(client->propagate());

  ObjectId world = client->getObject("world");
  check_error(world.isValid());
  // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
  ConstrainedVariableId horizonStart = world->getVariable("world.m_horizonStart");
  check_error(horizonStart.isValid());
  ConstrainedVariableId horizonEnd = world->getVariable("world.m_horizonEnd");
  check_error(horizonEnd.isValid());
  int start = (int) horizonStart->baseDomain().getSingletonValue();
  int end = (int) horizonEnd->baseDomain().getSingletonValue();
  db1.horizon->setHorizon(start, end);

  // Create and run the planner
  ConstrainedVariableId maxPlannerSteps = world->getVariable("world.m_maxPlannerSteps");
  check_error(maxPlannerSteps.isValid());
  int steps = (int) maxPlannerSteps->baseDomain().getSingletonValue();

  int res = db1.planner->run(steps);

  assert(res == CBPlanner::PLAN_FOUND);

  PlanDatabaseWriter::write(db1.planDatabase, std::cout);

  // Store transactions for recreation of database
  if(replay) {
    std::stringstream os1;
    db1.planDatabase->getClient()->toStream(os1);
    std::ofstream out(TX_LOG);
    db1.txLog->flush(out);
    out.close();

    std::stringstream os2;
    SamplePlanDatabase db(schema, true);
    DbClientTransactionPlayer player(db.planDatabase->getClient());
    std::ifstream in(TX_LOG);
    player.play(in);
    db.planDatabase->getClient()->toStream(os2);

    std::string s1 = os1.str();
    std::string s2 = os2.str();
    assert(s1 == s2);
  }

  return true;
}


bool copyFromFile(){
  // Populate plan database from transaction log
  std::stringstream os1;
  {
    SamplePlanDatabase db(schema, true);
    DbClientTransactionPlayer player(db.planDatabase->getClient());
    std::ifstream in(TX_LOG);
    player.play(in);
    db.planDatabase->getClient()->toStream(os1);
  }
  std::stringstream os2;
  {
    SamplePlanDatabase db(schema, true);
    DbClientTransactionPlayer player(db.planDatabase->getClient());
    std::ifstream in(TX_LOG);
    player.play(in);
    db.planDatabase->getClient()->toStream(os2);
  }

  std::string s1 = os1.str();
  std::string s2 = os2.str();
  assert(s1 == s2);

  return true;
}


int main(int argc, const char ** argv) {
  const char* libPath;
  char* error_msg;
  void* libHandle;
  SchemaId (*fcn_schema)();   //function pointer to NDDL::schema()

  switch (argc) {
    case 1:
      printf("usage: runModel <Model shared library path> <initial transaction file>\n");
      exit(1);
    case 3:
      libPath = argv[1];
      initialTransactions = argv[2];
      break;
    default:
      printf("Too many arguments\n");
      exit(1);
  }

  printf("runModel:dlopen() file: %s\n", libPath);
  fflush(stdout);

  //load model library using full path
  libHandle = dlopen(libPath, RTLD_NOW);

  if (!libHandle) {
    error_msg = dlerror();
    printf("Error during dlopen() of %s:\n", libPath);
    check_error(!error_msg, error_msg); 
  }

  //locate the NDDL 'schema' function in the library and check for errors
  fcn_schema = (SchemaId (*)())dlsym(libHandle, "loadSchema");
  if (!fcn_schema) {
    error_msg = dlerror();
    printf("dlsym: Error locating NDDL::schema:\n");
    check_error(!error_msg, error_msg); 
  } 

  assert(Schema::instance().isValid());

  // Initialize constraint factories
  SamplePlanDatabase::initialize();

  // call the NDDL::schema function
  schema = (*fcn_schema)();

#ifdef PERFORMANCE
  replay = false;
  runTest(runPlanner);
#else
  for(int i= 0; i<1; i++){
    replay = true;
    runTest(runPlanner);
    runTest(copyFromFile);
  }
#endif

  SamplePlanDatabase::terminate();

  if (dlclose(libHandle)) {
    error_msg = dlerror();
    printf("Error during dlclose():\n");
    check_error(!error_msg, error_msg); 
  }

  printf("Model Library Unloaded\n");
  fflush(stdout);
  std::cout << "Finished" << std::endl;
}

#ifdef __BEOS__

void __assert_fail(const char *__assertion,
                   const char *__file,
                   unsigned int __line,
                   const char *__function)
{
  debugger(__assertion);
}

#endif
