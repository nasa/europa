//#include "PLASMA/Utils/base/Log4cppDebug.hh" //why doesn't this work!!!

#include <ostream>

#include "Logger.hh" 

using namespace std;

namespace EUROPA {
    namespace Utils {
	namespace test {


class LoggerTest {
public:
    static Logger &LOGGER;
    LoggerTest();
    static int testLogger();
};

// class LoggerStreamTest {
// public:
    
//     friend ostream &operator<<(ostream &os, LoggerStreamTest test );
// private:
//     string name;
//     int data;
// };

	} //namespace test
    } //namespace Utils
} //namespace EUROPA
