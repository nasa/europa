// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "NddlDefs.hh"

#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"

// Support for registered constraints
#include "ConstraintFactory.hh"
#include "Constraints.hh"
#include "EqualityConstraintPropagator.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"

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

void initialize(CESchema* ces) 
{
  /*
   *  TODO: constraint registration below needs to be removed, ModuleConstraintLibrary::initialize takes care of this
   *  leaving it for now for backwards compatibility since some constraints are named differently
   * and some other constraints like Lock and Ancestor are not registered by ModuleConstraintLibrary::initialize for some reason
   */
     
  // Procedural Constraints used with Default Propagation
  REGISTER_CONSTRAINT(ces,EqualConstraint, "eq", "Default");
  REGISTER_CONSTRAINT(ces,NotEqualConstraint, "neq", "Default");
  REGISTER_CONSTRAINT(ces,LessThanEqualConstraint, "leq", "Default");
  REGISTER_CONSTRAINT(ces,LessThanConstraint, "lessThan", "Default");
  REGISTER_CONSTRAINT(ces,AddEqualConstraint, "addEq", "Default");
  REGISTER_CONSTRAINT(ces,NegateConstraint, "neg", "Default");
  REGISTER_CONSTRAINT(ces,MultEqualConstraint, "mulEq", "Default");
  REGISTER_CONSTRAINT(ces,AddMultEqualConstraint, "addMulEq", "Default");
  REGISTER_CONSTRAINT(ces,AddMultEqualConstraint, "addmuleq", "Default");
  REGISTER_CONSTRAINT(ces,SubsetOfConstraint, "subsetOf", "Default");
  REGISTER_CONSTRAINT(ces,SubsetOfConstraint, "Singleton", "Default");
  REGISTER_CONSTRAINT(ces,LockConstraint, "Lock", "Default");
  REGISTER_CONSTRAINT(ces,CommonAncestorConstraint, "commonAncestor", "Default");
  REGISTER_CONSTRAINT(ces,HasAncestorConstraint, "hasAncestor", "Default");
  REGISTER_CONSTRAINT(ces,TestEQ, "testEQ", "Default");
  REGISTER_CONSTRAINT(ces,TestLEQ, "testLEQ", "Default");
  REGISTER_CONSTRAINT(ces,EqualSumConstraint, "sum", "Default");
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
    initialize((CESchema*)getComponent("CESchema"));
}

NddlTestEngine::~NddlTestEngine() 
{
    doShutdown();
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

  Schema* schema = (Schema*) engine.getComponent("Schema");   
  EUROPA::NDDL::loadSchema(schema->getId()); // Allocate the schema with a call to the linked in model function - eventually make this call via dlopen  
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
