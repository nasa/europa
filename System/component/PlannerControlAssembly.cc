#include "PlannerControlAssembly.hh"

// Support for required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ResourceDefs.hh"
#include "ResourcePropagator.hh"
#include "Transaction.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
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
#include "ObjectTokenRelation.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"

#include "NddlDefs.hh"

// For cleanup purging
#include "TokenFactory.hh"
#include "ObjectFactory.hh"
#include "Rule.hh"

// Misc
#include "Utils.hh"

// Planner Support
#include "CBPlanner.hh"
#include "PartialPlanWriter.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "ResourceOpenDecisionManager.hh"

// Test Support
#include "PLASMAPerformanceConstraint.hh"
#include "LoraxConstraints.hh"
#include "TestSupport.hh"

#include <string>
#include <fstream>

namespace EUROPA {

  PlannerControlAssembly::PlannerControlAssembly(const SchemaId& schema) : StandardAssembly(schema) { }

  PlannerControlAssembly::~PlannerControlAssembly() {}


  CBPlanner::Status PlannerControlAssembly::initPlan(const char* txSource){
    m_horizon = (new Horizon())->getId();
    m_planner = (new CBPlanner(m_planDatabase, m_horizon))->getId();
    m_ppw = new PlanWriter::PartialPlanWriter(m_planDatabase, m_constraintEngine, m_rulesEngine, m_planner);

    std::cout << "Set up Resource Decision Manager" << std::endl;
    // Set up Resource Decision Manager
    DecisionManagerId local_dm = m_planner->getDecisionManager();
    ResourceOpenDecisionManagerId local_rodm = (new ResourceOpenDecisionManager(local_dm))->getId();
    local_dm->setOpenDecisionManager(local_rodm);

    std::cout << "Now process the transactions" << std::endl;
    // Now process the transactions
    if(!playTransactions(txSource))
      return CBPlanner::INITIALLY_INCONSISTENT;

    std::cout << "Configure the planner from data in the initial state" << std::endl;
    // Configure the planner from data in the initial state
    std::list<ObjectId> configObjects;
    m_planDatabase->getObjectsByType("PlannerConfig", configObjects); // Standard configuration class

    check_error(configObjects.size() == 1,
                "Expect exactly one instance of the class 'PlannerConfig'");

    ObjectId configSource = configObjects.front();
    check_error(configSource.isValid());

    const std::vector<ConstrainedVariableId>& variables = configSource->getVariables();
    check_error(variables.size() == 3,
                "Expecting exactly 3 configuration variables");

    std::cout << "Set up the horizon  from the model now." << std::endl;
    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = variables[0];
    ConstrainedVariableId horizonEnd = variables[1];
    ConstrainedVariableId plannerSteps = variables[2];

    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    m_horizon->setHorizon(start, end);

    std::cout << "Now get planner step max" << std::endl;
    // Now get planner step max
    int steps = (int) plannerSteps->baseDomain().getSingletonValue();

    std::cout << "Now initialize it" << std::endl;
    // Now initialize it
    CBPlanner::Status res = m_planner->initRun(steps);

    std::cout << "Planner ready to step  Status is " << res << std::endl;

    return res;
  }

}
