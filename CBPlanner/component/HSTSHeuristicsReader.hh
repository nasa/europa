#ifndef _H_HSTSHeuristicsReader
#define _H_HSTSHeuristicsReader

#include "CBPlannerDefs.hh"
#include "HSTSHeuristics.hh"
#include <iostream>

class TiXmlElement;

namespace PLASMA {

  class HSTSHeuristicsReader {
  public:
    HSTSHeuristicsReader(HSTSHeuristicsId& heuristics, const SchemaId& schema);
    virtual ~HSTSHeuristicsReader();

    void read(const std::string& fileName);
  protected:
    void readElement(const TiXmlElement& element);
    void readDefaults(const TiXmlElement& element);
    void readDefaultPriorityPref(const TiXmlElement& element);
    void readDefaultCompatibility(const TiXmlElement& element);
    void readDefaultToken(const TiXmlElement& element);
    void readDefaultConstrainedVariable(const TiXmlElement& element);
    void readPriorityPref(const TiXmlElement& element);
    void readVariableSpecification(const TiXmlElement& element);
    void readTokenSpecification(const TiXmlElement& element);
    void readPriority(const TiXmlElement& element, HSTSHeuristics::Priority& p);
    void readPredicateSpec(const TiXmlElement& element, LabelStr& pred, std::vector<std::pair<LabelStr,LabelStr> >& domainSpec);
    void readDecisionPreference(const TiXmlElement& element, std::vector<LabelStr>& states, std::vector<HSTSHeuristics::CandidateOrder>& orders);
    void readConstrainedVariable(const TiXmlElement& element);
    void readPreference(const TiXmlElement& element, LabelStr& genName, HSTSHeuristics::DomainOrder& dorder, std::vector<LabelStr>& values); 
    void readToken(const TiXmlElement& element);
    void readStateOrder(const TiXmlElement& element, LabelStr& state, HSTSHeuristics::CandidateOrder& order);
    void readState(const TiXmlElement& element, LabelStr& state);
    void readValueOrder(const TiXmlElement& element, HSTSHeuristics::DomainOrder& order);
    void readParameter(const TiXmlElement& element, int& index, char* value);
    void readMaster(const TiXmlElement& element, HSTSHeuristics::Relationship& rel, TokenTypeId& ttm);
    void readRelation(const TiXmlElement& element, HSTSHeuristics::Relationship& rel);  
    void readPredicateParameters(const TiXmlElement& element, std::vector<std::pair<LabelStr,LabelStr> >& domainSpecs);
    void readDomainOrder(const TiXmlElement& element, std::vector<LabelStr>& values);
    void readVariableSpec(const TiXmlElement& element, int& index, LabelStr& name);
    void readIndex(const TiXmlElement& element, int& index);
  private:
    HSTSHeuristicsId m_heuristics;
    SchemaId m_schema;

    const std::string getTextChild(const TiXmlElement& element);
    GeneratorId getGeneratorFromName(std::string genName);
  };
}

#endif
