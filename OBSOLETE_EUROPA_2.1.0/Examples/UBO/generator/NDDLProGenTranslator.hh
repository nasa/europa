#ifndef NDDL_PRO_GEN_PARSER_HEADER_
#define NDDL_PRO_GEN_PARSER_HEADER_

/**
 * @file NDDLProGenTranslator.hh
 * @author David Rijsman
 * @brief Defines the translater for the ProGen/max RCPSP input format to NDDL
 * @date May 2006
 * @ingroup Resource
 */

#include "ProGenParser.hh"
#include <map>
#include <set>
#include <iostream>
#include <fstream>

template<class Type> std::string toString( Type& t )
{
  std::ostringstream ss;
  
  if( !(ss << t ) )
    {
      std::cerr << "Error: can not parse value '" << t << "' to string." << std::endl;
      
      abort();
    }
  
  return ss.str();
}


/**
 * @brief
 */
class NDDLProGenTranslator:
  public ProGenParser
{
public:
  /**
   * @brief Constructor
   */
  NDDLProGenTranslator();
  /**
   * @brief Resets the invoking instance. 
   */
  void reset();
  /**
   * @brief Will limit the upperbound of each activity to \a value 
   * The last activity end will be limited to value + 1 because we set
   * the duration of this activity to 1 because EUROPA can not handle 0.
   */
  void setUpperBound( int value );
  /**
   * @brief Creates the NDDL model and saves it under the name \a file
   *
   * Makes a model for ProGen/max RCPSP input format in NDDL
   */
  void generateModel();
  const std::string& getModelFileName() const { return m_ModelName; }
private:
  /**
   * @brief Constructs the NDLL file (the problem instance)
   */
  void finished( const std::string& file );
  /**
   * @brief
   */
  void updateAllocation( int node, int resource, int allocation ) ;
  /**
   * @brief
   */
  void updateCapacity( int resource, int capacity ) ;
  /**
   * @brief
   */
  void updateDuration( int node, int duration ) ;
  /**
   * @brief
   */
  void updateNumberOfRealActivities( int n ) ;
  /**
   * @brief
   */
  void updateNumberOfRenewableResources( int p ) ;
  /**
   * @brief
   */
  void updateNumberOfSuccessors( int node, int directSuccessors ) ;
  /**
   * @brief
   */
  void updateSuccessor( int node, int successor, int weight ) ;

  typedef std::map<int,int> Int2Int;
  typedef std::pair<int,int> IntPair;
  typedef std::set< IntPair > IntPairSet;
  typedef std::map<int,IntPair> Int2IntPair;
  typedef std::map<int,IntPairSet> Int2IntPairSet;

  std::string m_ModelName;
  int m_NumberOfActivities;
  int m_NumberOfResources;
  bool m_UpperBoundSet;
  int m_UpperBound;

  Int2Int m_Capacities;
  Int2Int m_Durations;
  Int2IntPairSet m_Allocations;
  Int2IntPairSet m_Successors;
};

#endif //NDDL_PRO_GEN_PARSER_HEADER_
