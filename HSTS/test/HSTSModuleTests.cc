#include "HSTSModuleTests.hh"
#include "TestSupport.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Variable.hh"
#include "ObjectDecisionPoint.hh"
#include "Choice.hh"
#include "StringDomain.hh"
#include "NumericDomain.hh"
#include "Generator.hh"
#include "HSTSHeuristicsReader.hh"
#include "HSTSNoBranchCondition.hh"
#include "HSTSPlanIdReader.hh"
#include "HSTSOpenDecisionManager.hh"
#include "AtSubgoalRule.hh"

extern bool loggingEnabled();

namespace PLASMA {

  /**
   * @brief Creates the type specifications required for testing
   */
  void initCBPTestSchema(){
    const SchemaId& schema = Schema::instance();
    schema->reset();
    schema->addObjectType("Objects");

    schema->addPredicate("Objects.PredicateA");
    schema->addMember("Objects.PredicateA", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.PredicateB");
    schema->addPredicate("Objects.PredicateC");
    schema->addPredicate("Objects.PredicateD");
    schema->addPredicate("Objects.PADDED");

    schema->addPredicate("Objects.P1");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam0");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam1");
    schema->addMember("Objects.P1", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.P1True");
    schema->addMember("Objects.P1True", BoolDomain().getTypeName(), "BoolParam");
    schema->addPredicate("Objects.P1False");
  }

  void initHeuristicsSchema(){
    const SchemaId& rover = Schema::instance();
    rover->reset();
    rover->addObjectType(LabelStr("Object"));
    rover->addObjectType(LabelStr("Timeline"), LabelStr("Object"));
    rover->addObjectType(LabelStr("NddlResource"));
    rover->addObjectType("Resource", "NddlResource");
    rover->addMember("Resource", "float", "initialCapacity");
    rover->addMember("Resource", "float", "levelLimitMin");
    rover->addMember("Resource", "float", "levelLimitMax");
    rover->addMember("Resource", "float", "productionRateMax");
    rover->addMember("Resource", "float", "productionMax");
    rover->addMember("Resource", "float", "consumptionRateMax");
    rover->addMember("Resource", "float", "consumptionMax");
    rover->addPredicate("Resource.change");
    rover->addMember("Resource.change", "float", "quantity");
    rover->addObjectType("UnaryResource", "Timeline");
    rover->addPredicate("UnaryResource.uses");
    rover->addObjectType("Battery", "Resource");
    rover->addObjectType("Location", "Object");
    // rover->isObjectType("Location");
    rover->addMember("Location", "string", "name");
    rover->addMember("Location", "int", "x");
    rover->addMember("Location", "int", "y");
    rover->addObjectType("Path", "Object");
    rover->addMember("Path", "string", "name");
    rover->addMember("Path", "Location", "from");
    rover->addMember("Path", "Location", "to");
    rover->addMember("Path", "float", "cost");
    rover->addObjectType("Navigator", "Timeline");
    rover->addPredicate("Navigator.At");
    rover->addMember("Navigator.At", "Location", "location");
    rover->addPredicate("Navigator.Going");
    rover->addMember("Navigator.Going", "Location", "from");
    rover->addMember("Navigator.Going", "Location", "to");
    rover->addObjectType("Commands", "Timeline");
    rover->addPredicate("Commands.TakeSample");
    rover->addMember("Commands.TakeSample", "Location", "rock");
    rover->addPredicate("Commands.PhoneHome");
    rover->addPredicate("Commands.PhoneLander");
    rover->addObjectType("Instrument", "Timeline");
    rover->addPredicate("Instrument.TakeSample");
    rover->addMember("Instrument.TakeSample", "Location", "rock");
    rover->addPredicate("Instrument.Place");
    rover->addMember("Instrument.Place", "Location", "rock");
    rover->addPredicate("Instrument.Stow");
    rover->addPredicate("Instrument.Unstow");
    rover->addPredicate("Instrument.Stowed");
    rover->addObjectType("Rover", "Object");
    rover->addMember("Rover", "Commands", "commands");
    rover->addMember("Rover", "Navigator", "navigator");
    rover->addMember("Rover", "Instrument", "instrument");
    rover->addMember("Rover", "Battery", "mainBattery");
    rover->addObjectType("NddlWorld", "Timeline");
    rover->addMember("NddlWorld", "int", "m_horizonStart");
    rover->addMember("NddlWorld", "int", "m_horizonEnd");
    rover->addMember("NddlWorld", "int", "m_maxPlannerSteps");
    rover->addPredicate("NddlWorld.initialState");
    rover->addEnum("TokenStates");
    rover->addValue("TokenStates", LabelStr("INACTIVE"));
    rover->addValue("TokenStates", LabelStr("ACTIVE"));
    rover->addValue("TokenStates", LabelStr("MERGED"));
    rover->addValue("TokenStates", LabelStr("REJECTED"));
    rover->addMember("Navigator.Going", "Path", "p");
    rover->addMember("Commands.TakeSample", "Rover", "rovers");
    rover->addMember("Commands.TakeSample", "bool", "OR");
    rover->addMember("Instrument.TakeSample", "Rover", "rovers");
    rover->addMember("Instrument.Place", "Rover", "rovers");
    rover->addMember("Instrument.Unstow", "Rover", "rovers");
    rover->addMember("Instrument.Stow", "Rover", "rovers");

    // extra stuff to test
    rover->addObjectType("Telemetry", "Object");
    rover->addPredicate("Telemetry.Communicate");
    rover->addMember("Telemetry.Communicate", "int", "minutes");
    rover->addMember("Telemetry.Communicate", "float", "bandwidth");
    rover->addMember("Telemetry.Communicate", "bool", "encoded");
    rover->addMember("Telemetry.Communicate", "Mode", "mode");
    rover->addEnum("Mode");
    rover->addValue("Mode", LabelStr("high"));
    rover->addValue("Mode", LabelStr("medium-high"));
    rover->addValue("Mode", LabelStr("medium"));
    rover->addValue("Mode", LabelStr("medium-low"));
    rover->addValue("Mode", LabelStr("low"));
  }

  bool testHSTSNoBranchConditionImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm) {

    HSTSNoBranchCondition cond(dm.getId());
    assert(dm.getConditions().size() == 1);

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());

    const LabelStr var1Name("AnObj.APred.Var1");

    noBranchSpec->addNoBranch(var1Name);

    Variable<IntervalIntDomain> var1(ce.getId(), IntervalIntDomain(), true, var1Name);
    Variable<IntervalIntDomain> var2(ce.getId(), IntervalIntDomain(), true, LabelStr("AnObj.APred.Var2"));

    //std::cout << " var1 name = " << var1.getName().c_str() << std::endl;
    //    std::cout << " var2 name = " << var2.getName().c_str() << std::endl;

    cond.initialize(noBranchSpec);

    assert(!cond.test(var1.getId()));
    assert(cond.test(var2.getId()));

    noBranchSpec.remove();

    return true;
  }

  bool testTokenTypeImpl(HSTSHeuristics& heuristics) {
    std::vector<std::pair<LabelStr,LabelStr> > ds1;
    TokenType tt1(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), ds1);

    TokenType tt2(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), ds1);

    assert(tt1.conflicts(tt2.getId()));

    std::vector<std::pair<LabelStr,LabelStr> > ds3;
    ds3.push_back(std::make_pair<LabelStr,LabelStr>("v1","val1"));
    TokenType tt3(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), ds3);

    assert(tt1.conflicts(tt3.getId()));

    std::vector<std::pair<LabelStr,LabelStr> > ds4;
    ds4.push_back(std::make_pair<LabelStr,LabelStr>("v2","val2"));
    TokenType tt4(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), ds4);

    assert(tt1.conflicts(tt4.getId()));
    assert(tt3.conflicts(tt4.getId()));

    std::vector<std::pair<LabelStr,LabelStr> > ds5;
    ds5.push_back(std::make_pair<LabelStr,LabelStr>("v1","val2"));
    TokenType tt5(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), ds5);

    assert(tt1.conflicts(tt5.getId()));
    assert(tt4.conflicts(tt5.getId()));
    assert(!tt3.conflicts(tt5.getId()));

    return true;
  }

  bool testDefaultInitializationImpl(HSTSHeuristics& heuristics) {
    heuristics.setDefaultPriorityPreference(HSTSHeuristics::HIGH);

    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    TokenType tt(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);

    heuristics.setDefaultPriorityForTokenDPsWithParent(20.3, tt.getId());

    heuristics.setDefaultPriorityForTokenDPs(10000.0);
    
    heuristics.setDefaultPriorityForConstrainedVariableDPs(10000.0);

    std::vector<LabelStr> states;
    std::vector<HSTSHeuristics::CandidateOrder> orders;
    states.push_back(Token::REJECTED);
    states.push_back(Token::MERGED);
    //    states.push_back(Token::DEFER);
    states.push_back(Token::ACTIVE);
    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    //    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    heuristics.setDefaultPreferenceForTokenDPs(states,orders);

    heuristics.setDefaultPreferenceForConstrainedVariableDPs(HSTSHeuristics::ASCENDING);

    return true;
  }

  bool testTokenInitializationImpl(HSTSHeuristics& heuristics) {
    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    TokenType tta(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left"), domainSpecs);
    heuristics.setHeuristicsForTokenDPsWithParent(5456.2,tta.getId());

    std::vector<std::pair<LabelStr,LabelStr> > dsb;
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("CON")));
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("DES")));
    TokenType ttb(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsb);
    std::vector<LabelStr> statesb;
    statesb.push_back(Token::REJECTED);
    statesb.push_back(Token::MERGED);
    //    statesb.push_back(LabelStr::DEFER);
    statesb.push_back(Token::ACTIVE);
    std::vector<HSTSHeuristics::CandidateOrder> ordersb;
    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::NEAR);
    //    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::MIN_FLEXIBLE);
    heuristics.setHeuristicsForTokenDP(334.5, ttb.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), statesb, ordersb);

    std::vector<std::pair<LabelStr,LabelStr> > dsc;
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("CON")));
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("DES")));
    TokenType ttc(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsc);
    TokenType mttc(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);
    std::vector<LabelStr> statesc;
    std::vector<HSTSHeuristics::CandidateOrder> ordersc;
    statesc.push_back(Token::MERGED);
    statesc.push_back(Token::ACTIVE);
    ordersc.push_back(HSTSHeuristics::TGENERATOR);
    ordersc.push_back(HSTSHeuristics::LATE);
    heuristics.setHeuristicsForTokenDP(6213.7, ttc.getId(), HSTSHeuristics::AFTER, mttc.getId(), statesc, ordersc);

    std::vector<std::pair<LabelStr,LabelStr> > dsd;
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("SIN")));
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("RES")));
    TokenType ttd(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsd);
    TokenType mttd(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);

    std::vector<LabelStr> statesd;
    std::vector<HSTSHeuristics::CandidateOrder> ordersd;
    statesd.push_back(Token::ACTIVE);
    ordersd.push_back(HSTSHeuristics::EARLY);
    heuristics.setHeuristicsForTokenDP(6213.7, ttd.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), statesd, ordersd);

    /* this rightly produces a duplicate entry error in the code 
    std::vector<LabelStr> statese;
    std::vector<HSTSHeuristics::CandidateOrder> orderse;
    heuristics.setHeuristicsForTokenDP(7652.4, ttd.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), statese, orderse);
    */

    return true;
  }

  bool testVariableInitializationImpl(HSTSHeuristics& heuristics) {
    /*  doesn't work because the schema doesn't agree with the data used
	here 
    std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
    TokenType tt(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"),domainSpec);
    std::vector<LabelStr> aenums;
    heuristics.setHeuristicsForConstrainedVariableDP(443.6, LabelStr("with"), tt.getId(), HSTSHeuristics::ASCENDING, NO_STRING, aenums);

    aenums.push_back(LabelStr("SIN"));
    aenums.push_back(LabelStr("CON"));
    aenums.push_back(LabelStr("SAL"));
    aenums.push_back(LabelStr("PEP"));
    heuristics.setHeuristicsForConstrainedVariableDP(443.6, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, aenums);

    std::vector<LabelStr> emptyList;
    heuristics.setHeuristicsForConstrainedVariableDP(2269.3, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with"), TokenTypeId::noId(), HSTSHeuristics::VGENERATOR, LabelStr("Generator1"),emptyList);

    std::vector<LabelStr> benums;
    heuristics.setHeuristicsForConstrainedVariableDP(234.5, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_sys"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);

    std::vector<LabelStr> cenums;
    cenums.push_back(LabelStr("CON"));
    //    LabelStr parentName = stripVariableName("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with");
    LabelStr parentName = LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left");
    std::vector<std::pair<LabelStr,LabelStr> > cds;
    cds.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("ON")));
    TokenType ctt(parentName, cds);
    heuristics.setHeuristicsForConstrainedVariableDP(6234.7, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_with"), ctt.getId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);
    */
    return true;
  }

  bool testReaderImpl(HSTSHeuristics& heuristics) {
    initHeuristicsSchema();

    HSTSHeuristicsReader reader(heuristics.getNonConstId());

    reader.read("../core/Heuristics-HSTS.xml");
    
    //    heuristics.write();

    return true;
  }

  bool testHSTSPlanIdReaderImpl() {

    initHeuristicsSchema();

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../core/NoBranch.pi");

    return true;
  }

  bool testHSTSNoBranchImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner& planner) {
    initHeuristicsSchema();

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../core/NoBranch.pi");

    DecisionManagerId dm = planner.getDecisionManager();
    HSTSNoBranchCondition cond(dm);
    assert(dm->getConditions().size() == 4);

    Variable<IntervalIntDomain> var1(ce.getId(), IntervalIntDomain(), true, LabelStr("Commands.TakeSample.rock"));
    Variable<IntervalIntDomain> var2(ce.getId(), IntervalIntDomain(), true, LabelStr("AnObj.APred.Var2"));

    //    std::cout << " var1 name = " << var1.getName().c_str() << std::endl;
    //    std::cout << " var2 name = " << var2.getName().c_str() << std::endl;

    cond.initialize(noBranchSpec);

    assert(!cond.test(var1.getId()));
    assert(cond.test(var2.getId()));

    return true;
  }

  bool testHSTSHeuristicsAssemblyImpl(ConstraintEngine& ce, PlanDatabase& db, CBPlanner& planner, HSTSHeuristics& heuristics) {
    initHeuristicsSchema();
    HSTSHeuristicsReader hreader(heuristics.getNonConstId());
    hreader.read("../core/Heuristics-HSTS.xml");
    //    heuristics.write();

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader pireader(noBranchSpec);
    pireader.read("../core/NoBranch.pi");

    DecisionManagerId& dm = planner.getDecisionManager();
    HSTSNoBranchCondition cond(dm);
    cond.initialize(noBranchSpec);
    HSTSOpenDecisionManagerId odm = (new HSTSOpenDecisionManager(dm, heuristics.getId()))->getId();
    dm->setOpenDecisionManager(odm);

    Timeline com(db.getId(),LabelStr("Commands"),LabelStr("com1"));
    Timeline ins(db.getId(),LabelStr("Instrument"),LabelStr("ins1"));
    Timeline nav(db.getId(),LabelStr("Navigator"),LabelStr("nav1"));
    Timeline tel(db.getId(),LabelStr("Telemetry"),LabelStr("tel1"));

    Object loc1(db.getId(),LabelStr("Location"),LabelStr("Loc1"));
    Object loc2(db.getId(),LabelStr("Location"),LabelStr("Loc2"));
    Object loc3(db.getId(),LabelStr("Location"),LabelStr("Loc3"));
    Object loc4(db.getId(),LabelStr("Location"),LabelStr("Loc4"));
    Object loc5(db.getId(),LabelStr("Location"),LabelStr("Loc5"));

    db.close();

    std::list<ObjectId> results;
    db.getObjectsByType("Location",results);
    ObjectDomain allLocs(results,"Location");

    std::list<double> values;
    values.push_back(LabelStr("high"));
    values.push_back(LabelStr("medium-high"));
    values.push_back(LabelStr("medium"));
    values.push_back(LabelStr("medium-low"));
    values.push_back(LabelStr("low"));
    EnumeratedDomain allModes(values,false,"Mode");

    IntervalToken tok0(db.getId(),LabelStr("Telemetry.Communicate"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,100), IntervalIntDomain(1,100), "tel1", false);
    tok0.addParameter(IntervalDomain("int"), LabelStr("minutes"));
    ConstrainedVariableId vmin = tok0.getVariable("minutes");
    vmin->specify(IntervalIntDomain(60,120));
    tok0.addParameter(IntervalDomain("float"), LabelStr("bandwidth"));
    ConstrainedVariableId vband = tok0.getVariable("bandwidth");
    vband->specify(IntervalDomain(500.3,1200.4));
    //    tok0.addParameter(BoolDomain(), LabelStr("encoded"));  token
    //    fails to recognize BoolDomain().
    tok0.addParameter(allModes, LabelStr("mode"));
    tok0.close();

    IntervalToken tok1(db.getId(),LabelStr("Commands.TakeSample"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,100), IntervalIntDomain(1,100), "com1", false);
    tok1.addParameter(allLocs, LabelStr("rock"));
    tok1.close();
    ConstrainedVariableId vrock = tok1.getVariable("rock");
    vrock->specify(db.getObject("Loc3"));

    IntervalToken tok2(db.getId(),LabelStr("Instrument.TakeSample"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "ins1", false);
    tok2.addParameter(allLocs, LabelStr("rock"));
    tok2.close();

    IntervalToken tok3(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "nav1", false);
    tok3.addParameter(allLocs, LabelStr("location"));
    tok3.close();

    IntervalToken tok4(db.getId(),LabelStr("Navigator.Going"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "nav1", false);
    tok4.addParameter(allLocs, LabelStr("from"));
    tok4.addParameter(allLocs, LabelStr("to"));
    tok4.close();

    IntervalToken tok5(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "nav1", false);
    tok5.addParameter(allLocs, LabelStr("location"));
    tok5.close();
    ConstrainedVariableId vatloc = tok5.getVariable("location");
    vatloc->specify(db.getObject("Loc3"));

    AtSubgoalRule r("Navigator.At");

    CBPlanner::Status res = planner.run();
    assert(res == CBPlanner::PLAN_FOUND);

    return true;
  }

}
