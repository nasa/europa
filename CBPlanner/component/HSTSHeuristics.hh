#ifndef _H_HSTSHeuristics
#define _H_HSTSHeuristics

#include "CBPlanner.hh"
#include "LabelStr.hh"
#include "TokenDecisionPoint.hh"
#include "Generator.hh"

#include <vector>

namespace Prototype {

#define MAX_PRIORITY 1000000000.0 
#define MIN_PRIORITY 0.0 
#define DELIMITER ':'
#define FULL_DOMAIN "*"

  class HSTSHeuristics;
  class TokenType;
  typedef Id<TokenType> TokenTypeId;
  class VariableEntry;
  class TokenEntry;

  static const LabelStr NO_STRING("");

  // note: we assume that token types, if specified, are specified to singleton.
  // Any other domain restrictions are not supported.

  class TokenType {
  public:
    TokenType(const LabelStr& predicateName, const std::vector<std::pair<LabelStr,LabelStr> >& domainSpecs);
    virtual ~TokenType();
    
    const TokenTypeId& getId() const;
    const LabelStr& getPredicate() const;
    const std::vector<std::pair<LabelStr, LabelStr> >& getDomainSpecs() const;

    static const LabelStr getIndexKey(const TokenTypeId& tt);
    static const TokenTypeId getTokenType(const LabelStr& indexKey);
    static const TokenTypeId createTokenType(const TokenId& token);
    bool matches(const TokenTypeId& tt);
    bool conflicts(const TokenTypeId& tt);
  private:
    static void split(const std::string& str, const char& delim, std::vector<std::string>& strings);
    LabelStr m_predicateName;
    std::vector<std::pair<LabelStr, LabelStr> > m_domainSpecs;
    TokenTypeId m_id;
  };
  typedef Id<TokenType> TokenTypeId;

  class HSTSHeuristics {
  public:
    enum Origin {
      FREE = 0, /* free token */
      INITIAL, /* free in the intial state */
      SUBGOAL /* has a master */
    };

    // use rbegin() instead of begin() to iterate over the set if priority
    // preference is HIGH 
    enum PriorityPref {
      LOW = 0,
      HIGH
    };

    enum Relationship {
      BEFORE = 0,
      AFTER,
      OTHER,
      ANY
    };

    enum CandidateOrder {
      TGENERATOR = 0,
      NEAR,
      FAR,
      EARLY,
      LATE,
      MAX_FLEXIBLE,
      MIN_FLEXIBLE,
      LEAST_SPECIFIED,
      MOST_SPECIFIED,
      NONE
    };

    enum DomainOrder {
      VGENERATOR = 0,
      ASCENDING,
      DESCENDING,
      ENUMERATION
    };

    typedef double Priority;

    class TokenEntry {
    public:
      TokenEntry();
      TokenEntry(const Priority p, const std::vector<TokenDecisionPoint::State>& states, const std::vector<CandidateOrder>& orders, const std::vector<LabelStr>& generatorNames);
      virtual ~TokenEntry();

      void setPriority(const Priority p);
      const Priority getPriority();
      const std::vector<TokenDecisionPoint::State>& getStates();
      const std::vector<CandidateOrder>& getOrders();
      const std::vector<GeneratorId>& getGenerators();
    private:
      Priority m_priority;
      std::vector<TokenDecisionPoint::State> m_states;
      std::vector<CandidateOrder> m_orders;
      std::vector<GeneratorId> m_generators;
    };

    class VariableEntry {
    public:
      VariableEntry();
      VariableEntry(const std::list<double>& domain, const Priority p, const DomainOrder order, const LabelStr& generatorName);
      VariableEntry(const std::list<double>& domain, const Priority p, const DomainOrder order, const std::list<double>& enumeration);
      virtual ~VariableEntry();
      void setPriority(const Priority p);
      void setDomainOrder(const std::list<double>& domain, const DomainOrder order, const LabelStr& generatorName);
      Priority getPriority();
      const std::list<double>& getDomain();
      const GeneratorId& getGenerator();
    private:
      std::list<double> m_domain;
      Priority m_priority;
      GeneratorId m_generator;
    };

    HSTSHeuristics();
    virtual ~HSTSHeuristics();

    void setDefaultPriorityPreference(const PriorityPref pp);
    void setDefaultPriorityForTokenDPsWithParent(const Priority p, const TokenTypeId& tt);
    void setDefaultPriorityForTokenDPs(const Priority p);
    void setDefaultPriorityForConstrainedVariableDPs(const Priority p);
    void setDefaultPreferenceForTokenDPs(const std::vector<TokenDecisionPoint::State>& states, const std::vector<CandidateOrder>& orders);
    void setDefaultPreferenceForConstrainedVariableDPs(const DomainOrder order);

    void setHeuristicsForConstrainedVariableDP(const Priority p, const LabelStr variableName, const TokenTypeId& tt, const DomainOrder order, const LabelStr& generatorName, const std::list<double>& enumeration);

    void setHeuristicsForTokenDP(const Priority p, const TokenTypeId& tt, const Relationship rel, const TokenTypeId& mastertt, const Origin o, const std::vector<TokenDecisionPoint::State>& states, const std::vector<CandidateOrder>& orders, const std::vector<LabelStr>& generatorNames);

    void setHeuristicsForTokenDPsWithParent(const Priority p, const TokenTypeId& tt);

    const Priority getPriorityForConstrainedVariableDP(const ConstrainedVariableDecisionPointId& varDec);
    const Priority getPriorityForTokenDP(const TokenDecisionPointId& tokDec);
    const Priority getPriorityForObjectDP(const ObjectDecisionPointId& objDec);

  /*
    void setPriorityForConstrainedVariableDP(const Priority p, const LabelStr variableName, const TokenTypeId& tt);
    void setPriorityForTokenDP(const Priority p, const TokenTypeId& tt, const Relationship rel, const TokenTypeId& mastertt, const Origin o);

    void setPreferenceForTokenValueChoice(const std::vector<TokenDecisionPoint::State>& states, const std::vector<CandidateOrder>& orders, const std::vector<LabelStr>& generatorNames, const TokenTypeId& tt, Relationship rel, const TokenTypeId& mastertt, const Origin o);
    void setPreferenceForVariableValueChoice(const DomainOrder do, const LabelStr& variableName, const TokenTypeId& parenttt, const LabelStr& generatorName, const list<double>& enumeration);
    */

  private:
    // Generator methods
    void addSuccTokenGenerator(const GeneratorId& generator);
    void addVariableGenerator(const GeneratorId& generator);
    const GeneratorId& getGeneratorByName(const LabelStr& name) const ;

    // Auxiliary methods:
    static const LabelStr getIndexKey(const LabelStr& variableName, const TokenTypeId& tt);
    static const LabelStr getIndexKey(const TokenTypeId& tt, const Relationship rel, const TokenTypeId& mastertt, const Origin o);

    // Internal Queries:
    const Priority getInternalPriorityForConstrainedVariableDP(const LabelStr variableName, const TokenTypeId& tt);
    const Priority getInternalPriorityForTokenDP(const TokenTypeId& tt, Relationship rel, const TokenTypeId& mastertt, const Origin o);

    const std::vector<TokenDecisionPoint::State>& getInternalStatePreferenceForTokenValueChoice(const TokenTypeId& tt, Relationship rel, const TokenTypeId& mastertt, const Origin o);
    const std::vector<CandidateOrder>& getInternalOrderPreferenceForTokenValueChoice(const TokenTypeId& tt, const Relationship rel, const TokenTypeId& mastertt, const Origin o);

    const std::list<double>& getInternalPreferenceForVariableValueChoice(const LabelStr& variableName, const TokenTypeId& tt);
    const DomainOrder& getInternalPreferenceForConstrainedVariableDP(const LabelStr& variableName, const TokenTypeId& tt, const LabelStr& generatorName);

    PriorityPref m_defaultPriorityPreference;
    std::map<double, Priority> m_defaultCompatibilityPriority;
    Priority m_defaultTokenPriority;
    Priority m_defaultVariablePriority;
    std::vector<TokenDecisionPoint::State> m_defaultTokenStates;
    std::vector<CandidateOrder> m_defaultCandidateOrders;
    DomainOrder m_defaultDomainOrder;

    std::map<double, TokenEntry> m_tokenHeuristics;
    std::map<double, VariableEntry> m_variableHeuristics;

    std::set<GeneratorId> m_succTokenGenerators;
    std::set<GeneratorId> m_variableGenerators;

    std::map<double, GeneratorId> m_generatorsByName;
  };
}
#endif
