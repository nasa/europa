// For performance tests only
#include "PLASMAPerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// Support for planner
//#include "CBPlanner.hh"
//#include "DecisionPoint.hh"
#include "DecisionManager.hh"

#undef publish

#include "PlanDatabaseWriter.hh"
#include "EventAggregator.hh"

#include <fstream>

SchemaId schema;

extern void averInit(const PLASMA::PlanDatabaseId& db,
                     const PLASMA::DecisionManagerId& dm,
                     const PLASMA::ConstraintEngineId& ce,
                     const PLASMA::RulesEngineId& re);
extern void averDeinit();

const char* TX_LOG = "TransactionLog.xml";
const char* TX_REPLAY_LOG = "ReplayedTransactions.xml";
bool replay = true;

//#include "k9-initial.hh"
#include "module-tests.hh"
#include "IntervalIntDomain.hh"

// System/test/k9-initial.xml

//#include "k9-initial.hh"
#include "NddlUtils.hh"

namespace NDDL {
  
  
  // NddlWorld.nddl:2 NddlWorld
  NddlWorld::NddlWorld(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "NddlWorld", name, true) {
  }
  NddlWorld::NddlWorld(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  NddlWorld::NddlWorld(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "NddlWorld", name, true) {}
  NddlWorld::NddlWorld(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void NddlWorld::handleDefaults(bool autoClose) {
    if(m_horizonStart.isNoId()){
      check_error(!ObjectId::convertable(m_horizonStart)); // Object Variables must be explicitly initialized to a singleton
      m_horizonStart = addVariable(IntervalIntDomain("int"), "m_horizonStart");
    }
    if(m_horizonEnd.isNoId()){
      check_error(!ObjectId::convertable(m_horizonEnd)); // Object Variables must be explicitly initialized to a singleton
      m_horizonEnd = addVariable(IntervalIntDomain("int"), "m_horizonEnd");
    }
    if(m_maxPlannerSteps.isNoId()){
      check_error(!ObjectId::convertable(m_maxPlannerSteps)); // Object Variables must be explicitly initialized to a singleton
      m_maxPlannerSteps = addVariable(IntervalIntDomain("int"), "m_maxPlannerSteps");
    }
    if(autoClose) close();
  }
  
  
  
  // NddlWorld.nddl:9 initialState
  NddlWorld::initialState::initialState(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  NddlWorld::initialState::initialState(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void NddlWorld::initialState::handleDefaults() {
    close();
  }
  
  
  // NddlWorld.nddl:2 NddlWorld
  void NddlWorld::constructor(int start, int end, int maxPlannerSteps) {
    m_horizonStart = addVariable(IntervalIntDomain(start, start, "int"), "m_horizonStart");
    m_horizonEnd = addVariable(IntervalIntDomain(end, end, "int"), "m_horizonEnd");
    m_maxPlannerSteps = addVariable(IntervalIntDomain(maxPlannerSteps, maxPlannerSteps, "int"), "m_maxPlannerSteps");
  }
  
  // NddlWorld.nddl:2 NddlWorld
  class NddlWorldFactory0: public ConcreteObjectFactory {
  public:
    NddlWorldFactory0(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 3);
      check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                                TypeFactory::baseDomain("int")), 
                  "Cannot convert " + arguments[0].first.toString() + " to int");
      check_error(arguments[0].second->isSingleton());
      int start((int)arguments[0].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[1].second, 
                                                TypeFactory::baseDomain("int")), 
                  "Cannot convert " + arguments[1].first.toString() + " to int");
      check_error(arguments[1].second->isSingleton());
      int end((int)arguments[1].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[2].second, 
                                                TypeFactory::baseDomain("int")), 
                  "Cannot convert " + arguments[2].first.toString() + " to int");
      check_error(arguments[2].second->isSingleton());
      int maxPlannerSteps((int)arguments[2].second->getSingletonValue());
      
      NddlWorldId instance = (new NddlWorld(planDb, objectType, objectName))->getId();
      instance->constructor(start, end, maxPlannerSteps);
      instance->handleDefaults();
      return instance;
    }
  };
  
  // NddlWorld.nddl:2 NddlWorld
  void NddlWorld::constructor() {
    m_horizonStart = addVariable(IntervalIntDomain(0, 0, "int"), "m_horizonStart");
    m_horizonEnd = addVariable(IntervalIntDomain(100, 100, "int"), "m_horizonEnd");
    m_maxPlannerSteps = addVariable(IntervalIntDomain(200, 200, "int"), "m_maxPlannerSteps");
  }
  
  // NddlWorld.nddl:2 NddlWorld
  class NddlWorldFactory1: public ConcreteObjectFactory {
  public:
    NddlWorldFactory1(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 0);
      NddlWorldId instance = (new NddlWorld(planDb, objectType, objectName))->getId();
      instance->constructor();
      instance->handleDefaults();
      return instance;
    }
  };
  
  
  // k9-initial.nddl:5 Location
  Location::Location(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "Location", name, true) {
  }
  Location::Location(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  Location::Location(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "Location", name, true) {}
  Location::Location(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void Location::handleDefaults(bool autoClose) {
    if(autoClose) close();
  }
  
  // implicit constructor
  void Location::constructor() {
  }
  
  
  // k9-initial.nddl:5 Location
  DECLARE_DEFAULT_OBJECT_FACTORY(LocationFactory2, Location);
  
  
  // k9-initial.nddl:7 NotTracked
  Location::NotTracked::NotTracked(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  Location::NotTracked::NotTracked(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  // default initialization of member variables
  void Location::NotTracked::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:12 trackstart
  Location::trackstart::trackstart(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  Location::trackstart::trackstart(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  // default initialization of member variables
  void Location::trackstart::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:17 Tracked
  Location::Tracked::Tracked(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  Location::Tracked::Tracked(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  // default initialization of member variables
  void Location::Tracked::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:22 trackstop
  Location::trackstop::trackstop(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  Location::trackstop::trackstop(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("target")));
      token_constraint(eq, vars);
    }
  }
  
  // default initialization of member variables
  void Location::trackstop::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:30 Path
  Path::Path(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Object(planDatabase, "Path", name, true) {
  }
  Path::Path(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Object(planDatabase, type, name, true) {
  }
  Path::Path(const ObjectId& parent, const LabelStr& name)
   : Object(parent, "Path", name, true) {}
  Path::Path(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Object(parent, type, name, true) {}
  // default initialization of member variables
  void Path::handleDefaults(bool autoClose) {
    check_error(m_from.isValid(), "object variables must be initialized explicitly");
    
    check_error(m_to.isValid(), "object variables must be initialized explicitly");
    
    if(autoClose) close();
  }
  
  
  // k9-initial.nddl:30 Path
  void Path::constructor(const LocationId& from, const LocationId& to) {
    m_from = addVariable(LocationDomain(from, "Location"), "m_from");
    m_to = addVariable(LocationDomain(to, "Location"), "m_to");
  }
  
  // k9-initial.nddl:30 Path
  class PathFactory3: public ConcreteObjectFactory {
  public:
    PathFactory3(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 2);
      check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                                TypeFactory::baseDomain("Location")), 
                  "Cannot convert " + arguments[0].first.toString() + " to Location");
      check_error(arguments[0].second->isSingleton());
      LocationId from((LocationId)arguments[0].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[1].second, 
                                                TypeFactory::baseDomain("Location")), 
                  "Cannot convert " + arguments[1].first.toString() + " to Location");
      check_error(arguments[1].second->isSingleton());
      LocationId to((LocationId)arguments[1].second->getSingletonValue());
      
      PathId instance = (new Path(planDb, objectType, objectName))->getId();
      instance->constructor(from, to);
      instance->handleDefaults();
      return instance;
    }
  };
  
  
  // k9-initial.nddl:39 Position
  Position::Position(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "Position", name, true) {
  }
  Position::Position(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  Position::Position(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "Position", name, true) {}
  Position::Position(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void Position::handleDefaults(bool autoClose) {
    if(autoClose) close();
  }
  
  // implicit constructor
  void Position::constructor() {
  }
  
  
  // k9-initial.nddl:39 Position
  DECLARE_DEFAULT_OBJECT_FACTORY(PositionFactory4, Position);
  
  
  // k9-initial.nddl:40 At
  Position::At::At(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(autoClose);
  }
  
  Position::At::At(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, false) {
    handleDefaults(autoClose);
  }
  
  // default initialization of member variables
  void Position::At::handleDefaults(const bool& autoClose) {
    if(location.isNoId()){
      location = addParameter(ObjectDomain("Location"), "location");
      completeObjectParam(Location, location);
    }
    if (autoClose)
      close();
  }
  
  
  
  // k9-initial.nddl:44 navigate
  Position::navigate::navigate(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(autoClose);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("from")));
      vars.push_back(var(getId(),std::string("to")));
      token_constraint(neq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("duration")));
      vars.push_back(predicateVariable(IntervalIntDomain(1)));
      token_constraint(eq, vars);
    }
  }
  
  Position::navigate::navigate(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, false) {
    handleDefaults(autoClose);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("from")));
      vars.push_back(var(getId(),std::string("to")));
      token_constraint(neq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("duration")));
      vars.push_back(predicateVariable(IntervalIntDomain(1)));
      token_constraint(eq, vars);
    }
  }
  
  // default initialization of member variables
  void Position::navigate::handleDefaults(const bool& autoClose) {
    if(from.isNoId()){
      from = addParameter(ObjectDomain("Location"), "from");
      completeObjectParam(Location, from);
    }
    if(to.isNoId()){
      to = addParameter(ObjectDomain("Location"), "to");
      completeObjectParam(Location, to);
    }
    if (autoClose)
      close();
  }
  
  
  // k9-initial.nddl:52 At
  class Position$At$0$0: public RuleInstance {
  public:
    Position$At$0$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Position$At$0, Position$At$0$0, Position.At, "k9-initial.nddl,52");
  
  void Position$At$0$0::handleExecute() {
    slave(Position::navigate, Position.navigate, a, LabelStr("meets"));
    meets(this, a);
    slave(Position::navigate, Position.navigate, b, LabelStr("met_by"));
    met_by(this, b);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("location")));
      vars.push_back(varfromtok(tok(getId(), std::string("a")), std::string("from")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("location")));
      vars.push_back(varfromtok(tok(getId(), std::string("b")), std::string("to")));
      rule_constraint(eq, vars);
    }
  }
  
  // k9-initial.nddl:62 navigate
  class Position$navigate$1$0: public RuleInstance {
  public:
    Position$navigate$1$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Position$navigate$1, Position$navigate$1$0, Position.navigate, "k9-initial.nddl,62");
  
  void Position$navigate$1$0::handleExecute() {
    objectVar(Path, path, true);
    slave(Position::At, Position.At, a, LabelStr("meets"));
    meets(this, a);
    slave(Position::At, Position.At, b, LabelStr("met_by"));
    met_by(this, b);
    declareFilter(Path,path);
    allocateFilterCondition(path, Location, var(getId(),std::string("from")), m_from, eq);
    allocateFilterCondition(path, Location, var(getId(),std::string("to")), m_to, eq);
    allocateFilterConstraint(path, CONSTRAIN);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("to")));
      vars.push_back(varfromtok(tok(getId(), std::string("a")), std::string("location")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("b")), std::string("location")));
      vars.push_back(var(getId(),std::string("from")));
      rule_constraint(eq, vars);
    }
  }
  
  
  // k9-initial.nddl:77 Tracker
  Tracker::Tracker(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "Tracker", name, true) {
  }
  Tracker::Tracker(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  Tracker::Tracker(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "Tracker", name, true) {}
  Tracker::Tracker(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void Tracker::handleDefaults(bool autoClose) {
    if(autoClose) close();
  }
  
  // implicit constructor
  void Tracker::constructor() {
  }
  
  
  // k9-initial.nddl:77 Tracker
  DECLARE_DEFAULT_OBJECT_FACTORY(TrackerFactory5, Tracker);
  
  
  // k9-initial.nddl:79 TrackingOff
  Tracker::TrackingOff::TrackingOff(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::TrackingOff::TrackingOff(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::TrackingOff::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:82 trackloadgroup
  Tracker::trackloadgroup::trackloadgroup(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::trackloadgroup::trackloadgroup(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::trackloadgroup::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:85 LandmarksDefined
  Tracker::LandmarksDefined::LandmarksDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::LandmarksDefined::LandmarksDefined(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::LandmarksDefined::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:88 StartTracking
  Tracker::StartTracking::StartTracking(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::StartTracking::StartTracking(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::StartTracking::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:91 TrackingOn
  Tracker::TrackingOn::TrackingOn(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::TrackingOn::TrackingOn(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::TrackingOn::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:94 trackfreeze
  Tracker::trackfreeze::trackfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::trackfreeze::trackfreeze(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::trackfreeze::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:97 TrackingFrozen
  Tracker::TrackingFrozen::TrackingFrozen(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::TrackingFrozen::TrackingFrozen(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::TrackingFrozen::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:100 trackunfreeze
  Tracker::trackunfreeze::trackunfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  Tracker::trackunfreeze::trackunfreeze(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void Tracker::trackunfreeze::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:105 OpportunisticScience
  OpportunisticScience::OpportunisticScience(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "OpportunisticScience", name, true) {
  }
  OpportunisticScience::OpportunisticScience(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  OpportunisticScience::OpportunisticScience(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "OpportunisticScience", name, true) {}
  OpportunisticScience::OpportunisticScience(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void OpportunisticScience::handleDefaults(bool autoClose) {
    if(autoClose) close();
  }
  
  // implicit constructor
  void OpportunisticScience::constructor() {
  }
  
  
  // k9-initial.nddl:105 OpportunisticScience
  DECLARE_DEFAULT_OBJECT_FACTORY(OpportunisticScienceFactory6, OpportunisticScience);
  
  
  // k9-initial.nddl:107 OppSciIdle
  OpportunisticScience::OppSciIdle::OppSciIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  OpportunisticScience::OppSciIdle::OppSciIdle(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void OpportunisticScience::OppSciIdle::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:110 oppscidefineproc
  OpportunisticScience::oppscidefineproc::oppscidefineproc(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  OpportunisticScience::oppscidefineproc::oppscidefineproc(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void OpportunisticScience::oppscidefineproc::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:113 OppSciProcDefined
  OpportunisticScience::OppSciProcDefined::OppSciProcDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  OpportunisticScience::OppSciProcDefined::OppSciProcDefined(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void OpportunisticScience::OppSciProcDefined::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:116 oppscisetparams
  OpportunisticScience::oppscisetparams::oppscisetparams(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  OpportunisticScience::oppscisetparams::oppscisetparams(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void OpportunisticScience::oppscisetparams::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:119 OppSciParamsSet
  OpportunisticScience::OppSciParamsSet::OppSciParamsSet(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  OpportunisticScience::OppSciParamsSet::OppSciParamsSet(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void OpportunisticScience::OppSciParamsSet::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:122 oppscilooknow
  OpportunisticScience::oppscilooknow::oppscilooknow(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("target")));
      vars.push_back(var(getId(),std::string("at_loc")));
      token_constraint(neq, vars);
    }
  }
  
  OpportunisticScience::oppscilooknow::oppscilooknow(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("target")));
      vars.push_back(var(getId(),std::string("at_loc")));
      token_constraint(neq, vars);
    }
  }
  
  // default initialization of member variables
  void OpportunisticScience::oppscilooknow::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:128 OppSciDoneLookNow
  OpportunisticScience::OppSciDoneLookNow::OppSciDoneLookNow(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("target")));
      vars.push_back(var(getId(),std::string("at_loc")));
      token_constraint(neq, vars);
    }
  }
  
  OpportunisticScience::OppSciDoneLookNow::OppSciDoneLookNow(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("target")));
      vars.push_back(var(getId(),std::string("at_loc")));
      token_constraint(neq, vars);
    }
  }
  
  // default initialization of member variables
  void OpportunisticScience::OppSciDoneLookNow::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:134 oppscigetstatus
  OpportunisticScience::oppscigetstatus::oppscigetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("target")));
      vars.push_back(var(getId(),std::string("at_loc")));
      token_constraint(neq, vars);
    }
  }
  
  OpportunisticScience::oppscigetstatus::oppscigetstatus(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("target")));
      vars.push_back(var(getId(),std::string("at_loc")));
      token_constraint(neq, vars);
    }
  }
  
  // default initialization of member variables
  void OpportunisticScience::oppscigetstatus::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:141 CHAMP
  CHAMP::CHAMP(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "CHAMP", name, true) {
  }
  CHAMP::CHAMP(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  CHAMP::CHAMP(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "CHAMP", name, true) {}
  CHAMP::CHAMP(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void CHAMP::handleDefaults(bool autoClose) {
    if(autoClose) close();
  }
  
  // implicit constructor
  void CHAMP::constructor() {
  }
  
  
  // k9-initial.nddl:141 CHAMP
  DECLARE_DEFAULT_OBJECT_FACTORY(CHAMPFactory7, CHAMP);
  
  
  // k9-initial.nddl:142 IPIdle
  CHAMP::IPIdle::IPIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::IPIdle::IPIdle(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::IPIdle::handleDefaults() {
    close();
  }
  
  
  
  // k9-initial.nddl:145 ipgetname
  CHAMP::ipgetname::ipgetname(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::ipgetname::ipgetname(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::ipgetname::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:151 IPHaveName
  CHAMP::IPHaveName::IPHaveName(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::IPHaveName::IPHaveName(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::IPHaveName::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:156 ipsettarget
  CHAMP::ipsettarget::ipsettarget(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::ipsettarget::ipsettarget(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::ipsettarget::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:162 IPTargetSet
  CHAMP::IPTargetSet::IPTargetSet(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::IPTargetSet::IPTargetSet(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::IPTargetSet::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:167 ipplaceinstrument
  CHAMP::ipplaceinstrument::ipplaceinstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::ipplaceinstrument::ipplaceinstrument(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::ipplaceinstrument::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:173 IPDonePlaceInstrument
  CHAMP::IPDonePlaceInstrument::IPDonePlaceInstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::IPDonePlaceInstrument::IPDonePlaceInstrument(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::IPDonePlaceInstrument::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:178 ipgetstatus
  CHAMP::ipgetstatus::ipgetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable, const bool& autoClose)
   : NddlToken(planDatabase, name, rejectable, autoClose) {
    handleDefaults();
  }
  
  CHAMP::ipgetstatus::ipgetstatus(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose)
    : NddlToken(parent, relation, name, autoClose) {
    handleDefaults();
  }
  
  // default initialization of member variables
  void CHAMP::ipgetstatus::handleDefaults() {
    if(target.isNoId()){
      target = addParameter(ObjectDomain("Location"), "target");
      completeObjectParam(Location, target);
    }
    if(at_loc.isNoId()){
      at_loc = addParameter(ObjectDomain("Location"), "at_loc");
      completeObjectParam(Location, at_loc);
    }
    close();
  }
  
  
  
  // k9-initial.nddl:185 Rover
  Rover::Rover(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Object(planDatabase, "Rover", name, true) {
  }
  Rover::Rover(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Object(planDatabase, type, name, true) {
  }
  Rover::Rover(const ObjectId& parent, const LabelStr& name)
   : Object(parent, "Rover", name, true) {}
  Rover::Rover(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Object(parent, type, name, true) {}
  // default initialization of member variables
  void Rover::handleDefaults(bool autoClose) {
    check_error(m_position.isValid(), "object variables must be initialized explicitly");
    
    check_error(m_tracker.isValid(), "object variables must be initialized explicitly");
    
    check_error(m_oppsci.isValid(), "object variables must be initialized explicitly");
    
    check_error(m_champ.isValid(), "object variables must be initialized explicitly");
    
    if(autoClose) close();
  }
  
  
  // k9-initial.nddl:185 Rover
  void Rover::constructor() {
    m_position = addVariable(PositionDomain((new Position(m_id, "m_position"))->getId(), "Position"), "m_position");
    Id<Position>(singleton(m_position))->constructor();
    Id<Position>(singleton(m_position))->handleDefaults();
    m_tracker = addVariable(TrackerDomain((new Tracker(m_id, "m_tracker"))->getId(), "Tracker"), "m_tracker");
    Id<Tracker>(singleton(m_tracker))->constructor();
    Id<Tracker>(singleton(m_tracker))->handleDefaults();
    m_oppsci = addVariable(OpportunisticScienceDomain((new OpportunisticScience(m_id, "m_oppsci"))->getId(), "OpportunisticScience"), "m_oppsci");
    Id<OpportunisticScience>(singleton(m_oppsci))->constructor();
    Id<OpportunisticScience>(singleton(m_oppsci))->handleDefaults();
    m_champ = addVariable(CHAMPDomain((new CHAMP(m_id, "m_champ"))->getId(), "CHAMP"), "m_champ");
    Id<CHAMP>(singleton(m_champ))->constructor();
    Id<CHAMP>(singleton(m_champ))->handleDefaults();
  }
  
  // k9-initial.nddl:185 Rover
  class RoverFactory8: public ConcreteObjectFactory {
  public:
    RoverFactory8(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 0);
      RoverId instance = (new Rover(planDb, objectType, objectName))->getId();
      instance->constructor();
      instance->handleDefaults();
      return instance;
    }
  };
  
} // namespace NDDL


namespace NDDL {
  SchemaId schema(){
    Id<Schema> id = Schema::instance("System/test/k9-initial");

    
    // Invoke commands to populate schema with type definitions
    id->addObjectType(LabelStr("Object"));
    id->addObjectType(LabelStr("Timeline"), LabelStr("Object"));
    id->addObjectType(LabelStr("NddlResource"));
    id->addObjectType("NddlWorld", "Timeline");
    id->addMember("NddlWorld", "int", "m_horizonStart");
    id->addMember("NddlWorld", "int", "m_horizonEnd");
    id->addMember("NddlWorld", "int", "m_maxPlannerSteps");
    id->addPredicate("NddlWorld.initialState");
    id->addObjectType("Location", "Timeline");
    id->addPredicate("Location.NotTracked");
    id->addMember("Location.NotTracked", "Location", "target");
    id->addPredicate("Location.trackstart");
    id->addMember("Location.trackstart", "Location", "target");
    id->addPredicate("Location.Tracked");
    id->addMember("Location.Tracked", "Location", "target");
    id->addPredicate("Location.trackstop");
    id->addMember("Location.trackstop", "Location", "target");
    id->addObjectType("Path", "Object");
    id->addMember("Path", "Location", "m_from");
    id->addMember("Path", "Location", "m_to");
    id->addObjectType("Position", "Timeline");
    id->addPredicate("Position.At");
    id->addMember("Position.At", "Location", "location");
    id->addPredicate("Position.navigate");
    id->addMember("Position.navigate", "Location", "from");
    id->addMember("Position.navigate", "Location", "to");
    id->addObjectType("Tracker", "Timeline");
    id->addPredicate("Tracker.TrackingOff");
    id->addPredicate("Tracker.trackloadgroup");
    id->addPredicate("Tracker.LandmarksDefined");
    id->addPredicate("Tracker.StartTracking");
    id->addPredicate("Tracker.TrackingOn");
    id->addPredicate("Tracker.trackfreeze");
    id->addPredicate("Tracker.TrackingFrozen");
    id->addPredicate("Tracker.trackunfreeze");
    id->addObjectType("OpportunisticScience", "Timeline");
    id->addPredicate("OpportunisticScience.OppSciIdle");
    id->addPredicate("OpportunisticScience.oppscidefineproc");
    id->addPredicate("OpportunisticScience.OppSciProcDefined");
    id->addPredicate("OpportunisticScience.oppscisetparams");
    id->addPredicate("OpportunisticScience.OppSciParamsSet");
    id->addPredicate("OpportunisticScience.oppscilooknow");
    id->addMember("OpportunisticScience.oppscilooknow", "Location", "target");
    id->addMember("OpportunisticScience.oppscilooknow", "Location", "at_loc");
    id->addPredicate("OpportunisticScience.OppSciDoneLookNow");
    id->addMember("OpportunisticScience.OppSciDoneLookNow", "Location", "target");
    id->addMember("OpportunisticScience.OppSciDoneLookNow", "Location", "at_loc");
    id->addPredicate("OpportunisticScience.oppscigetstatus");
    id->addMember("OpportunisticScience.oppscigetstatus", "Location", "target");
    id->addMember("OpportunisticScience.oppscigetstatus", "Location", "at_loc");
    id->addObjectType("CHAMP", "Timeline");
    id->addPredicate("CHAMP.IPIdle");
    id->addPredicate("CHAMP.ipgetname");
    id->addMember("CHAMP.ipgetname", "Location", "target");
    id->addMember("CHAMP.ipgetname", "Location", "at_loc");
    id->addPredicate("CHAMP.IPHaveName");
    id->addMember("CHAMP.IPHaveName", "Location", "target");
    id->addMember("CHAMP.IPHaveName", "Location", "at_loc");
    id->addPredicate("CHAMP.ipsettarget");
    id->addMember("CHAMP.ipsettarget", "Location", "target");
    id->addMember("CHAMP.ipsettarget", "Location", "at_loc");
    id->addPredicate("CHAMP.IPTargetSet");
    id->addMember("CHAMP.IPTargetSet", "Location", "target");
    id->addMember("CHAMP.IPTargetSet", "Location", "at_loc");
    id->addPredicate("CHAMP.ipplaceinstrument");
    id->addMember("CHAMP.ipplaceinstrument", "Location", "target");
    id->addMember("CHAMP.ipplaceinstrument", "Location", "at_loc");
    id->addPredicate("CHAMP.IPDonePlaceInstrument");
    id->addMember("CHAMP.IPDonePlaceInstrument", "Location", "target");
    id->addMember("CHAMP.IPDonePlaceInstrument", "Location", "at_loc");
    id->addPredicate("CHAMP.ipgetstatus");
    id->addMember("CHAMP.ipgetstatus", "Location", "target");
    id->addMember("CHAMP.ipgetstatus", "Location", "at_loc");
    id->addObjectType("Rover", "Object");
    id->addMember("Rover", "Position", "m_position");
    id->addMember("Rover", "Tracker", "m_tracker");
    id->addMember("Rover", "OpportunisticScience", "m_oppsci");
    id->addMember("Rover", "CHAMP", "m_champ");
    id->addMember("Position.navigate", "Path", "path");
    // Force allocation of model specific type factories
    // REGISTER FACTORIES
    
    // REGISTER FACTORIES
    REGISTER_TOKEN_FACTORY(NddlWorld::initialState::Factory);
    REGISTER_TOKEN_FACTORY(Location::NotTracked::Factory);
    REGISTER_TOKEN_FACTORY(Location::trackstart::Factory);
    REGISTER_TOKEN_FACTORY(Location::Tracked::Factory);
    REGISTER_TOKEN_FACTORY(Location::trackstop::Factory);
    REGISTER_TOKEN_FACTORY(Position::At::Factory);
    REGISTER_TOKEN_FACTORY(Position::navigate::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::TrackingOff::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::trackloadgroup::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::LandmarksDefined::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::StartTracking::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::TrackingOn::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::trackfreeze::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::TrackingFrozen::Factory);
    REGISTER_TOKEN_FACTORY(Tracker::trackunfreeze::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::OppSciIdle::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::oppscidefineproc::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::OppSciProcDefined::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::oppscisetparams::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::OppSciParamsSet::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::oppscilooknow::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::OppSciDoneLookNow::Factory);
    REGISTER_TOKEN_FACTORY(OpportunisticScience::oppscigetstatus::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::IPIdle::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::ipgetname::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::IPHaveName::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::ipsettarget::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::IPTargetSet::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::ipplaceinstrument::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::IPDonePlaceInstrument::Factory);
    REGISTER_TOKEN_FACTORY(CHAMP::ipgetstatus::Factory);
    REGISTER_TYPE_FACTORY(NddlWorld, ObjectDomain("NddlWorld"));
    REGISTER_OBJECT_FACTORY(NddlWorldFactory0, NddlWorld:int:int:int);
    REGISTER_OBJECT_FACTORY(NddlWorldFactory1, NddlWorld);
    REGISTER_TYPE_FACTORY(Location, ObjectDomain("Location"));
    REGISTER_OBJECT_FACTORY(LocationFactory2, Location);
    REGISTER_TYPE_FACTORY(Path, ObjectDomain("Path"));
    REGISTER_OBJECT_FACTORY(PathFactory3, Path:Location:Location);
    REGISTER_OBJECT_FACTORY(PathFactory3, Path:Location:Timeline);
    REGISTER_OBJECT_FACTORY(PathFactory3, Path:Timeline:Location);
    REGISTER_OBJECT_FACTORY(PathFactory3, Path:Timeline:Timeline);
    REGISTER_TYPE_FACTORY(Position, ObjectDomain("Position"));
    REGISTER_OBJECT_FACTORY(PositionFactory4, Position);
    REGISTER_TYPE_FACTORY(Tracker, ObjectDomain("Tracker"));
    REGISTER_OBJECT_FACTORY(TrackerFactory5, Tracker);
    REGISTER_TYPE_FACTORY(OpportunisticScience, ObjectDomain("OpportunisticScience"));
    REGISTER_OBJECT_FACTORY(OpportunisticScienceFactory6, OpportunisticScience);
    REGISTER_TYPE_FACTORY(CHAMP, ObjectDomain("CHAMP"));
    REGISTER_OBJECT_FACTORY(CHAMPFactory7, CHAMP);
    REGISTER_TYPE_FACTORY(Rover, ObjectDomain("Rover"));
    REGISTER_OBJECT_FACTORY(RoverFactory8, Rover);
    
    // Allocate rules
    new Rule$Position$navigate$1();
    new Rule$Position$At$0();
    return id;
  }
  
}

bool runTestLangTest(){
    SamplePlanDatabase db1(schema, replay);
    averInit(db1.planDatabase, PLASMA::DecisionManagerId::noId(),
                 db1.constraintEngine, db1.rulesEngine);

    DbClientId client = db1.planDatabase->getClient();

    std::vector<ConstructorArgument> arguments;
    arguments.push_back(ConstructorArgument(LabelStr("int"), new IntervalIntDomain(0)));
    arguments.push_back(ConstructorArgument(LabelStr("int"), new IntervalIntDomain(100)));
    arguments.push_back(ConstructorArgument(LabelStr("int"), new IntervalIntDomain(500)));
    NDDL::NddlWorldId world = client->createObject("NddlWorld", "world", arguments);

    NDDL::RoverId rover = client->createObject("Rover", "rover");
    NDDL::LocationId l1 = client->createObject("Location", "l1");
    NDDL::LocationId l2 = client->createObject("Location", "l2");
    NDDL::LocationId l3 = client->createObject("Location", "l3");
    NDDL::LocationId l4 = client->createObject("Location", "l4");
    NDDL::LocationId l5 = client->createObject("Location", "l5");
    NDDL::LocationId l6 = client->createObject("Location", "l6");

    std::vector<ConstructorArgument> arguments1;
    arguments1.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l1, "Location")));
    arguments1.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l2, "Location")));
    client->createObject("Path", "p1", arguments1);

    std::vector<ConstructorArgument> arguments2;
    arguments2.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l1, "Location")));
    arguments2.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l3, "Location")));
    client->createObject("Path", "p2", arguments2);

    std::vector<ConstructorArgument> arguments3;
    arguments3.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l2, "Location")));
    arguments3.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l5, "Location")));
    client->createObject("Path", "p3", arguments3);

    std::vector<ConstructorArgument> arguments4;
    arguments4.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l5, "Location")));
    arguments4.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l6, "Location")));
    client->createObject("Path", "p4", arguments4);

    std::vector<ConstructorArgument> arguments5;
    arguments5.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l3, "Location")));
    arguments5.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l6, "Location")));
    client->createObject("Path", "p5", arguments5);

    std::vector<ConstructorArgument> arguments6;
    arguments6.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l2, "Location")));
    arguments6.push_back(ConstructorArgument(LabelStr("Location"), new NDDL::LocationDomain(l3, "Location")));
    client->createObject("Path", "p6", arguments6);

    client->close();

    // Create initial state
    Id<NDDL::Position::At> a = client->createToken("Position.At");
    client->activate(a);
    client->specify(a->getObject(), rover->m_position->baseDomain().getSingletonValue());

    Id<NDDL::Position::At> b = client->createToken("Position.At");
    client->activate(b);
    client->specify(b->getObject(), rover->m_position->baseDomain().getSingletonValue());

    std::vector<ConstrainedVariableId> scope;
    scope.push_back(world->m_horizonStart);
    scope.push_back(a->start);
    client->createConstraint("leq", scope);

    std::vector<ConstrainedVariableId> scope2;
    scope2.push_back(world->m_horizonStart);
    scope2.push_back(b->start);
    client->createConstraint("leq", scope2);

    std::vector<ConstrainedVariableId> scope3;
    scope3.push_back(a->end);
    scope3.push_back(world->m_horizonEnd);
    client->createConstraint("leq", scope3);

    std::vector<ConstrainedVariableId> scope4;
    scope4.push_back(b->end);
    scope4.push_back(world->m_horizonEnd);
    client->createConstraint("leq", scope4);

    client->specify(a->location, l1);

    client->specify(b->location, l5);

    std::vector<ConstrainedVariableId> scope7;
    scope7.push_back(a->end);
    scope7.push_back(b->start);
    client->createConstraint("leq", scope7);

    assert(client->propagate());

    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = world->getVariable(LabelStr("world.m_horizonStart"));
    check_error(horizonStart.isValid());
    ConstrainedVariableId horizonEnd = world->getVariable(LabelStr("world.m_horizonEnd"));
    check_error(horizonEnd.isValid());
    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    db1.horizon->setHorizon(start, end);

    PlanDatabaseWriter::write(db1.planDatabase, std::cout);
    PLASMA::EventAggregator::instance()->notifyAssignNextSucceeded(DecisionPointId::noId());
    //db1.writer->write();
    PlanDatabaseWriter::write(db1.planDatabase, std::cout);

    averDeinit();
    return true;
}

int main(int argc, const char ** argv){
  // Initialize constraint factories
  SamplePlanDatabase::initialize();
  //schema = PLASMA::Schema::instance();
  schema = NDDL::schema();

  replay = false;
  runTest(runTestLangTest);

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
