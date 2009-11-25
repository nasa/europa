
#ifndef LOGGERMGR_HH_
#define LOGGERMGR_HH_

#include <map>
#include <list>
#include <fstream>
#include <iostream>


#include "Id.hh"
#include "LoggerInterface.hh"

namespace EUROPA
{

//	class EmptyStream : public std::basic_ostream< char, std::char_traits<char> >  
//	{
//	private:
//			std::ostream &m_stream;
//	public:
//		EmptyStream() :
//			m_stream( std::cout ),
//			std::basic_ostream< char, std::char_traits<char> >( m_stream )
//		{
//		}
//	};
	
	typedef std::map<std::string, LoggerInterface *> LoggerNameMap;
	typedef std::list<std::string> StringList;
	
	class LoggerMgr
	{
	public:
		static LoggerMgr *instance();
		virtual ~LoggerMgr();

		bool loggerExists( std::string categoryName );
		LoggerInterface *getLogger( std::string categoryName );
		LoggerInterface *getLogger( std::string categoryName, LoggerLevel defaultLevel );

		std::ostream &getStream();
		std::ostream &getEmptyStream();
		std::ostream &getAppendedStream( std::string categoryName, std::string file, int line );
		
	private:
		LoggerMgr();
		LoggerMgr( const LoggerMgr & );
		LoggerInterface *getRootLogger( );
		bool readOldStyleConfigurationFile( std::string filename );
		bool readConfigurationFile( std::string filename );
		LoggerLevel getBestLevel( std::string categoryName );
		void processAppenders( StringList &list );
		void processLoggers( StringList &list );
		void parseLoggerLine( const std::string &string );
		void parseRootLoggerLine( const std::string &string );

		static LoggerMgr *m_instance;
		static LoggerInterface *LOGGER;
		std::ostream &m_stream;
		std::ofstream *m_fstream;
		std::ostream *m_emptyStream;
		LoggerNameMap m_loggerNameMap;
		
		//formatting helpers
		std::ostream &printHeader( std::ostream &stream, std::string catName, std::string file, int line ); 	
	};
	
} //namespace EUROPA

#endif /*LOGGERMGR_HH_*/
