#include "HSTSHeuristics.hh"
#include "Generator.hh"
#include "TestSupport.hh"
#include "LabelStr.hh"

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

    heuristics.setDefaultPriorityPreference(PriorityPref::HIGH);

    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    HSTSHeuristics::TokenType tt("SEP_Thrust_Timer_SV.Max_Thrust_Time", domainSpecs);
    heuristics.setDefaultPriorityForTokenDPsWithParent(20.3, tt.getId());

    heuristics.setDefaultPriorityForTokenDPs(10000.0);
    
    heuristics.setDefaultPriorityForConstrainedVariableDPs(10000.0);

    std::vector<TokenDecisionPoint::State> states;
    std::vector<CandidateOrder> orders;
    states.push_back(TokenDecisionPoint::State::REJECT);
    states.push_back(TokenDecisionPoint::State::MERGE);
    states.push_back(TokenDecisionPoint::State::DEFER);
    states.push_back(TokenDecisionPoint::State::ACTIVATE);
    orders.push_back(CandidateOrder::NONE);
    orders.push_back(CandidateOrder::EARLY);
    orders.push_back(CandidateOrder::NONE);
    orders.push_back(CandidateOrder::EARLY);
    heuristics.setDefaultPreferenceForTokenDPs(states,orders);

    heuristics.setDefaultPreferenceForConstrainedVariableDPs(DomainOrder::ASCENDING);

    DEFAULT_TEARDOWN();
    return true;
  }
  static bool testTokenHeuristics() {
    DEFAULT_SETUP();
    TokenType tta("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left", domainSpecs);
    heuristics.setHeuristicsForTokenDPsWithParent(5456.2,tta.getId());

    std::vector<std::pair<LabelStr,LabelStr> > dsb;
    dsb.push_back(std::make_pair<LabelStr,LabelStr>("m_with", "SIN"));
    TokenType ttb("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd", dsb);
    std::vector<TokenDecisionPoint::State> statesb;
    statesb.push_back(TokenDecisionPoint::State::REJECT);
    statesb.push_back(TokenDecisionPoint::State::MERGE);
    statesb.push_back(TokenDecisionPoint::State::DEFER);
    statesb.push_back(TokenDecisionPoint::State::ACTIVATE);
    std::vector<CandidateOrder> ordersb;
    ordersb.push_back(CandidateOrder::NONE);
    ordersb.push_back(CandidateOrder::CLOSE);
    ordersb.push_back(CandidateOrder::NONE);
    ordersb.push_back(CandidateOrder::MIN_FLEXIBLE);
    std::vector<LabelStr> gensb;
    heuristics.setHeuristicsForTokenDP(334.5, ttb.getId(), Relationship::ANY, TokenTypeId::noId(), Origin::FREE, states, orders, gensb);

    std::vector<std::pair<LabelStr,LabelStr> > dsc;
    dsc.push_back(std::make_pair<LabelStr,LabelStr>("m_with","CON"));
    dsc.push_back(std::make_pair<LabelStr,LabelStr>("m_sys","RES"));
    TokenType ttc("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd", dsc);
    TokenType mttc("SEP_Thrust_Timer_SV.Max_Thrust_Time", domainSpec);
    std::vector<TokenDecisionPoint::State> statesc;
    std::vector<CandidateOrder> ordersc;
    statesc.push_back(TokenDecisionPoint::State::MERGE);
    statesc.push_back(TokenDecisionPoint::State::ACTIVATE);
    ordersc.push_back(CandidateOrder::GENERATOR);
    ordersc.push_back(CandidateOrder::LATE);
    std::vector<LabelStr> gensc;
    gensc.push_back("Generator1");
    heuristics.setHeuristicsForTokenDP(6213.7, ttc.getId(), Relationship::AFTER, mttc.getId(), Origin::FREE, statesc, ordersc, gensc);

    std::vector<LabelStr,LabelStr> dsd;
    dsd.push_back(std::make_pair<LabelStr,LabelStr>("m_trans","NO_RIGHT"));
    std::vector<TokenDecisionPoint::State> statesc;
    std::vector<CandidateOrder> ordersc;
    statesd.push_back(TokenDecisionPoint::State::ACTIVATE);
    ordersd.push_back(CandidateOrder::EARLY);
    std::vector<LabelStr> gensd;
    heuristics.setHeuristicsForTokenDP(6213.7, ttd.getId(), Relationship::ANY, TokenTypeId::noId(), Origin::ANY, statesd, ordersd, gensd);

    std::vector<TokenDecisionPoint::State> statese;
    std::vector<CandidateOrder> orderse;
    std::vector<LabelStr> gense;
    heuristics.setHeuristicsForTokenDP(7652.4, tte.getId(), Relationship::ANY, TokenTypeId::noId(), Origin::FREE, statese, orderse, gense);

    DEFAULT_TEARDOWN();
  }
  static bool testVariableHeuristics() {
    DEFAULT_SETUP();

    std::list<double> aenums;
    heuristics.setHeuristicsForConstrainedVariableDPs(443.6, "Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with", TokenType::noId(), DomainOrder::ASCENDING, NO_STRING, aenums);

    aenums.push_back("SIN");
    aenums.push_back("CON");
    aenums.push_back("SAL");
    aenums.push_back("PEP");
    heuristics.setHeuristicsForConstrainedVariableDPs(443.6, "Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with", TokenType::noId(), DomainOrder::ENUMERATION, NO_STRING, aenums);

    heuristics.setHeuristicsForConstrainedVariableDPs(2269.3, "Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with", TokenType::noId(), DomainOrder::GENERATOR, "Generator1");

    std::list<double> benums;
    heuristics.setHeuristicsForConstrainedVariableDPs(234.5, "Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_sys", TokenType::noId(), DomainOrder::ENUMERATION, NO_STRING, benums);

    std::list<double> cenums;
    cenums.push_back("CON");
    LabelStr parentName = stripVariableName("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with");
    std::vector<std::pair<LabelStr,LabelStr> > cds;
    cds.push_back(std::make_pair<LabelStr,LabelStr>("m_sys","ON");
		  TokenType ctt(parentName, cds));
    heuristics.setHeuristicsForConstrainedVariableDPs(6234.7, "Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_with", ctt.getId(), DomainOrder::ENUMERATION, NO_STRING, benums);

    DEFAULT_TEARDOWN();
  }
};

