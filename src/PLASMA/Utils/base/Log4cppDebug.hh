#ifndef LOG4CPP_DEBUG
#define LOG4CPP_DEBUG

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
    
#include "log4cpp/Portability.hh"
#include "log4cpp/Category.hh"
#include "log4cpp/CategoryStream.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/BasicLayout.hh"

using std::string;
using std::exception;
using std::ifstream;
using std::set;

namespace EUROPA {
	namespace Utils {

class Logger; //forward decl

class LoggerConfigException : public exception {
    string _msg;
public:
    LoggerConfigException( string msg ) : exception() { _msg = msg; }
    ~LoggerConfigException() throw() {}
    const char* what() const throw() { return _msg.c_str(); }
};

/**
 *  Singleton class to provide a way to read a configuration and
 *  prepare the necessary categories on startup.  This class depends
 *  on the Log4cpp API and requires Log4CPP to be installed on the
 *  developers workstation.  LoggerConfig is intended to be an
 *  internal class for the Logger and should not be included in Europa
 *  code.
 *
 *  <br> As near as I can tell, the "marker" of the previous debug
 *  class is synonomous with the "category" of Log4cpp.
 *
 *  <br> For details on using the this class, see the short tutorial
 *  files labelled TestLog4CPP* in the Utils/test directory.
 *  
 *  @author Mark Roberts, 2009
 */
class LoggerConfig {
public:
    LoggerConfig *getInstance( string configFilename ) {
	if( instance == NULL ) {
	    getInstance();
	}
	instance->readConfiguration( configFilename ); 
	return instance;
    }

    LoggerConfig *getInstance() {
	if( instance == NULL ) {
	    instance = new LoggerConfig(); //no configuration file specified
	    loggers = new set<Logger *>();
	} 
	return instance;
    }
    ~LoggerConfig() {
	if( loggers->size() > 0 ) {
	    //TODO - mcr - iterate through and delete loggers
	}
	delete loggers;
	if( instance != NULL ) {delete instance;}
    }

    void readConfiguration( string configFilename ) { 
	ifstream file;
	file.exceptions( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	file.open( configFilename.c_str() );
	try {
	    //process config file here
	    //while (!file.eof()) file.get();
	} catch (ifstream::failure e) {
	    log4cpp::Category::getRoot().log( log4cpp::Priority::WARN, 
			   "EUROPA::Utils::LoggerConfig",
			      "Exception opening/reading Logger configuration file" );
	    //+ configFilename); // + ":" + e.what());
	} catch( LoggerConfigException e ) {
	    log4cpp::Category::getRoot().log( log4cpp::Priority::WARN, 
			   "EUROPA::Utils::LoggerConfig",
			      "Exception parsing Logger configuration file");
	    //	      + configFilename );// + ":" + e.what());
	}

    }
    void printConfiguration( std::ostream &stream ) {
    	stream << "Not yet implemented" << std::endl;
    }
    void registerLogger( Logger *logger ) {
	loggers->insert( logger );
    }

    
private:
    static LoggerConfig *instance;
    static set<Logger*> *loggers;
    LoggerConfig(  ) {
    }
    //singleton class == private constructors (assuming no inheritance)
    LoggerConfig(LoggerConfig &) { }  //singleton class == private copy constructor
    LoggerConfig &operator=(const LoggerConfig &) { return *instance;} 
};
//LoggerConfig *LoggerConfig::instance = NULL;
//set<Logger*> *LoggerConfig::loggers = NULL;


/**
 *  This class also consolidates the Error.hh and Debug.hh macros.  
 *  
 *  Provides a wrapper class to the Log4cpp API that (hopefully)
 *  unifies the logging for all classes.  Maintenance on this class
 *  will result in changes for all logging throughout Europa.  If, in
 *  the future, a change in the logging is warranted (for example,
 *  away from Log4cpp), this abstraction should pay off.
 *
 */
class Logger {
public:
    // Log4cpp has a very rich structure that we probably don't need:
    //    EMERG < FATAL < ALERT < CRIT < ERROR < WARN < NOTICE < INFO < DEBUG < NOTSET 
    enum Level {
	//EMERG = log4cpp::Priority::EMERG,
	//FATAL = log4cpp::Priority::FATAL,
	//ALERT = log4cpp::Priority::ALERT,
	//CRIT = log4cpp::Priority::CRIT,
	ERROR = log4cpp::Priority::ERROR,
	WARN =  log4cpp::Priority::WARN,
	//NOTICE = log4cpp::Priority::NOTICE,
	INFO =  log4cpp::Priority::INFO,
	DEBUG = log4cpp::Priority::DEBUG,
	NOTSET = log4cpp::Priority::NOTSET,
    };


    Level intToLevel( int value ) {
	return Level( value );
    }

    static Logger &getInstance( string categoryName ) {
	return *(new Logger( categoryName )); 
    }

    static Logger &getInstance( string categoryName, Level level ) {
	Logger *logger = new Logger( categoryName );
	logger->setLevel( level );
	return *logger;
    }

    /** Returns a logger instance that outputs to a specific category
     * with an inherited priority.
     *
     * This is useful for a "per class" instance that writes to a
     * Category with the class name of the logger.
     */
    Logger( string catName ) {
	categoryName = catName;
	currentCategory = &(log4cpp::Category::getInstance( categoryName ));
    }     	  

    ~Logger() { }
//     Logger(Logger &) { }  
//     Logger &operator=(const Logger &) { return *instance;} 


    /** Changes the debug level for the current category.
     */
    void setLevel( Level level ) {
 	currentCategory->setPriority( level );
    }
    /** Returns the debug level for the current category.
     */
    Level getLevel() { 
	return intToLevel( currentCategory->getPriority()); 
    }

    /** Logs a message at the default level for this category.  This
     * message will always be visible, regardless of the level.
     *
     * This method is deprecated and will be removed once all old
     * Europa debug messages are converted to the new class.
     * 
     * @deprecated - use a Logger instance directly
     */
    static void msg( string catName, string msg ) {
	//not yet implemented
    }
		     

    /** Logs a message at the default level for this category.  This
     * message will always be visible, regardless of the level.
     */
    void log( string msg ) {
	uint level = currentCategory->getPriority();
	currentCategory->log( level, msg );
    }

    /** Logs a trace message at the specified level.  This message may
     * or may not show depending on whether the specified level is less
     * than or equal to the current level.
     */
    void log( Level level, string msg ) {
	currentCategory->log( level, msg );
    }

//      /** Breaks encapsulation of the Log4cpp class in case the client
//       *  really, really wants to manipulate Log4cpp structures.
//       *
//       *  For example, one could use the returned category to get the
//       *  root category.  Or one could use the stream passing methods.
//       */
//      log4cpp::Category *getCategory() { 
//   	return currentCategory;
//      }
    

    log4cpp::CategoryStream getStream() {
//	return log4cpp::CategoryStream( *currentCategory, getLevel() );
	return currentCategory->critStream();
    }

     log4cpp::CategoryStream operator<<( Level level ) {
 	return log4cpp::CategoryStream( *currentCategory,  (int) level );
 //	return (*currentCategory) << (int) level;	    
     }

    log4cpp::CategoryStream& eol (log4cpp::CategoryStream& os) {
	return log4cpp::eol( os );
    }                 

//     log4cpp::CategoryStream operator<<( CategoryStream stream ) {
// 	return log4cpp::CategoryStream( *currentCategory, getLevel() );
//     }
//    ostream friend log4cpp::CategoryStream& operator<<( Level level );
    
private:
    log4cpp::Category *currentCategory;
    string categoryName;

}; //class Logger


// ostream& operator<<(ostream& stream, Logger logger) {
//     stream<<  loggerob.x<<' '<<ob.y<<' '<<ob.z<<'\n';
//     return stream;
// }
    


	}//namespace Utils
} // namespace EUROPA



// Pragmas defined in this declaration header also provide the basic functionality to
// make using the logging system as easy as possible.  

#ifdef NO_DEBUG_MESSAGE_SUPPORT
// This pragma allows the compiler to completely eliminate DEBUG
// tracing in optimized code.  Note that if NO_DEBUG_MESSAGE_SUPPORT
// is specified, then absolutely no trace message above DEBUG is
// produced; however trace messages set to INFO or lower are still
// produced. 

  //legacy definitions
  #define debugMsg(marker, data)
  #define condDebugMsg(cond, marker, data)
  #define debugStmt(marker, stmt)
  #define condDebugStmt(cond, marker, stmt)
#else
   //legacy definitions -- will be deprecated Logger.msg( marker, data )
   #define debugMsg(marker, data) { std::cout << "Deprecated debugMsg:"  << __FILE__ << " at " << __LINE__  << std::endl; }
   #define condDebugMsg(cond, marker, data) { std::cout << "Deprecated condDebugMsg:"  << __FILE__ << " at " << __LINE__  << std::endl; }
   #define debugStmt(marker, stmt) { std::cout << "Deprecated debugStmt:"  << __FILE__ << " at " << __LINE__  << std::endl; }
   #define condDebugStmt(cond, marker, stmt) { std::cout << "Deprecated condDebugStmt:"  << __FILE__ << " at " << __LINE__  << std::endl; }
#endif //NO_DEBUG_MESSAGE_SUPPORT
#endif //LOG4CPP_DEBUG
