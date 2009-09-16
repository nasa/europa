
#include "Debug.hh"
#include "LoggerMgr.hh"

namespace EUROPA
{


LoggerMgr *LoggerMgr::m_instance = NULL;

LoggerMgr::LoggerMgr()
 	: m_stream( std::cout ),
 	m_fstream( NULL )
{
	m_fstream = new std::ofstream( "EuropaLogger.log", std::ios_base::trunc );
	m_loggerNameMap.clear();
}

LoggerMgr::~LoggerMgr()
{
	m_instance->getRootLogger()->getStream() << "EUROPA::LoggerMgr::~LoggerMgr() - cleaning up" << std::endl;
	if( m_fstream != NULL )
	{
		m_fstream->close();
		delete m_fstream;
	}
	m_instance->getRootLogger()->getStream() << "EUROPA::LoggerMgr::~LoggerMgr() - destroyed LoggerMgr" << std::endl;
}

LoggerMgr * 
LoggerMgr::instance()
{
	if( m_instance == NULL )
	{
		m_instance = new LoggerMgr();	
		m_instance->getRootLogger( );
		m_instance->getRootLogger()->getStream() << "EUROPA::LoggerMgr::instance() - instantiated LoggerMgr" << std::endl;
		m_instance->readOldStyleConfigurationFile( std::string( "Debug.cfg" ));
		m_instance->readConfigurationFile( std::string( "EuropaLogger.cfg" ));
		m_instance->getRootLogger()->getStream() << "EUROPA::LoggerMgr::instance() - finished reading configuration files" << std::endl;
	}	
	return m_instance;
}

LoggerInterface *
LoggerMgr::getRootLogger( )
{
	return getLogger( EUROPA_LOGGER_ROOT_DEFAULT_NAME, EUROPA_LOGGER_ROOT_DEFAULT_LEVEL );
}

bool
LoggerMgr::readOldStyleConfigurationFile( std::string filename )
{
	std::ifstream is( filename.c_str() );
	if( !is.good() )
	{ 
		std::cerr <<  "WARNING: readOldStyleConfigurationFile() - cannot read Debug.cfg" << std::endl;
	}
	std::string input;
  	while (is.good() && !is.eof()) {
    	getline(is, input);
    	if (input.empty()) 
    	{
      		continue;
    	}
    	std::string::size_type i = 0;
    	std::string::size_type len = input.length();
    	while (i < len && isascii(input[i]) && isspace(input[i]))
    	{
      		i++;
    	}
    	if (input[i] == ';' || input[i] == '#' || input[i] == '/') 
    	{
      		continue; // input is a comment
    	}
	    input = input.substr(i); // after white space
	    i = input.find_first_of(";#/");
	    input = input.substr(0, i); // chop off comment
	    i = input.length();
	    while (i > 0 && isascii(input[i - 1]) && isspace(input[i - 1]))
    		--i;
	    if (i <= 0) 
	    {
	    	continue; // should be impossible
	    }
    	input = input.substr(0, i);
    	i = input.find(":");
    	std::string pattern;
    	if (i < input.length() && input[i] == ':') {
      		pattern = input.substr(i + 1);
      		input = input.substr(0, i);
    	}
    	//enableMatchingMsgs(input, pattern);
    	m_instance->getLogger( pattern );
  	}
  	if( ! is.eof() )
  	{
  		std::cerr << "WARNING: problem while reading Debug.cfg file" << std::endl;
  	}
  	return(is.eof());
}

bool
LoggerMgr::readConfigurationFile( std::string filename )
{
	std::ifstream is( filename.c_str() );
	if( !is.good() )
	{ 
		m_instance->getRootLogger()->getStream() <<  "WARNING: readConfigurationFile() - cannot read EuropaLogger.cfg" << std::endl;
	}
    std::string input;
	StringList loggingLines;
	while( is.good() && !is.eof() ) {
    	getline(is, input);
		//skip this line if it's empty, it is a comment, or it does not start with log4j
    	if (input.empty() || input[0] == '#' || input[0] != 'l' )  
    	{
      		continue;
    	}
    	loggingLines.push_back( input );		
	}
	is.close();

	m_instance->getRootLogger()->getStream() << "Relevant logging and appender lines from file:" << filename << std::endl;	
	for( StringList::iterator iter = loggingLines.begin();
		 iter != loggingLines.end();
		 ++iter )
	{
		 m_instance->getRootLogger()->getStream() << "\t" << (*iter) << std::endl;	
	}
	
	processAppenders( loggingLines );

	m_instance->getRootLogger()->getStream() << "Remaining logging lines from file:" << filename << std::endl;	
	for( StringList::iterator iter = loggingLines.begin();
		 iter != loggingLines.end();
		 ++iter )
	{
		m_instance->getRootLogger()->getStream() << "\t" << (*iter) << std::endl;	
	}
	
	processLoggers( loggingLines );

	m_instance->getRootLogger()->getStream() << "Remaining unused lines from file:" << filename << std::endl;	
	for( StringList::iterator iter = loggingLines.begin();
		 iter != loggingLines.end();
		 ++iter )
	{
		m_instance->getRootLogger()->getStream() << "\t" << (*iter) << std::endl;	
	}
	return true;
}

void
LoggerMgr::processAppenders( StringList &loggingLines )
{
	m_instance->getRootLogger()->getStream() << "Processing appenders" << std::endl;	
	for( StringList::iterator iter = loggingLines.begin();  iter != loggingLines.end(); )
	{
		if( (*iter).find( "appender" ) != std::string::npos )
		{
			m_instance->getRootLogger()->getStream() << "\t IGNORED appender line: " << (*iter) << std::endl;	
			iter = loggingLines.erase( iter );	
		}
		else 
		{
			++iter;	
		}
	}
}

void
LoggerMgr::parseLoggerLine( const std::string &line )
{
	*(m_instance->getRootLogger()) << "\t Processing logger line: " << line << std::endl;	
	size_t found = line.find_first_of( "=" );
	std::string category("");
	std::string level(""); 
	if( found != std::string::npos )
	{	
		category = line.substr(0, found);
		level = line.substr(found+1, line.length());
	} 
	else 
	{
		category = line;
	}
	//create a new Logger for this category
	LoggerInterface *logger = new EuropaLogger( *this, category ); //TODO this should use LoggerFactory!!
	if( level.compare("") != 0 ) {
		logger->setLevel( logger->stringToLoggerLevel( level ) );
	} 	
}

void
LoggerMgr::parseRootLoggerLine( const std::string &line )
{
}

void
LoggerMgr::processLoggers( StringList &loggingLines )
{
	m_instance->getRootLogger()->getStream() << "Processing loggers" << std::endl;	
	for( StringList::iterator iter = loggingLines.begin(); iter != loggingLines.end(); )
	{
		if( (*iter).find( "logger" ) != std::string::npos )
		{
			parseLoggerLine( *iter );			
			iter = loggingLines.erase( iter );
		}
		else if( (*iter).find( "rootLogger" ) != std::string::npos )
		{
			parseRootLoggerLine( *iter );			
			iter = loggingLines.erase( iter );
		}
		else 
		{
			++iter;	
		}
	}
}

bool 
LoggerMgr::loggerExists( std::string categoryName )
{
	LoggerNameMap::iterator iter = m_loggerNameMap.find( categoryName );
	if( iter != m_loggerNameMap.end() )
	{
		return true;
	}
	return false;
}

LoggerInterface *
LoggerMgr::getLogger( std::string categoryName )
{
	LoggerNameMap::iterator iter = m_loggerNameMap.find( categoryName );
	if( iter != m_loggerNameMap.end() )
	{
		return (*iter).second;
	}

	LoggerLevel level = getBestLevel( categoryName );
	LoggerInterface *logger = new EuropaLogger( *this, categoryName, level ); 
	m_loggerNameMap.insert( std::make_pair< std::string, LoggerInterface *>( categoryName, logger ));

	return logger;
}

LoggerInterface *
LoggerMgr::getLogger( std::string categoryName, LoggerLevel defaultLevel )
{
	LoggerNameMap::iterator iter = m_loggerNameMap.find( categoryName );
	if( iter != m_loggerNameMap.end() )
	{
		((*iter).second)->setLevel( defaultLevel );
		return (*iter).second;
	}
	LoggerInterface *logger = new EuropaLogger( *this, categoryName, defaultLevel ); 
	m_loggerNameMap.insert( std::make_pair< std::string, LoggerInterface *>( categoryName, logger ));
	return logger;
}

LoggerLevel
LoggerMgr::getBestLevel( std::string categoryName )
{
	//TODO need a more thorough algorithm to determine best default level
	LoggerInterface *root = getRootLogger( );		
	return root->getLevel();
}

std::ostream &
LoggerMgr::getStream()
{
	if( m_fstream != NULL && m_fstream->good() )
	{
		return *m_fstream;
	}
	return m_stream;	
}

std::ostream &
LoggerMgr::getEmptyStream()
{
	return m_stream; //TODO need to return the emtpy stream here
}

std::ostream &
LoggerMgr::getAppendedStream( std::string categoryName, char *file, char *line )
{
	return printHeader( m_stream, categoryName, file, line );		
}

std::ostream &
LoggerMgr::printHeader( std::ostream &stream, std::string catName, char *file, char *line )
{
	stream << file << " " << line << " " << catName << " ";
	return stream;		
} 	


} //namespace EUROPA
