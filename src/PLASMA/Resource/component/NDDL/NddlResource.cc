#include "NddlResource.hh"
#include "Debug.hh"
#include "DataTypes.hh"
#include "Domains.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "ConstrainedVariable.hh"
#include "Constraints.hh"
#include "Transaction.hh"
#include "Instant.hh"
#include "Profile.hh"

namespace NDDL {

// -------------------------------------------------------------------------------------------------------
//  	First pass at code that allows user to use NDDL to select profile and fv detector to use
// -------------------------------------------------------------------------------------------------------

static const std::string PARAM_PROFILE_TYPE("profileType");
static const std::string PARAM_DETECTOR_TYPE("detectorType");

bool isValidCombo(const std::string& profileName, const std::string& detectorName)
{
	if(profileName == "GroundedProfile" && detectorName != "GroundedFVDetector")
		return false;

	return true;
}


// For getting either the profile or detector name specified by the given parameter.  We also check that each is valid, and that the two
// are valid to use together
std::pair <LabelStr, LabelStr> getProfileAndDetectorNames(const Object* res, const std::string& defaultProfile, const std::string& defaultDetector)
{
	std::string pFullName = res->getName().toString()+"."+PARAM_PROFILE_TYPE;
	std::string dFullName = res->getName().toString()+"."+PARAM_DETECTOR_TYPE;

	ConstrainedVariableId pNameVar = res->getVariable(pFullName);
	ConstrainedVariableId dNameVar = res->getVariable(dFullName);

	LabelStr pName(defaultProfile);
	LabelStr dName(defaultDetector);

	if (!pNameVar.isNoId()) {
		debugMsg("NDDL","Using resource profile variable : " << pNameVar->toString());
		pName = LabelStr(pNameVar->derivedDomain().getSingletonValue());
	}

	if (!dNameVar.isNoId()) {
		debugMsg("NDDL","Using resource detector variable : " << dNameVar->toString());
		dName = LabelStr(dNameVar->derivedDomain().getSingletonValue());
	}

	check_error(isValidCombo(pName.toString(), dName.toString()), "Invalid combination of profile " + pName.toString() + " and detector " + dName.toString());
	return std::make_pair(pName, dName);
}

// -------------------------------------------------------------------------------------------------------

  NddlUnaryToken::NddlUnaryToken(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, const bool& rejectable, const bool& isFact, const bool& close)
    : EUROPA::UnaryToken(planDatabase, predicateName, rejectable, isFact, IntervalIntDomain(), IntervalIntDomain(), IntervalIntDomain(1, PLUS_INFINITY),
			       EUROPA::Token::noObject(), false, false) {
    commonInit(close);
  }

  NddlUnaryToken::NddlUnaryToken(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, const bool& close)
    : EUROPA::UnaryToken(master, relation, predicateName, IntervalIntDomain(), IntervalIntDomain(), IntervalIntDomain(1, PLUS_INFINITY),
			       EUROPA::Token::noObject(), false, false) {
    commonInit(close);
  }

  void NddlUnaryToken::handleDefaults(const bool&) {}

  void NddlUnaryToken::commonInit(const bool& autoClose) {
    state = getState();
    object = getObject();
    tStart = start();
    tEnd = end();
    tDuration = duration();
    if(autoClose)
      close();
  }

  NddlUnary::NddlUnary(const PlanDatabaseId& planDatabase,
			     const LabelStr& type,
			     const LabelStr& name,
			     bool open)
    : EUROPA::Reusable(planDatabase, type, name, open) {}

  NddlUnary::NddlUnary(const ObjectId& parent,
			     const LabelStr& type,
			     const LabelStr& name,
			     bool open)
    : EUROPA::Reusable(parent, type, name, open) {}


  void NddlUnary::close() {
    static const unsigned int CMAX = 0;
    check_error_variable(static const unsigned int ARG_COUNT = 1);
    static const std::string PARAM_CONSUMPTION_MAX("consumptionMax");

    check_error(m_variables.size() >= ARG_COUNT);
    check_error(m_variables[CMAX]->getName().toString().find(PARAM_CONSUMPTION_MAX) != std::string::npos);

    check_error(m_variables[CMAX]->derivedDomain().isSingleton());

    std::pair <LabelStr, LabelStr> pd = getProfileAndDetectorNames(this, "IncrementalFlowProfile", "ClosedWorldFVDetector");

    init(1, 1, //capacity lb, capacity ub
         0, 1,//lower limit, upper limit
         PLUS_INFINITY, PLUS_INFINITY, //max inst production, max inst consumption
         m_variables[CMAX]->derivedDomain().getSingletonValue(), m_variables[CMAX]->derivedDomain().getSingletonValue(), //max production, max consumption
         pd.second, pd.first);
    EUROPA::Resource::close();
  }

  void NddlUnary::handleDefaults(bool autoClose) {
    if(consumptionMax.isNoId()) {
      consumptionMax = addVariable(IntervalDomain(FloatDT::instance()), "consumptionMax");
    }
    if(autoClose)
      close();
  }

  void NddlUnary::constructor(float c_max) {
    consumptionMax = addVariable(IntervalDomain(c_max, c_max, FloatDT::instance()), "consumptionMax");
  }

  void NddlUnary::constructor() {
    consumptionMax = addVariable(IntervalDomain(PLUS_INFINITY, PLUS_INFINITY, FloatDT::instance()), "consumptionMax");
  }

  NddlUnary::use::use(const PlanDatabaseId& planDatabase,
                      const LabelStr& predicateName,
                      bool rejectable,
                      bool isFact,
                      bool close)
    : EUROPA::ReusableToken(planDatabase, predicateName, IntervalIntDomain(), IntervalIntDomain(), IntervalIntDomain(1, PLUS_INFINITY),
                                  IntervalDomain(1.0), Token::noObject(), false) {
    handleDefaults(close);
  }

  NddlUnary::use::use(const TokenId& master,
				const LabelStr& predicateName,
				const LabelStr& relation,
				bool close)
    : EUROPA::ReusableToken(master, relation, predicateName, IntervalIntDomain(), IntervalIntDomain(), IntervalIntDomain(1, PLUS_INFINITY),
                                  IntervalDomain(1.0), Token::noObject(), false) {
    handleDefaults(close);
  }

  void NddlUnary::use::close() {
    EUROPA::ReusableToken::close();
  }

  void NddlUnary::use::handleDefaults(bool autoClose) {
    state = getState();
    object = getObject();
    tStart = start();
    tEnd = end();
    tDuration = duration();

    if(autoClose)
      close();
  }

  /*===============*/

  NddlReusable::NddlReusable(const PlanDatabaseId& planDatabase,
			     const LabelStr& type,
			     const LabelStr& name,
			     bool open)
    : EUROPA::Reusable(planDatabase, type, name, open) {}

  NddlReusable::NddlReusable(const ObjectId& parent,
			     const LabelStr& type,
			     const LabelStr& name,
			     bool open)
    : EUROPA::Reusable(parent, type, name, open) {}

  void NddlReusable::close() {
    static const unsigned int C = 0;
    static const unsigned int LLMIN = 1;
    static const unsigned int CRMAX = 2;
    static const unsigned int CMAX = 3;
    check_error_variable(static const unsigned int ARG_COUNT = 4);
    static const std::string PARAM_CAPACITY("capacity");
    static const std::string PARAM_LEVEL_LIMIT_MIN("levelLimitMin");
    static const std::string PARAM_CONSUMPTION_RATE_MAX("consumptionRateMax");
    static const std::string PARAM_CONSUMPTION_MAX("consumptionMax");

    check_error(m_variables.size() >= ARG_COUNT);
    check_error(m_variables[C]->getName().toString().find(PARAM_CAPACITY) != std::string::npos);
    check_error(m_variables[LLMIN]->getName().toString().find(PARAM_LEVEL_LIMIT_MIN) != std::string::npos);
    check_error(m_variables[CRMAX]->getName().toString().find(PARAM_CONSUMPTION_RATE_MAX) != std::string::npos);
    check_error(m_variables[CMAX]->getName().toString().find(PARAM_CONSUMPTION_MAX) != std::string::npos);

    check_error(m_variables[C]->derivedDomain().isSingleton());
    check_error(m_variables[LLMIN]->derivedDomain().isSingleton());
    check_error(m_variables[CRMAX]->derivedDomain().isSingleton());
    check_error(m_variables[CMAX]->derivedDomain().isSingleton());

    std::pair <LabelStr, LabelStr> pd = getProfileAndDetectorNames(this, "IncrementalFlowProfile", "ClosedWorldFVDetector");

    // TBS:  Use PLUS_INFINITY for upper limit, since TimetableProfile/GroundedProfile could compute
    // upper bounds above capacity, even though we know those don't correspond to real flaws (because a reusable
    // resource can't ever really exceed capacity)
    init(m_variables[C]->derivedDomain().getSingletonValue(), m_variables[C]->derivedDomain().getSingletonValue(),
	 m_variables[LLMIN]->derivedDomain().getSingletonValue(), PLUS_INFINITY,
	 m_variables[CRMAX]->derivedDomain().getSingletonValue(), m_variables[CRMAX]->derivedDomain().getSingletonValue(),
	 m_variables[CMAX]->derivedDomain().getSingletonValue(), m_variables[CMAX]->derivedDomain().getSingletonValue(),
	 pd.second, pd.first);

    EUROPA::Resource::close();
  }

  void NddlReusable::handleDefaults(bool autoClose) {
    if(capacity.isNoId()) {
      capacity = addVariable(IntervalDomain(FloatDT::instance()), "capacity");
    }
    if(levelLimitMin.isNoId()) {
      levelLimitMin = addVariable(IntervalDomain(FloatDT::instance()), "levelLimitMin");
    }
    if(consumptionRateMax.isNoId()) {
      consumptionRateMax = addVariable(IntervalDomain(FloatDT::instance()), "consumptionRateMax");
    }
    if(consumptionMax.isNoId()) {
      consumptionMax = addVariable(IntervalDomain(FloatDT::instance()), "consumptionMax");
    }
    if(autoClose)
      close();
  }

  void NddlReusable::constructor(float c, float ll_min) {
    capacity = addVariable(IntervalDomain(c, c, FloatDT::instance()), "capacity");
    levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, FloatDT::instance()), "levelLimitMin");
    consumptionRateMax = addVariable(IntervalDomain(PLUS_INFINITY, PLUS_INFINITY, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(PLUS_INFINITY, PLUS_INFINITY, FloatDT::instance()), "consumptionMax");
  }
  void NddlReusable::constructor(float c, float ll_min, float cr_max) {
    capacity = addVariable(IntervalDomain(c, c, FloatDT::instance()), "capacity");
    levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, FloatDT::instance()), "levelLimitMin");
    consumptionRateMax = addVariable(IntervalDomain(cr_max, cr_max, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(PLUS_INFINITY, PLUS_INFINITY, FloatDT::instance()), "consumptionMax");
  }
  void NddlReusable::constructor(float c, float ll_min, float c_max, float cr_max) {
    capacity = addVariable(IntervalDomain(c, c, FloatDT::instance()), "capacity");
    levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, FloatDT::instance()), "levelLimitMin");
    consumptionRateMax = addVariable(IntervalDomain(cr_max, cr_max, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(c_max, c_max, FloatDT::instance()), "consumptionMax");
  }

  void NddlReusable::constructor() {
    capacity = addVariable(IntervalDomain(PLUS_INFINITY, PLUS_INFINITY, FloatDT::instance()), "capacity");
    levelLimitMin = addVariable(IntervalDomain(MINUS_INFINITY, MINUS_INFINITY, FloatDT::instance()), "levelLimitMin");
    consumptionRateMax = addVariable(IntervalDomain(PLUS_INFINITY, PLUS_INFINITY, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(PLUS_INFINITY, PLUS_INFINITY, FloatDT::instance()), "consumptionMax");
  }

  NddlReusable::uses::uses(const PlanDatabaseId& planDatabase,
				const LabelStr& predicateName,
				bool rejectable,
				bool isFact,
				bool close)
    : EUROPA::ReusableToken(planDatabase, predicateName, IntervalIntDomain(), IntervalIntDomain(), IntervalIntDomain(1, PLUS_INFINITY),
				  Token::noObject(), false) {
    handleDefaults(close);
  }

  NddlReusable::uses::uses(const TokenId& master,
				const LabelStr& predicateName,
				const LabelStr& relation,
				bool close)
    : EUROPA::ReusableToken(master, relation, predicateName, IntervalIntDomain(), IntervalIntDomain(), IntervalIntDomain(1, PLUS_INFINITY),
				  Token::noObject(), false) {
    handleDefaults(close);
  }

  void NddlReusable::uses::close() {
    EUROPA::ReusableToken::close();
  }

  void NddlReusable::uses::handleDefaults(bool autoClose) {
    state = getState();
    object = getObject();
    tStart = start();
    tEnd = end();
    tDuration = duration();
    quantity = getQuantity();

    if(autoClose)
      close();
  }

  /*===============*/

  NddlCBReusable::NddlCBReusable(const PlanDatabaseId& planDatabase,
                 const LabelStr& type,
                 const LabelStr& name,
                 bool open)
    : EUROPA::CBReusable(planDatabase, type, name, open) {}

  NddlCBReusable::NddlCBReusable(const ObjectId& parent,
                 const LabelStr& type,
                 const LabelStr& name,
                 bool open)
    : EUROPA::CBReusable(parent, type, name, open) {}

  void NddlCBReusable::close() {
    static const unsigned int C = 0;
    static const unsigned int LLMIN = 1;
    static const unsigned int CRMAX = 2;
    static const unsigned int CMAX = 3;
    check_error_variable(static const unsigned int ARG_COUNT = 4);
    static const std::string PARAM_CAPACITY("capacity");
    static const std::string PARAM_LEVEL_LIMIT_MIN("levelLimitMin");
    static const std::string PARAM_CONSUMPTION_RATE_MAX("consumptionRateMax");
    static const std::string PARAM_CONSUMPTION_MAX("consumptionMax");

    check_error(m_variables.size() >= ARG_COUNT);
    check_error(m_variables[C]->getName().toString().find(PARAM_CAPACITY) != std::string::npos);
    check_error(m_variables[LLMIN]->getName().toString().find(PARAM_LEVEL_LIMIT_MIN) != std::string::npos);
    check_error(m_variables[CRMAX]->getName().toString().find(PARAM_CONSUMPTION_RATE_MAX) != std::string::npos);
    check_error(m_variables[CMAX]->getName().toString().find(PARAM_CONSUMPTION_MAX) != std::string::npos);

    check_error(m_variables[C]->derivedDomain().isSingleton());
    check_error(m_variables[LLMIN]->derivedDomain().isSingleton());
    check_error(m_variables[CRMAX]->derivedDomain().isSingleton());
    check_error(m_variables[CMAX]->derivedDomain().isSingleton());

    std::pair <LabelStr, LabelStr> pd = getProfileAndDetectorNames(this, "IncrementalFlowProfile", "ClosedWorldFVDetector");

    // TBS:  Use PLUS_INFINITY for upper limit, since TimetableProfile/GroundedProfile could compute
    // upper bounds above capacity, even though we know those don't correspond to real flaws (because a reusable
    // resource can't ever really exceed capacity)
    init(m_variables[C]->derivedDomain().getSingletonValue(), m_variables[C]->derivedDomain().getSingletonValue(),
     m_variables[LLMIN]->derivedDomain().getSingletonValue(), PLUS_INFINITY,
     m_variables[CRMAX]->derivedDomain().getSingletonValue(), m_variables[CRMAX]->derivedDomain().getSingletonValue(),
     m_variables[CMAX]->derivedDomain().getSingletonValue(), m_variables[CMAX]->derivedDomain().getSingletonValue(),
     pd.second, pd.first);

    EUROPA::Resource::close();
  }

  void NddlCBReusable::handleDefaults(bool autoClose) {
    if(capacity.isNoId()) {
      capacity = addVariable(IntervalDomain(FloatDT::instance()), "capacity");
    }
    if(levelLimitMin.isNoId()) {
      levelLimitMin = addVariable(IntervalDomain(FloatDT::instance()), "levelLimitMin");
    }
    if(consumptionRateMax.isNoId()) {
      consumptionRateMax = addVariable(IntervalDomain(FloatDT::instance()), "consumptionRateMax");
    }
    if(consumptionMax.isNoId()) {
      consumptionMax = addVariable(IntervalDomain(FloatDT::instance()), "consumptionMax");
    }
    if(autoClose)
      close();
  }

  void NddlCBReusable::constructor() {
      constructor(
              PLUS_INFINITY,  // capacity
              MINUS_INFINITY, // level limit min
              PLUS_INFINITY,  // consumption max
              PLUS_INFINITY   // consumption rate max
      );
  }
  void NddlCBReusable::constructor(float c, float ll_min) {
      constructor(
              c,              // capacity
              ll_min,         // level limit min
              PLUS_INFINITY,  // consumption max
              PLUS_INFINITY   // consumption rate max
      );
  }
  void NddlCBReusable::constructor(float c, float ll_min, float cr_max) {
      constructor(
              c,              // capacity
              ll_min,         // level limit min
              PLUS_INFINITY,  // consumption max
              cr_max          // consumption rate max
      );
  }
  void NddlCBReusable::constructor(float c, float ll_min, float c_max, float cr_max) {
    capacity = addVariable(IntervalDomain(c, c, FloatDT::instance()), "capacity");
    levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, FloatDT::instance()), "levelLimitMin");
    consumptionRateMax = addVariable(IntervalDomain(cr_max, cr_max, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(c_max, c_max, FloatDT::instance()), "consumptionMax");
  }

  /*===============*/

  NddlReservoir::NddlReservoir(const PlanDatabaseId& planDatabase,
			       const LabelStr& type,
			       const LabelStr& name,
			       bool open)
    : EUROPA::Reservoir(planDatabase, type, name, open) {}

  NddlReservoir::NddlReservoir(const ObjectId parent,
			       const LabelStr& type,
			       const LabelStr& name,
			       bool open)
    : EUROPA::Reservoir(parent, type, name, open) {}

  void NddlReservoir::close() {
    static const int IC = 0;
    static const int LLMIN = 1;
    static const int LLMAX = 2;
    static const int PRMAX = 3;
    static const int PMAX = 4;
    static const int CRMAX = 5;
    static const int CMAX = 6;
    check_error_variable(static const unsigned int ARG_COUNT = 7);
    static const std::string PARAM_INITIAL_CAPACITY("initialCapacity");
    static const std::string PARAM_LEVEL_LIMIT_MIN("levelLimitMin");
    static const std::string PARAM_LEVEL_LIMIT_MAX("levelLimitMax");
    static const std::string PARAM_PRODUCTION_RATE_MAX("productionRateMax");
    static const std::string PARAM_PRODUCTION_MAX("productionMax");
    static const std::string PARAM_CONSUMPTION_RATE_MAX("consumptionRateMax");
    static const std::string PARAM_CONSUMPTION_MAX("consumptionMax");

    // Ensure the binding of variable names is as expected

    check_error(m_variables.size() >= ARG_COUNT);
    check_error(m_variables[IC]->getName().toString().find(PARAM_INITIAL_CAPACITY)  != std::string::npos);
    check_error(m_variables[LLMIN]->getName().toString().find(PARAM_LEVEL_LIMIT_MIN)  != std::string::npos);
    check_error(m_variables[LLMAX]->getName().toString().find(PARAM_LEVEL_LIMIT_MAX)  != std::string::npos);
    check_error(m_variables[PRMAX]->getName().toString().find(PARAM_PRODUCTION_RATE_MAX)  != std::string::npos);
    check_error(m_variables[PMAX]->getName().toString().find(PARAM_PRODUCTION_MAX)  != std::string::npos);
    check_error(m_variables[CRMAX]->getName().toString().find(PARAM_CONSUMPTION_RATE_MAX)  != std::string::npos);
    check_error(m_variables[CMAX]->getName().toString().find(PARAM_CONSUMPTION_MAX)  != std::string::npos);


    // Ensure all values have been set to singletons already
    check_error(m_variables[IC]->derivedDomain().isSingleton());
    check_error(m_variables[LLMIN]->derivedDomain().isSingleton());
    check_error(m_variables[LLMAX]->derivedDomain().isSingleton());
    check_error(m_variables[PRMAX]->derivedDomain().isSingleton());
    check_error(m_variables[PMAX]->derivedDomain().isSingleton());
    check_error(m_variables[CRMAX]->derivedDomain().isSingleton());
    check_error(m_variables[CMAX]->derivedDomain().isSingleton());

    std::pair <LabelStr, LabelStr> pd = getProfileAndDetectorNames(this, "IncrementalFlowProfile", "ClosedWorldFVDetector");

    init(m_variables[IC]->derivedDomain().getSingletonValue(), m_variables[IC]->derivedDomain().getSingletonValue(),
	 m_variables[LLMIN]->derivedDomain().getSingletonValue(), m_variables[LLMAX]->derivedDomain().getSingletonValue(),
	 m_variables[PRMAX]->derivedDomain().getSingletonValue(), (m_variables[CRMAX]->derivedDomain().getSingletonValue()),
	 m_variables[PMAX]->derivedDomain().getSingletonValue(), (m_variables[CMAX]->derivedDomain().getSingletonValue()),
	 pd.second, pd.first);
    EUROPA::Resource::close();
  }

  void NddlReservoir::handleDefaults(bool autoClose) {
    if(initialCapacity.isNoId()){
      initialCapacity = addVariable(IntervalDomain(FloatDT::instance()), "initialCapacity");
    }
    if(levelLimitMin.isNoId()){
      levelLimitMin = addVariable(IntervalDomain(FloatDT::instance()), "levelLimitMin");
    }
    if(levelLimitMax.isNoId()){
      levelLimitMax = addVariable(IntervalDomain(FloatDT::instance()), "levelLimitMax");
    }
    if(productionRateMax.isNoId()){
      productionRateMax = addVariable(IntervalDomain(FloatDT::instance()), "productionRateMax");
    }
    if(productionMax.isNoId()){
      productionMax = addVariable(IntervalDomain(FloatDT::instance()), "productionMax");
    }
    if(consumptionRateMax.isNoId()){
      consumptionRateMax = addVariable(IntervalDomain(FloatDT::instance()), "consumptionRateMax");
    }
    if(consumptionMax.isNoId()){
      consumptionMax = addVariable(IntervalDomain(FloatDT::instance()), "consumptionMax");
    }
    if (autoClose)
      close();
  }

  void NddlReservoir::constructor(float ic, float ll_min, float ll_max) {
    initialCapacity = addVariable(IntervalDomain(ic, ic, FloatDT::instance()), "initialCapacity");
    levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, FloatDT::instance()), "levelLimitMin");
    levelLimitMax = addVariable(IntervalDomain(ll_max, ll_max, FloatDT::instance()), "levelLimitMax");
    productionRateMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "productionRateMax");
    productionMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "productionMax");
    consumptionRateMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "consumptionMax");
  }

  void NddlReservoir::constructor(float ic, float ll_min, float ll_max, float p_max, float c_max) {
    initialCapacity = addVariable(IntervalDomain(ic, ic, FloatDT::instance()), "initialCapacity");
    levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, FloatDT::instance()), "levelLimitMin");
    levelLimitMax = addVariable(IntervalDomain(ll_max, ll_max, FloatDT::instance()), "levelLimitMax");
    productionRateMax = addVariable(IntervalDomain(p_max, p_max, FloatDT::instance()), "productionRateMax");
    productionMax = addVariable(IntervalDomain(p_max, p_max, FloatDT::instance()), "productionMax");
    consumptionRateMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(c_max, c_max, FloatDT::instance()), "consumptionMax");
  }

  void NddlReservoir::constructor(float ic, float ll_min, float ll_max, float pr_max, float p_max, float cr_max, float c_max) {
    initialCapacity = addVariable(IntervalDomain(ic, ic, FloatDT::instance()), "initialCapacity");
    levelLimitMin = addVariable(IntervalDomain(ll_min, ll_min, FloatDT::instance()), "levelLimitMin");
    levelLimitMax = addVariable(IntervalDomain(ll_max, ll_max, FloatDT::instance()), "levelLimitMax");
    productionRateMax = addVariable(IntervalDomain(pr_max, pr_max, FloatDT::instance()), "productionRateMax");
    productionMax = addVariable(IntervalDomain(p_max, p_max, FloatDT::instance()), "productionMax");
    consumptionRateMax = addVariable(IntervalDomain(cr_max, cr_max, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(c_max, c_max, FloatDT::instance()), "consumptionMax");
  }


  void NddlReservoir::constructor() {
    initialCapacity = addVariable(IntervalDomain(0, 0, FloatDT::instance()), "initialCapacity");
    levelLimitMin = addVariable(IntervalDomain(-inf, -inf, FloatDT::instance()), "levelLimitMin");
    levelLimitMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "levelLimitMax");
    productionRateMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "productionRateMax");
    productionMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "productionMax");
    consumptionRateMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "consumptionRateMax");
    consumptionMax = addVariable(IntervalDomain(+inf, +inf, FloatDT::instance()), "consumptionMax");
  }


  NddlReservoir::produce::produce(const PlanDatabaseId& planDatabase,
				    const LabelStr& predicateName,
				    bool rejectable,
				    bool isFact,
				    bool close)
    : EUROPA::ProducerToken(planDatabase, predicateName, rejectable, isFact, IntervalIntDomain(), Token::noObject(), false) {
    handleDefaults(close);
  }

  NddlReservoir::produce::produce(const TokenId& master,
			       const LabelStr& predicateName,
			       const LabelStr& relation,
			       bool close)
    : EUROPA::ProducerToken(master, relation, predicateName, IntervalIntDomain(), Token::noObject(), false) {
    handleDefaults(close);
  }

  void NddlReservoir::produce::close() {
    EUROPA::ProducerToken::close();
  }

  void NddlReservoir::produce::handleDefaults(bool autoClose) {
    state = getState();
    object = getObject();
    tStart = start();
    tEnd = end();
    tDuration = duration();
    time = getTime();
    quantity = getQuantity();

    if (autoClose)
      close();
  }

  NddlReservoir::consume::consume(const PlanDatabaseId& planDatabase,
				    const LabelStr& predicateName,
				    bool rejectable,
				    bool isFact,
				    bool close)
    : EUROPA::ConsumerToken(planDatabase, predicateName, rejectable, isFact, IntervalIntDomain(), Token::noObject(), false) {
    handleDefaults(close);
  }

  NddlReservoir::consume::consume(const TokenId& master,
			       const LabelStr& predicateName,
			       const LabelStr& relation,
			       bool close)
    : EUROPA::ConsumerToken(master, relation, predicateName, IntervalIntDomain(), Token::noObject(), false) {
    handleDefaults(close);
  }

  void NddlReservoir::consume::close() {
    EUROPA::ConsumerToken::close();
  }

  void NddlReservoir::consume::handleDefaults(bool autoClose) {
    state = getState();
    object = getObject();
    tStart = start();
    tEnd = end();
    tDuration = duration();
    time = getTime();
    quantity = getQuantity();

    if (autoClose)
      close();
  }
} // namespace NDDL
