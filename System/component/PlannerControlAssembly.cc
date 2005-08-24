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
#include "Solver.hh"
#include "Filters.hh"

// Test Support
#include "PLASMAPerformanceConstraint.hh"
#include "LoraxConstraints.hh"
#include "TestSupport.hh"

#include "ComponentFactory.hh"
#include "OpenConditionDecisionPoint.hh"
#include "OpenConditionManager.hh"
#include "ThreatDecisionPoint.hh"
#include "ThreatManager.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "UnboundVariableManager.hh"
#include "SolverDecisionPoint.hh"
#include "MatchingRule.hh"
#include "Filters.hh"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

#include <string>
#include <fstream>

namespace EUROPA {

  PlannerControlAssembly::PlannerControlAssembly(const SchemaId& schema) : StandardAssembly(schema) { }

  PlannerControlAssembly::~PlannerControlAssembly() {}


  PlannerStatus PlannerControlAssembly::initPlan(const char* txSource, const char* plannerConfig){
    static bool initFactories = true;
    if(initFactories) {
      REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::MinValue, MinValue);
      REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager);
      
      REGISTER_OPENCONDITION_DECISION_FACTORY(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler);
      REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager);
      
      REGISTER_THREAT_DECISION_FACTORY(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler);
      REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::ThreatManager, ThreatManager);
      
      REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter);
      REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::HorizonFilter, HorizonFilter);
      initFactories = !initFactories;
    }

    m_step = 0;
    std::cout << "Now process the transactions" << std::endl;
    if(!playTransactions(txSource))
      return INITIALLY_INCONSISTENT;
    
    TiXmlDocument doc(plannerConfig);
    doc.LoadFile();

    m_planner = (new SOLVERS::Solver(m_planDatabase, *(doc.RootElement())))->getId();
    m_ppw = new SOLVERS::PlanWriter::PartialPlanWriter(m_planDatabase, m_constraintEngine, m_rulesEngine, m_planner);
    m_listener = (new StatusListener(m_planner))->getId();

    std::cout << "Configure the planner from data in the initial state" << std::endl;

    // Configure the planner from data in the initial state
    std::list<ObjectId> configObjects;
    m_planDatabase->getObjectsByType("PlannerConfig", configObjects); // Standard configuration class

    check_error(configObjects.size() == 1,
                "Expect exactly one instance of the class 'PlannerConfig'");

    ObjectId configSource = configObjects.front();
    check_error(configSource.isValid());

    const std::vector<ConstrainedVariableId>& variables = configSource->getVariables();
    check_error(variables.size() == 4,
                "Expecting exactly 4 configuration variables");

    std::cout << "Set up the horizon  from the model now." << std::endl;
    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = variables[0];
    ConstrainedVariableId horizonEnd = variables[1];
    ConstrainedVariableId plannerSteps = variables[2];
    ConstrainedVariableId plannerDepth = variables[3];

    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    SOLVERS::HorizonFilter::getHorizon() = IntervalIntDomain(start, end);


    std::cout << "Now get planner step max" << std::endl;
    m_planner->setMaxSteps((unsigned int) plannerSteps->baseDomain().getSingletonValue());

    std::cout << "Now get planner depth max" << std::endl;
    m_planner->setMaxDepth((unsigned int) plannerDepth->baseDomain().getSingletonValue());

    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 1;
    SOLVERS::PlanWriter::PartialPlanWriter::writeStep = 1;

    return IN_PROGRESS;
  }

  const int PlannerControlAssembly::getPlannerStatus() const {
    return ((StatusListener*)m_listener)->getStatus();
  }

  int PlannerControlAssembly::writeStep(int step) {
    std::cout << "Write step " << step << std::endl;
    if(m_step || step)
      while(m_step < step) {
        std::cout << "Skipping step " << m_step << std::endl;
        m_planner->step();
        m_step++;
        if(getPlannerStatus() != IN_PROGRESS) {
          std::cout << "Terminated before step." << std::endl;
          return m_step;
        }
      }
    std::cout << "Writing step " << m_step << std::endl;
    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 0;
    m_planner->step();
    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 1;
    return m_step;
  }

  int PlannerControlAssembly::writeNext(int n) {
    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 0;
    while(n) {
      m_planner->step();
      m_step++;
      if(getPlannerStatus() != IN_PROGRESS)
        return m_step;
      n--;
    }
    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 1;
    return m_step;
  }

  int PlannerControlAssembly::completeRun() {
    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 1;
    for(;;) {
      m_planner->step();
      std::cout << "Completed step " << m_step << std::endl;
      m_step++;
      if(getPlannerStatus() != IN_PROGRESS) {
        m_ppw->write();
        return m_step;
      }
    }
    check_error(false);
    return m_step;
  }

}
