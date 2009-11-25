/* This file is included simply to force a compilation test of the
 * straight Log4cxx library and headers.  No code from this file is
 * run in any test.  If you get a failure of this file, then it means
 * your environment does not contain the proper setup for Log4cxx (a
 * required library for Europa).
 *
 */



/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <log4cxx/logstring.h>
#include <stdlib.h>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/ndc.h>
#include <locale.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

int runTest()
{
//     setlocale(LC_ALL, "");
  int result = EXIT_SUCCESS;
  try
    {
      BasicConfigurator::configure();
      LoggerPtr rootLogger = Logger::getRootLogger();
      
      NDC::push("trivial context");
      LOG4CXX_DEBUG(rootLogger, "debug message");
      LOG4CXX_INFO(rootLogger, "info message");
      LOG4CXX_WARN(rootLogger, "warn message");
      LOG4CXX_ERROR(rootLogger, "error message");
      LOG4CXX_FATAL(rootLogger, "fatal message");
      
      NDC::pop();
    }
  catch(std::exception&)
    {
      result = EXIT_FAILURE;
    }
  
  return result;
}
