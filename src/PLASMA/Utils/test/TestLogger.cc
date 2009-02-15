
#include "TestLogger.hh"

using log4cpp::Category;
using EUROPA::Utils::Logger; 

namespace EUROPA {
    namespace Utils {
	namespace test {


Logger  &LoggerTest::LOGGER = Logger::getInstance( "EUROPA::Utils::test::LoggerTest", Logger::INFO );

int LoggerTest::testLogger()
{
    LOGGER.log( Logger::ERROR, "ERROR message" );
    LOGGER.log( Logger::WARN, "WARN message" );
    LOGGER.log( Logger::DEBUG, "DEBUG message" );
    LOGGER.log( Logger::INFO, "INFO message" );
    
    LOGGER.log( "Message that will always show");
    
    return 0;
}

	} //namespace test
    } //namespace Utils
} //namespace EUROPA
