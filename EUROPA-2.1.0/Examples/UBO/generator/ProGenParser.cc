//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software

#include <fstream>
#include <iostream>
#include "ProGenParser.hh"
#include <vector>

#define LINESIZE 1024


ProGenParser::ProGenParser()
{
}

ProGenParser::~ProGenParser()
{
}


bool ProGenParser::parse( const std::string& file, const std::string& basename )
{
  if( !doesFileExist( file ) )
    {
      std::cerr << "Parse failed: file '"
		<< file << "' does not exist or is not readable." << std::endl;

      return false;
    }

  int lineCount = 0;

  char buf[ LINESIZE ];
  
  int numberOfRealActivities = 0; 
  int numberOfRenewableResources = 0; 

  std::ifstream script( file.c_str() );
  
  while( script.getline(buf, LINESIZE ) )
    {
      ++lineCount;

      std::istringstream iss( buf );

      if( 1 == lineCount )
	{
	  if( !parse<int>( lineCount, buf, iss, numberOfRealActivities, "Could not retrieve the number of real activities.") )
	    return false;

	  updateNumberOfRealActivities( numberOfRealActivities );

	  if( !parse<int>( lineCount, buf, iss, numberOfRenewableResources, "Could not retrieve the number of renewable resources.") )
	    return false;

	  updateNumberOfRenewableResources( numberOfRealActivities );
	}
      else if( lineCount <= numberOfRealActivities + 3 )
	{
	  if( lineCount == numberOfRealActivities + 3 )
	    continue;

	  int index; 

	  if( !parse<int>( lineCount, buf, iss, index, "Could not retrieve the index of the line defining the direct successors.") ) 
	    return false;

	  int redundantOne;

	  if( !parse<int>( lineCount, buf, iss, redundantOne, "Could not retrieve the redundant one of the line defining the direct successors.") )
	    return false;

	  int directSuccessors;

	  if( !parse<int>( lineCount, buf, iss, directSuccessors, "Could not retrieve the number of direct successors" ) )
	    return false;
	  
	  updateNumberOfSuccessors( index, directSuccessors );

	  std::vector<int> successors;
	  std::vector<int> weights;

	  for( int i = 0; i < directSuccessors; ++i )
	    {
	      int successor;

	      if( !parse<int>( lineCount, buf, iss, successor, "Could not retrieve a successor" ) )
		return false;

	      successors.push_back( successor );
	    }
	  
	  for( int i = 0; i < directSuccessors; ++i )
	    {
	      std::string weight;
	      int weightValue;

	      if( !parse<std::string>( lineCount, buf, iss, weight, "Could not retrieve a weight" ) )
		return false;

	      if( !getWeight( weight, weightValue ) )
		return false;

	      weights.push_back( weightValue );
	    }

	  if( successors.size() != weights.size() )
	    {
	      std::cerr << "Expected as many weights ("
			<< weights.size() << ") as successors ("
			<< successors.size() << ") for node " 
			<< index << std::endl;

	      return false;
	    }

	  for( int i = 0; i < successors.size(); i++ )
	    {
	      updateSuccessor( index, successors[i], weights[i] );
	    }
	}
      else if( lineCount <= 2 * numberOfRealActivities + 5 )
	{
	  int index; 

	  if( !parse<int>( lineCount, buf, iss, index, "Could not retrieve the index of the line defining the duration of nodes.") ) 
	    return false;

	  int redundantOne;

	  if( !parse<int>( lineCount, buf, iss, redundantOne, "Could not retrieve the redundant one of the line defining the direct successors.") )
	    return false;

	  int duration;

	  if( !parse<int>( lineCount, buf, iss, duration, "Could not retrieve the duration.") )
	    return false;
	  
	  updateDuration( index, duration );

	  for( int i = 0; i < numberOfRenewableResources; ++i )
	    {
	      int allocation;

	      if( !parse<int>( lineCount, buf, iss, allocation, "Could not retrieve the allocation of a resource.") )
		return false;

	      updateAllocation( index, i, allocation );      
	    }
	}
      else if( lineCount == 2 * numberOfRealActivities + 6 )
	{
	  for( int i = 0; i < numberOfRenewableResources; ++i )
	    {
	      int capacity;

	      if( !parse<int>( lineCount, buf, iss, capacity, "Could not retrieve the capacity of a resource.") )
		return false;

	      updateCapacity( i, capacity );      
	    }
	  
	}
    }

  if( lineCount < 2 * numberOfRealActivities + 6 )
    {
      std::cerr << "Not enough input from file '" 
		<< file << "' counted " 
		<< lineCount << " lines, requires at least " 
		<< 2 * numberOfRealActivities + 6 << std::endl;
  
      return false;
    }

  
  finished( basename );

  return true;
}

bool ProGenParser::getWeight( const std::string& weightStr, int& weightValue ) const
{
  std::string weight = weightStr;

  int pos = weight.find('[');

  if( std::string::npos == pos )
    {
      std::cerr << "Expected the weight value to be [<value>], can not open square bracket in '"
		<< weightStr << "'" << std::endl;

      return false;
    }
	      
  weight.erase( 0, pos + 1);

  pos = weight.find('-');
	      
  int sign = 1;
	      
  if( std::string::npos != pos )
    {
      sign = -1;
      weight.erase( 0, pos + 1);
    }
  
  std::istringstream iss( weight );
	      
  iss >> weightValue;
	      
  if( iss.fail() )
    {
      std::cerr << "Expected the weight value to be [<value>], can not parse the value from '"
		<< weightStr << "'" << std::endl;
      
      return false;
    }

  weightValue *= sign;
	      
  return true;
}

bool ProGenParser::doesFileExist( const std::string& name ) const
{
  if( !name.empty() )
    {  
      FILE* test;
      
      if( (test = fopen( name.c_str(), "rb") ) != NULL ) 
	{
	  fclose(test); 
	  
	  return true;
	}
    }
  
  return false;
}


