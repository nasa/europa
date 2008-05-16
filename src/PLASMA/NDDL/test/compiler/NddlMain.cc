// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"

#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "floatType.hh"
#include "intType.hh"

// Support for Temporal Network
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"

// Support for registered constraints
#include "ConstraintLibrary.hh"
#include "Constraints.hh"
#include "EqualityConstraintPropagator.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"

#include "NddlDefs.hh"

// For cleanup purging
#include "TokenFactory.hh"
#include "ObjectFactory.hh"
#include "Rule.hh"

// Misc
#include "Utils.hh"
#include "Engine.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleNddl.hh"

#include <fstream>
#include <sstream>

using namespace EUROPA;

void initialize() 
{
  /*
   *  TODO: constraint registration below needs to be removed, initConstraintLibrary takes care of this
   *  leaving it for now for backwards compatibility since some constraints are named differently
   * and some other constraints like Lock and Ancestor are not registered by initConstraintLibrary for some reason
   */
     
  // Procedural Constraints used with Default Propagation
  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
  REGISTER_CONSTRAINT(NotEqualConstraint, "neq", "Default");
  REGISTER_CONSTRAINT(LessThanEqualConstraint, "leq", "Default");
  REGISTER_CONSTRAINT(LessThanConstraint, "lessThan", "Default");
  REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
  REGISTER_CONSTRAINT(NegateConstraint, "neg", "Default");
  REGISTER_CONSTRAINT(MultEqualConstraint, "mulEq", "Default");
  REGISTER_CONSTRAINT(AddMultEqualConstraint, "addMulEq", "Default");
  REGISTER_CONSTRAINT(AddMultEqualConstraint, "addmuleq", "Default");
  REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");
  REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_CONSTRAINT(LockConstraint, "Lock", "Default");
  REGISTER_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
  REGISTER_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");
  REGISTER_CONSTRAINT(TestEQ, "testEQ", "Default");
  REGISTER_CONSTRAINT(TestLEQ, "testLEQ", "Default");
  REGISTER_CONSTRAINT(EqualSumConstraint, "sum", "Default");
}

class NddlTestEngine : public EngineBase  
{
  public:  
	NddlTestEngine();
	virtual ~NddlTestEngine();
	
  protected: 
	void createModules();
};

NddlTestEngine::NddlTestEngine() 
{
    createModules();
    doStart();
    initialize();
}

NddlTestEngine::~NddlTestEngine() 
{
    // TODO: this is going into an infinite loop!!!
    // in the debugger, it's happenning when the PlanDatabase is deleted and the listeners are notified
    // ask Tristan since he recently did some cleanupin the listeners
    //doShutdown(); 
}

void NddlTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
    addModule((new ModuleRulesEngine())->getId());
    addModule((new ModuleTemporalNetwork())->getId());
    addModule((new ModuleNddl())->getId());
}

/**
 * @file Provides main execution program to run a test which plays transactions
 * on a database for a given model. The model must currently be linked to the executable
 * statically, but we will eventually get to dynamically linking to a model as an argumnet.
 */
int main(int argc, const char ** argv) 
{
  if (argc != 2) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  NddlTestEngine engine;

  SchemaId schema = EUROPA::NDDL::loadSchema(); // Allocate the schema with a call to the linked in model function - eventually make this call via dlopen  
  PlanDatabase* planDatabase = (PlanDatabase*) engine.getComponent("PlanDatabase");   
  planDatabase->getClient()->enableTransactionLogging();

  const char* txSource = argv[1];
  engine.executeScript("nddl-xml-txn",txSource,true /*isFile*/);  

  ConstraintEngine* ce = (ConstraintEngine*)engine.getComponent("ConstraintEngine");
  assert(ce->constraintConsistent());

  PlanDatabaseWriter::write(planDatabase->getId(), std::cout);
  std::cout << "Finished\n";
  
  return 0;
}
