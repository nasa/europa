include(CheckIncludeFileCXX)

include(EuropaModule)
set(internal_dependencies TinyXml)
set(root_sources CommonDefs.cc)
set(base_sources Debug.cc Engine.cc Entity.cc Error.cc EuropaLogger.cc Factory.cc IdTable.cc LabelStr.cc LoggerMgr.cc Mutex.cc Pdlfcn.cc Utils.cc XMLUtils.cc)
set(component_sources "")
#Log4CppTest.cc Log4cxxTest.cc LoggerTest.cc TestLogger.cc
set(test_sources TestData.cc module-tests.cc util-test-module.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(Utils "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")

file(GLOB test_inputs test/*.xml test/*.dtd test/*.cfg)
file(COPY ${test_inputs} DESTINATION .)

#how do I handle the dlopen and pthreads link requirements?  do I need to?
find_library(libdl_library dl libdl ltdl libltdl)
target_link_libraries("Utils${EUROPA_SUFFIX}" ${libdl_library})



