find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)

# add_jar(PSEngineGenerated
#   SOURCES 
#   generated/psengine/PSConstraint.java
#   generated/psengine/PSConstraintEngineListener.java
#   generated/psengine/PSConstraintList.java
#   generated/psengine/PSDataType.java
#   generated/psengine/PSDataTypeList.java
#   generated/psengine/PSEngine.java
#   generated/psengine/PSEngineInterface.java
#   generated/psengine/PSEngineInterfaceJNI.java
#   generated/psengine/PSEntity.java
#   generated/psengine/PSException.java
#   generated/psengine/PSIntList.java
#   generated/psengine/PSLanguageException.java
#   generated/psengine/PSLanguageExceptionList.java
#   generated/psengine/PSObject.java
#   generated/psengine/PSObjectList.java
#   generated/psengine/PSObjectType.java
#   generated/psengine/PSObjectTypeList.java
#   generated/psengine/PSPlanDatabaseClient.java
#   generated/psengine/PSPlanDatabaseListener.java
#   generated/psengine/PSResource.java
#   generated/psengine/PSResourceProfile.java
#   generated/psengine/PSSchema.java
#   generated/psengine/PSSolver.java
#   generated/psengine/PSStringList.java
#   generated/psengine/PSTimePointList.java
#   generated/psengine/PSToken.java
#   generated/psengine/PSTokenList.java
#   generated/psengine/PSTokenState.java
#   generated/psengine/PSTokenType.java
#   generated/psengine/PSTokenTypeList.java
#   generated/psengine/PSValueList.java
#   generated/psengine/PSVarType.java
#   generated/psengine/PSVarValue.java
#   generated/psengine/PSVariable.java
#   generated/psengine/PSVariableList.java
#   generated/psengine/StringErrorStream.java
#   generated/psengine/EngineConfig.java
#   )

#set(CMAKE_JAVA_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR})
#list(APPEND CMAKE_JAVA_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/PSEngineGenerated.jar)
add_jar(PSEngineJar
  SOURCES
  src/psengine/PSUtil.java 
  src/psengine/util/LibraryLoader.java 
  src/psengine/util/SimpleTimer.java 
  generated/psengine/EngineConfig.java
  generated/psengine/PSConstraint.java
  generated/psengine/PSConstraintEngineListener.java
  generated/psengine/PSConstraintList.java
  generated/psengine/PSDataType.java
  generated/psengine/PSDataTypeList.java
  generated/psengine/PSEngine.java
  generated/psengine/PSEngineInterface.java
  generated/psengine/PSEngineInterfaceJNI.java
  generated/psengine/PSEntity.java
  generated/psengine/PSException.java
  generated/psengine/PSIntList.java
  generated/psengine/PSLanguageException.java
  generated/psengine/PSLanguageExceptionList.java
  generated/psengine/PSObject.java
  generated/psengine/PSObjectList.java
  generated/psengine/PSObjectType.java
  generated/psengine/PSObjectTypeList.java
  generated/psengine/PSPlanDatabaseClient.java
  generated/psengine/PSPlanDatabaseListener.java
  generated/psengine/PSResource.java
  generated/psengine/PSResourceProfile.java
  generated/psengine/PSSchema.java
  generated/psengine/PSSolver.java
  generated/psengine/PSStringList.java
  generated/psengine/PSTimePointList.java
  generated/psengine/PSToken.java
  generated/psengine/PSTokenList.java
  generated/psengine/PSTokenState.java
  generated/psengine/PSTokenType.java
  generated/psengine/PSTokenTypeList.java
  generated/psengine/PSValueList.java
  generated/psengine/PSVarType.java
  generated/psengine/PSVarValue.java
  generated/psengine/PSVariable.java
  generated/psengine/PSVariableList.java
  generated/psengine/StringErrorStream.java
  # INCLUDE_JARS
  # PSEngineGenerated.jar
  OUTPUT_NAME
  PSEngine)
add_dependencies(PSEngineJar PSEngineGenerated)
install_jar(PSEngineJar ${EUROPA_ROOT}/dist/europa)