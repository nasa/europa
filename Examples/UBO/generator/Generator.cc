#include "NDDLProGenTranslator.hh"
#include <iostream>
#include <string>
#include <sstream>

void usage( const std::string& name )
{
  std::cerr << "Generator for creating NDDL models and initial state from ProGen/max RCPSP input format\n" << std::endl;
  std::cerr << "Usage: " << name << " [option]" << std::endl;
  std::cerr << "\t --help                                        : display this help and exit" << std::endl;
  std::cerr << "\t --model                                       : create the nddl model file for the ProGen/max RCPSP input format" << std::endl;
  std::cerr << "\t --problem  [FILE] [OUTPUT-BASE-NAME] <value>  : create the nddl initial state for the problem instance in ProGen/max RCPSP input format (output file is [FILE].<value>.nddl)" << std::endl;
  std::cerr << "\t                                                 if value is provided it limits the upperbound of activities to integer 'value'" << std::endl;
}

template<class Type> Type toValue( const std::string& st )
{
  std::istringstream sst( st );
  
  Type value;
  
  if( !( sst >> value ) )
    {
      std::cerr<< "Can not parse string '" << st << "' to requested type." << std::endl;
      
      abort();
    }
  
  return value;
}

int main( int argc, char *argv[] )
{
  NDDLProGenTranslator translator;

  std::string name = argv[0];

  if( argc < 2 || (argc > 1 && strcmp( "--help", argv[1] ) == 0 ))
    {
      usage( name );
    }
  else if( strcmp( argv[1],"--model") == 0 )
    {
      translator.generateModel();

      std::cout << "Generated model file '" << translator.getModelFileName() << "'" << std::endl;
    }
  else if( strcmp( argv[1],"--problem") == 0 )
    {
      if( argc < 4 )
	{
	  usage( name );
	}
      else
	{
	  if( argc > 4 )
	    {
	      translator.setUpperBound( toValue<int>( argv[4] ) );
	    } 
	      
	  translator.parse( argv[2], argv[3] );
	}
    }
  else
    {
      usage( name );
    }

  return 0;
}
