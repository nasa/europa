
#include "LoggerTest.hh"

// using log4cpp::Category;
// using log4cpp::CategoryStream;
using EUROPA::Utils::Logger; 

namespace EUROPA {
    namespace Utils {
	namespace test {


//using EUROPA::Utils::Logger; 
//namespace System { //TODO mcr
Logger  &LoggerTest::LOGGER = Logger::getInstance( "EUROPA::Utils::test::LoggerTest", Logger::INFO );

LoggerTest::LoggerTest() 
{
}

int LoggerTest::testLogger()
{
    LOGGER.log( Logger::ERROR, "ERROR message test" );
    LOGGER.log( Logger::WARN, "WARN message test" );
    LOGGER.log( Logger::DEBUG, "DEBUG message test" );
    LOGGER.log( Logger::INFO, "INFO message test" );
    
    LOGGER.log( "Message that will always show");

    //    int value = 0;
//     LOGGER << Logger::INFO <<  "test stream output value:" << value << Logger::eol;
    return 0;
}

// ostream &LoggerStreamTest::operator<<( ostream &os, LoggerStreamTest test ) {
//     os << " LST name:" << name << " data:" << data;
//     return os;
// }



	} //namespace test
    } //namespace Utils
} //namespace EUROPA
