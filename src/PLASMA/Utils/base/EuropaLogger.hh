#ifndef EUROPA_LOGGER_HH_
#define EUROPA_LOGGER_HH_

#include "LoggerInterface.hh"
#include "LoggerMgr.hh"

namespace EUROPA 
{

	class EuropaLogger : public LoggerInterface
	{
	public:
		EuropaLogger( LoggerMgr &manager, std::string catName );
		EuropaLogger( LoggerMgr &manager, std::string catName, LoggerLevel defaultLevel );
		virtual ~EuropaLogger();

		void setLevel( LoggerLevel level ) ;
		LoggerLevel getLevel( void ) const { return m_defaultLevel; }
		std::string getCategory( void ) const;
		bool levelEnabled( LoggerLevel level ) const;

		bool operator<( const LoggerInterface &right ) const;

		std::string loggerLevelToString( LoggerLevel level );
		LoggerLevel stringToLoggerLevel( std::string in );

		std::ostream &log( LoggerLevel level, std::string msg );
		std::ostream &alwaysLog( std::string msg );

		std::ostream &operator<<( LoggerLevel level );
		std::ostream &operator<<( const char *msg );
		std::ostream &operator<<( const std::string msg );
		std::ostream &getStream();
		std::ostream &getAppendedStream( char *file, char *line );
		
	
	private:
		EuropaLogger( const EuropaLogger &logger );
	    LoggerMgr &m_manager;
		std::string m_categoryName;
		LoggerLevel m_defaultLevel;
	};
} //end namespace EUROPA



//inline std::ostream& operator<<(std::ostream& os, const EuropaLogger& logger) {
//  return logger.print(&os);
//  return(os);
//}
//
#endif /*EUROPA_LOGGER_HH_*/
