#include "HSTSHeuristics.hh"
#include "Generator.hh"
#include "TestSupport.hh"
#include "LabelStr.hh"
#include "TokenDecisionPoint.hh"
#include "HSTSHeuristicsReader.hh"

#include <iostream>

#define DEFAULT_SETUP() \
  HSTSHeuristics heuristics; 

#define DEFAULT_TEARDOWN()

class SetupTest {
public:
  static bool test() {
    runTest(testDefaults);
    runTest(testTokenHeuristics);
    runTest(testVariableHeuristics);
    return true;
  }
private:
  static bool testDefaults() {
    DEFAULT_SETUP();

    heuristics.setDefaultPriorityPreference(HSTSHeuristics::HIGH);

    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    TokenType tt(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);
    heuristics.setDefaultPriorityForTokenDPsWithParent(20.3, tt.getId());

    heuristics.setDefaultPriorityForTokenDPs(10000.0);
    
    heuristics.setDefaultPriorityForConstrainedVariableDPs(10000.0);

    std::vector<TokenDecisionPoint::State> states;
    std::vector<HSTSHeuristics::CandidateOrder> orders;
    states.push_back(TokenDecisionPoint::REJECTED);
    states.push_back(TokenDecisionPoint::MERGED);
    //    states.push_back(TokenDecisionPoint::DEFER);
    states.push_back(TokenDecisionPoint::ACTIVE);
    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    heuristics.setDefaultPreferenceForTokenDPs(states,orders);

    heuristics.setDefaultPreferenceForConstrainedVariableDPs(HSTSHeuristics::ASCENDING);

    DEFAULT_TEARDOWN();
    return true;
  }
  static bool testTokenHeuristics() {
    DEFAULT_SETUP();
    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    TokenType tta(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left"), domainSpecs);
    heuristics.setHeuristicsForTokenDPsWithParent(5456.2,tta.getId());

    std::vector<std::pair<LabelStr,LabelStr> > dsb;
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"), LabelStr("SIN")));
    TokenType ttb(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsb);
    std::vector<TokenDecisionPoint::State> statesb;
    statesb.push_back(TokenDecisionPoint::REJECTED);
    statesb.push_back(TokenDecisionPoint::MERGED);
    //    statesb.push_back(TokenDecisionPoint::State::DEFER);
    statesb.push_back(TokenDecisionPoint::ACTIVE);
    std::vector<HSTSHeuristics::CandidateOrder> ordersb;
    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::NEAR);
    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::MIN_FLEXIBLE);
    std::vector<LabelStr> gensb;
    heuristics.setHeuristicsForTokenDP(334.5, ttb.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), HSTSHeuristics::FREE, statesb, ordersb, gensb);

    std::vector<std::pair<LabelStr,LabelStr> > dsc;
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("CON")));
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("RES")));
    TokenType ttc(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsc);
    TokenType mttc(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);
    std::vector<TokenDecisionPoint::State> statesc;
    std::vector<HSTSHeuristics::CandidateOrder> ordersc;
    statesc.push_back(TokenDecisionPoint::MERGED);
    statesc.push_back(TokenDecisionPoint::ACTIVE);
    ordersc.push_back(HSTSHeuristics::TGENERATOR);
    ordersc.push_back(HSTSHeuristics::LATE);
    std::vector<LabelStr> gensc;
    gensc.push_back(LabelStr("Generator1"));
    heuristics.setHeuristicsForTokenDP(6213.7, ttc.getId(), HSTSHeuristics::AFTER, mttc.getId(), HSTSHeuristics::FREE, statesc, ordersc, gensc);

    std::vector<std::pair<LabelStr,LabelStr> > dsd;
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    TokenType ttd(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsd);
    TokenType mttd(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);

    std::vector<TokenDecisionPoint::State> statesd;
    std::vector<HSTSHeuristics::CandidateOrder> ordersd;
    statesd.push_back(TokenDecisionPoint::ACTIVE);
    ordersd.push_back(HSTSHeuristics::EARLY);
    std::vector<LabelStr> gensd;
    heuristics.setHeuristicsForTokenDP(6213.7, ttd.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), HSTSHeuristics::FREE, statesd, ordersd, gensd);

    std::vector<TokenDecisionPoint::State> statese;
    std::vector<HSTSHeuristics::CandidateOrder> orderse;
    std::vector<LabelStr> gense;
    heuristics.setHeuristicsForTokenDP(7652.4, ttd.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), HSTSHeuristics::FREE, statese, orderse, gense);

    DEFAULT_TEARDOWN();
    return true;
  }
  static bool testVariableHeuristics() {
    DEFAULT_SETUP();

    std::list<double> aenums;
    heuristics.setHeuristicsForConstrainedVariableDP(443.6, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with"), TokenTypeId::noId(), HSTSHeuristics::ASCENDING, NO_STRING, aenums);

    aenums.push_back(LabelStr("SIN"));
    aenums.push_back(LabelStr("CON"));
    aenums.push_back(LabelStr("SAL"));
    aenums.push_back(LabelStr("PEP"));
    heuristics.setHeuristicsForConstrainedVariableDP(443.6, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, aenums);

    std::list<double> emptyList;
    heuristics.setHeuristicsForConstrainedVariableDP(2269.3, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with"), TokenTypeId::noId(), HSTSHeuristics::VGENERATOR, LabelStr("Generator1"),emptyList);

    std::list<double> benums;
    heuristics.setHeuristicsForConstrainedVariableDP(234.5, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_sys"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);

    std::list<double> cenums;
    cenums.push_back(LabelStr("CON"));
    //    LabelStr parentName = stripVariableName("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with");
    LabelStr parentName = LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left");
    std::vector<std::pair<LabelStr,LabelStr> > cds;
    cds.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("ON")));
    TokenType ctt(parentName, cds);
    heuristics.setHeuristicsForConstrainedVariableDP(6234.7, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_with"), ctt.getId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);

    DEFAULT_TEARDOWN();
    return true;
  }
};

class HeuristicsReaderTest {
public:
  static bool test() {
    runTest(testReader);
    return true;
  }
private:
  static bool testReader() {
    DEFAULT_SETUP();
    HSTSHeuristicsReader reader(heuristics);
    reader.read("Heuristics-HSTS.xml");
    DEFAULT_TEARDOWN();
    return true;
  }
};

int main() {
  runTestSuite(SetupTest::test);
  runTestSuite(HeuristicsReaderTest::test);
  std::cout << "Finished" << std::endl;
}
