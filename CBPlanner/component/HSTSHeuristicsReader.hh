#ifndef _H_HSTSHeuristicsReader
#define _H_HSTSHeuristicsReader

#include "CBPlannerDefs.hh"
#include "HSTSHeuristics.hh"
#include <iostream>

class TiXmlElement;

namespace PLASMA {

  class HSTSHeuristicsReader {
  public:
    HSTSHeuristicsReader(HSTSHeuristics& heuristics);
    virtual ~HSTSHeuristicsReader();

    void read(const std::string& fileName);
  protected:
    void readElement(const TiXmlElement& element);
    void readDefaults(const TiXmlElement& element);
    void readPriorityPref(const TiXmlElement& element);
    void readVariableSpecification(const TiXmlElement& element);
    void readTokenSpecification(const TiXmlElement& element);
    void readCompatibility(const TiXmlElement& element);
    void readPriority(const TiXmlElement& element, HSTSHeuristics::Priority& p);
    void readPredicateSpec(const TiXmlElement& element, TokenTypeId& tt);
    void readDecisionPreference(const TiXmlElement& element);
    void readConstrainedVariable(const TiXmlElement& element);
    void readPreference(const TiXmlElement& element);
    void readToken(const TiXmlElement& element);
    void readStateOrder(const TiXmlElement& element, LabelStr& state, HSTSHeuristics::CandidateOrder& order);
    void readState(const TiXmlElement& element, LabelStr& state);
    void readValueOrder(const TiXmlElement& element, HSTSHeuristics::DomainOrder& order);
    void readParameter(const TiXmlElement& element, int& index, char* value);
    void readMaster(const TiXmlElement& element, HSTSHeuristics::Relationship& rel, TokenTypeId& ttm);
    void readRelation(const TiXmlElement& element, HSTSHeuristics::Relationship& rel);  
    void readPredicateParameters(const TiXmlElement& element, std::vector<std::pair<LabelStr,LabelStr> >& domainSpecs);
  private:
    HSTSHeuristics m_heuristics;

    const std::string getTextChild(const TiXmlElement& element);
    GeneratorId getGeneratorFromName(std::string genName);

    /*    
	  ConstrainedVariableId parseVariable(std::string variable);
	  ConstrainedVariableId parseToken(std::string token);
    */
   
  };
}

#endif
