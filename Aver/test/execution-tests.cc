// k9.backtrack.moderate-transaction.xml

#include "execution-tests.hh"
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
  
  
  
  // Plasma.nddl:20 Resource
  Resource::Resource(const PlanDatabaseId& planDatabase, const LabelStr& name)
   : NddlResource(planDatabase, "Resource", name, true) {
  }
  Resource::Resource(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
   : NddlResource(planDatabase, type, name, true) {
  }
  Resource::Resource(const ObjectId& parent, const LabelStr& name)
   : NddlResource(parent, "Resource", name, true) {}
  Resource::Resource(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
   : NddlResource(parent, type, name, true) {}
  // default initialization of member variables
  void Resource::handleDefaults(bool autoClose) {
    if(initialCapacity.isNoId()){
      check_error(!ObjectId::convertable(initialCapacity)); // Object Variables must be explicitly initialized to a singleton
      initialCapacity = addVariable(IntervalDomain("float"), "initialCapacity");
    }
    if(levelLimitMin.isNoId()){
      check_error(!ObjectId::convertable(levelLimitMin)); // Object Variables must be explicitly initialized to a singleton
      levelLimitMin = addVariable(IntervalDomain("float"), "levelLimitMin");
    }
    if(levelLimitMax.isNoId()){
      check_error(!ObjectId::convertable(levelLimitMax)); // Object Variables must be explicitly initialized to a singleton
      levelLimitMax = addVariable(IntervalDomain("float"), "levelLimitMax");
    }
    if(productionRateMax.isNoId()){
      check_error(!ObjectId::convertable(productionRateMax)); // Object Variables must be explicitly initialized to a singleton
      productionRateMax = addVariable(IntervalDomain("float"), "productionRateMax");
    }
    if(productionMax.isNoId()){
      check_error(!ObjectId::convertable(productionMax)); // Object Variables must be explicitly initialized to a singleton
      productionMax = addVariable(IntervalDomain("float"), "productionMax");
    }
    if(consumptionRateMax.isNoId()){
      check_error(!ObjectId::convertable(consumptionRateMax)); // Object Variables must be explicitly initialized to a singleton
      consumptionRateMax = addVariable(IntervalDomain("float"), "consumptionRateMax");
    }
    if(consumptionMax.isNoId()){
      check_error(!ObjectId::convertable(consumptionMax)); // Object Variables must be explicitly initialized to a singleton
      consumptionMax = addVariable(IntervalDomain("float"), "consumptionMax");
    }
    if (autoClose)
      close();
}



// Plasma.nddl:31 change
Resource::change::change(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlResourceTransaction(planDatabase, name, rejectable, false) {
  handleDefaults(close);
}

Resource::change::change(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlResourceTransaction(parent, name, relation, false) {
  handleDefaults(close);
}

// default initialization of member variables
void Resource::change::handleDefaults(bool autoClose) {
  NddlResourceTransaction::handleDefaults(false);
  if(quantity.isNoId()){
    quantity = addParameter(IntervalDomain("float"), "quantity");
  }
  if (autoClose)
    close();
}


// Plasma.nddl:20 Resource
void Resource::constructor(float ic, float ll_min, float ll_max) {
initialCapacity = addVariable(IntervalDomain(ic, ic, "float"), "initialCapacity");
levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, "float"), "levelLimitMin");
levelLimitMax = addVariable(IntervalDomain(ll_max, ll_max, "float"), "levelLimitMax");
productionRateMax = addVariable(IntervalDomain(+inf, +inf, "float"), "productionRateMax");
productionMax = addVariable(IntervalDomain(+inf, +inf, "float"), "productionMax");
consumptionRateMax = addVariable(IntervalDomain(-inf, -inf, "float"), "consumptionRateMax");
consumptionMax = addVariable(IntervalDomain(-inf, -inf, "float"), "consumptionMax");
}

// Plasma.nddl:20 Resource
class ResourceFactory0: public ConcreteObjectFactory {
public:
ResourceFactory0(const LabelStr& name): ConcreteObjectFactory(name){}
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
  
  ResourceId instance = (new Resource(planDb, objectType, objectName))->getId();
  instance->constructor(ic, ll_min, ll_max);
  instance->handleDefaults();
  return instance;
}
};

// Plasma.nddl:20 Resource
void Resource::constructor(float ic, float ll_min, float ll_max, float p_max, float c_max) {
initialCapacity = addVariable(IntervalDomain(ic, ic, "float"), "initialCapacity");
levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, "float"), "levelLimitMin");
levelLimitMax = addVariable(IntervalDomain(ll_max, ll_max, "float"), "levelLimitMax");
productionRateMax = addVariable(IntervalDomain(p_max, p_max, "float"), "productionRateMax");
productionMax = addVariable(IntervalDomain(p_max, p_max, "float"), "productionMax");
consumptionRateMax = addVariable(IntervalDomain(-16, -16, "float"), "consumptionRateMax");
consumptionMax = addVariable(IntervalDomain(c_max, c_max, "float"), "consumptionMax");
}

// Plasma.nddl:20 Resource
class ResourceFactory1: public ConcreteObjectFactory {
public:
ResourceFactory1(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
  check_error(arguments.size() == 5);
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
  
  check_error(AbstractDomain::canBeCompared(*arguments[3].second, 
                                            TypeFactory::baseDomain("float")), 
              "Cannot convert " + arguments[3].first.toString() + " to float");
  check_error(arguments[3].second->isSingleton());
  float p_max((float)arguments[3].second->getSingletonValue());
  
  check_error(AbstractDomain::canBeCompared(*arguments[4].second, 
                                            TypeFactory::baseDomain("float")), 
              "Cannot convert " + arguments[4].first.toString() + " to float");
  check_error(arguments[4].second->isSingleton());
  float c_max((float)arguments[4].second->getSingletonValue());
  
  ResourceId instance = (new Resource(planDb, objectType, objectName))->getId();
  instance->constructor(ic, ll_min, ll_max, p_max, c_max);
  instance->handleDefaults();
  return instance;
}
};

// Plasma.nddl:20 Resource
void Resource::constructor(float ic, float ll_min, float ll_max, float pr_max, float p_max, float cr_max, float c_max) {
initialCapacity = addVariable(IntervalDomain(ic, ic, "float"), "initialCapacity");
levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, "float"), "levelLimitMin");
levelLimitMax = addVariable(IntervalDomain(ll_max, ll_max, "float"), "levelLimitMax");
productionRateMax = addVariable(IntervalDomain(pr_max, pr_max, "float"), "productionRateMax");
productionMax = addVariable(IntervalDomain(p_max, p_max, "float"), "productionMax");
consumptionRateMax = addVariable(IntervalDomain(cr_max, cr_max, "float"), "consumptionRateMax");
consumptionMax = addVariable(IntervalDomain(c_max, c_max, "float"), "consumptionMax");
}

// Plasma.nddl:20 Resource
class ResourceFactory2: public ConcreteObjectFactory {
public:
ResourceFactory2(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
  check_error(arguments.size() == 7);
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
  
  check_error(AbstractDomain::canBeCompared(*arguments[3].second, 
                                            TypeFactory::baseDomain("float")), 
              "Cannot convert " + arguments[3].first.toString() + " to float");
  check_error(arguments[3].second->isSingleton());
  float pr_max((float)arguments[3].second->getSingletonValue());
  
  check_error(AbstractDomain::canBeCompared(*arguments[4].second, 
                                            TypeFactory::baseDomain("float")), 
              "Cannot convert " + arguments[4].first.toString() + " to float");
  check_error(arguments[4].second->isSingleton());
  float p_max((float)arguments[4].second->getSingletonValue());
  
  check_error(AbstractDomain::canBeCompared(*arguments[5].second, 
                                            TypeFactory::baseDomain("float")), 
              "Cannot convert " + arguments[5].first.toString() + " to float");
  check_error(arguments[5].second->isSingleton());
  float cr_max((float)arguments[5].second->getSingletonValue());
  
  check_error(AbstractDomain::canBeCompared(*arguments[6].second, 
                                            TypeFactory::baseDomain("float")), 
              "Cannot convert " + arguments[6].first.toString() + " to float");
  check_error(arguments[6].second->isSingleton());
  float c_max((float)arguments[6].second->getSingletonValue());
  
  ResourceId instance = (new Resource(planDb, objectType, objectName))->getId();
  instance->constructor(ic, ll_min, ll_max, pr_max, p_max, cr_max, c_max);
  instance->handleDefaults();
  return instance;
}
};

// Plasma.nddl:20 Resource
void Resource::constructor() {
initialCapacity = addVariable(IntervalDomain(0, 0, "float"), "initialCapacity");
levelLimitMin = addVariable(IntervalDomain(-inf, -inf, "float"), "levelLimitMin");
levelLimitMax = addVariable(IntervalDomain(+inf, +inf, "float"), "levelLimitMax");
productionRateMax = addVariable(IntervalDomain(+inf, +inf, "float"), "productionRateMax");
productionMax = addVariable(IntervalDomain(+inf, +inf, "float"), "productionMax");
consumptionRateMax = addVariable(IntervalDomain(-inf, -inf, "float"), "consumptionRateMax");
consumptionMax = addVariable(IntervalDomain(-inf, -inf, "float"), "consumptionMax");
}

// Plasma.nddl:20 Resource
class ResourceFactory3: public ConcreteObjectFactory {
public:
ResourceFactory3(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
  check_error(arguments.size() == 0);
  ResourceId instance = (new Resource(planDb, objectType, objectName))->getId();
  instance->constructor();
  instance->handleDefaults();
  return instance;
}
};


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
DECLARE_DEFAULT_OBJECT_FACTORY(UnaryResourceFactory4, UnaryResource);


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


Types TypesBaseDomain(){
static Types sl_enum("Types");
if (sl_enum.isOpen()) {
// Insert values to initialize
sl_enum.insert(LabelStr("ACTION"));
sl_enum.insert(LabelStr("FLUENT"));
sl_enum.close();
}
return(sl_enum);
}

Data DataBaseDomain(){
static Data sl_enum("Data");
if (sl_enum.isOpen()) {
// Insert values to initialize
sl_enum.insert(LabelStr("CHAMPIMAGE"));
sl_enum.insert(LabelStr("MOSSBAUER"));
sl_enum.insert(LabelStr("OPPSCI"));
sl_enum.close();
}
return(sl_enum);
}


// k9.model.backtrack.moderate.nddl:6 Location
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
if (autoClose)
close();
}

// implicit constructor
void Location::constructor() {
}


// k9.model.backtrack.moderate.nddl:6 Location
DECLARE_DEFAULT_OBJECT_FACTORY(LocationFactory5, Location);


// k9.model.backtrack.moderate.nddl:9 Target
Target::Target(const PlanDatabaseId& planDatabase, const LabelStr& name)
 : Timeline(planDatabase, "Target", name, true) {
}
Target::Target(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
 : Timeline(planDatabase, type, name, true) {
}
Target::Target(const ObjectId& parent, const LabelStr& name)
 : Timeline(parent, "Target", name, true) {}
Target::Target(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
 : Timeline(parent, type, name, true) {}
// default initialization of member variables
void Target::handleDefaults(bool autoClose) {
if (autoClose)
close();
}

// implicit constructor
void Target::constructor() {
}


// k9.model.backtrack.moderate.nddl:9 Target
DECLARE_DEFAULT_OBJECT_FACTORY(TargetFactory6, Target);


// k9.model.backtrack.moderate.nddl:11 NotTracked
Target::NotTracked::NotTracked(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Target::NotTracked::NotTracked(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Target::NotTracked::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("target")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:18 trackstart
Target::trackstart::trackstart(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Target::trackstart::trackstart(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Target::trackstart::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("target")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:33 Tracked
Target::Tracked::Tracked(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Target::Tracked::Tracked(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Target::Tracked::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("target")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:40 trackstop
Target::trackstop::trackstop(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Target::trackstop::trackstop(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Target::trackstop::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("target")));
token_constraint(eq, vars);
}
}


// k9.model.backtrack.moderate.nddl:56 NotTracked
class Target$NotTracked$0$0: public RuleInstance {
public:
Target$NotTracked$0$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Target$NotTracked$0, Target$NotTracked$0$0, Target.NotTracked, "k9.model.backtrack.moderate.nddl,56");

void Target$NotTracked$0$0::handleExecute() {
slave(Target::trackstart, Target.trackstart, tstart, LabelStr("meets"));
sameObject(object, tstart);
meets(this, tstart);
slave(Target::trackstop, Target.trackstop, tstop, LabelStr("met_by"));
sameObject(object, tstop);
met_by(this, tstop);
}

// k9.model.backtrack.moderate.nddl:61 trackstart
class Target$trackstart$1$0: public RuleInstance {
public:
Target$trackstart$1$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Target$trackstart$1, Target$trackstart$1$0, Target.trackstart, "k9.model.backtrack.moderate.nddl,61");

void Target$trackstart$1$0::handleExecute() {
slave(Target::Tracked, Target.Tracked, t, LabelStr("meets"));
sameObject(object, t);
meets(this, t);
slave(Target::NotTracked, Target.NotTracked, nt, LabelStr("met_by"));
sameObject(object, nt);
met_by(this, nt);
slave(Tracker::LandmarksDefined, Tracker.LandmarksDefined, l, LabelStr("contained_by"));
contained_by(this, l);
}

// k9.model.backtrack.moderate.nddl:68 Tracked
class Target$Tracked$2$0: public RuleInstance {
public:
Target$Tracked$2$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Target$Tracked$2, Target$Tracked$2$0, Target.Tracked, "k9.model.backtrack.moderate.nddl,68");

void Target$Tracked$2$0::handleExecute() {
slave(Target::trackstop, Target.trackstop, tstop, LabelStr("meets"));
sameObject(object, tstop);
meets(this, tstop);
slave(Target::trackstart, Target.trackstart, tstart, LabelStr("met_by"));
sameObject(object, tstart);
met_by(this, tstart);
}

// k9.model.backtrack.moderate.nddl:73 trackstop
class Target$trackstop$3$0: public RuleInstance {
public:
Target$trackstop$3$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Target$trackstop$3, Target$trackstop$3$0, Target.trackstop, "k9.model.backtrack.moderate.nddl,73");

void Target$trackstop$3$0::handleExecute() {
slave(Target::NotTracked, Target.NotTracked, nt, LabelStr("meets"));
sameObject(object, nt);
meets(this, nt);
slave(Target::Tracked, Target.Tracked, t, LabelStr("met_by"));
sameObject(object, t);
met_by(this, t);
}


// k9.model.backtrack.moderate.nddl:79 Path
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

if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:79 Path
void Path::constructor(const LocationId& from, const LocationId& to) {
m_from = addVariable(LocationDomain(from, "Location"), "m_from");
m_to = addVariable(LocationDomain(to, "Location"), "m_to");
}

// k9.model.backtrack.moderate.nddl:79 Path
class PathFactory7: public ConcreteObjectFactory {
public:
PathFactory7(const LabelStr& name): ConcreteObjectFactory(name){}
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


// k9.model.backtrack.moderate.nddl:88 CHAMP_Accessable
CHAMP_Accessable::CHAMP_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& name)
 : Object(planDatabase, "CHAMP_Accessable", name, true) {
}
CHAMP_Accessable::CHAMP_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
 : Object(planDatabase, type, name, true) {
}
CHAMP_Accessable::CHAMP_Accessable(const ObjectId& parent, const LabelStr& name)
 : Object(parent, "CHAMP_Accessable", name, true) {}
CHAMP_Accessable::CHAMP_Accessable(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
 : Object(parent, type, name, true) {}
// default initialization of member variables
void CHAMP_Accessable::handleDefaults(bool autoClose) {
check_error(m_from.isValid(), "object variables must be initialized explicitly");

check_error(m_to.isValid(), "object variables must be initialized explicitly");

if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:88 CHAMP_Accessable
void CHAMP_Accessable::constructor(const LocationId& from, const TargetId& to) {
m_from = addVariable(LocationDomain(from, "Location"), "m_from");
m_to = addVariable(TargetDomain(to, "Target"), "m_to");
}

// k9.model.backtrack.moderate.nddl:88 CHAMP_Accessable
class CHAMP_AccessableFactory8: public ConcreteObjectFactory {
public:
CHAMP_AccessableFactory8(const LabelStr& name): ConcreteObjectFactory(name){}
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
                                          TypeFactory::baseDomain("Target")), 
            "Cannot convert " + arguments[1].first.toString() + " to Target");
check_error(arguments[1].second->isSingleton());
TargetId to((TargetId)arguments[1].second->getSingletonValue());

CHAMP_AccessableId instance = (new CHAMP_Accessable(planDb, objectType, objectName))->getId();
instance->constructor(from, to);
instance->handleDefaults();
return instance;
}
};


// k9.model.backtrack.moderate.nddl:98 OppSci_Accessable
OppSci_Accessable::OppSci_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& name)
 : Object(planDatabase, "OppSci_Accessable", name, true) {
}
OppSci_Accessable::OppSci_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
 : Object(planDatabase, type, name, true) {
}
OppSci_Accessable::OppSci_Accessable(const ObjectId& parent, const LabelStr& name)
 : Object(parent, "OppSci_Accessable", name, true) {}
OppSci_Accessable::OppSci_Accessable(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
 : Object(parent, type, name, true) {}
// default initialization of member variables
void OppSci_Accessable::handleDefaults(bool autoClose) {
check_error(m_from.isValid(), "object variables must be initialized explicitly");

check_error(m_to.isValid(), "object variables must be initialized explicitly");

if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:98 OppSci_Accessable
void OppSci_Accessable::constructor(const LocationId& from, const TargetId& to) {
m_from = addVariable(LocationDomain(from, "Location"), "m_from");
m_to = addVariable(TargetDomain(to, "Target"), "m_to");
}

// k9.model.backtrack.moderate.nddl:98 OppSci_Accessable
class OppSci_AccessableFactory9: public ConcreteObjectFactory {
public:
OppSci_AccessableFactory9(const LabelStr& name): ConcreteObjectFactory(name){}
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
                                          TypeFactory::baseDomain("Target")), 
            "Cannot convert " + arguments[1].first.toString() + " to Target");
check_error(arguments[1].second->isSingleton());
TargetId to((TargetId)arguments[1].second->getSingletonValue());

OppSci_AccessableId instance = (new OppSci_Accessable(planDb, objectType, objectName))->getId();
instance->constructor(from, to);
instance->handleDefaults();
return instance;
}
};


// k9.model.backtrack.moderate.nddl:108 Position
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
check_error(m_rover.isValid(), "object variables must be initialized explicitly");

if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:108 Position
void Position::constructor(const RoverId& rover) {
m_rover = addVariable(RoverDomain(rover, "Rover"), "m_rover");
}

// k9.model.backtrack.moderate.nddl:108 Position
class PositionFactory10: public ConcreteObjectFactory {
public:
PositionFactory10(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
check_error(arguments.size() == 1);
check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                          TypeFactory::baseDomain("Rover")), 
            "Cannot convert " + arguments[0].first.toString() + " to Rover");
check_error(arguments[0].second->isSingleton());
RoverId rover((RoverId)arguments[0].second->getSingletonValue());

PositionId instance = (new Position(planDb, objectType, objectName))->getId();
instance->constructor(rover);
instance->handleDefaults();
return instance;
}
};


// k9.model.backtrack.moderate.nddl:115 At
Position::At::At(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Position::At::At(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Position::At::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(location.isNoId()){
location = addParameter(ObjectDomain("Location"), "location");
completeObjectParam(Location, location);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:121 navigate
Position::navigate::navigate(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Position::navigate::navigate(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Position::navigate::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(from.isNoId()){
from = addParameter(ObjectDomain("Location"), "from");
completeObjectParam(Location, from);
}
if(to.isNoId()){
to = addParameter(ObjectDomain("Location"), "to");
completeObjectParam(Location, to);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean_multiplier.isNoId()){
m_duration_mean_multiplier = addParameter(IntervalDomain("float"), "m_duration_mean_multiplier");
}
if(m_duration_std_multiplier.isNoId()){
m_duration_std_multiplier = addParameter(IntervalDomain("float"), "m_duration_std_multiplier");
}
if(m_energy_mean_multiplier.isNoId()){
m_energy_mean_multiplier = addParameter(IntervalDomain("float"), "m_energy_mean_multiplier");
}
if(m_energy_std_multiplier.isNoId()){
m_energy_std_multiplier = addParameter(IntervalDomain("float"), "m_energy_std_multiplier");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean_multiplier")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std_multiplier")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean_multiplier")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std_multiplier")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("from")));
vars.push_back(var(getId(),std::string("to")));
token_constraint(neq, vars);
}
}


// k9.model.backtrack.moderate.nddl:138 At
class Position$At$4$0: public RuleInstance {
public:
Position$At$4$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Position$At$4, Position$At$4$0, Position.At, "k9.model.backtrack.moderate.nddl,138");

void Position$At$4$0::handleExecute() {
slave(Position::navigate, Position.navigate, a, LabelStr("meets"));
sameObject(object, a);
meets(this, a);
slave(Position::navigate, Position.navigate, b, LabelStr("met_by"));
sameObject(object, b);
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

// k9.model.backtrack.moderate.nddl:148 navigate
class Position$navigate$5$0: public RuleInstance {
public:
Position$navigate$5$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Position$navigate$5, Position$navigate$5$0, Position.navigate, "k9.model.backtrack.moderate.nddl,148");

void Position$navigate$5$0::handleExecute() {
objectVar(Path, path, true);
objectVar(Rover, rovers, false);
slave(Position::At, Position.At, a, LabelStr("meets"));
sameObject(object, a);
meets(this, a);
slave(Position::At, Position.At, b, LabelStr("met_by"));
sameObject(object, b);
met_by(this, b);
slave(Tracker::TrackingOn, Tracker.TrackingOn, tOn, LabelStr("contained_by"));
contained_by(this, tOn);
slave(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle, oppsci, LabelStr("contained_by"));
contained_by(this, oppsci);
slave(CHAMP::IPIdle, CHAMP.IPIdle, ipidle, LabelStr("contained_by"));
contained_by(this, ipidle);
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
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tOn")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("oppsci")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("ipidle")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
}


// k9.model.backtrack.moderate.nddl:174 Tracker
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
check_error(m_rover.isValid(), "object variables must be initialized explicitly");

if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:174 Tracker
void Tracker::constructor(const RoverId& rover) {
m_rover = addVariable(RoverDomain(rover, "Rover"), "m_rover");
}

// k9.model.backtrack.moderate.nddl:174 Tracker
class TrackerFactory11: public ConcreteObjectFactory {
public:
TrackerFactory11(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
check_error(arguments.size() == 1);
check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                          TypeFactory::baseDomain("Rover")), 
            "Cannot convert " + arguments[0].first.toString() + " to Rover");
check_error(arguments[0].second->isSingleton());
RoverId rover((RoverId)arguments[0].second->getSingletonValue());

TrackerId instance = (new Tracker(planDb, objectType, objectName))->getId();
instance->constructor(rover);
instance->handleDefaults();
return instance;
}
};


// k9.model.backtrack.moderate.nddl:181 TrackingOff
Tracker::TrackingOff::TrackingOff(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::TrackingOff::TrackingOff(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::TrackingOff::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:186 trackloadgroup
Tracker::trackloadgroup::trackloadgroup(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::trackloadgroup::trackloadgroup(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::trackloadgroup::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:199 LandmarksDefined
Tracker::LandmarksDefined::LandmarksDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::LandmarksDefined::LandmarksDefined(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::LandmarksDefined::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:204 StartTracking
Tracker::StartTracking::StartTracking(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::StartTracking::StartTracking(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::StartTracking::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:217 TrackingOn
Tracker::TrackingOn::TrackingOn(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::TrackingOn::TrackingOn(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::TrackingOn::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:222 trackfreeze
Tracker::trackfreeze::trackfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::trackfreeze::trackfreeze(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::trackfreeze::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:235 TrackingFrozen
Tracker::TrackingFrozen::TrackingFrozen(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::TrackingFrozen::TrackingFrozen(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::TrackingFrozen::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:240 trackunfreeze
Tracker::trackunfreeze::trackunfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

Tracker::trackunfreeze::trackunfreeze(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void Tracker::trackunfreeze::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}


// k9.model.backtrack.moderate.nddl:255 TrackingOff
class Tracker$TrackingOff$6$0: public RuleInstance {
public:
Tracker$TrackingOff$6$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$TrackingOff$6, Tracker$TrackingOff$6$0, Tracker.TrackingOff, "k9.model.backtrack.moderate.nddl,255");

void Tracker$TrackingOff$6$0::handleExecute() {
slave(Tracker::trackloadgroup, Tracker.trackloadgroup, tlgp, LabelStr("meets"));
sameObject(object, tlgp);
meets(this, tlgp);
}

// k9.model.backtrack.moderate.nddl:259 trackloadgroup
class Tracker$trackloadgroup$7$0: public RuleInstance {
public:
Tracker$trackloadgroup$7$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$trackloadgroup$7, Tracker$trackloadgroup$7$0, Tracker.trackloadgroup, "k9.model.backtrack.moderate.nddl,259");

void Tracker$trackloadgroup$7$0::handleExecute() {
slave(Tracker::LandmarksDefined, Tracker.LandmarksDefined, tld, LabelStr("meets"));
sameObject(object, tld);
meets(this, tld);
slave(Tracker::TrackingOff, Tracker.TrackingOff, toff, LabelStr("met_by"));
sameObject(object, toff);
met_by(this, toff);
}

// k9.model.backtrack.moderate.nddl:264 LandmarksDefined
class Tracker$LandmarksDefined$8$0: public RuleInstance {
public:
Tracker$LandmarksDefined$8$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$LandmarksDefined$8, Tracker$LandmarksDefined$8$0, Tracker.LandmarksDefined, "k9.model.backtrack.moderate.nddl,264");

void Tracker$LandmarksDefined$8$0::handleExecute() {
slave(Tracker::trackloadgroup, Tracker.trackloadgroup, tlgp, LabelStr("met_by"));
sameObject(object, tlgp);
met_by(this, tlgp);
slave(Tracker::StartTracking, Tracker.StartTracking, tst, LabelStr("meets"));
sameObject(object, tst);
meets(this, tst);
}

// k9.model.backtrack.moderate.nddl:269 StartTracking
class Tracker$StartTracking$9$0: public RuleInstance {
public:
Tracker$StartTracking$9$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$StartTracking$9, Tracker$StartTracking$9$0, Tracker.StartTracking, "k9.model.backtrack.moderate.nddl,269");

void Tracker$StartTracking$9$0::handleExecute() {
slave(Tracker::LandmarksDefined, Tracker.LandmarksDefined, tld, LabelStr("met_by"));
sameObject(object, tld);
met_by(this, tld);
slave(Tracker::TrackingOn, Tracker.TrackingOn, tto, LabelStr("meets"));
sameObject(object, tto);
meets(this, tto);
}

// k9.model.backtrack.moderate.nddl:274 TrackingOn
class Tracker$TrackingOn$10$0: public RuleInstance {
public:
Tracker$TrackingOn$10$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

class Tracker$TrackingOn$10$0$0: public RuleInstance {
public:
Tracker$TrackingOn$10$0$0(const RuleInstanceId& parent, const ConstrainedVariableId& var, const AbstractDomain& domain)
: RuleInstance(parent, var, domain){}
void handleExecute();
};

class Tracker$TrackingOn$10$0$1: public RuleInstance {
public:
Tracker$TrackingOn$10$0$1(const RuleInstanceId& parent, const ConstrainedVariableId& var, const AbstractDomain& domain)
: RuleInstance(parent, var, domain){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$TrackingOn$10, Tracker$TrackingOn$10$0, Tracker.TrackingOn, "k9.model.backtrack.moderate.nddl,274");

void Tracker$TrackingOn$10$0::handleExecute() {
localVar(BoolDomain(), OR, true);
slave(Tracker::trackfreeze, Tracker.trackfreeze, tfz, LabelStr("meets"));
sameObject(object, tfz);
meets(this, tfz);
addChildRule(new Tracker$TrackingOn$10$0$0(m_id, var(getId(),std::string("OR")), BoolDomain(true)));
addChildRule(new Tracker$TrackingOn$10$0$1(m_id, var(getId(),std::string("OR")), BoolDomain(false)));
}
void Tracker$TrackingOn$10$0$0::handleExecute() {
slave(Tracker::StartTracking, Tracker.StartTracking, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
}
void Tracker$TrackingOn$10$0$1::handleExecute() {
slave(Tracker::trackunfreeze, Tracker.trackunfreeze, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
}

// k9.model.backtrack.moderate.nddl:285 trackfreeze
class Tracker$trackfreeze$11$0: public RuleInstance {
public:
Tracker$trackfreeze$11$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$trackfreeze$11, Tracker$trackfreeze$11$0, Tracker.trackfreeze, "k9.model.backtrack.moderate.nddl,285");

void Tracker$trackfreeze$11$0::handleExecute() {
slave(Tracker::TrackingOn, Tracker.TrackingOn, tld, LabelStr("met_by"));
sameObject(object, tld);
met_by(this, tld);
slave(Tracker::TrackingFrozen, Tracker.TrackingFrozen, tto, LabelStr("meets"));
sameObject(object, tto);
meets(this, tto);
}

// k9.model.backtrack.moderate.nddl:290 TrackingFrozen
class Tracker$TrackingFrozen$12$0: public RuleInstance {
public:
Tracker$TrackingFrozen$12$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$TrackingFrozen$12, Tracker$TrackingFrozen$12$0, Tracker.TrackingFrozen, "k9.model.backtrack.moderate.nddl,290");

void Tracker$TrackingFrozen$12$0::handleExecute() {
slave(Tracker::trackfreeze, Tracker.trackfreeze, tld, LabelStr("met_by"));
sameObject(object, tld);
met_by(this, tld);
slave(Tracker::trackunfreeze, Tracker.trackunfreeze, tto, LabelStr("meets"));
sameObject(object, tto);
meets(this, tto);
}

// k9.model.backtrack.moderate.nddl:295 trackunfreeze
class Tracker$trackunfreeze$13$0: public RuleInstance {
public:
Tracker$trackunfreeze$13$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$Tracker$trackunfreeze$13, Tracker$trackunfreeze$13$0, Tracker.trackunfreeze, "k9.model.backtrack.moderate.nddl,295");

void Tracker$trackunfreeze$13$0::handleExecute() {
slave(Tracker::TrackingFrozen, Tracker.TrackingFrozen, tld, LabelStr("met_by"));
sameObject(object, tld);
met_by(this, tld);
slave(Tracker::TrackingOn, Tracker.TrackingOn, tto, LabelStr("meets"));
sameObject(object, tto);
meets(this, tto);
}


// k9.model.backtrack.moderate.nddl:300 OpportunisticScience
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
check_error(m_rover.isValid(), "object variables must be initialized explicitly");

if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:300 OpportunisticScience
void OpportunisticScience::constructor(const RoverId& rover) {
m_rover = addVariable(RoverDomain(rover, "Rover"), "m_rover");
}

// k9.model.backtrack.moderate.nddl:300 OpportunisticScience
class OpportunisticScienceFactory12: public ConcreteObjectFactory {
public:
OpportunisticScienceFactory12(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
check_error(arguments.size() == 1);
check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                          TypeFactory::baseDomain("Rover")), 
            "Cannot convert " + arguments[0].first.toString() + " to Rover");
check_error(arguments[0].second->isSingleton());
RoverId rover((RoverId)arguments[0].second->getSingletonValue());

OpportunisticScienceId instance = (new OpportunisticScience(planDb, objectType, objectName))->getId();
instance->constructor(rover);
instance->handleDefaults();
return instance;
}
};


// k9.model.backtrack.moderate.nddl:307 OppSciIdle
OpportunisticScience::OppSciIdle::OppSciIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::OppSciIdle::OppSciIdle(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::OppSciIdle::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:312 oppscidefineproc
OpportunisticScience::oppscidefineproc::oppscidefineproc(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::oppscidefineproc::oppscidefineproc(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::oppscidefineproc::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:325 OppSciProcDefined
OpportunisticScience::OppSciProcDefined::OppSciProcDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::OppSciProcDefined::OppSciProcDefined(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::OppSciProcDefined::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:330 oppscisetparams
OpportunisticScience::oppscisetparams::oppscisetparams(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::oppscisetparams::oppscisetparams(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::oppscisetparams::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:343 OppSciParamsSet
OpportunisticScience::OppSciParamsSet::OppSciParamsSet(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::OppSciParamsSet::OppSciParamsSet(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::OppSciParamsSet::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:348 oppscilooknow
OpportunisticScience::oppscilooknow::oppscilooknow(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::oppscilooknow::oppscilooknow(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::oppscilooknow::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:363 OppSciDoneLookNow
OpportunisticScience::OppSciDoneLookNow::OppSciDoneLookNow(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::OppSciDoneLookNow::OppSciDoneLookNow(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::OppSciDoneLookNow::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:370 oppscigetstatus
OpportunisticScience::oppscigetstatus::oppscigetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

OpportunisticScience::oppscigetstatus::oppscigetstatus(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void OpportunisticScience::oppscigetstatus::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}


// k9.model.backtrack.moderate.nddl:386 oppscidefineproc
class OpportunisticScience$oppscidefineproc$14$0: public RuleInstance {
public:
OpportunisticScience$oppscidefineproc$14$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$OpportunisticScience$oppscidefineproc$14, OpportunisticScience$oppscidefineproc$14$0, OpportunisticScience.oppscidefineproc, "k9.model.backtrack.moderate.nddl,386");

void OpportunisticScience$oppscidefineproc$14$0::handleExecute() {
slave(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(OpportunisticScience::OppSciProcDefined, OpportunisticScience.OppSciProcDefined, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:391 OppSciProcDefined
class OpportunisticScience$OppSciProcDefined$15$0: public RuleInstance {
public:
OpportunisticScience$OppSciProcDefined$15$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$OpportunisticScience$OppSciProcDefined$15, OpportunisticScience$OppSciProcDefined$15$0, OpportunisticScience.OppSciProcDefined, "k9.model.backtrack.moderate.nddl,391");

void OpportunisticScience$OppSciProcDefined$15$0::handleExecute() {
slave(OpportunisticScience::oppscidefineproc, OpportunisticScience.oppscidefineproc, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(OpportunisticScience::oppscisetparams, OpportunisticScience.oppscisetparams, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:396 oppscisetparams
class OpportunisticScience$oppscisetparams$16$0: public RuleInstance {
public:
OpportunisticScience$oppscisetparams$16$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$OpportunisticScience$oppscisetparams$16, OpportunisticScience$oppscisetparams$16$0, OpportunisticScience.oppscisetparams, "k9.model.backtrack.moderate.nddl,396");

void OpportunisticScience$oppscisetparams$16$0::handleExecute() {
slave(OpportunisticScience::OppSciProcDefined, OpportunisticScience.OppSciProcDefined, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(OpportunisticScience::OppSciParamsSet, OpportunisticScience.OppSciParamsSet, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:401 OppSciParamsSet
class OpportunisticScience$OppSciParamsSet$17$0: public RuleInstance {
public:
OpportunisticScience$OppSciParamsSet$17$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$OpportunisticScience$OppSciParamsSet$17, OpportunisticScience$OppSciParamsSet$17$0, OpportunisticScience.OppSciParamsSet, "k9.model.backtrack.moderate.nddl,401");

void OpportunisticScience$OppSciParamsSet$17$0::handleExecute() {
slave(OpportunisticScience::oppscisetparams, OpportunisticScience.oppscisetparams, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(OpportunisticScience::oppscilooknow, OpportunisticScience.oppscilooknow, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:406 oppscilooknow
class OpportunisticScience$oppscilooknow$18$0: public RuleInstance {
public:
OpportunisticScience$oppscilooknow$18$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$OpportunisticScience$oppscilooknow$18, OpportunisticScience$oppscilooknow$18$0, OpportunisticScience.oppscilooknow, "k9.model.backtrack.moderate.nddl,406");

void OpportunisticScience$oppscilooknow$18$0::handleExecute() {
objectVar(OppSci_Accessable, oppsci, true);
objectVar(Rover, rovers, false);
slave(OpportunisticScience::OppSciParamsSet, OpportunisticScience.OppSciParamsSet, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(OpportunisticScience::OppSciDoneLookNow, OpportunisticScience.OppSciDoneLookNow, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
slave(Target::Tracked, Target.Tracked, tr, LabelStr("contained_by"));
contained_by(this, tr);
slave(Tracker::TrackingFrozen, Tracker.TrackingFrozen, tfo, LabelStr("contained_by"));
contained_by(this, tfo);
slave(Position::At, Position.At, at, LabelStr("contained_by"));
contained_by(this, at);
slave(CHAMP::IPIdle, CHAMP.IPIdle, ipidle, LabelStr("contained_by"));
contained_by(this, ipidle);
declareFilter(OppSci_Accessable,oppsci);
allocateFilterCondition(oppsci, Location, var(getId(),std::string("at_loc")), m_from, eq);
allocateFilterCondition(oppsci, Target, var(getId(),std::string("target")), m_to, eq);
allocateFilterConstraint(oppsci, CONSTRAIN);
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tr")), std::string("object")));
vars.push_back(var(getId(),std::string("target")));
rule_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tfo")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
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
vars.push_back(varfromtok(tok(getId(), std::string("ipidle")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
}

// k9.model.backtrack.moderate.nddl:428 OppSciDoneLookNow
class OpportunisticScience$OppSciDoneLookNow$19$0: public RuleInstance {
public:
OpportunisticScience$OppSciDoneLookNow$19$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$OpportunisticScience$OppSciDoneLookNow$19, OpportunisticScience$OppSciDoneLookNow$19$0, OpportunisticScience.OppSciDoneLookNow, "k9.model.backtrack.moderate.nddl,428");

void OpportunisticScience$OppSciDoneLookNow$19$0::handleExecute() {
slave(OpportunisticScience::oppscilooknow, OpportunisticScience.oppscilooknow, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(OpportunisticScience::oppscigetstatus, OpportunisticScience.oppscigetstatus, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:433 oppscigetstatus
class OpportunisticScience$oppscigetstatus$20$0: public RuleInstance {
public:
OpportunisticScience$oppscigetstatus$20$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$OpportunisticScience$oppscigetstatus$20, OpportunisticScience$oppscigetstatus$20$0, OpportunisticScience.oppscigetstatus, "k9.model.backtrack.moderate.nddl,433");

void OpportunisticScience$oppscigetstatus$20$0::handleExecute() {
objectVar(OppSci_Accessable, oppsci, true);
objectVar(Rover, rovers, false);
slave(OpportunisticScience::OppSciDoneLookNow, OpportunisticScience.OppSciDoneLookNow, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
slave(Target::Tracked, Target.Tracked, tr, LabelStr("contained_by"));
contained_by(this, tr);
slave(Tracker::TrackingFrozen, Tracker.TrackingFrozen, tfo, LabelStr("contained_by"));
contained_by(this, tfo);
slave(Position::At, Position.At, at, LabelStr("contained_by"));
contained_by(this, at);
slave(CHAMP::IPIdle, CHAMP.IPIdle, ipidle, LabelStr("contained_by"));
contained_by(this, ipidle);
declareFilter(OppSci_Accessable,oppsci);
allocateFilterCondition(oppsci, Location, var(getId(),std::string("at_loc")), m_from, eq);
allocateFilterCondition(oppsci, Target, var(getId(),std::string("target")), m_to, eq);
allocateFilterConstraint(oppsci, CONSTRAIN);
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tr")), std::string("object")));
vars.push_back(var(getId(),std::string("target")));
rule_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tfo")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
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
vars.push_back(varfromtok(tok(getId(), std::string("ipidle")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
}


// k9.model.backtrack.moderate.nddl:455 CHAMP
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
check_error(m_rover.isValid(), "object variables must be initialized explicitly");

if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:455 CHAMP
void CHAMP::constructor(const RoverId& rover) {
m_rover = addVariable(RoverDomain(rover, "Rover"), "m_rover");
}

// k9.model.backtrack.moderate.nddl:455 CHAMP
class CHAMPFactory13: public ConcreteObjectFactory {
public:
CHAMPFactory13(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
check_error(arguments.size() == 1);
check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                          TypeFactory::baseDomain("Rover")), 
            "Cannot convert " + arguments[0].first.toString() + " to Rover");
check_error(arguments[0].second->isSingleton());
RoverId rover((RoverId)arguments[0].second->getSingletonValue());

CHAMPId instance = (new CHAMP(planDb, objectType, objectName))->getId();
instance->constructor(rover);
instance->handleDefaults();
return instance;
}
};


// k9.model.backtrack.moderate.nddl:462 IPIdle
CHAMP::IPIdle::IPIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::IPIdle::IPIdle(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::IPIdle::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:467 ipgetname
CHAMP::ipgetname::ipgetname(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::ipgetname::ipgetname(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::ipgetname::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:483 IPHaveName
CHAMP::IPHaveName::IPHaveName(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::IPHaveName::IPHaveName(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::IPHaveName::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:490 ipsettarget
CHAMP::ipsettarget::ipsettarget(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::ipsettarget::ipsettarget(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::ipsettarget::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:506 IPTargetSet
CHAMP::IPTargetSet::IPTargetSet(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::IPTargetSet::IPTargetSet(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::IPTargetSet::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:513 ipplaceinstrument
CHAMP::ipplaceinstrument::ipplaceinstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::ipplaceinstrument::ipplaceinstrument(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::ipplaceinstrument::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:529 IPDonePlaceInstrument
CHAMP::IPDonePlaceInstrument::IPDonePlaceInstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::IPDonePlaceInstrument::IPDonePlaceInstrument(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::IPDonePlaceInstrument::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("FLUENT"), "Types")));
token_constraint(eq, vars);
}
}



// k9.model.backtrack.moderate.nddl:536 ipgetstatus
CHAMP::ipgetstatus::ipgetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

CHAMP::ipgetstatus::ipgetstatus(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void CHAMP::ipgetstatus::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if(target.isNoId()){
target = addParameter(ObjectDomain("Target"), "target");
completeObjectParam(Target, target);
}
if(at_loc.isNoId()){
at_loc = addParameter(ObjectDomain("Location"), "at_loc");
completeObjectParam(Location, at_loc);
}
if(TYPE.isNoId()){
TYPE = addParameter(TypesBaseDomain(), "TYPE");
}
if(m_duration_mean.isNoId()){
m_duration_mean = addParameter(IntervalDomain("float"), "m_duration_mean");
}
if(m_duration_std.isNoId()){
m_duration_std = addParameter(IntervalDomain("float"), "m_duration_std");
}
if(m_energy_mean.isNoId()){
m_energy_mean = addParameter(IntervalDomain("float"), "m_energy_mean");
}
if(m_energy_std.isNoId()){
m_energy_std = addParameter(IntervalDomain("float"), "m_energy_std");
}
if (autoClose)
close();

// Post parameter constraints
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("TYPE")));
vars.push_back(predicateVariable(SymbolDomain(LabelStr("ACTION"), "Types")));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_duration_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_mean")));
vars.push_back(predicateVariable(IntervalIntDomain(1)));
token_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(var(getId(),std::string("m_energy_std")));
vars.push_back(predicateVariable(IntervalIntDomain(0)));
token_constraint(eq, vars);
}
}


// k9.model.backtrack.moderate.nddl:553 ipgetname
class CHAMP$ipgetname$21$0: public RuleInstance {
public:
CHAMP$ipgetname$21$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$CHAMP$ipgetname$21, CHAMP$ipgetname$21$0, CHAMP.ipgetname, "k9.model.backtrack.moderate.nddl,553");

void CHAMP$ipgetname$21$0::handleExecute() {
objectVar(CHAMP_Accessable, champ, true);
objectVar(Rover, rovers, false);
slave(CHAMP::IPIdle, CHAMP.IPIdle, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(CHAMP::IPHaveName, CHAMP.IPHaveName, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
slave(Target::Tracked, Target.Tracked, tr, LabelStr("contained_by"));
contained_by(this, tr);
slave(Tracker::TrackingOn, Tracker.TrackingOn, tfo, LabelStr("contained_by"));
contained_by(this, tfo);
slave(Position::At, Position.At, at, LabelStr("contained_by"));
contained_by(this, at);
slave(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle, oppsciidle, LabelStr("contained_by"));
contained_by(this, oppsciidle);
declareFilter(CHAMP_Accessable,champ);
allocateFilterCondition(champ, Location, var(getId(),std::string("at_loc")), m_from, eq);
allocateFilterCondition(champ, Target, var(getId(),std::string("target")), m_to, eq);
allocateFilterConstraint(champ, CONSTRAIN);
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tr")), std::string("object")));
vars.push_back(var(getId(),std::string("target")));
rule_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tfo")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
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
vars.push_back(varfromtok(tok(getId(), std::string("oppsciidle")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
}

// k9.model.backtrack.moderate.nddl:583 iIPHaveName
class CHAMP$iIPHaveName$22$0: public RuleInstance {
public:
CHAMP$iIPHaveName$22$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$CHAMP$iIPHaveName$22, CHAMP$iIPHaveName$22$0, CHAMP.iIPHaveName, "k9.model.backtrack.moderate.nddl,583");

void CHAMP$iIPHaveName$22$0::handleExecute() {
slave(CHAMP::ipgetname, CHAMP.ipgetname, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(CHAMP::ipsettarget, CHAMP.ipsettarget, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:588 ipsettarget
class CHAMP$ipsettarget$23$0: public RuleInstance {
public:
CHAMP$ipsettarget$23$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$CHAMP$ipsettarget$23, CHAMP$ipsettarget$23$0, CHAMP.ipsettarget, "k9.model.backtrack.moderate.nddl,588");

void CHAMP$ipsettarget$23$0::handleExecute() {
objectVar(CHAMP_Accessable, champ, true);
objectVar(Rover, rovers, false);
slave(CHAMP::IPHaveName, CHAMP.IPHaveName, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(CHAMP::IPTargetSet, CHAMP.IPTargetSet, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
slave(Target::Tracked, Target.Tracked, tr, LabelStr("contained_by"));
contained_by(this, tr);
slave(Tracker::TrackingOn, Tracker.TrackingOn, tfo, LabelStr("contained_by"));
contained_by(this, tfo);
slave(Position::At, Position.At, at, LabelStr("contained_by"));
contained_by(this, at);
slave(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle, oppsciidle, LabelStr("contained_by"));
contained_by(this, oppsciidle);
declareFilter(CHAMP_Accessable,champ);
allocateFilterCondition(champ, Location, var(getId(),std::string("at_loc")), m_from, eq);
allocateFilterCondition(champ, Target, var(getId(),std::string("target")), m_to, eq);
allocateFilterConstraint(champ, CONSTRAIN);
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tr")), std::string("object")));
vars.push_back(var(getId(),std::string("target")));
rule_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tfo")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
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
vars.push_back(varfromtok(tok(getId(), std::string("oppsciidle")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
}

// k9.model.backtrack.moderate.nddl:611 IPTargetSet
class CHAMP$IPTargetSet$24$0: public RuleInstance {
public:
CHAMP$IPTargetSet$24$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$CHAMP$IPTargetSet$24, CHAMP$IPTargetSet$24$0, CHAMP.IPTargetSet, "k9.model.backtrack.moderate.nddl,611");

void CHAMP$IPTargetSet$24$0::handleExecute() {
slave(CHAMP::ipsettarget, CHAMP.ipsettarget, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(CHAMP::ipplaceinstrument, CHAMP.ipplaceinstrument, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:617 ipplaceinstrument
class CHAMP$ipplaceinstrument$25$0: public RuleInstance {
public:
CHAMP$ipplaceinstrument$25$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$CHAMP$ipplaceinstrument$25, CHAMP$ipplaceinstrument$25$0, CHAMP.ipplaceinstrument, "k9.model.backtrack.moderate.nddl,617");

void CHAMP$ipplaceinstrument$25$0::handleExecute() {
objectVar(CHAMP_Accessable, champ, true);
objectVar(Rover, rovers, false);
slave(CHAMP::IPTargetSet, CHAMP.IPTargetSet, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(CHAMP::IPDonePlaceInstrument, CHAMP.IPDonePlaceInstrument, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
slave(Target::Tracked, Target.Tracked, tr, LabelStr("contained_by"));
contained_by(this, tr);
slave(Tracker::TrackingOn, Tracker.TrackingOn, tfo, LabelStr("contained_by"));
contained_by(this, tfo);
slave(Position::At, Position.At, at, LabelStr("contained_by"));
contained_by(this, at);
slave(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle, oppsciidle, LabelStr("contained_by"));
contained_by(this, oppsciidle);
declareFilter(CHAMP_Accessable,champ);
allocateFilterCondition(champ, Location, var(getId(),std::string("at_loc")), m_from, eq);
allocateFilterCondition(champ, Target, var(getId(),std::string("target")), m_to, eq);
allocateFilterConstraint(champ, CONSTRAIN);
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tr")), std::string("object")));
vars.push_back(var(getId(),std::string("target")));
rule_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tfo")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
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
vars.push_back(varfromtok(tok(getId(), std::string("oppsciidle")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
}

// k9.model.backtrack.moderate.nddl:641 IPDonePlaceInstrument
class CHAMP$IPDonePlaceInstrument$26$0: public RuleInstance {
public:
CHAMP$IPDonePlaceInstrument$26$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$CHAMP$IPDonePlaceInstrument$26, CHAMP$IPDonePlaceInstrument$26$0, CHAMP.IPDonePlaceInstrument, "k9.model.backtrack.moderate.nddl,641");

void CHAMP$IPDonePlaceInstrument$26$0::handleExecute() {
slave(CHAMP::ipplaceinstrument, CHAMP.ipplaceinstrument, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(CHAMP::ipgetstatus, CHAMP.ipgetstatus, s1, LabelStr("meets"));
sameObject(object, s1);
meets(this, s1);
}

// k9.model.backtrack.moderate.nddl:647 ipgetstatus
class CHAMP$ipgetstatus$27$0: public RuleInstance {
public:
CHAMP$ipgetstatus$27$0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
: RuleInstance(rule, token, planDb){}
void handleExecute();
};

DECLARE_AND_DEFINE_RULE(Rule$CHAMP$ipgetstatus$27, CHAMP$ipgetstatus$27$0, CHAMP.ipgetstatus, "k9.model.backtrack.moderate.nddl,647");

void CHAMP$ipgetstatus$27$0::handleExecute() {
objectVar(CHAMP_Accessable, champ, true);
objectVar(Rover, rovers, false);
slave(CHAMP::IPDonePlaceInstrument, CHAMP.IPDonePlaceInstrument, s0, LabelStr("met_by"));
sameObject(object, s0);
met_by(this, s0);
slave(Target::Tracked, Target.Tracked, tr, LabelStr("contained_by"));
contained_by(this, tr);
slave(Tracker::TrackingOn, Tracker.TrackingOn, tfo, LabelStr("contained_by"));
contained_by(this, tfo);
slave(Position::At, Position.At, at, LabelStr("contained_by"));
contained_by(this, at);
slave(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle, oppsciidle, LabelStr("contained_by"));
contained_by(this, oppsciidle);
declareFilter(CHAMP_Accessable,champ);
allocateFilterCondition(champ, Location, var(getId(),std::string("at_loc")), m_from, eq);
allocateFilterCondition(champ, Target, var(getId(),std::string("target")), m_to, eq);
allocateFilterConstraint(champ, CONSTRAIN);
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tr")), std::string("object")));
vars.push_back(var(getId(),std::string("target")));
rule_constraint(eq, vars);
}
{
std::vector<ConstrainedVariableId> vars;
vars.push_back(varfromtok(tok(getId(), std::string("tfo")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
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
vars.push_back(varfromtok(tok(getId(), std::string("oppsciidle")), std::string("object")));
vars.push_back(var(getId(),std::string("object")));
vars.push_back(var(getId(),std::string("rovers")));
rule_constraint(commonAncestor, vars);
}
}


// k9.model.backtrack.moderate.nddl:669 Energy
Energy::Energy(const PlanDatabaseId& planDatabase, const LabelStr& name)
 : Resource(planDatabase, "Energy", name) {
}
Energy::Energy(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
 : Resource(planDatabase, type, name) {
}
Energy::Energy(const ObjectId& parent, const LabelStr& name)
 : Resource(parent, "Energy", name) {}
Energy::Energy(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
 : Resource(parent, type, name) {}
// default initialization of member variables
void Energy::handleDefaults(bool autoClose) {
Resource::handleDefaults(false);
if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:669 Energy
void Energy::constructor(float ic, float ll_min, float ll_max) {
Resource::constructor(ic, ll_min, ll_max);
}

// k9.model.backtrack.moderate.nddl:669 Energy
class EnergyFactory14: public ConcreteObjectFactory {
public:
EnergyFactory14(const LabelStr& name): ConcreteObjectFactory(name){}
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

EnergyId instance = (new Energy(planDb, objectType, objectName))->getId();
instance->constructor(ic, ll_min, ll_max);
instance->handleDefaults();
return instance;
}
};


// k9.model.backtrack.moderate.nddl:676 Rover
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

check_error(m_energy.isValid(), "object variables must be initialized explicitly");

if(BATTERY_MIN.isNoId()){
check_error(!ObjectId::convertable(BATTERY_MIN)); // Object Variables must be explicitly initialized to a singleton
BATTERY_MIN = addVariable(IntervalDomain("float"), "BATTERY_MIN");
}
if(BATTERY_MAX.isNoId()){
check_error(!ObjectId::convertable(BATTERY_MAX)); // Object Variables must be explicitly initialized to a singleton
BATTERY_MAX = addVariable(IntervalDomain("float"), "BATTERY_MAX");
}
if (autoClose)
close();
}


// k9.model.backtrack.moderate.nddl:676 Rover
void Rover::constructor(float ic) {
BATTERY_MIN = addVariable(IntervalDomain(0, 0, "float"), "BATTERY_MIN");
BATTERY_MAX = addVariable(IntervalDomain(518400, 518400, "float"), "BATTERY_MAX");
m_position = addVariable(PositionDomain((new Position(m_id, "m_position"))->getId(), "Position"), "m_position");
Id<Position>(singleton(m_position))->constructor(m_id);
Id<Position>(singleton(m_position))->handleDefaults();
m_tracker = addVariable(TrackerDomain((new Tracker(m_id, "m_tracker"))->getId(), "Tracker"), "m_tracker");
Id<Tracker>(singleton(m_tracker))->constructor(m_id);
Id<Tracker>(singleton(m_tracker))->handleDefaults();
m_oppsci = addVariable(OpportunisticScienceDomain((new OpportunisticScience(m_id, "m_oppsci"))->getId(), "OpportunisticScience"), "m_oppsci");
Id<OpportunisticScience>(singleton(m_oppsci))->constructor(m_id);
Id<OpportunisticScience>(singleton(m_oppsci))->handleDefaults();
m_champ = addVariable(CHAMPDomain((new CHAMP(m_id, "m_champ"))->getId(), "CHAMP"), "m_champ");
Id<CHAMP>(singleton(m_champ))->constructor(m_id);
Id<CHAMP>(singleton(m_champ))->handleDefaults();
m_energy = addVariable(EnergyDomain((new Energy(m_id, "m_energy"))->getId(), "Energy"), "m_energy");
Id<Energy>(singleton(m_energy))->constructor(ic, 0, 518400);
Id<Energy>(singleton(m_energy))->handleDefaults();
}

// k9.model.backtrack.moderate.nddl:676 Rover
class RoverFactory15: public ConcreteObjectFactory {
public:
RoverFactory15(const LabelStr& name): ConcreteObjectFactory(name){}
private:
ObjectId createInstance(const PlanDatabaseId& planDb,
                        const LabelStr& objectType, 
                        const LabelStr& objectName,
                        const std::vector<ConstructorArgument>& arguments) const {
check_error(arguments.size() == 1);
check_error(AbstractDomain::canBeCompared(*arguments[0].second, 
                                          TypeFactory::baseDomain("float")), 
            "Cannot convert " + arguments[0].first.toString() + " to float");
check_error(arguments[0].second->isSingleton());
float ic((float)arguments[0].second->getSingletonValue());

RoverId instance = (new Rover(planDb, objectType, objectName))->getId();
instance->constructor(ic);
instance->handleDefaults();
return instance;
}
};


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
if (autoClose)
close();
}



// NddlWorld.nddl:9 initialState
NddlWorld::initialState::initialState(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool close)
 : NddlToken(planDatabase, name, rejectable, false) {
handleDefaults(close);
}

NddlWorld::initialState::initialState(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)
 : NddlToken(parent, name, relation, false) {
handleDefaults(close);
}

// default initialization of member variables
void NddlWorld::initialState::handleDefaults(bool autoClose) {
NddlToken::handleDefaults(false);
if (autoClose)
close();
}


// NddlWorld.nddl:2 NddlWorld
void NddlWorld::constructor(int start, int end, int maxPlannerSteps) {
m_horizonStart = addVariable(IntervalIntDomain(start, start, "int"), "m_horizonStart");
m_horizonEnd = addVariable(IntervalIntDomain(end, end, "int"), "m_horizonEnd");
m_maxPlannerSteps = addVariable(IntervalIntDomain(maxPlannerSteps, maxPlannerSteps, "int"), "m_maxPlannerSteps");
}

// NddlWorld.nddl:2 NddlWorld
class NddlWorldFactory16: public ConcreteObjectFactory {
public:
NddlWorldFactory16(const LabelStr& name): ConcreteObjectFactory(name){}
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
class NddlWorldFactory17: public ConcreteObjectFactory {
public:
NddlWorldFactory17(const LabelStr& name): ConcreteObjectFactory(name){}
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

} // namespace NDDL


namespace NDDL {
// Boot-strap code to initialize schema
extern "C" SchemaId loadSchema(){
Id<Schema> id = Schema::instance("k9.backtrack.moderate-transaction");
id->reset(); // Reset prior data if present. 

// Invoke commands to populate schema with type definitions
id->addPrimitive("int");
id->addPrimitive("float");
id->addPrimitive("bool");
id->addPrimitive("string");
id->addObjectType("Object");
id->addObjectType("Timeline", "Object");
id->addObjectType("NddlResource", "Object");
id->addObjectType("Resource", "NddlResource");
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
id->addObjectType("Location", "Object");
id->addObjectType("Target", "Timeline");
id->addPredicate("Target.NotTracked");
id->addMember("Target.NotTracked", "Target", "target");
id->addMember("Target.NotTracked", "Types", "TYPE");
id->addPredicate("Target.trackstart");
id->addMember("Target.trackstart", "Target", "target");
id->addMember("Target.trackstart", "Types", "TYPE");
id->addMember("Target.trackstart", "float", "m_duration_mean");
id->addMember("Target.trackstart", "float", "m_duration_std");
id->addMember("Target.trackstart", "float", "m_energy_mean");
id->addMember("Target.trackstart", "float", "m_energy_std");
id->addPredicate("Target.Tracked");
id->addMember("Target.Tracked", "Target", "target");
id->addMember("Target.Tracked", "Types", "TYPE");
id->addPredicate("Target.trackstop");
id->addMember("Target.trackstop", "Target", "target");
id->addMember("Target.trackstop", "Types", "TYPE");
id->addMember("Target.trackstop", "float", "m_duration_mean");
id->addMember("Target.trackstop", "float", "m_duration_std");
id->addMember("Target.trackstop", "float", "m_energy_mean");
id->addMember("Target.trackstop", "float", "m_energy_std");
id->addObjectType("Path", "Object");
id->addMember("Path", "Location", "m_from");
id->addMember("Path", "Location", "m_to");
id->addObjectType("CHAMP_Accessable", "Object");
id->addMember("CHAMP_Accessable", "Location", "m_from");
id->addMember("CHAMP_Accessable", "Target", "m_to");
id->addObjectType("OppSci_Accessable", "Object");
id->addMember("OppSci_Accessable", "Location", "m_from");
id->addMember("OppSci_Accessable", "Target", "m_to");
id->addObjectType("Position", "Timeline");
id->addMember("Position", "Rover", "m_rover");
id->addPredicate("Position.At");
id->addMember("Position.At", "Location", "location");
id->addMember("Position.At", "Types", "TYPE");
id->addPredicate("Position.navigate");
id->addMember("Position.navigate", "Location", "from");
id->addMember("Position.navigate", "Location", "to");
id->addMember("Position.navigate", "Types", "TYPE");
id->addMember("Position.navigate", "float", "m_duration_mean_multiplier");
id->addMember("Position.navigate", "float", "m_duration_std_multiplier");
id->addMember("Position.navigate", "float", "m_energy_mean_multiplier");
id->addMember("Position.navigate", "float", "m_energy_std_multiplier");
id->addObjectType("Tracker", "Timeline");
id->addMember("Tracker", "Rover", "m_rover");
id->addPredicate("Tracker.TrackingOff");
id->addMember("Tracker.TrackingOff", "Types", "TYPE");
id->addPredicate("Tracker.trackloadgroup");
id->addMember("Tracker.trackloadgroup", "Types", "TYPE");
id->addMember("Tracker.trackloadgroup", "float", "m_duration_mean");
id->addMember("Tracker.trackloadgroup", "float", "m_duration_std");
id->addMember("Tracker.trackloadgroup", "float", "m_energy_mean");
id->addMember("Tracker.trackloadgroup", "float", "m_energy_std");
id->addPredicate("Tracker.LandmarksDefined");
id->addMember("Tracker.LandmarksDefined", "Types", "TYPE");
id->addPredicate("Tracker.StartTracking");
id->addMember("Tracker.StartTracking", "Types", "TYPE");
id->addMember("Tracker.StartTracking", "float", "m_duration_mean");
id->addMember("Tracker.StartTracking", "float", "m_duration_std");
id->addMember("Tracker.StartTracking", "float", "m_energy_mean");
id->addMember("Tracker.StartTracking", "float", "m_energy_std");
id->addPredicate("Tracker.TrackingOn");
id->addMember("Tracker.TrackingOn", "Types", "TYPE");
id->addPredicate("Tracker.trackfreeze");
id->addMember("Tracker.trackfreeze", "Types", "TYPE");
id->addMember("Tracker.trackfreeze", "float", "m_duration_mean");
id->addMember("Tracker.trackfreeze", "float", "m_duration_std");
id->addMember("Tracker.trackfreeze", "float", "m_energy_mean");
id->addMember("Tracker.trackfreeze", "float", "m_energy_std");
id->addPredicate("Tracker.TrackingFrozen");
id->addMember("Tracker.TrackingFrozen", "Types", "TYPE");
id->addPredicate("Tracker.trackunfreeze");
id->addMember("Tracker.trackunfreeze", "Types", "TYPE");
id->addMember("Tracker.trackunfreeze", "float", "m_duration_mean");
id->addMember("Tracker.trackunfreeze", "float", "m_duration_std");
id->addMember("Tracker.trackunfreeze", "float", "m_energy_mean");
id->addMember("Tracker.trackunfreeze", "float", "m_energy_std");
id->addObjectType("OpportunisticScience", "Timeline");
id->addMember("OpportunisticScience", "Rover", "m_rover");
id->addPredicate("OpportunisticScience.OppSciIdle");
id->addMember("OpportunisticScience.OppSciIdle", "Types", "TYPE");
id->addPredicate("OpportunisticScience.oppscidefineproc");
id->addMember("OpportunisticScience.oppscidefineproc", "Types", "TYPE");
id->addMember("OpportunisticScience.oppscidefineproc", "float", "m_duration_mean");
id->addMember("OpportunisticScience.oppscidefineproc", "float", "m_duration_std");
id->addMember("OpportunisticScience.oppscidefineproc", "float", "m_energy_mean");
id->addMember("OpportunisticScience.oppscidefineproc", "float", "m_energy_std");
id->addPredicate("OpportunisticScience.OppSciProcDefined");
id->addMember("OpportunisticScience.OppSciProcDefined", "Types", "TYPE");
id->addPredicate("OpportunisticScience.oppscisetparams");
id->addMember("OpportunisticScience.oppscisetparams", "Types", "TYPE");
id->addMember("OpportunisticScience.oppscisetparams", "float", "m_duration_mean");
id->addMember("OpportunisticScience.oppscisetparams", "float", "m_duration_std");
id->addMember("OpportunisticScience.oppscisetparams", "float", "m_energy_mean");
id->addMember("OpportunisticScience.oppscisetparams", "float", "m_energy_std");
id->addPredicate("OpportunisticScience.OppSciParamsSet");
id->addMember("OpportunisticScience.OppSciParamsSet", "Types", "TYPE");
id->addPredicate("OpportunisticScience.oppscilooknow");
id->addMember("OpportunisticScience.oppscilooknow", "Target", "target");
id->addMember("OpportunisticScience.oppscilooknow", "Location", "at_loc");
id->addMember("OpportunisticScience.oppscilooknow", "Types", "TYPE");
id->addMember("OpportunisticScience.oppscilooknow", "float", "m_duration_mean");
id->addMember("OpportunisticScience.oppscilooknow", "float", "m_duration_std");
id->addMember("OpportunisticScience.oppscilooknow", "float", "m_energy_mean");
id->addMember("OpportunisticScience.oppscilooknow", "float", "m_energy_std");
id->addPredicate("OpportunisticScience.OppSciDoneLookNow");
id->addMember("OpportunisticScience.OppSciDoneLookNow", "Target", "target");
id->addMember("OpportunisticScience.OppSciDoneLookNow", "Location", "at_loc");
id->addMember("OpportunisticScience.OppSciDoneLookNow", "Types", "TYPE");
id->addPredicate("OpportunisticScience.oppscigetstatus");
id->addMember("OpportunisticScience.oppscigetstatus", "Target", "target");
id->addMember("OpportunisticScience.oppscigetstatus", "Location", "at_loc");
id->addMember("OpportunisticScience.oppscigetstatus", "Types", "TYPE");
id->addMember("OpportunisticScience.oppscigetstatus", "float", "m_duration_mean");
id->addMember("OpportunisticScience.oppscigetstatus", "float", "m_duration_std");
id->addMember("OpportunisticScience.oppscigetstatus", "float", "m_energy_mean");
id->addMember("OpportunisticScience.oppscigetstatus", "float", "m_energy_std");
id->addObjectType("CHAMP", "Timeline");
id->addMember("CHAMP", "Rover", "m_rover");
id->addPredicate("CHAMP.IPIdle");
id->addMember("CHAMP.IPIdle", "Types", "TYPE");
id->addPredicate("CHAMP.ipgetname");
id->addMember("CHAMP.ipgetname", "Target", "target");
id->addMember("CHAMP.ipgetname", "Location", "at_loc");
id->addMember("CHAMP.ipgetname", "Types", "TYPE");
id->addMember("CHAMP.ipgetname", "float", "m_duration_mean");
id->addMember("CHAMP.ipgetname", "float", "m_duration_std");
id->addMember("CHAMP.ipgetname", "float", "m_energy_mean");
id->addMember("CHAMP.ipgetname", "float", "m_energy_std");
id->addPredicate("CHAMP.IPHaveName");
id->addMember("CHAMP.IPHaveName", "Target", "target");
id->addMember("CHAMP.IPHaveName", "Location", "at_loc");
id->addMember("CHAMP.IPHaveName", "Types", "TYPE");
id->addPredicate("CHAMP.ipsettarget");
id->addMember("CHAMP.ipsettarget", "Target", "target");
id->addMember("CHAMP.ipsettarget", "Location", "at_loc");
id->addMember("CHAMP.ipsettarget", "Types", "TYPE");
id->addMember("CHAMP.ipsettarget", "float", "m_duration_mean");
id->addMember("CHAMP.ipsettarget", "float", "m_duration_std");
id->addMember("CHAMP.ipsettarget", "float", "m_energy_mean");
id->addMember("CHAMP.ipsettarget", "float", "m_energy_std");
id->addPredicate("CHAMP.IPTargetSet");
id->addMember("CHAMP.IPTargetSet", "Target", "target");
id->addMember("CHAMP.IPTargetSet", "Location", "at_loc");
id->addMember("CHAMP.IPTargetSet", "Types", "TYPE");
id->addPredicate("CHAMP.ipplaceinstrument");
id->addMember("CHAMP.ipplaceinstrument", "Target", "target");
id->addMember("CHAMP.ipplaceinstrument", "Location", "at_loc");
id->addMember("CHAMP.ipplaceinstrument", "Types", "TYPE");
id->addMember("CHAMP.ipplaceinstrument", "float", "m_duration_mean");
id->addMember("CHAMP.ipplaceinstrument", "float", "m_duration_std");
id->addMember("CHAMP.ipplaceinstrument", "float", "m_energy_mean");
id->addMember("CHAMP.ipplaceinstrument", "float", "m_energy_std");
id->addPredicate("CHAMP.IPDonePlaceInstrument");
id->addMember("CHAMP.IPDonePlaceInstrument", "Target", "target");
id->addMember("CHAMP.IPDonePlaceInstrument", "Location", "at_loc");
id->addMember("CHAMP.IPDonePlaceInstrument", "Types", "TYPE");
id->addPredicate("CHAMP.ipgetstatus");
id->addMember("CHAMP.ipgetstatus", "Target", "target");
id->addMember("CHAMP.ipgetstatus", "Location", "at_loc");
id->addMember("CHAMP.ipgetstatus", "Types", "TYPE");
id->addMember("CHAMP.ipgetstatus", "float", "m_duration_mean");
id->addMember("CHAMP.ipgetstatus", "float", "m_duration_std");
id->addMember("CHAMP.ipgetstatus", "float", "m_energy_mean");
id->addMember("CHAMP.ipgetstatus", "float", "m_energy_std");
id->addObjectType("Energy", "Resource");
id->addObjectType("Rover", "Object");
id->addMember("Rover", "Position", "m_position");
id->addMember("Rover", "Tracker", "m_tracker");
id->addMember("Rover", "OpportunisticScience", "m_oppsci");
id->addMember("Rover", "CHAMP", "m_champ");
id->addMember("Rover", "Energy", "m_energy");
id->addMember("Rover", "float", "BATTERY_MIN");
id->addMember("Rover", "float", "BATTERY_MAX");
id->addObjectType("NddlWorld", "Timeline");
id->addMember("NddlWorld", "int", "m_horizonStart");
id->addMember("NddlWorld", "int", "m_horizonEnd");
id->addMember("NddlWorld", "int", "m_maxPlannerSteps");
id->addPredicate("NddlWorld.initialState");
id->addEnum("TokenStates");
id->addValue("TokenStates", LabelStr("INACTIVE"));
id->addValue("TokenStates", LabelStr("ACTIVE"));
id->addValue("TokenStates", LabelStr("MERGED"));
id->addValue("TokenStates", LabelStr("REJECTED"));
id->addEnum("Types");
id->addValue("Types", LabelStr("ACTION"));
id->addValue("Types", LabelStr("FLUENT"));
id->addEnum("Data");
id->addValue("Data", LabelStr("CHAMPIMAGE"));
id->addValue("Data", LabelStr("MOSSBAUER"));
id->addValue("Data", LabelStr("OPPSCI"));
id->addMember("Position.navigate", "Path", "path");
id->addMember("Position.navigate", "Rover", "rovers");
id->addMember("Tracker.TrackingOn", "bool", "OR");
id->addMember("OpportunisticScience.oppscilooknow", "OppSci_Accessable", "oppsci");
id->addMember("OpportunisticScience.oppscilooknow", "Rover", "rovers");
id->addMember("OpportunisticScience.oppscigetstatus", "OppSci_Accessable", "oppsci");
id->addMember("OpportunisticScience.oppscigetstatus", "Rover", "rovers");
id->addMember("CHAMP.ipgetname", "CHAMP_Accessable", "champ");
id->addMember("CHAMP.ipgetname", "Rover", "rovers");
id->addMember("CHAMP.ipsettarget", "CHAMP_Accessable", "champ");
id->addMember("CHAMP.ipsettarget", "Rover", "rovers");
id->addMember("CHAMP.ipplaceinstrument", "CHAMP_Accessable", "champ");
id->addMember("CHAMP.ipplaceinstrument", "Rover", "rovers");
id->addMember("CHAMP.ipgetstatus", "CHAMP_Accessable", "champ");
id->addMember("CHAMP.ipgetstatus", "Rover", "rovers");
// Force allocation of model specific type factories
// REGISTER FACTORIES
REGISTER_TYPE_FACTORY(TokenStates, TokenStatesBaseDomain());
REGISTER_TYPE_FACTORY(Types, TypesBaseDomain());
REGISTER_TYPE_FACTORY(Data, DataBaseDomain());

// REGISTER FACTORIES
REGISTER_TOKEN_FACTORY(Resource::change::Factory);
REGISTER_TOKEN_FACTORY(UnaryResource::uses::Factory);
REGISTER_TOKEN_FACTORY(Target::NotTracked::Factory);
REGISTER_TOKEN_FACTORY(Target::trackstart::Factory);
REGISTER_TOKEN_FACTORY(Target::Tracked::Factory);
REGISTER_TOKEN_FACTORY(Target::trackstop::Factory);
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
REGISTER_TOKEN_FACTORY(NddlWorld::initialState::Factory);
REGISTER_TYPE_FACTORY(Object, ObjectDomain("Object"));
REGISTER_TYPE_FACTORY(Timeline, ObjectDomain("Timeline"));
REGISTER_TYPE_FACTORY(Resource, ObjectDomain("Resource"));
REGISTER_OBJECT_FACTORY(ResourceFactory0, Resource:float:float:float);
REGISTER_OBJECT_FACTORY(ResourceFactory1, Resource:float:float:float:float:float);
REGISTER_OBJECT_FACTORY(ResourceFactory2, Resource:float:float:float:float:float:float:float);
REGISTER_OBJECT_FACTORY(ResourceFactory3, Resource);
REGISTER_TYPE_FACTORY(UnaryResource, ObjectDomain("UnaryResource"));
REGISTER_OBJECT_FACTORY(UnaryResourceFactory4, UnaryResource);
REGISTER_TYPE_FACTORY(Location, ObjectDomain("Location"));
REGISTER_OBJECT_FACTORY(LocationFactory5, Location);
REGISTER_TYPE_FACTORY(Target, ObjectDomain("Target"));
REGISTER_OBJECT_FACTORY(TargetFactory6, Target);
REGISTER_TYPE_FACTORY(Path, ObjectDomain("Path"));
REGISTER_OBJECT_FACTORY(PathFactory7, Path:Location:Location);
REGISTER_OBJECT_FACTORY(PathFactory7, Path:Location:Object);
REGISTER_OBJECT_FACTORY(PathFactory7, Path:Object:Location);
REGISTER_OBJECT_FACTORY(PathFactory7, Path:Object:Object);
REGISTER_TYPE_FACTORY(CHAMP_Accessable, ObjectDomain("CHAMP_Accessable"));
REGISTER_OBJECT_FACTORY(CHAMP_AccessableFactory8, CHAMP_Accessable:Location:Target);
REGISTER_OBJECT_FACTORY(CHAMP_AccessableFactory8, CHAMP_Accessable:Location:Timeline);
REGISTER_OBJECT_FACTORY(CHAMP_AccessableFactory8, CHAMP_Accessable:Object:Target);
REGISTER_OBJECT_FACTORY(CHAMP_AccessableFactory8, CHAMP_Accessable:Object:Timeline);
REGISTER_TYPE_FACTORY(OppSci_Accessable, ObjectDomain("OppSci_Accessable"));
REGISTER_OBJECT_FACTORY(OppSci_AccessableFactory9, OppSci_Accessable:Location:Target);
REGISTER_OBJECT_FACTORY(OppSci_AccessableFactory9, OppSci_Accessable:Location:Timeline);
REGISTER_OBJECT_FACTORY(OppSci_AccessableFactory9, OppSci_Accessable:Object:Target);
REGISTER_OBJECT_FACTORY(OppSci_AccessableFactory9, OppSci_Accessable:Object:Timeline);
REGISTER_TYPE_FACTORY(Position, ObjectDomain("Position"));
REGISTER_OBJECT_FACTORY(PositionFactory10, Position:Rover);
REGISTER_OBJECT_FACTORY(PositionFactory10, Position:Object);
REGISTER_TYPE_FACTORY(Tracker, ObjectDomain("Tracker"));
REGISTER_OBJECT_FACTORY(TrackerFactory11, Tracker:Rover);
REGISTER_OBJECT_FACTORY(TrackerFactory11, Tracker:Object);
REGISTER_TYPE_FACTORY(OpportunisticScience, ObjectDomain("OpportunisticScience"));
REGISTER_OBJECT_FACTORY(OpportunisticScienceFactory12, OpportunisticScience:Rover);
REGISTER_OBJECT_FACTORY(OpportunisticScienceFactory12, OpportunisticScience:Object);
REGISTER_TYPE_FACTORY(CHAMP, ObjectDomain("CHAMP"));
REGISTER_OBJECT_FACTORY(CHAMPFactory13, CHAMP:Rover);
REGISTER_OBJECT_FACTORY(CHAMPFactory13, CHAMP:Object);
REGISTER_TYPE_FACTORY(Energy, ObjectDomain("Energy"));
REGISTER_OBJECT_FACTORY(EnergyFactory14, Energy:float:float:float);
REGISTER_TYPE_FACTORY(Rover, ObjectDomain("Rover"));
REGISTER_OBJECT_FACTORY(RoverFactory15, Rover:float);
REGISTER_TYPE_FACTORY(NddlWorld, ObjectDomain("NddlWorld"));
REGISTER_OBJECT_FACTORY(NddlWorldFactory16, NddlWorld:int:int:int);
REGISTER_OBJECT_FACTORY(NddlWorldFactory17, NddlWorld);

// Allocate rules
new Rule$CHAMP$ipsettarget$23();
new Rule$OpportunisticScience$oppscilooknow$18();
new Rule$CHAMP$ipgetname$21();
new Rule$OpportunisticScience$OppSciProcDefined$15();
new Rule$Tracker$TrackingOn$10();
new Rule$Target$trackstart$1();
new Rule$CHAMP$ipgetstatus$27();
new Rule$OpportunisticScience$oppscisetparams$16();
new Rule$CHAMP$iIPHaveName$22();
new Rule$Tracker$trackfreeze$11();
new Rule$Target$Tracked$2();
new Rule$Target$trackstop$3();
new Rule$OpportunisticScience$OppSciParamsSet$17();
new Rule$OpportunisticScience$oppscidefineproc$14();
new Rule$Tracker$StartTracking$9();
new Rule$Position$navigate$5();
new Rule$Tracker$trackunfreeze$13();
new Rule$Tracker$TrackingFrozen$12();
new Rule$Tracker$trackloadgroup$7();
new Rule$Tracker$TrackingOff$6();
new Rule$Target$NotTracked$0();
new Rule$CHAMP$IPDonePlaceInstrument$26();
new Rule$Position$At$4();
new Rule$OpportunisticScience$OppSciDoneLookNow$19();
new Rule$Tracker$LandmarksDefined$8();
new Rule$CHAMP$IPTargetSet$24();
new Rule$OpportunisticScience$oppscigetstatus$20();
new Rule$CHAMP$ipplaceinstrument$25();
return id;
}

}
