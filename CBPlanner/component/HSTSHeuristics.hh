#ifndef _H_HSTSHeuristics
#define _H_HSTSHeuristics

#include "CBPlanner.hh"
#include "LabelStr.hh"
#include "TokenDecisionPoint.hh"
#include "Generator.hh"

#include <vector>

namespace PLASMA {

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
      TokenEntry(const Priority p, const std::vector<LabelStr>& states, const std::vector<CandidateOrder>& orders);
      virtual ~TokenEntry();

      void setPriority(const Priority p);
      const Priority getPriority() const;
      const std::vector<LabelStr>& getStates() const;
      const std::vector<CandidateOrder>& getOrders() const;
    private:
      Priority m_priority;
      std::vector<LabelStr> m_states;
      std::vector<CandidateOrder> m_orders;
    };

    class VariableEntry {
    public:
      VariableEntry();
      VariableEntry(const std::set<double>& domain, const Priority p, const DomainOrder order, const LabelStr& generatorName, const std::list<LabelStr>& enumeration);
      virtual ~VariableEntry();
      void setDomain(const std::list<LabelStr>& domain);
      const Priority getPriority() const;
      const std::list<LabelStr>& getDomain() const;
      const DomainOrder& getDomainOrder() const;
      const GeneratorId& getGenerator() const;
    private:
      std::list<LabelStr> m_domain;
      DomainOrder m_order;
      Priority m_priority;
      GeneratorId m_generator;
    };

    HSTSHeuristics();
    virtual ~HSTSHeuristics();

    void setDefaultPriorityPreference(const PriorityPref pp);
    const PriorityPref getDefaultPriorityPreference() const;
    void setDefaultPriorityForTokenDPsWithParent(const Priority p, const TokenTypeId& tt);
    const Priority getDefaultPriorityForTokenDPsWithParent(const TokenTypeId& tt) const;
    void setDefaultPriorityForTokenDPs(const Priority p);
    const Priority getDefaultPriorityForTokenDPs() const;
    void setDefaultPriorityForConstrainedVariableDPs(const Priority p);
    const Priority getDefaultPriorityForConstrainedVariableDPs() const;
    void setDefaultPreferenceForTokenDPs(const std::vector<LabelStr>& states, const std::vector<CandidateOrder>& orders);
    void setDefaultPreferenceForConstrainedVariableDPs(const DomainOrder order);
    const DomainOrder getDefaultPreferenceForConstrainedVariableDPs() const;

    void setHeuristicsForConstrainedVariableDP(const Priority p, const LabelStr variableName, const TokenTypeId& tt, const DomainOrder order, const LabelStr& generatorName, const std::list<LabelStr>& enumeration);

    void setHeuristicsForTokenDP(const Priority p, const TokenTypeId& tt, const Relationship rel, const TokenTypeId& mastertt, const std::vector<LabelStr>& states, const std::vector<CandidateOrder>& orders);

    void setHeuristicsForTokenDPsWithParent(const Priority p, const TokenTypeId& tt);

    const Priority getPriorityForConstrainedVariableDP(const ConstrainedVariableDecisionPointId& varDec);
    const Priority getPriorityForTokenDP(const TokenDecisionPointId& tokDec);
    const Priority getPriorityForObjectDP(const ObjectDecisionPointId& objDec);

    void write(std::ostream& os = std::cout);

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

    const std::vector<LabelStr>& getInternalStatePreferenceForTokenValueChoice(const TokenTypeId& tt, Relationship rel, const TokenTypeId& mastertt, const Origin o);
    const std::vector<CandidateOrder>& getInternalOrderPreferenceForTokenValueChoice(const TokenTypeId& tt, const Relationship rel, const TokenTypeId& mastertt, const Origin o);

    const std::list<LabelStr>& getInternalPreferenceForVariableValueChoice(const LabelStr& variableName, const TokenTypeId& tt);
    const DomainOrder& getInternalPreferenceForConstrainedVariableDP(const LabelStr& variableName, const TokenTypeId& tt, const LabelStr& generatorName);

    static void candidateOrderToString(const CandidateOrder& order, LabelStr& str);
    static void relationshipToString(const Relationship& rel, LabelStr& str);
    static void originToString(const Origin& orig, LabelStr& str);
    static void domainOrderToString(const DomainOrder dorder, LabelStr& str);

    PriorityPref m_defaultPriorityPreference;
    std::map<LabelStr, Priority> m_defaultCompatibilityPriority;
    Priority m_defaultTokenPriority;
    Priority m_defaultVariablePriority;
    std::vector<LabelStr> m_defaultTokenStates;
    std::vector<CandidateOrder> m_defaultCandidateOrders;
    DomainOrder m_defaultDomainOrder;

    std::map<LabelStr, TokenEntry> m_tokenHeuristics;
    std::map<LabelStr, VariableEntry> m_variableHeuristics;

    std::set<GeneratorId> m_succTokenGenerators;
    std::set<GeneratorId> m_variableGenerators;

    std::map<LabelStr, GeneratorId> m_generatorsByName;
  };
}
#endif
