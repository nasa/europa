#ifndef PRO_GEN_PARSER_HEADER__
#define PRO_GEN_PARSER_HEADER__

/**
 * @file ProGenParser.hh
 * @author David Rijsman
 * @brief Defines the parser for the ProGen/max RCPSP input format
 * @date May 2006
 * @ingroup Resource
*/

#include <sstream>
#include <string>
#include <iostream>

/**
 * @brief Parser for the ProGen/max RCPSP input format
*/
class ProGenParser
{
public:
  /**
   * @brief Constructor
   */
  ProGenParser();
  /**
   * @brief Destructor
   */
  virtual ~ProGenParser();
  /**
   * @brief Parses input file \a file and invokes the corresponding API defined 
   * in this class. \a basename is the base name of the output file created.
   * @return True in case the file is parsed with success otherwise returns false.
   */
  bool parse( const std::string& file, const std::string& basename );
private:
  /**
   * @brief Helper function to determing if the file \a name corresponds to a 
   * readable file.
   * @returns True in case a readable file at location \a name exists otherwise
   * return false
   */
  bool doesFileExist( const std::string& name ) const;
  /**
   * @brief This function is invoked at the end of a successful parse invocation
   */
  virtual void finished(const std::string& file ) {};
  /**
   * @brief Unmarshalles the weight string in the ProGen/max RCPSP input format
   * and populates the parameter \a weight with the actual value.
   * @return True in case of a successful unmarshalling otherwise returns false.
   */
  bool getWeight( const std::string& weightStr, int& weight ) const;
  /**
   * @brief Helper function to parse type \a Type from \a iss
   * @return True in case of success false in case of failure.
   */
  template< class Type >
  bool parse( int line, const char* org, std::istringstream& iss, Type& instance, const std::string& errorString ) const;
  /**
   * @brief
   */
  virtual void updateAllocation( int node, int resource, int allocation ) {}
  /**
   * @brief
   */
  virtual void updateCapacity( int resource, int capacity ) {}
  /**
   * @brief
   */
  virtual void updateDuration( int node, int duration ) {}
  /**
   * @brief
   */
  virtual void updateNumberOfRealActivities( int n ) {}
  /**
   * @brief
   */
  virtual void updateNumberOfRenewableResources( int p ) {}
  /**
   * @brief
   */
  virtual void updateNumberOfSuccessors( int node, int directSuccessors ) {}
  /**
   * @brief
   */
  virtual void updateSuccessor( int node, int successor, int weight ) {}
};

template< class Type >
bool ProGenParser::parse( int line, const char* org, std::istringstream& iss, Type& instance, const std::string& errorString ) const
{
  iss >> instance;
  
  if( iss.fail() )
    {
      std::cerr << errorString
		<< " Parsing line "
		<< line << ":'"
		<< org << " ' failed." << std::endl;
      
      return false;
    }
  
  return true;
}


#endif //PRO_GEN_PARSER_HEADER__
