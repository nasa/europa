#ifndef H_HeuristicsReader
#define H_HeuristicsReader

#include "HSTSDefs.hh"
#include "XMLUtils.hh"
#include "Heuristic.hh"
#include "CommonDefs.hh"

/**
 * @author Conor McGann
 */
namespace EUROPA {

  class HeuristicsReader {
  public:
    HeuristicsReader(const HeuristicsEngineId& he);

    /**
     * @brief Primary method to initialize the engine using a configuration file
     */
    void read(const std::string& fileName, bool initialize = true);

    /**
     * @brief Create a heuristic from configuration data in the given xml element. Used internally and exposed
     * for testing.
     */
    static HeuristicId createHeuristic(const HeuristicsEngineId& he, const TiXmlElement& element);

  private:
    static void HeuristicsReader::readHeuristic(const HeuristicsEngineId& he, const TiXmlElement& configData);

    /* Declare constants for tags */
    DECLARE_STATIC_CLASS_CONST(char*, PRIORITY_TAG, "Priority");
    DECLARE_STATIC_CLASS_CONST(char*, PREFERENCE_TAG, "Preference");
    DECLARE_STATIC_CLASS_CONST(char*, VARIABLE_TAG, "VariableSpec");
    DECLARE_STATIC_CLASS_CONST(char*, PREDICATE_SPEC_TAG, "PredicateSpec");
    DECLARE_STATIC_CLASS_CONST(char*, PREDICATE_NAME_TAG, "PredicateName");
    DECLARE_STATIC_CLASS_CONST(char*, PREDICATE_PARAMETERS_TAG, "PredicateParameters");
    DECLARE_STATIC_CLASS_CONST(char*, PREDICATE_PARAMETER_TAG, "Parameter");
    DECLARE_STATIC_CLASS_CONST(char*, MASTER_SPEC_TAG, "Master");

    static bool readPriority(const TiXmlElement& configData, Priority& priority);

    static bool readPreference(const TiXmlElement& configData, 
			       VariableHeuristic::DomainOrder& domainOrder, 
			       std::list<double>& values);

    static void readValueOrder(const TiXmlElement& element, VariableHeuristic::DomainOrder& order);

    static void readDomainOrder(const TiXmlElement& element, std::list<double>& values);

    static bool readVariable(const TiXmlElement& configData, const LabelStr& predicate, LabelStr& varName);

    static bool readPredicateSpec(const TiXmlElement& configData, 
				  LabelStr& predicate,
				  std::vector< GuardEntry >& guards);

    static void readGuards(const TiXmlElement& configData, const LabelStr& predicate, std::vector< GuardEntry >& guards);

    static void readParameter(const TiXmlElement& element, const LabelStr& predicate, int& index, double& value);

    static void readIndex(const TiXmlElement& element, int& index);

    static void readValue(const TiXmlElement& element, double& value);

    static bool readMaster(const TiXmlElement& element, 
			   MasterRelation& rel,
			   LabelStr& masterPredicate,
			   std::vector< GuardEntry >& guards );

    static void readRelation(const TiXmlElement& element, MasterRelation& rel);

    static void readDecisionPreference(const TiXmlElement& element, 
				       std::vector<LabelStr>& states,
				       std::vector<TokenHeuristic::CandidateOrder>& orders);

    static void readStateOrder(const TiXmlElement& element, 
			       LabelStr& state, 
			       TokenHeuristic::CandidateOrder& order);

    static void readState(const TiXmlElement& element, LabelStr& state);

    void readDefaults(const TiXmlElement& element);
    void readDefaultCreationPref(const TiXmlElement& element);
    void readDefaultPriorityPref(const TiXmlElement& element);
    void readDefaultCompatibility(const TiXmlElement& element);
    void readDefaultConstrainedVariable(const TiXmlElement& element);
    void readDefaultToken(const TiXmlElement& element);

    const HeuristicsEngineId m_he;
  };
}
#endif
