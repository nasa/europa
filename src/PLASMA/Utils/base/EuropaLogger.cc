#include "EuropaLogger.hh"
#include "LoggerMgr.hh"

namespace EUROPA
{

std::string 
EuropaLogger::loggerLevelToString( LoggerLevel level ) {
	switch( level )
	{
		case ALL:
			return "ALL";	
			break;
		case TRACE:
			return "TRACE";	
			break;
		case DEBUG:
			return "DEBUG";	
			break;
		case INFO:
			return "INFO";	
			break;
		case WARN:
			return "WARN";	
			break;
		case ERROR_LVL:
			return "ERROR";	
			break;
		case FATAL:
			return "FATAL";	
			break;
		case UNDEFINED:
			return "UNDEFINED";	
			break;
		default:
			return "UNDEFINED";	
	}
}	
	
LoggerLevel 
EuropaLogger::stringToLoggerLevel( std::string in ) {
	if( in.compare( "ALL" ) == 0 ) {
		return ALL;
	}
	if( in.compare( "TRACE" ) == 0 ) {
		return TRACE;
	}
	if( in.compare( "DEBUG" ) == 0 ) {
		return DEBUG;
	}
	if( in.compare( "INFO" ) == 0 ) {
		return INFO;
	}
	if( in.compare( "WARN" ) == 0 ) {
		return WARN;
	}
	if( in.compare( "ERROR" ) == 0 ) {
	        return ERROR_LVL;
	}
	if( in.compare( "FATAL" ) == 0 ) {
		return FATAL;
	}
	if( in.compare( "UNDEFINED" ) == 0 ) {
		return UNDEFINED;
	}
	return UNDEFINED;
}				

EuropaLogger::EuropaLogger( LoggerMgr &manager, std::string catName )
	: m_manager( manager ), 
	  m_categoryName( catName ),	  
	  m_defaultLevel( UNDEFINED )
{
}

EuropaLogger::EuropaLogger( LoggerMgr &manager, std::string catName, LoggerLevel defaultLevel )
	: m_manager( manager ), 
	  m_categoryName( catName ),
	  m_defaultLevel( defaultLevel )
{
}

EuropaLogger::EuropaLogger( const EuropaLogger& logger )
	: m_manager( logger.m_manager ),
	  m_categoryName( logger.m_categoryName ),
	  m_defaultLevel( logger.m_defaultLevel )
{
}

EuropaLogger::~EuropaLogger()
{
}

void EuropaLogger::setLevel( LoggerLevel level ) 
{
	m_defaultLevel = level;
}

bool EuropaLogger::levelEnabled( LoggerLevel level ) const
{
	return level >= this->getLevel(); 
}

std::string EuropaLogger::getCategory( void ) const
{
	return m_categoryName;
}

bool EuropaLogger::operator<( const LoggerInterface &right ) const 
{
	return this->getCategory() < right.getCategory();
}

std::ostream &EuropaLogger::log( LoggerLevel level, std::string msg )
{
	if( level <= m_defaultLevel )
	{
		return m_manager.getStream() << msg;	
	}
	return m_manager.getStream() << "";
}

std::ostream &EuropaLogger::alwaysLog( std::string msg )
{
	return m_manager.getStream() << msg;
}

std::ostream &
EuropaLogger::getStream()
{
	return m_manager.getStream();
}


std::ostream &
EuropaLogger::getAppendedStream( std::string file, int line )
{
	return m_manager.getAppendedStream( m_categoryName, file, line );
}


std::ostream & EuropaLogger::operator<<( const char *msg )
{
	return m_manager.getStream() << msg;
}

std::ostream & EuropaLogger::operator<<( const std::string msg )
{
	return m_manager.getStream() << msg;
}

std::ostream &EuropaLogger::operator<<( LoggerLevel level )
{
	if( levelEnabled( level )) 
	{
		return m_manager.getStream();
	}
	else
	{
		return m_manager.getEmptyStream();
	}
}
	
} //namespace EUROPA

