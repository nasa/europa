// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "EUROPAHeuristicStrategy.hh"

#include <fstream>

SchemaId schema;

#define REPLAY_DECISIONS

const char* TX_LOG = "TransactionLog.xml";
const char* TX_REPLAY_LOG = "ReplayedTransactions.xml";

bool runPlanner(bool replay){

  SamplePlanDatabase db1(schema, true);

  // Initialize the plan database
  NDDL::initialize(db1.planDatabase);

  // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
  std::list<ObjectId> objects;
  db1.planDatabase->getObjectsByType(LabelStr("World"), objects);
  ObjectId world = objects.front();
  check_error(objects.size() == 1);
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
  CBPlanner planner(db1.planDatabase->getClient(), db1.flawQuery, steps);
  EUROPAHeuristicStrategy strategy;
      
  int res = planner.run(strategy.getId(), loggingEnabled());

  check_error(res == 1);
      
  std::cout << "Nr of Decisions = " << planner.getClosedDecisions().size() << std::endl;

  // Store transactions for recreation of database
  {
    std::cout << "Saving Transactions.." << std::endl;
    std::ofstream out(TX_LOG);
    db1.txLog->flush(out);
    out.close();
  }

  std::cout << "Plan Database:" << std::endl;
  PlanDatabaseWriter::write(db1.planDatabase, std::cout);

  cout << "PASSED runPlanner\n";
  return true;
}

bool copyFromFile(){
  // Populate plan database from transaction log
    SamplePlanDatabase db1(schema, true);
    DbClientTransactionPlayer player1(db1.planDatabase->getClient());
    std::ifstream in(TX_LOG);
    player1.play(in);

    /*
    SamplePlanDatabase db2(schema, true);
    DbClientTransactionPlayer player2(db2.planDatabase->getClient());
    player2.play(db1.txLog);
    */

    std::cout << "Plan Database:" << std::endl;
    PlanDatabaseWriter::write(db1.planDatabase, std::cout);

    cout << "PASSED copyFromFile\n"; 
    return true;
}

int main(int argc, const char ** argv){
  // Initialize constraint factories
  SamplePlanDatabase::initialize();
  schema = NDDL::schema();

  assert(runPlanner(true));
  assert(copyFromFile());

  SamplePlanDatabase::terminate();

  std::cout << "Finished" << std::endl;
}
