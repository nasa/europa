#ifndef _H_HSTSHeuristicsReader
#define _H_HSTSHeuristicsReader

#include "CBPlannerDefs.hh"
#include "HSTSHeuristics.hh"
#include <iostream>

class TiXmlElement;

namespace Prototype {

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
    void readPredicateSpec(const TiXmlElement& element);
    void readDecisionPreference(const TiXmlElement& element);
    void readConstrainedVariable(const TiXmlElement& element);
    void readPreference(const TiXmlElement& element);
    void readMaster(const TiXmlElement& element);
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
