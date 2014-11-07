find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)

#set(CMAKE_JAVA_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR})
#list(APPEND CMAKE_JAVA_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/PSEngineGenerated.jar)
add_jar(PSEngineJar
  SOURCES
  src/psengine/PSUtil.java 
  src/psengine/util/LibraryLoader.java 
  src/psengine/util/SimpleTimer.java 
  ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/EngineConfig.java
  ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSConstraint.java
  ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSConstraintEngineListener.java
  ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSConstraintList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSDataType.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSDataTypeList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSEngine.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSEngineInterface.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSEngineInterfaceJNI.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSEntity.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSException.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSIntList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSLanguageException.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSLanguageExceptionList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSObject.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSObjectList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSObjectType.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSObjectTypeList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSPlanDatabaseClient.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSPlanDatabaseListener.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSResource.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSResourceProfile.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSSchema.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSSolver.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSStringList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSTimePointList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSToken.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSTokenList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSTokenState.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSTokenType.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSTokenTypeList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSValueList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSVarType.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSVarValue.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSVariable.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/PSVariableList.java
   ${CMAKE_BINARY_DIR}/src/PLASMA/System/psengine/StringErrorStream.java
  # INCLUDE_JARS
  # PSEngineGenerated.jar
  OUTPUT_NAME
  PSEngine)

add_dependencies(PSEngineJar PSEngineGenerated)
install_jar(PSEngineJar ${EUROPA_ROOT}/dist/europa)