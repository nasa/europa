// For performance tests only
#include "PrototypePerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"

#include "PlanDatabaseWriter.hh"

#include <fstream>

SchemaId schema;

//#define PERFORMANCE

const char* TX_LOG = "TransactionLog.xml";
const char* TX_REPLAY_LOG = "ReplayedTransactions.xml";
bool replay = true;

#include "k9-initial.hh"
#include "IntervalIntDomain.hh"

bool runPlanner(){
    SamplePlanDatabase db1(schema, replay);

    DbClientId client = db1.planDatabase->getClient();

    std::vector<ConstructorArgument> arguments;
    arguments.push_back(ConstructorArgument(LabelStr("int"), new IntervalIntDomain(0, DomainListenerId::noId(), LabelStr("int"))));
    arguments.push_back(ConstructorArgument(LabelStr("int"), new IntervalIntDomain(100, DomainListenerId::noId(), LabelStr("int"))));
    arguments.push_back(ConstructorArgument(LabelStr("int"), new IntervalIntDomain(500, DomainListenerId::noId(), LabelStr("int"))));
    NDDL::NddlWorldId world = client->createObject(LabelStr("NddlWorld"), LabelStr("world"), arguments);

    NDDL::RoverId rover = client->createObject(LabelStr("Rover"), LabelStr("rover"));
    NDDL::LocationId l1 = client->createObject(LabelStr("Location"), LabelStr("l1"));
    NDDL::LocationId l2 = client->createObject(LabelStr("Location"), LabelStr("l2"));
    NDDL::LocationId l3 = client->createObject(LabelStr("Location"), LabelStr("l3"));
    NDDL::LocationId l4 = client->createObject(LabelStr("Location"), LabelStr("l4"));
    NDDL::LocationId l5 = client->createObject(LabelStr("Location"), LabelStr("l5"));
    NDDL::LocationId l6 = client->createObject(LabelStr("Location"), LabelStr("l6"));

    std::vector<ConstructorArgument> arguments1;
    arguments1.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l1)));
    arguments1.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l2)));
    client->createObject(LabelStr("Path"), LabelStr("p1"), arguments1);

    std::vector<ConstructorArgument> arguments2;
    arguments2.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l1)));
    arguments2.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l3)));
    client->createObject(LabelStr("Path"), LabelStr("p2"), arguments2);

    std::vector<ConstructorArgument> arguments3;
    arguments3.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l2)));
    arguments3.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l5)));
    client->createObject(LabelStr("Path"), LabelStr("p3"), arguments3);

    std::vector<ConstructorArgument> arguments4;
    arguments4.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l5)));
    arguments4.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l6)));
    client->createObject(LabelStr("Path"), LabelStr("p4"), arguments4);

    std::vector<ConstructorArgument> arguments5;
    arguments5.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l3)));
    arguments5.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l6)));
    client->createObject(LabelStr("Path"), LabelStr("p5"), arguments5);

    std::vector<ConstructorArgument> arguments6;
    arguments6.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l2)));
    arguments6.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l3)));
    client->createObject(LabelStr("Path"), LabelStr("p6"), arguments6);

    client->close();

    // Create initial state
    Id<NDDL::Position::At> a = client->createToken(LabelStr("Position.At"));
    client->activate(a);
    client->specify(a->getObject(), rover->m_position->getBaseDomain().getSingletonValue());

    Id<NDDL::Position::At> b = client->createToken(LabelStr("Position.At"));
    client->activate(b);
    client->specify(b->getObject(), rover->m_position->getBaseDomain().getSingletonValue());

    std::vector<ConstrainedVariableId> scope;
    scope.push_back(world->m_horizonStart);
    scope.push_back(a->start);
    client->createConstraint(LabelStr("leq"), scope);

    std::vector<ConstrainedVariableId> scope2;
    scope2.push_back(world->m_horizonStart);
    scope2.push_back(b->start);
    client->createConstraint(LabelStr("leq"), scope2);

    std::vector<ConstrainedVariableId> scope3;
    scope3.push_back(a->end);
    scope3.push_back(world->m_horizonEnd);
    client->createConstraint(LabelStr("leq"), scope3);

    std::vector<ConstrainedVariableId> scope4;
    scope4.push_back(b->end);
    scope4.push_back(world->m_horizonEnd);
    client->createConstraint(LabelStr("leq"), scope4);

    client->specify(a->location, l1);

    client->specify(b->location, l5);

    std::vector<ConstrainedVariableId> scope7;
    scope7.push_back(a->end);
    scope7.push_back(b->start);
    client->createConstraint(LabelStr("leq"), scope7);

    assert(client->propagate());

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

    int res = db1.planner->run(steps);

    PlanDatabaseWriter::write(db1.planDatabase, std::cout);

    assert(res == CBPlanner::PLAN_FOUND);

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
