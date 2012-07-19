#ifndef EUROPA_LOGGER
#define EUROPA_LOGGER

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
using std::exception;
using std::ifstream;
using std::set;
using std::string;

namespace EUROPA {
	namespace Utils {


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

  #define LOGGER_CLASS_INSTANCE() 
  #define LOGGER_CLASS_INSTANCE_IMPL( className, category, level ) {  
  #define LOGGER_CLASS_DEBUG_MSG( className, msg ) 
  #define LOGGER_CLASS_COND_DEBUG_MSG( className, cond, msg )
  #define LOGGER_DEBUG_MSG(category, msg)
  #define LOGGER_COND_DEBUGM_SG(cond, category, msg)
  #define LOGGER_DEBUG_STMT(category, stmt)
  #define LOGGER_COND_DEBUG_STMT(cond, marker, stmt)
#else
   //legacy definitions -- will be deprecated Logger.msg( marker, data )
//    #define debugMsg(marker, data) { std::cout << "Deprecated debugMsg:"  << __FILE__ << " at " << __LINE__  << std::endl; }
//    #define condDebugMsg(cond, marker, data) { std::cout << "Deprecated condDebugMsg:"  << __FILE__ << " at " << __LINE__  << std::endl; }
//    #define debugStmt(marker, stmt) { std::cout << "Deprecated debugStmt:"  << __FILE__ << " at " << __LINE__  << std::endl; }
//    #define condDebugStmt(cond, marker, stmt) { std::cout << "Deprecated condDebugStmt:"  << __FILE__ << " at " << __LINE__  << std::endl; }

  #define LOGGER_CLASS_INSTANCE() 

  #define LOGGER_CLASS_INSTANCE_IMPL( className, category, level )  

  #define LOGGER_CLASS_DEBUG_MSG( className, level, msg ) LOGGER_CLASS_COND_DEBUG_MSG(className, true, level, msg)
  #define LOGGER_CLASS_COND_DEBUG_MSG( className, cond, level, msg ) 	{}
  
  #define LOGGER_DEBUG_MSG( level, msg ) LOGGER_COND_DEBUG_MSG(true, level, msg)
  #define LOGGER_COND_DEBUG_MSG( cond, level, msg )  {}

  #define LOGGER_DEBUG_STMT( stmt ) LOGGER_COND_DEBUG_STMT(true, stmt)
  #define LOGGER_COND_DEBUG_STMT(cond, stmt) {            \
    if( cond ) {                                          \
      stmt ;                                              \
    }                                                     \
  }

#endif //NO_DEBUG_MESSAGE_SUPPORT
#endif //EUROPA_LOGGER
