// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// For basic types
#include "TypeFactory.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "ResourceOpenDecisionManager.hh"
#include "PlanDatabaseWriter.hh"

#include "Constraints.hh"

#include <fstream>

SchemaId schema;

#define PERFORMANCE

extern void averInit(const PLASMA::PlanDatabaseId& db,
                     const PLASMA::DecisionManagerId& dm,
                     const PLASMA::ConstraintEngineId& ce,
                     const PLASMA::RulesEngineId& re);

extern void averDeinit();

const char* TX_LOG = "TransactionLog.xml";
const char* TX_REPLAY_LOG = "ReplayedTransactions.xml";
bool replay = true;

const char * initialTransactions = NULL;

bool runPlanner() {
  std::cout << "Just inside runPlanner(), IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
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
  std::cout << "Just after propagating initial transactions, IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';

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

#ifndef PERFORMANCE
  averInit(db1.planDatabase, db1.planner->getDecisionManager(),
           db1.constraintEngine, db1.rulesEngine);
#endif

  int res = db1.planner->run(steps);
  std::cout << "Just after finding plan, IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';

#ifndef PERFORMANCE
  averDeinit();
#endif

  assert(res == CBPlanner::PLAN_FOUND);

  PlanDatabaseWriter::write(db1.planDatabase, std::cout);

  // Store transactions for recreation of database
  if (replay) {
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
  std::cout << "Just before returning from runPlanner(), IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << "\nand ";
  IdTable::printTypeCnts(std::cout);
  return(true);
}

bool copyFromFile() {
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
  return(true);
}

int main(int argc, const char ** argv) {
  std::cout << "Just inside main(), IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
  if (argc < 2) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }
  initialTransactions = argv[1];
  // Initialize constraint factories
  SamplePlanDatabase::initialize();
  std::cout << "Just after SPD::init(), IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
  schema = NDDL::loadSchema();
  std::cout << "Just after NDDL::loadSchema(), IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
#ifdef PERFORMANCE
  replay = false;
  for (int i = 0; i < 1; i++) {
    runTest(runPlanner);
    std::cout << "Just after replay false runTest(runPlanner) #" << i << ", IdTable::size() is " << IdTable::size()
              << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
  }
#else
  replay = true;
  for (int i = 0; i < 2; i++) {
    runTest(runPlanner);
    std::cout << "Just after replay true runTest(runPlanner) #" << i << ", IdTable::size() is " << IdTable::size()
              << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
    runTest(copyFromFile);
    std::cout << "Just after replay true runTest(copyFromFile) #" << i << ", IdTable::size() is " << IdTable::size()
              << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
  }
#endif
  SamplePlanDatabase::terminate();
  std::cout << "Just after SPD::terminate(), IdTable::size() is " << IdTable::size()
            << " and LabelStr::getSize() is " << LabelStr::getSize() << '\n';
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
