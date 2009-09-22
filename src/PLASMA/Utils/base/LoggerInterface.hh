#ifndef LOGGER_INTERFACE_H_
#define LOGGER_INTERFACE_H_

#include <iostream>

namespace EUROPA
{

	enum EuropaDebugInt { //copied from log4cxx
		OFF_INT = 1000000,
		FATAL_INT = 50000,
		ERROR_INT = 40000,
		WARN_INT = 30000,
		INFO_INT = 20000,
		DEBUG_INT = 10000,
		TRACE_INT = 5000,
		ALL_INT = 0
	};

	/**
 	* Provides an abstraction layer for levels so that other classes can implement
 	* their own level ints (for example, log4cxx or log4cpp).
 	*/
	enum LoggerLevel {
		ALL = ALL_INT,
		TRACE = TRACE_INT,
		DEBUG = DEBUG_INT,
		INFO = INFO_INT,
		WARN = WARN_INT,
		ERROR_LVL = ERROR_INT,  //ERROR is a windows macro, so we added "_LVL" to work around this problem
		FATAL = FATAL_INT,
		UNDEFINED = FATAL_INT + 10000
	};


	#define EUROPA_LOGGER_ROOT_DEFAULT_NAME "rootLogger"
	#define EUROPA_LOGGER_ROOT_DEFAULT_LEVEL DEBUG

	class LoggerInterface
	{
	public:
	    virtual ~LoggerInterface() {}

		virtual void setLevel( LoggerLevel level ) = 0;
		virtual LoggerLevel getLevel( void ) const = 0;
		virtual bool levelEnabled( LoggerLevel level ) const = 0;
		virtual std::string getCategory( void ) const = 0;

		virtual bool operator<( const LoggerInterface &right ) const = 0;

		//helper methods
		virtual std::string loggerLevelToString( LoggerLevel level ) = 0;
		virtual LoggerLevel stringToLoggerLevel( std::string in ) = 0;

		//for function-based logging
		virtual std::ostream &log( LoggerLevel level, std::string msg ) = 0;
		virtual std::ostream &alwaysLog( std::string msg ) = 0;

		//for stream-based logging
		virtual std::ostream &getStream() = 0;
		virtual std::ostream &getAppendedStream( char *file, char *line ) = 0;
		//virtual std::ostream &operator<<( LoggerLevel level ) = 0;
		virtual std::ostream &operator<<( const std::string msg ) = 0;
		virtual std::ostream &operator<<( const char *data ) = 0;
	};

}//namespace EUROPA::Utils::Logging

#endif /*LOGGER_INTERFACE_H_*/
