// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"

#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

#ifndef NO_RESOURCES
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ResourceDefs.hh"
#include "ResourcePropagator.hh"
#include "Transaction.hh"

#include "SAVH_Profile.hh"
#include "SAVH_FVDetector.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_TimetableFVDetector.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#endif

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

#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleNddl.hh"
#ifndef NO_RESOURCES
#include "ModuleResource.hh"
#endif

#include <fstream>
#include <sstream>

using namespace EUROPA;

class NddlTestEngine  
{
  public:  
	NddlTestEngine() {}
	virtual ~NddlTestEngine() {}
	
	static void initialize();
	static void terminate();

  protected: 
	static void createModules();
	static void initializeModules();
	static void uninitializeModules();
	static std::vector<ModuleId> m_modules;	    
};

std::vector<ModuleId> NddlTestEngine::m_modules;

void NddlTestEngine::initialize()
{
	initializeModules();    	
}

void NddlTestEngine::terminate()
{
	uninitializeModules();
}

void NddlTestEngine::createModules()
{
    // TODO: make this data-driven
    m_modules.push_back(new ModuleConstraintEngine()); 
    m_modules.push_back(new ModuleConstraintLibrary());
    m_modules.push_back(new ModulePlanDatabase());
    m_modules.push_back(new ModuleRulesEngine());
    m_modules.push_back(new ModuleTemporalNetwork());
    m_modules.push_back(new ModuleNddl());
#ifndef NO_RESOURCES	    
    m_modules.push_back(new ModuleResource());
#endif
}

void NddlTestEngine::initializeModules()
{
    createModules();
  
    for (unsigned int i=0;i<m_modules.size();i++) {
    	m_modules[i]->initialize();
    }	  
}

void NddlTestEngine::uninitializeModules()
{
    Entity::purgeStarted();      
    for (unsigned int i=m_modules.size();i>0;i--) {
    	unsigned int idx = i-1;
    	m_modules[idx]->uninitialize();
    	m_modules[idx].release();
    }	  
    Entity::purgeEnded();	  

    m_modules.clear();	  
}


void initialize() {
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

/**
 * @file Provides main execution program to run a test which plays transactions
 * on a database for a given model. The model must currently be linked to the executable
 * statically, but we will eventually get to dynamically linking to a model as an argumnet.
 * @author Conor McGann
 */
int main(int argc, const char ** argv) {
  if (argc != 2) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  const char* txSource = argv[1];

  NddlTestEngine::initialize();  
  initialize();

  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  SchemaId schema = EUROPA::NDDL::loadSchema();

  
  // Allocate the Constraint Engine
  ConstraintEngineId m_constraintEngine = (new ConstraintEngine())->getId();

  // Allocate the plan database
  PlanDatabaseId m_planDatabase = (new PlanDatabase(m_constraintEngine, schema))->getId();

  // Construct propagators - order of introduction determines order of propagation.
  // Note that propagators will subsequently be managed by the constraint engine
  new DefaultPropagator(LabelStr("Default"), m_constraintEngine);
  new TemporalPropagator(LabelStr("Temporal"), m_constraintEngine);
  // Link up the Temporal Advisor in the PlanDatabase so that it can use the temporal
  // network for determining temporal distances between time points.
  PropagatorId temporalPropagator =
    m_constraintEngine->getPropagatorByName(LabelStr("Temporal"));
  m_planDatabase->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());

#ifndef NO_RESOURCES
  new ResourcePropagator(LabelStr("Resource"), m_constraintEngine, m_planDatabase);
  new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_constraintEngine);
#endif

  // Allocate the rules engine to process rules
  RulesEngineId m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();

  m_planDatabase->getClient()->enableTransactionLogging();

  // Obtain the client to play transactions on.
  DbClientId client = m_planDatabase->getClient();
  
  // Construct player
  DbClientTransactionPlayer player(client);
  
  // Open transaction source and play transactions
  std::ifstream in(txSource);
  
  check_error(in, "Invalid transaction source '" + std::string(txSource) + "'.");
  player.play(in);
  
  assert(m_constraintEngine->constraintConsistent());
  PlanDatabaseWriter::write(m_planDatabase, std::cout);

  NddlTestEngine::terminate();
  std::cout << "Finished\n";
  exit(0);
}
