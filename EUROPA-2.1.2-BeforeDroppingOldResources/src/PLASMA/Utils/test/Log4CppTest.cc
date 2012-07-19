// TestLog4CPP.cpp : A small exerciser for log4cpp
// The following example was adapted by Mark Roberts from
// http://developers.sun.com/solaris/articles/logging.html

//To run, compile and install the log4cpp packages, then
//g++ -DHAVE_CONFIG_H -I. -I log4cpp/include/ -pthread  -O2 -DNDEBUG -Wall -Wno-unused -pedantic  -llog4cpp  TestLog4CPP.cpp


#include <stdio.h>
#include "log4cpp/Portability.hh"
#include"log4cpp/Category.hh"
#include"log4cpp/FileAppender.hh"
#include"log4cpp/BasicLayout.hh"

//TODO - mcr - link this with cppunit testing

int testLog4CPP()
{

    // 1 instantiate an appender object that will append to a log file
    log4cpp::Appender* app = new log4cpp::FileAppender("FileAppender", "testlog4cpp.log");

    // 2. Instantiate a layout object.  
    // Two layouts come already available in log4cpp unless you create your own.
    // BasicLayout includes a time stamp
    log4cpp::Layout* layout = new log4cpp::BasicLayout();

    // 3. attach the layout object to the appender object
    app->setLayout(layout);

    // 4. Instantiate the category object you may extract the root category, but it is
    // usually more practical to directly instance a child category
    log4cpp::Category &main_cat = log4cpp::Category::getInstance("main_cat");

    // 5. Step 1
    // an Appender when added to a category becomes an additional output destination unless
    // Additivity is set to false when it is false, the appender added to the category replaces
    // all previously existing appenders
    main_cat.setAdditivity(false);

    // 5. Step 2
    // this appender becomes the only one
    main_cat.setAppender(app);

    // 6. Set up the priority for the category
    //  EMERG < FATAL < ALERT < CRIT < ERROR < WARN < NOTICE < INFO < DEBUG < NOTSET 
    // attempts to log messages of higher priority than the current priority have no effect
    main_cat.setPriority(log4cpp::Priority::ERROR);

    // Demonstrate all categories
    main_cat.debug("debug"); 
    main_cat.info("info");
    main_cat.notice("notice");
    main_cat.warn("warn");
    main_cat.error("error");
    main_cat.crit("crit");
    main_cat.alert("alert");
    main_cat.fatal("fatal");
    main_cat.emerg("emerg");

    // so we log some examples
    main_cat.debug("This trace is for debugging");
    main_cat.info("This trace prints even if debugging is off");
    main_cat.alert("An unexpected, but not fatal problem was encountered");
    main_cat.fatal("An unexpected and fatal problem was encountered");

    // you can call log with a priority which can provide programmatic control over priority levels
    main_cat.log(log4cpp::Priority::WARN, "A known problem has occured.");
    log4cpp::Priority::PriorityLevel localPriority;
    bool this_is_critical = true;
    if(this_is_critical) {
	localPriority = log4cpp::Priority::CRIT;
    } else {
	localPriority = log4cpp::Priority::DEBUG;
    }

    // this may not be logged
    main_cat.log(localPriority, "Importance depends on context");

    // You may also log by using stream style operations on
    main_cat.critStream() << "This will show up as " << 1 << " critical message" << log4cpp::eol;
    main_cat.emergStream() << "This will show up as " << 1 << " emergency message" << log4cpp::eol;

    // Stream operations are supported when the severity level precedes the message
    main_cat << log4cpp::Priority::ERROR  << "And this will be an error" << log4cpp::eol;

    main_cat.debug("Shutting down");// this will be skipped

    // clean up and flush all appenders
    log4cpp::Category::shutdown();

    return 0;
}
