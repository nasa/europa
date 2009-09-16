#include "EuropaLogger.hh"

using EUROPA::EuropaLogger;
using EUROPA::LoggerMgr;

/**
 @brief Returns the current level of the given marker.  If no level is provided
        in a configuration file for this marker, this method attempts to emulate
        the tree-based structure of log4cxx.  If the level cannot be determined
        by inheritance, then this method returns the lowest level.
*/
#define debugGetLevel( marker )

/** @brief Use the debugMsg() macro to create a debug message that
  will be printed when the code is executed if and only if this
  particular debug message has been 'enabled'.  
  
  @param marker A string that "marks" the message to enable it by.  
  @param data The data to be printed when the message is enabled.  
  
  @brief The data argument can be any "chain" of C++ expressions to output to the debug stream
  returned by DebugMessage::getStream() when this debug message is
  enabled (via, e.g. DebugMessage::enable() or DebugMessage::enableAll()).  
  
  @see condDebugMsg @see debugStmt 
  @see  condDebugStmt @see DebugMessage
*/
#define debugMsg(marker, data) condDebugMsg( true, marker, data )

/**
 @brief Level-specific version of debugMsg
 @note  Levels are ignored in this version of logging
 */
#define debugMsgLvl(marker, level, data) condDebugMsgLvl( true, marker, level, data ) 

/**
  @brief Create a conditional debug message, which will
         only be created or used when the given condition is true at run time.
  @param cond An additional condition to be checked before printing the message,
              which can be any C/C++ expression that could be used in an if statement.
  @param marker A string that "marks" the message to enable it by.
  @param data The data to be printed when the message is enabled.
  @see debugMsg
  @see condDebugMsg
  @see debugStmt
  @see condDebugStmt
  @see DebugMessage
*/
#define condDebugMsg(cond, marker, data) \
{ \
	if ( cond ) { \
		LoggerMgr *mgr = LoggerMgr::instance(); \
		if( mgr != NULL && mgr->loggerExists( std::string( marker ))) { \
			LoggerInterface *logger = mgr->getLogger( std::string( marker )); \
			if( logger != NULL ) { \
				logger->getStream() << data; \
			} \
		}\
	}\
}		

/**
 @brief Level-specific version of condDebugMsg
 */
#define condDebugMsgLvl(cond, marker, level, data) \
{ \
	if ( cond ) { \
		LoggerMgr *mgr = LoggerMgr.instance(); \
		if( mgr != NULL ) { \
			LoggerInterface *logger = mgr->getLogger( marker ); \
			if( logger != NULL && logger->levelEnabled( level )) { \
				logger->getStream() << data; \
			} \
		}\
	}\
}		

/**
  @brief Add code to be executed only if the DebugMessage is enabled.
  @param marker A string that "marks" the message to enable it by.
  @param stmt The code to be executed when the message is enabled.
  @see debugMsg
  @see condDebugMsg
  @see condDebugStmt
  @see DebugMessage
*/
#define debugStmt(marker, stmt) condDebugStmt( true, marker, stmt )

/**
 @brief Level-specific version of debuStmt
 @see debugStmtLvl
 */
#define debugStmtLvl(marker, level, stmt) condDebugStmtLvl(true, marker, level, stmt)

/**
  @brief Add code to be executed only if the DebugMessage is enabled and
         the condition is true.
  @param cond An additional condition to be checked before printing the message,
         which can be any C/C++ expression that could be used in an if statement.
  @param marker A string that "marks" the message to enable it by.
  @param stmt The code to be executed when the message is enabled and the condition
         is true.
  @see debugMsg
  @see condDebugMsg
  @see debugStmt
  @see DebugMessage
*/
#define condDebugStmt(cond, marker, stmt) \
{ \
	if ( cond ) { \
		LoggerMgr *mgr = LoggerMgr::instance(); \
		if( mgr != NULL && mgr->loggerExists( std::string( marker ))) { \
			if( mgr->hasLogger( marker )) { \
				stmt; \
			} \
		}\
	}\
}		


/**
 @brief Level-specific version of condDebugStmt
 @see condDebugStmt
 */
#define condDebugStmtLvl(cond, marker, level, stmt) \
{ \
	if ( cond ) { \
		LoggerMgr *mgr = LoggerMgr.instance(); \
		if( mgr != NULL ) { \
			if( mgr->hasLogger( marker )) { \
				LoggerInterface *logger = mgr->getLogger( marker ); \
				if( logger != NULL && logger->levelEnabled( level )) { \
					stmt; \
				} \
			} \
		}\
	}\
}		


//#define CHECK_DEBUG_STREAM	LoggerMgr *mgr = LoggerMgr.instance(); check_error( mgr != NULL ); check_error( (mgr->getStream()).good() ); 
#define CHECK_DEBUG_STREAM	
