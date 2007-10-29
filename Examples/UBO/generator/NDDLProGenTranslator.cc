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

#include "NDDLProGenTranslator.hh"

NDDLProGenTranslator::NDDLProGenTranslator():
  m_NumberOfActivities( 0 ),
  m_NumberOfResources( 0 ),
  m_UpperBoundSet( false ),
  m_ModelName("RCSPSProgen.nddl")
{
}

void NDDLProGenTranslator::reset() 
{
  m_NumberOfActivities = 0;
  m_NumberOfResources = 0;

  m_Capacities.clear();
  m_Durations.clear();
  m_Allocations.clear();
  m_Successors.clear();
}

void NDDLProGenTranslator::setUpperBound( int value )
{
  m_UpperBoundSet = true;
  m_UpperBound = value;
}

void NDDLProGenTranslator::generateModel()
{
  std::string outputFile = m_ModelName;

  std::ofstream output( outputFile.c_str() );

  output << "/**" << std::endl;

  output << "/**" << std::endl;
  output << "* @file " << outputFile << std::endl;
  output << "* @author Automated ProGen/max RCPSP input format converter to NDDL (David Rijsman)" << std::endl;
  output << "*/" << std::endl;
  output << std::endl;
  output << "#include \"Plasma.nddl\"" << std::endl;
  output << "#include \"PlannerConfig.nddl\"" << std::endl;
  output << std::endl;
  output << "class CapacityResource extends Reusable {" << std::endl;
  output << "	CapacityResource( float mincapacity, float maxcapacity ) {" << std::endl;
  output << "	  super( maxcapacity, mincapacity );" << std::endl;
  output << "      } " << std::endl;
  output << "}" << std::endl;
  output << std::endl;
  output << "class Allocation {" << std::endl;
  output << "	int m_identifier;" << std::endl;
  output << "	float m_quantity;" << std::endl;
  output << "	CapacityResource m_resource;" << std::endl;
  output << "" << std::endl;
  output << "	Allocation( CapacityResource resource, int activityId, float quantity ) {" << std::endl;
  output << "	m_identifier = activityId;" << std::endl;
  output << "	m_resource = resource;" << std::endl;
  output << "	m_quantity = quantity;" << std::endl;
  output << "	}" << std::endl;
  output << "}" << std::endl;
  output << std::endl;
  output << "class ProblemInstance extends Object {" << std::endl;
  output << "	predicate Activity {" << std::endl;
  output << "	int m_identifier;" << std::endl;
  output << "	bool b;" << std::endl;
  output << "	}" << std::endl;
  output << "}" << std::endl;
  output << "" << std::endl;
  output << "ProblemInstance::Activity{" << std::endl;
  output << "	  eq(start, [ 0 +inf ]);" << std::endl;
  output << std::endl;
  output << "	Allocation allocations;" << std::endl;
  output << "	eq( allocations.m_identifier, m_identifier );" << std::endl;
  output << std::endl;
  output << "	if( b == true ) {" << std::endl;
  output << "	  foreach( a in allocations ) {" << std::endl;
  output << "	  equals( a.m_resource.uses u );" << std::endl;
  output << "	  eq( u.quantity, a.m_quantity );" << std::endl;
  output << "	  }" << std::endl;
  output << "	}" << std::endl;
  output << "}\n" << std::endl;

  output.close();

}

void NDDLProGenTranslator::finished( const std::string& file ) 
{
  std::string outputFile = file;
  if( m_UpperBoundSet )
    {
      outputFile.append(".");
      outputFile.append( toString< int >( m_UpperBound ) );
    }

  outputFile.append(".nddl");

  std::ofstream output( outputFile.c_str() );

  output << "/**" << std::endl;
  output << "* Original file '" << file << "'" << std::endl;
  output << "*/" << std::endl;
  output << std::endl;
  
  output << "#include \"" << m_ModelName << "\"\n" << std::endl;
  output << "PlannerConfig c = new PlannerConfig(0, 1000, +inf );\n" << std::endl;
  output << "ProblemInstance problem = new ProblemInstance();\n" << std::endl;
			
  {
    Int2Int::const_iterator ite = m_Capacities.begin();
    Int2Int::const_iterator end = m_Capacities.end();
    
    for( ; ite != end; ++ite )
    {
      int resource = (*ite).first;
      int capacity = (*ite).second;

      int lowerLimit = 0;
      int upperLimit = 0;

      if( capacity > 0 )
	upperLimit = capacity;
      else
	lowerLimit = capacity;

      output << "CapacityResource resource" << resource << " = new CapacityResource( " << std::showpoint << (float) lowerLimit << " , " << std::showpoint << (float) upperLimit << " );" << std::endl;
    }
  }

  output << std::endl;

  {
    Int2IntPairSet::const_iterator ite = m_Allocations.begin();
    Int2IntPairSet::const_iterator end = m_Allocations.end();

    output << "Allocation a0 = new Allocation( resource0 , 0 , 0.0 );" << std::endl;

    int counter = 1;

    for( ; ite != end; ++ite )
      {
	int activity = (*ite).first;

	Int2Int::const_iterator dIte = m_Durations.find( activity );

	if( m_Durations.end() != dIte )
	  {
	    int duration = (*dIte).second;

	    const IntPairSet& intPairSet = (*ite).second;
	    
	    IntPairSet::const_iterator ite2 = intPairSet.begin();
	    IntPairSet::const_iterator end2 = intPairSet.end();
	    
	    for( ; ite2 != end2; ++ite2 )
	      {
		int resource = (*ite2).first;
		int allocation = (*ite2).second;

		if( allocation != 0 )
		  {
		    output << "Allocation a" <<  counter << " = new Allocation( resource" << resource << ", " << activity << ", " << std::showpoint << (float) allocation << " );" << std::endl;

		    ++counter;
		  }

	      }
	  }	  
      }

    output << "Allocation a" <<  counter << " = new Allocation( resource0, " << m_NumberOfActivities + 1 << ", 0.0 );" << std::endl;

  }

  output << std::endl;

  {
    for( int i = 0; i < m_NumberOfActivities + 2; ++i )
      {
	output << "goal( problem.Activity activity" << i << " );" << std::endl;

	Int2Int::const_iterator dIte = m_Durations.find( i );
	if( m_Durations.end() != dIte )
	  {
	    int duration = (*dIte).second;
	    
	    if( 0 != duration )
	      {
		output << "eq( activity" << i << ".duration, " << duration << " );" << std::endl;

	    //output << "temporalDistance( activity" << i << ".start, [ " << duration << " " << duration << " ], activity" << i << ".end );" << std::endl;
	      }
	    else
	      {
		// EUROPA does not handle zero duration very well
		//output << "temporalDistance( activity" << i << ".start, [ 1 1 ], activity" << i << ".end );" << std::endl;
		output << "eq( activity" << i << ".duration, 1 );" << std::endl;
	      }

	  }

	output << "eq( activity" << i << ".m_identifier, " << i << ");" << std::endl;

	if( m_UpperBoundSet )
	  {
	    if( i != m_NumberOfActivities + 1 )
	      output << "leq( activity" << i << ".end, " << m_UpperBound  << ");\n" << std::endl;
	    else
	      output << "leq( activity" << i << ".end, " << m_UpperBound + 1 << ");\n" << std::endl;
	  }

	output << std::endl;

      }
  }

  output << std::endl;

  {
    Int2IntPairSet::const_iterator ite = m_Successors.begin();
    Int2IntPairSet::const_iterator end = m_Successors.end();

    for( ; ite != end; ++ite )
      {
	int activity = (*ite).first;
	const IntPairSet& intPairSet = (*ite).second;
	    
	IntPairSet::const_iterator ite2 = intPairSet.begin();
	IntPairSet::const_iterator end2 = intPairSet.end();
	    
	for( ; ite2 != end2; ++ite2 )
	  {
	    int successor = (*ite2).first;
	    int weight = (*ite2).second;
	    
	    std::string lowerBound = "-inf";
	    std::string upperBound = "+inf";

	    if( weight < 0 )
	      {
		std::ostringstream iss;
		iss << -weight;

		upperBound = iss.str();

		output << "temporalDistance( activity" << successor << ".start, [ " << lowerBound << " " << upperBound << " ], activity" << activity << ".start );" << std::endl;
	      }
	    else
	      {
		std::ostringstream iss;
		iss << weight;

		lowerBound = iss.str();

		output << "temporalDistance( activity" << activity << ".start, [ " << lowerBound << " " << upperBound << " ], activity" << successor << ".start );" << std::endl;
	      }

	  }
      }
  }

  output << std::endl;

  {
    for( int i = 0; i < m_NumberOfActivities + 2; ++i )
      {
	output << "activity" << i << ".b.specify( true );" << std::endl;
      }
  }

  output << std::endl;

  output << "close();" << std::endl;

  output.close();

  std::cout << "Created '" << outputFile << "'" << std::endl;
}
  
void NDDLProGenTranslator::updateAllocation( int node, int resource, int allocation ) 
{
  Int2IntPairSet::iterator ite = m_Allocations.find( node );

  if( ite != m_Allocations.end() )
    {
      IntPairSet& intPairSet = (*ite).second;
      intPairSet.insert( std::make_pair( resource, allocation ) );
    }
  else
    {
      IntPairSet intPairSet;
      intPairSet.insert( std::make_pair( resource, allocation ) );

      m_Allocations[node] = intPairSet;
    }
}

void NDDLProGenTranslator::updateCapacity( int resource, int capacity ) 
{
  m_Capacities[resource] = capacity;
}

void NDDLProGenTranslator::updateDuration( int node, int duration ) 
{
  m_Durations[node] = duration;
}

void NDDLProGenTranslator::updateNumberOfRealActivities( int n ) 
{
  m_NumberOfActivities = n;
}

void NDDLProGenTranslator::updateNumberOfRenewableResources( int p ) 
{
  m_NumberOfResources = p;
}

void NDDLProGenTranslator::updateNumberOfSuccessors( int node, int directSuccessors ) 
{
}

void NDDLProGenTranslator::updateSuccessor( int node, int successor, int weight ) 
{
  Int2IntPairSet::iterator ite = m_Successors.find( node );

  if( ite != m_Successors.end() )
    {
      IntPairSet& intPairSet = (*ite).second;
      intPairSet.insert( std::make_pair( successor, weight ) );
    }
  else
    {
      IntPairSet intPairSet;
      intPairSet.insert( std::make_pair( successor, weight ) );

      m_Successors[node] = intPairSet;
    }
}

