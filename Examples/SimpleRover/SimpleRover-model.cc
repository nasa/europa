// Examples/SimpleRover/SimpleRover-model.xml

#include "SimpleRover-model.hh"
#include "NddlUtils.hh"

namespace NDDL {
  
  TokenStates TokenStatesBaseDomain(){
    static TokenStates sl_enum("TokenStates");
    if (sl_enum.isOpen()) {
      // Insert values to initialize
      sl_enum.insert(LabelStr("INACTIVE"));
      sl_enum.insert(LabelStr("ACTIVE"));
      sl_enum.insert(LabelStr("MERGED"));
      sl_enum.insert(LabelStr("REJECTED"));
      sl_enum.close();
    }
    return(sl_enum);
  }
  // SKIPPING IMPLEMENTATION FOR BUILT-IN CLASS Object
  
  // SKIPPING IMPLEMENTATION FOR BUILT-IN CLASS Timeline
  
  // SKIPPING IMPLEMENTATION FOR BUILT-IN CLASS Resource
  
  
  
  // Plasma.nddl:83 UnaryResource
  UnaryResource::UnaryResource(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "UnaryResource", name, true) {
  }
  UnaryResource::UnaryResource(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  UnaryResource::UnaryResource(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "UnaryResource", name, true) {}
  UnaryResource::UnaryResource(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void UnaryResource::handleDefaults(bool autoClose) {
    if (autoClose)
      close();
  }
  
  // implicit constructor
  void UnaryResource::constructor() {
  }
  
  
  // Plasma.nddl:83 UnaryResource
  DECLARE_DEFAULT_OBJECT_FACTORY(UnaryResourceFactory0, UnaryResource);
  
  
  // Plasma.nddl:84 uses
  UnaryResource::uses::uses(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  UnaryResource::uses::uses(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void UnaryResource::uses::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if (autoClose)
      close();
    
    // Post parameter constraints
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("state")));
      vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTIVE"), "TokenStates")));
      token_constraint(eq, vars);
    }
  }
  
  
  
  // Plasma.nddl:93 StringData
  StringData::StringData(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Object(planDatabase, "StringData", name, true) {
  }
  StringData::StringData(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Object(planDatabase, type, name, true) {
  }
  StringData::StringData(const ObjectId& parent, const LabelStr& name)
   : Object(parent, "StringData", name, true) {}
  StringData::StringData(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Object(parent, type, name, true) {}
  // default initialization of member variables
  void StringData::handleDefaults(bool autoClose) {
    if(data.isNoId()){
      data = addVariable(StringDomain("DefaultString"), "data");
    }
    if (autoClose)
      close();
  }
  
  
  // Plasma.nddl:93 StringData
  void StringData::constructor(const LabelStr& _data) {
    data = addVariable(StringDomain(_data, "string"), "data");
  }
  
  // Plasma.nddl:93 StringData
  class StringDataFactory1: public ConcreteObjectFactory {
  public:
    StringDataFactory1(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 1);
      check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                                TypeFactory::baseDomain("string")), 
                  "Cannot convert " + arguments[0].first.toString() + " to string");
      check_error(arguments[0].second->isSingleton());
      LabelStr _data((LabelStr)arguments[0].second->getSingletonValue());
      
      StringDataId instance = (new StringData(planDb, objectType, objectName))->getId();
      instance->constructor(_data);
      instance->handleDefaults();
      return instance;
    }
  };
  
  
  // PlannerConfig.nddl:7 PlannerConfig
  PlannerConfig::PlannerConfig(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Object(planDatabase, "PlannerConfig", name, true) {
  }
  PlannerConfig::PlannerConfig(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Object(planDatabase, type, name, true) {
  }
  PlannerConfig::PlannerConfig(const ObjectId& parent, const LabelStr& name)
   : Object(parent, "PlannerConfig", name, true) {}
  PlannerConfig::PlannerConfig(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Object(parent, type, name, true) {}
  // default initialization of member variables
  void PlannerConfig::handleDefaults(bool autoClose) {
    if(m_horizonStart.isNoId()){
      m_horizonStart = addVariable(IntervalIntDomain("int"), "m_horizonStart");
    }
    if(m_horizonEnd.isNoId()){
      m_horizonEnd = addVariable(IntervalIntDomain("int"), "m_horizonEnd");
    }
    if(m_maxPlannerSteps.isNoId()){
      m_maxPlannerSteps = addVariable(IntervalIntDomain("int"), "m_maxPlannerSteps");
    }
    if(m_maxPlannerDepth.isNoId()){
      m_maxPlannerDepth = addVariable(IntervalIntDomain("int"), "m_maxPlannerDepth");
    }
    if (autoClose)
      close();
  }
  
  
  // PlannerConfig.nddl:7 PlannerConfig
  void PlannerConfig::constructor(int start, int end, int maxPlannerSteps, int maxPlannerDepth) {
    m_horizonStart = addVariable(IntervalIntDomain(start, start, "int"), "m_horizonStart");
    m_horizonEnd = addVariable(IntervalIntDomain(end, end, "int"), "m_horizonEnd");
    m_maxPlannerSteps = addVariable(IntervalIntDomain(maxPlannerSteps, maxPlannerSteps, "int"), "m_maxPlannerSteps");
    m_maxPlannerDepth = addVariable(IntervalIntDomain(maxPlannerDepth, maxPlannerDepth, "int"), "m_maxPlannerDepth");
  }
  
  // PlannerConfig.nddl:7 PlannerConfig
  class PlannerConfigFactory2: public ConcreteObjectFactory {
  public:
    PlannerConfigFactory2(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 4);
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
      
      check_error(AbstractDomain::canBeCompared(*arguments[3].second, 
                                                TypeFactory::baseDomain("int")), 
                  "Cannot convert " + arguments[3].first.toString() + " to int");
      check_error(arguments[3].second->isSingleton());
      int maxPlannerDepth((int)arguments[3].second->getSingletonValue());
      
      PlannerConfigId instance = (new PlannerConfig(planDb, objectType, objectName))->getId();
      instance->constructor(start, end, maxPlannerSteps, maxPlannerDepth);
      instance->handleDefaults();
      return instance;
    }
  };
  
  // PlannerConfig.nddl:7 PlannerConfig
  void PlannerConfig::constructor(int start, int end, int maxPlannerSteps) {
    m_horizonStart = addVariable(IntervalIntDomain(start, start, "int"), "m_horizonStart");
    m_horizonEnd = addVariable(IntervalIntDomain(end, end, "int"), "m_horizonEnd");
    m_maxPlannerSteps = addVariable(IntervalIntDomain(maxPlannerSteps, maxPlannerSteps, "int"), "m_maxPlannerSteps");
    m_maxPlannerDepth = addVariable(IntervalIntDomain(+inf, +inf, "int"), "m_maxPlannerDepth");
  }
  
  // PlannerConfig.nddl:7 PlannerConfig
  class PlannerConfigFactory3: public ConcreteObjectFactory {
  public:
    PlannerConfigFactory3(const LabelStr& name): ConcreteObjectFactory(name){}
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
      
      PlannerConfigId instance = (new PlannerConfig(planDb, objectType, objectName))->getId();
      instance->constructor(start, end, maxPlannerSteps);
      instance->handleDefaults();
      return instance;
    }
  };
  
  // PlannerConfig.nddl:7 PlannerConfig
  void PlannerConfig::constructor() {
    m_horizonStart = addVariable(IntervalIntDomain(0, 0, "int"), "m_horizonStart");
    m_horizonEnd = addVariable(IntervalIntDomain(100, 100, "int"), "m_horizonEnd");
    m_maxPlannerSteps = addVariable(IntervalIntDomain(200, 200, "int"), "m_maxPlannerSteps");
    m_maxPlannerDepth = addVariable(IntervalIntDomain(+inf, +inf, "int"), "m_maxPlannerDepth");
  }
  
  // PlannerConfig.nddl:7 PlannerConfig
  class PlannerConfigFactory4: public ConcreteObjectFactory {
  public:
    PlannerConfigFactory4(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 0);
      PlannerConfigId instance = (new PlannerConfig(planDb, objectType, objectName))->getId();
      instance->constructor();
      instance->handleDefaults();
      return instance;
    }
  };
  
  
  // SimpleRover-model.nddl:8 Location
  Location::Location(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Object(planDatabase, "Location", name, true) {
  }
  Location::Location(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Object(planDatabase, type, name, true) {
  }
  Location::Location(const ObjectId& parent, const LabelStr& name)
   : Object(parent, "Location", name, true) {}
  Location::Location(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Object(parent, type, name, true) {}
  // default initialization of member variables
  void Location::handleDefaults(bool autoClose) {
    if(name.isNoId()){
      name = addVariable(StringDomain("DefaultString"), "name");
    }
    if(x.isNoId()){
      x = addVariable(IntervalIntDomain("int"), "x");
    }
    if(y.isNoId()){
      y = addVariable(IntervalIntDomain("int"), "y");
    }
    if (autoClose)
      close();
  }
  
  
  // SimpleRover-model.nddl:8 Location
  void Location::constructor(const LabelStr& _name, int _x, int _y) {
    name = addVariable(StringDomain(_name, "string"), "name");
    x = addVariable(IntervalIntDomain(_x, _x, "int"), "x");
    y = addVariable(IntervalIntDomain(_y, _y, "int"), "y");
  }
  
  // SimpleRover-model.nddl:8 Location
  class LocationFactory5: public ConcreteObjectFactory {
  public:
    LocationFactory5(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 3);
      check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                                TypeFactory::baseDomain("string")), 
                  "Cannot convert " + arguments[0].first.toString() + " to string");
      check_error(arguments[0].second->isSingleton());
      LabelStr _name((LabelStr)arguments[0].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[1].second, 
                                                TypeFactory::baseDomain("int")), 
                  "Cannot convert " + arguments[1].first.toString() + " to int");
      check_error(arguments[1].second->isSingleton());
      int _x((int)arguments[1].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[2].second, 
                                                TypeFactory::baseDomain("int")), 
                  "Cannot convert " + arguments[2].first.toString() + " to int");
      check_error(arguments[2].second->isSingleton());
      int _y((int)arguments[2].second->getSingletonValue());
      
      LocationId instance = (new Location(planDb, objectType, objectName))->getId();
      instance->constructor(_name, _x, _y);
      instance->handleDefaults();
      return instance;
    }
  };
  
  
  // SimpleRover-model.nddl:24 Path
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
    if(name.isNoId()){
      name = addVariable(StringDomain("DefaultString"), "name");
    }
    check_error(from.isValid(), "object variables must be initialized explicitly");
    
    check_error(to.isValid(), "object variables must be initialized explicitly");
    
    if(cost.isNoId()){
      cost = addVariable(IntervalDomain("float"), "cost");
    }
    if (autoClose)
      close();
  }
  
  
  // SimpleRover-model.nddl:24 Path
  void Path::constructor(const LabelStr& _name, const LocationId& _from, const LocationId& _to, float _cost) {
    name = addVariable(StringDomain(_name, "string"), "name");
    from = addVariable(LocationDomain(_from, "Location"), "from");
    to = addVariable(LocationDomain(_to, "Location"), "to");
    cost = addVariable(IntervalDomain(_cost, _cost, "float"), "cost");
  }
  
  // SimpleRover-model.nddl:24 Path
  class PathFactory6: public ConcreteObjectFactory {
  public:
    PathFactory6(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 4);
      check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                                TypeFactory::baseDomain("string")), 
                  "Cannot convert " + arguments[0].first.toString() + " to string");
      check_error(arguments[0].second->isSingleton());
      LabelStr _name((LabelStr)arguments[0].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[1].second, 
                                                TypeFactory::baseDomain("Location")), 
                  "Cannot convert " + arguments[1].first.toString() + " to Location");
      check_error(arguments[1].second->isSingleton());
      LocationId _from((LocationId)arguments[1].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[2].second, 
                                                TypeFactory::baseDomain("Location")), 
                  "Cannot convert " + arguments[2].first.toString() + " to Location");
      check_error(arguments[2].second->isSingleton());
      LocationId _to((LocationId)arguments[2].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[3].second, 
                                                TypeFactory::baseDomain("float")), 
                  "Cannot convert " + arguments[3].first.toString() + " to float");
      check_error(arguments[3].second->isSingleton());
      float _cost((float)arguments[3].second->getSingletonValue());
      
      PathId instance = (new Path(planDb, objectType, objectName))->getId();
      instance->constructor(_name, _from, _to, _cost);
      instance->handleDefaults();
      return instance;
    }
  };
  
  
  // SimpleRover-model.nddl:42 Navigator
  Navigator::Navigator(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "Navigator", name, true) {
  }
  Navigator::Navigator(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  Navigator::Navigator(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "Navigator", name, true) {}
  Navigator::Navigator(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void Navigator::handleDefaults(bool autoClose) {
    if (autoClose)
      close();
  }
  
  // implicit constructor
  void Navigator::constructor() {
  }
  
  
  // SimpleRover-model.nddl:42 Navigator
  DECLARE_DEFAULT_OBJECT_FACTORY(NavigatorFactory7, Navigator);
  
  
  // SimpleRover-model.nddl:45 At
  Navigator::At::At(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Navigator::At::At(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Navigator::At::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if(location.isNoId()){
      location = addParameter(ObjectDomain("Location"), "location");
      completeObjectParam(Location, location);
    }
    if (autoClose)
      close();
  }
  
  
  
  // SimpleRover-model.nddl:50 Going
  Navigator::Going::Going(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Navigator::Going::Going(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Navigator::Going::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
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
    
    // Post parameter constraints
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("from")));
      vars.push_back(var(getId(),std::string("to")));
      token_constraint(neq, vars);
    }
  }
  
  
  // SimpleRover-model.nddl:57 At
  class Navigator$At$0$0: public RuleInstance {
  public:
    Navigator$At$0$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Navigator$At$0, Navigator$At$0$0, Navigator.At, "SimpleRover-model.nddl,57");
  
  void Navigator$At$0$0::handleExecute() {
    slave(Navigator::Going, Navigator.Going, from, LabelStr("met_by"));
    sameObject(object, from);
    met_by(this, from);
    slave(Navigator::Going, Navigator.Going, to, LabelStr("meets"));
    sameObject(object, to);
    meets(this, to);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("from")), std::string("to")));
      vars.push_back(var(getId(),std::string("location")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("to")), std::string("from")));
      vars.push_back(var(getId(),std::string("location")));
      rule_constraint(eq, vars);
    }
  }
  
  // SimpleRover-model.nddl:64 Going
  class Navigator$Going$1$0: public RuleInstance {
  public:
    Navigator$Going$1$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  class Navigator$Going$1$0$0: public RuleInstance {
  public:
    Navigator$Going$1$0$0(const RuleInstanceId& parent, const std::vector<ConstrainedVariableId>& vars)
    : RuleInstance(parent, vars){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Navigator$Going$1, Navigator$Going$1$0, Navigator.Going, "SimpleRover-model.nddl,64");
  
  void Navigator$Going$1$0::handleExecute() {
    objectVar(Path, p, true, false);
    slave(Navigator::At, Navigator.At, _from, LabelStr("met_by"));
    sameObject(object, _from);
    met_by(this, _from);
    slave(Navigator::At, Navigator.At, _to, LabelStr("meets"));
    sameObject(object, _to);
    meets(this, _to);
    slave(Battery::change, Battery.change, tx, LabelStr("starts"));
    starts(this, tx);
    declareFilter(Path,p);
    allocateFilterCondition(p, Location, var(getId(),std::string("from")), from, eq);
    allocateFilterCondition(p, Location, var(getId(),std::string("to")), to, eq);
    allocateFilterConstraint(p);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("_from")), std::string("location")));
      vars.push_back(var(getId(),std::string("from")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("_to")), std::string("location")));
      vars.push_back(var(getId(),std::string("to")));
      rule_constraint(eq, vars);
    }
    addChildRule(new Navigator$Going$1$0$0(m_id, getVariables(":p:")));
  }
  void Navigator$Going$1$0$0::handleExecute() {
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("tx")), std::string("quantity")));
      vars.push_back(varfromobject(object(getId(),std::string("p")), std::string("cost")));
      rule_constraint(eq, vars);
    }
  }
  
  
  // SimpleRover-model.nddl:83 Commands
  Commands::Commands(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "Commands", name, true) {
  }
  Commands::Commands(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  Commands::Commands(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "Commands", name, true) {}
  Commands::Commands(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void Commands::handleDefaults(bool autoClose) {
    if (autoClose)
      close();
  }
  
  // implicit constructor
  void Commands::constructor() {
  }
  
  
  // SimpleRover-model.nddl:83 Commands
  DECLARE_DEFAULT_OBJECT_FACTORY(CommandsFactory8, Commands);
  
  
  // SimpleRover-model.nddl:84 TakeSample
  Commands::TakeSample::TakeSample(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Commands::TakeSample::TakeSample(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Commands::TakeSample::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if(rock.isNoId()){
      rock = addParameter(ObjectDomain("Location"), "rock");
      completeObjectParam(Location, rock);
    }
    if (autoClose)
      close();
    
    // Post parameter constraints
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(var(getId(),std::string("duration")));
      vars.push_back(predicateVariable(IntervalIntDomain(20, 25, "int")));
      token_constraint(eq, vars);
    }
  }
  
  
  
  // SimpleRover-model.nddl:90 PhoneHome
  Commands::PhoneHome::PhoneHome(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Commands::PhoneHome::PhoneHome(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Commands::PhoneHome::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if (autoClose)
      close();
  }
  
  
  
  // SimpleRover-model.nddl:93 PhoneLander
  Commands::PhoneLander::PhoneLander(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Commands::PhoneLander::PhoneLander(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Commands::PhoneLander::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if (autoClose)
      close();
  }
  
  
  // SimpleRover-model.nddl:97 TakeSample
  class Commands$TakeSample$2$0: public RuleInstance {
  public:
    Commands$TakeSample$2$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  class Commands$TakeSample$2$0$0: public RuleInstance {
  public:
    Commands$TakeSample$2$0$0(const RuleInstanceId& parent, const ConstrainedVariableId& var, const AbstractDomain& domain)
    : RuleInstance(parent, var, domain){}
    void handleExecute();
  };
  
  class Commands$TakeSample$2$0$1: public RuleInstance {
  public:
    Commands$TakeSample$2$0$1(const RuleInstanceId& parent, const ConstrainedVariableId& var, const AbstractDomain& domain)
    : RuleInstance(parent, var, domain){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Commands$TakeSample$2, Commands$TakeSample$2$0, Commands.TakeSample, "SimpleRover-model.nddl,97");
  
  void Commands$TakeSample$2$0::handleExecute() {
    objectVar(Rover, rovers, false, false);
    localVar(BoolDomain(), OR, true);
    slave(Instrument::TakeSample, Instrument.TakeSample, a, LabelStr("contains"));
    contains(this, a);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("a")), std::string("rock")));
      vars.push_back(var(getId(),std::string("rock")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("a")), std::string("object")));
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("rovers")));
      rule_constraint(commonAncestor, vars);
    }
    addChildRule(new Commands$TakeSample$2$0$0(m_id, var(getId(),std::string("OR")), BoolDomain(false, "bool")));
    addChildRule(new Commands$TakeSample$2$0$1(m_id, var(getId(),std::string("OR")), BoolDomain(true, "bool")));
  }
  void Commands$TakeSample$2$0$0::handleExecute() {
    slave(Commands::PhoneHome, Commands.PhoneHome, t0, LabelStr("meets"));
    sameObject(object, t0);
    meets(this, t0);
  }
  void Commands$TakeSample$2$0$1::handleExecute() {
    slave(Commands::PhoneLander, Commands.PhoneLander, t1, LabelStr("meets"));
    sameObject(object, t1);
    meets(this, t1);
  }
  
  // SimpleRover-model.nddl:115 PhoneHome
  class Commands$PhoneHome$3$0: public RuleInstance {
  public:
    Commands$PhoneHome$3$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Commands$PhoneHome$3, Commands$PhoneHome$3$0, Commands.PhoneHome, "SimpleRover-model.nddl,115");
  
  void Commands$PhoneHome$3$0::handleExecute() {
    slave(Battery::change, Battery.change, tx, LabelStr("starts"));
    starts(this, tx);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("tx")), std::string("quantity")));
      vars.push_back(ruleVariable(IntervalIntDomain(-600,-600, "int")));
      rule_constraint(eq, vars);
    }
  }
  
  // SimpleRover-model.nddl:120 PhoneLander
  class Commands$PhoneLander$4$0: public RuleInstance {
  public:
    Commands$PhoneLander$4$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Commands$PhoneLander$4, Commands$PhoneLander$4$0, Commands.PhoneLander, "SimpleRover-model.nddl,120");
  
  void Commands$PhoneLander$4$0::handleExecute() {
    slave(Battery::change, Battery.change, tx, LabelStr("starts"));
    starts(this, tx);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("tx")), std::string("quantity")));
      vars.push_back(ruleVariable(IntervalIntDomain(-20,-20, "int")));
      rule_constraint(eq, vars);
    }
  }
  
  
  // SimpleRover-model.nddl:126 Instrument
  Instrument::Instrument(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : Timeline(planDatabase, "Instrument", name, true) {
  }
  Instrument::Instrument(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : Timeline(planDatabase, type, name, true) {
  }
  Instrument::Instrument(const ObjectId& parent, const LabelStr& name)
   : Timeline(parent, "Instrument", name, true) {}
  Instrument::Instrument(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : Timeline(parent, type, name, true) {}
  // default initialization of member variables
  void Instrument::handleDefaults(bool autoClose) {
    if (autoClose)
      close();
  }
  
  // implicit constructor
  void Instrument::constructor() {
  }
  
  
  // SimpleRover-model.nddl:126 Instrument
  DECLARE_DEFAULT_OBJECT_FACTORY(InstrumentFactory9, Instrument);
  
  
  // SimpleRover-model.nddl:127 TakeSample
  Instrument::TakeSample::TakeSample(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Instrument::TakeSample::TakeSample(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Instrument::TakeSample::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if(rock.isNoId()){
      rock = addParameter(ObjectDomain("Location"), "rock");
      completeObjectParam(Location, rock);
    }
    if (autoClose)
      close();
    
    // Post parameter constraints
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(predicateVariable(IntervalIntDomain(10,10, "int")));
      vars.push_back(var(getId(),std::string("duration")));
      token_constraint(leq, vars);
    }
  }
  
  
  
  // SimpleRover-model.nddl:132 Place
  Instrument::Place::Place(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Instrument::Place::Place(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Instrument::Place::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if(rock.isNoId()){
      rock = addParameter(ObjectDomain("Location"), "rock");
      completeObjectParam(Location, rock);
    }
    if (autoClose)
      close();
    
    // Post parameter constraints
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(predicateVariable(IntervalIntDomain(3, 12, "int")));
      vars.push_back(var(getId(),std::string("duration")));
      token_constraint(eq, vars);
    }
  }
  
  
  
  // SimpleRover-model.nddl:137 Stow
  Instrument::Stow::Stow(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Instrument::Stow::Stow(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Instrument::Stow::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if (autoClose)
      close();
    
    // Post parameter constraints
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(predicateVariable(IntervalIntDomain(2, 6, "int")));
      vars.push_back(var(getId(),std::string("duration")));
      token_constraint(eq, vars);
    }
  }
  
  
  
  // SimpleRover-model.nddl:141 Unstow
  Instrument::Unstow::Unstow(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Instrument::Unstow::Unstow(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Instrument::Unstow::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if (autoClose)
      close();
    
    // Post parameter constraints
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(predicateVariable(IntervalIntDomain(2, 6, "int")));
      vars.push_back(var(getId(),std::string("duration")));
      token_constraint(eq, vars);
    }
  }
  
  
  
  // SimpleRover-model.nddl:145 Stowed
  Instrument::Stowed::Stowed(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
   : NddlToken(planDatabase, name, rejectable, false) {
    handleDefaults(close);
  }
  
  Instrument::Stowed::Stowed(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
   : NddlToken(parent, name, relation, false) {
    handleDefaults(close);
  }
  
  // default initialization of member variables
  void Instrument::Stowed::handleDefaults(bool autoClose) {
    NddlToken::handleDefaults(false);
    if (autoClose)
      close();
  }
  
  
  // SimpleRover-model.nddl:148 TakeSample
  class Instrument$TakeSample$5$0: public RuleInstance {
  public:
    Instrument$TakeSample$5$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Instrument$TakeSample$5, Instrument$TakeSample$5$0, Instrument.TakeSample, "SimpleRover-model.nddl,148");
  
  void Instrument$TakeSample$5$0::handleExecute() {
    objectVar(Rover, rovers, false, false);
    slave(Navigator::At, Navigator.At, at, LabelStr("contained_by"));
    contained_by(this, at);
    slave(Instrument::Place, Instrument.Place, b, LabelStr("met_by"));
    met_by(this, b);
    slave(Instrument::Stow, Instrument.Stow, c, LabelStr("meets"));
    meets(this, c);
    slave(Battery::change, Battery.change, tx, LabelStr("starts"));
    starts(this, tx);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("at")), std::string("location")));
      vars.push_back(var(getId(),std::string("rock")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("at")), std::string("object")));
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("rovers")));
      rule_constraint(commonAncestor, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("b")), std::string("rock")));
      vars.push_back(var(getId(),std::string("rock")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("tx")), std::string("quantity")));
      vars.push_back(ruleVariable(IntervalIntDomain(-120,-120, "int")));
      rule_constraint(eq, vars);
    }
  }
  
  // SimpleRover-model.nddl:166 Place
  class Instrument$Place$6$0: public RuleInstance {
  public:
    Instrument$Place$6$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Instrument$Place$6, Instrument$Place$6$0, Instrument.Place, "SimpleRover-model.nddl,166");
  
  void Instrument$Place$6$0::handleExecute() {
    objectVar(Rover, rovers, false, false);
    slave(Navigator::At, Navigator.At, at, LabelStr("contained_by"));
    contained_by(this, at);
    slave(Instrument::TakeSample, Instrument.TakeSample, a, LabelStr("meets"));
    meets(this, a);
    slave(Instrument::Unstow, Instrument.Unstow, b, LabelStr("met_by"));
    met_by(this, b);
    slave(Battery::change, Battery.change, tx, LabelStr("starts"));
    starts(this, tx);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("at")), std::string("location")));
      vars.push_back(var(getId(),std::string("rock")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("at")), std::string("object")));
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("rovers")));
      rule_constraint(commonAncestor, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("a")), std::string("rock")));
      vars.push_back(var(getId(),std::string("rock")));
      rule_constraint(eq, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("tx")), std::string("quantity")));
      vars.push_back(ruleVariable(IntervalIntDomain(-20,-20, "int")));
      rule_constraint(eq, vars);
    }
  }
  
  // SimpleRover-model.nddl:180 Unstow
  class Instrument$Unstow$7$0: public RuleInstance {
  public:
    Instrument$Unstow$7$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Instrument$Unstow$7, Instrument$Unstow$7$0, Instrument.Unstow, "SimpleRover-model.nddl,180");
  
  void Instrument$Unstow$7$0::handleExecute() {
    objectVar(Rover, rovers, false, false);
    slave(Navigator::At, Navigator.At, at, LabelStr("contained_by"));
    contained_by(this, at);
    slave(Instrument::Place, Instrument.Place, a, LabelStr("meets"));
    meets(this, a);
    slave(Instrument::Stowed, Instrument.Stowed, b, LabelStr("met_by"));
    met_by(this, b);
    slave(Battery::change, Battery.change, tx, LabelStr("starts"));
    starts(this, tx);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("at")), std::string("object")));
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("rovers")));
      rule_constraint(commonAncestor, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("tx")), std::string("quantity")));
      vars.push_back(ruleVariable(IntervalIntDomain(-20,-20, "int")));
      rule_constraint(eq, vars);
    }
  }
  
  // SimpleRover-model.nddl:192 Stow
  class Instrument$Stow$8$0: public RuleInstance {
  public:
    Instrument$Stow$8$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Instrument$Stow$8, Instrument$Stow$8$0, Instrument.Stow, "SimpleRover-model.nddl,192");
  
  void Instrument$Stow$8$0::handleExecute() {
    objectVar(Rover, rovers, false, false);
    slave(Navigator::At, Navigator.At, at, LabelStr("contained_by"));
    contained_by(this, at);
    slave(Instrument::Stowed, Instrument.Stowed, a, LabelStr("meets"));
    meets(this, a);
    slave(Instrument::TakeSample, Instrument.TakeSample, b, LabelStr("met_by"));
    met_by(this, b);
    slave(Battery::change, Battery.change, tx, LabelStr("starts"));
    starts(this, tx);
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("at")), std::string("object")));
      vars.push_back(var(getId(),std::string("object")));
      vars.push_back(var(getId(),std::string("rovers")));
      rule_constraint(commonAncestor, vars);
    }
    {
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(varfromtok(tok(getId(), std::string("tx")), std::string("quantity")));
      vars.push_back(ruleVariable(IntervalIntDomain(-20,-20, "int")));
      rule_constraint(eq, vars);
    }
  }
  
  // SimpleRover-model.nddl:204 Stowed
  class Instrument$Stowed$9$0: public RuleInstance {
  public:
    Instrument$Stowed$9$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
    void handleExecute();
  };
  
  DECLARE_AND_DEFINE_RULE(Rule$Instrument$Stowed$9, Instrument$Stowed$9$0, Instrument.Stowed, "SimpleRover-model.nddl,204");
  
  void Instrument$Stowed$9$0::handleExecute() {
    slave(Instrument::Stow, Instrument.Stow, a, LabelStr("met_by"));
    met_by(this, a);
    slave(Instrument::Unstow, Instrument.Unstow, b, LabelStr("meets"));
    meets(this, b);
  }
  
  
  // SimpleRover-model.nddl:209 Battery
  Battery::Battery(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : NddlResource(planDatabase, "Battery", name, true) {
  }
  Battery::Battery(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : NddlResource(planDatabase, type, name, true) {
  }
  Battery::Battery(const ObjectId& parent, const LabelStr& name)
   : NddlResource(parent, "Battery", name, true) {}
  Battery::Battery(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : NddlResource(parent, type, name, true) {}
  // default initialization of member variables
  void Battery::handleDefaults(bool autoClose) {
    if (autoClose)
      close();
  }
  
  
  // SimpleRover-model.nddl:209 Battery
  void Battery::constructor(float ic, float ll_min, float ll_max) {
    NddlResource::constructor(ic, ll_min, ll_max, 0.0, 0.0, MINUS_INFINITY, MINUS_INFINITY);
  }
  
  // SimpleRover-model.nddl:209 Battery
  class BatteryFactory10: public ConcreteObjectFactory {
  public:
    BatteryFactory10(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 3);
      check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                                TypeFactory::baseDomain("float")), 
                  "Cannot convert " + arguments[0].first.toString() + " to float");
      check_error(arguments[0].second->isSingleton());
      float ic((float)arguments[0].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[1].second, 
                                                TypeFactory::baseDomain("float")), 
                  "Cannot convert " + arguments[1].first.toString() + " to float");
      check_error(arguments[1].second->isSingleton());
      float ll_min((float)arguments[1].second->getSingletonValue());
      
      check_error(AbstractDomain::canBeCompared(*arguments[2].second, 
                                                TypeFactory::baseDomain("float")), 
                  "Cannot convert " + arguments[2].first.toString() + " to float");
      check_error(arguments[2].second->isSingleton());
      float ll_max((float)arguments[2].second->getSingletonValue());
      
      BatteryId instance = (new Battery(planDb, objectType, objectName))->getId();
      instance->constructor(ic, ll_min, ll_max);
      instance->handleDefaults();
      return instance;
    }
  };
  
  
  // SimpleRover-model.nddl:217 Rover
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
    check_error(commands.isValid(), "object variables must be initialized explicitly");
    
    check_error(navigator.isValid(), "object variables must be initialized explicitly");
    
    check_error(instrument.isValid(), "object variables must be initialized explicitly");
    
    check_error(mainBattery.isValid(), "object variables must be initialized explicitly");
    
    if (autoClose)
      close();
  }
  
  
  // SimpleRover-model.nddl:217 Rover
  void Rover::constructor(const BatteryId& r) {
    commands = addVariable(CommandsDomain((new Commands(m_id, "commands"))->getId(), "Commands"), "commands");
    Id<Commands>(singleton(commands))->constructor();
    Id<Commands>(singleton(commands))->handleDefaults();
    navigator = addVariable(NavigatorDomain((new Navigator(m_id, "navigator"))->getId(), "Navigator"), "navigator");
    Id<Navigator>(singleton(navigator))->constructor();
    Id<Navigator>(singleton(navigator))->handleDefaults();
    instrument = addVariable(InstrumentDomain((new Instrument(m_id, "instrument"))->getId(), "Instrument"), "instrument");
    Id<Instrument>(singleton(instrument))->constructor();
    Id<Instrument>(singleton(instrument))->handleDefaults();
    mainBattery = addVariable(BatteryDomain(r, "Battery"), "mainBattery");
  }
  
  // SimpleRover-model.nddl:217 Rover
  class RoverFactory11: public ConcreteObjectFactory {
  public:
    RoverFactory11(const LabelStr& name): ConcreteObjectFactory(name){}
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      check_error(arguments.size() == 1);
      check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                                TypeFactory::baseDomain("Battery")), 
                  "Cannot convert " + arguments[0].first.toString() + " to Battery");
      check_error(arguments[0].second->isSingleton());
      BatteryId r((BatteryId)arguments[0].second->getSingletonValue());
      
      RoverId instance = (new Rover(planDb, objectType, objectName))->getId();
      instance->constructor(r);
      instance->handleDefaults();
      return instance;
    }
  };
  
} // namespace NDDL


namespace NDDL {
  // Boot-strap code to initialize schema
  extern "C" SchemaId loadSchema(){
    Id<Schema> id = Schema::instance("Examples/SimpleRover/SimpleRover-model");
    id->reset(); // Reset prior data if present. 
    
    // Invoke commands to populate schema with type definitions
    id->addPrimitive("int");
    id->addPrimitive("float");
    id->addPrimitive("bool");
    id->addPrimitive("string");
    id->addObjectType("Object");
    id->addObjectType("Timeline","Object");
    id->addObjectType("Resource", "Object");
    id->addMember("Resource", "float", "initialCapacity");
    id->addMember("Resource", "float", "levelLimitMin");
    id->addMember("Resource", "float", "levelLimitMax");
    id->addMember("Resource", "float", "productionRateMax");
    id->addMember("Resource", "float", "productionMax");
    id->addMember("Resource", "float", "consumptionRateMax");
    id->addMember("Resource", "float", "consumptionMax");
    id->addPredicate("Resource.change");
    id->addMember("Resource.change", "float", "quantity");
    id->addObjectType("UnaryResource", "Timeline");
    id->addPredicate("UnaryResource.uses");
    id->addObjectType("StringData", "Object");
    id->addMember("StringData", "string", "data");
    id->addObjectType("PlannerConfig", "Object");
    id->addMember("PlannerConfig", "int", "m_horizonStart");
    id->addMember("PlannerConfig", "int", "m_horizonEnd");
    id->addMember("PlannerConfig", "int", "m_maxPlannerSteps");
    id->addMember("PlannerConfig", "int", "m_maxPlannerDepth");
    id->addObjectType("Location", "Object");
    id->addMember("Location", "string", "name");
    id->addMember("Location", "int", "x");
    id->addMember("Location", "int", "y");
    id->addObjectType("Path", "Object");
    id->addMember("Path", "string", "name");
    id->addMember("Path", "Location", "from");
    id->addMember("Path", "Location", "to");
    id->addMember("Path", "float", "cost");
    id->addObjectType("Navigator", "Timeline");
    id->addPredicate("Navigator.At");
    id->addMember("Navigator.At", "Location", "location");
    id->addPredicate("Navigator.Going");
    id->addMember("Navigator.Going", "Location", "from");
    id->addMember("Navigator.Going", "Location", "to");
    id->addObjectType("Commands", "Timeline");
    id->addPredicate("Commands.TakeSample");
    id->addMember("Commands.TakeSample", "Location", "rock");
    id->addPredicate("Commands.PhoneHome");
    id->addPredicate("Commands.PhoneLander");
    id->addObjectType("Instrument", "Timeline");
    id->addPredicate("Instrument.TakeSample");
    id->addMember("Instrument.TakeSample", "Location", "rock");
    id->addPredicate("Instrument.Place");
    id->addMember("Instrument.Place", "Location", "rock");
    id->addPredicate("Instrument.Stow");
    id->addPredicate("Instrument.Unstow");
    id->addPredicate("Instrument.Stowed");
    id->addObjectType("Battery", "Resource");
    id->addObjectType("Rover", "Object");
    id->addMember("Rover", "Commands", "commands");
    id->addMember("Rover", "Navigator", "navigator");
    id->addMember("Rover", "Instrument", "instrument");
    id->addMember("Rover", "Battery", "mainBattery");
    id->addEnum("TokenStates");
    id->addValue("TokenStates", LabelStr("INACTIVE"));
    id->addValue("TokenStates", LabelStr("ACTIVE"));
    id->addValue("TokenStates", LabelStr("MERGED"));
    id->addValue("TokenStates", LabelStr("REJECTED"));
    // Force allocation of model specific type factories
    // Allocate factories
    REGISTER_TOKEN_FACTORY(UnaryResource::uses::Factory);
    REGISTER_TOKEN_FACTORY(Navigator::At::Factory);
    REGISTER_TOKEN_FACTORY(Navigator::Going::Factory);
    REGISTER_TOKEN_FACTORY(Commands::TakeSample::Factory);
    REGISTER_TOKEN_FACTORY(Commands::PhoneHome::Factory);
    REGISTER_TOKEN_FACTORY(Commands::PhoneLander::Factory);
    REGISTER_TOKEN_FACTORY(Instrument::TakeSample::Factory);
    REGISTER_TOKEN_FACTORY(Instrument::Place::Factory);
    REGISTER_TOKEN_FACTORY(Instrument::Stow::Factory);
    REGISTER_TOKEN_FACTORY(Instrument::Unstow::Factory);
    REGISTER_TOKEN_FACTORY(Instrument::Stowed::Factory);
    REGISTER_TYPE_FACTORY(TokenStates, TokenStatesBaseDomain());
    REGISTER_TYPE_FACTORY(Object, ObjectDomain("Object"));
    REGISTER_TYPE_FACTORY(Timeline, ObjectDomain("Timeline"));
    REGISTER_TYPE_FACTORY(Resource, ObjectDomain("Resource"));
    REGISTER_TYPE_FACTORY(UnaryResource, ObjectDomain("UnaryResource"));
    REGISTER_OBJECT_FACTORY(UnaryResourceFactory0, UnaryResource);
    REGISTER_TYPE_FACTORY(StringData, ObjectDomain("StringData"));
    REGISTER_OBJECT_FACTORY(StringDataFactory1, StringData:string);
    REGISTER_TYPE_FACTORY(PlannerConfig, ObjectDomain("PlannerConfig"));
    REGISTER_OBJECT_FACTORY(PlannerConfigFactory2, PlannerConfig:int:int:int:int);
    REGISTER_OBJECT_FACTORY(PlannerConfigFactory3, PlannerConfig:int:int:int);
    REGISTER_OBJECT_FACTORY(PlannerConfigFactory4, PlannerConfig);
    REGISTER_TYPE_FACTORY(Location, ObjectDomain("Location"));
    REGISTER_OBJECT_FACTORY(LocationFactory5, Location:string:int:int);
    REGISTER_TYPE_FACTORY(Path, ObjectDomain("Path"));
    REGISTER_OBJECT_FACTORY(PathFactory6, Path:string:Location:Location:float);
    REGISTER_OBJECT_FACTORY(PathFactory6, Path:string:Location:Object:float);
    REGISTER_OBJECT_FACTORY(PathFactory6, Path:string:Object:Location:float);
    REGISTER_OBJECT_FACTORY(PathFactory6, Path:string:Object:Object:float);
    REGISTER_TYPE_FACTORY(Navigator, ObjectDomain("Navigator"));
    REGISTER_OBJECT_FACTORY(NavigatorFactory7, Navigator);
    REGISTER_TYPE_FACTORY(Commands, ObjectDomain("Commands"));
    REGISTER_OBJECT_FACTORY(CommandsFactory8, Commands);
    REGISTER_TYPE_FACTORY(Instrument, ObjectDomain("Instrument"));
    REGISTER_OBJECT_FACTORY(InstrumentFactory9, Instrument);
    REGISTER_TYPE_FACTORY(Battery, ObjectDomain("Battery"));
    REGISTER_OBJECT_FACTORY(BatteryFactory10, Battery:float:float:float);
    REGISTER_TYPE_FACTORY(Rover, ObjectDomain("Rover"));
    REGISTER_OBJECT_FACTORY(RoverFactory11, Rover:Battery);
    REGISTER_OBJECT_FACTORY(RoverFactory11, Rover:Object);
    REGISTER_OBJECT_FACTORY(RoverFactory11, Rover:Resource);
    // Allocate rules
    new Rule$Navigator$Going$1();
    new Rule$Navigator$At$0();
    new Rule$Commands$TakeSample$2();
    new Rule$Commands$PhoneLander$4();
    new Rule$Instrument$Stow$8();
    new Rule$Instrument$Unstow$7();
    new Rule$Instrument$TakeSample$5();
    new Rule$Instrument$Place$6();
    new Rule$Instrument$Stowed$9();
    new Rule$Commands$PhoneHome$3();
    return id;
  }
  
}
