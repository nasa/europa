// For performance tests only
#include "PrototypePerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "EUROPAHeuristicStrategy.hh"

#include "PlanDatabaseWriter.hh"

#include <fstream>

SchemaId schema;

//#define PERFORMANCE

const char* TX_LOG = "TransactionLog.xml";
const char* TX_REPLAY_LOG = "ReplayedTransactions.xml";
bool replay = true;

#include "k9-transaction.hh"
#include "IntervalIntDomain.hh"

bool runPlanner(){
    SamplePlanDatabase db1(schema, replay);

    DbClientId client = db1.planDatabase->getClient();

    DbClientTransactionPlayer player(client);
    std::ifstream in("k9-transaction.xml");
    player.play(in);

    assert(client->propagate());

	NDDL::NddlWorldId world = client->getObject(LabelStr("world"));
    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = world->getVariable(LabelStr("world.m_horizonStart"));
    check_error(horizonStart.isValid());
    ConstrainedVariableId horizonEnd = world->getVariable(LabelStr("world.m_horizonEnd"));
    check_error(horizonEnd.isValid());
    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    db1.horizon->setHorizon(start, end);

    // Create and run the planner
    ConstrainedVariableId maxPlannerSteps = world->getVariable(LabelStr("world.m_maxPlannerSteps"));
    check_error(maxPlannerSteps.isValid());
    int steps = (int) maxPlannerSteps->baseDomain().getSingletonValue();

    int res = db1.planner->run(loggingEnabled(), steps);

    PlanDatabaseWriter::write(db1.planDatabase, std::cout);

    assert(res == 1);

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

int main(int argc, const char ** argv){
  // Initialize constraint factories
  SamplePlanDatabase::initialize();
  schema = NDDL::schema();

#ifdef PERFORMANCE
  replay = false;
  runTest(runPlanner);
#else
  replay = true;
  runTest(runPlanner);
  runTest(copyFromFile);
#endif

  SamplePlanDatabase::terminate();

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
