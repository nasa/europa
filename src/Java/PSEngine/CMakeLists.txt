find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
else()
  message(STATUS "Found UseJava: ${java_included}")
endif(NOT java_included)

#set(CMAKE_JAVA_TARGET_OUTPUT_DIR ${)
#set(CMAKE_JAVA_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/generated;${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_JAVA_INCLUDE_PATH})
add_jar(PSEngineGenerated
  SOURCES 
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
  # generated/psengine/SWIGTYPE_p_p_EUROPA__PSConstraint.java
  # generated/psengine/SWIGTYPE_p_p_EUROPA__PSDataType.java
  # generated/psengine/SWIGTYPE_p_p_EUROPA__PSObject.java
  # generated/psengine/SWIGTYPE_p_p_EUROPA__PSObjectType.java
  # generated/psengine/SWIGTYPE_p_p_EUROPA__PSToken.java
  # generated/psengine/SWIGTYPE_p_p_EUROPA__PSTokenType.java
  # generated/psengine/SWIGTYPE_p_p_EUROPA__PSVariable.java
  generated/psengine/StringErrorStream.java
  generated/psengine/EngineConfig.java
  # src/psengine/PSUtil.java 
  # src/psengine/util/LibraryLoader.java 
  # src/psengine/util/SimpleTimer.java 
  )

#set(CMAKE_JAVA_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_JAVA_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/PSEngineGenerated.jar)
set(CMAKE_JAVA_TARGET_OUTPUT_NAME PSEngine.jar)
add_jar(PSEngineJar
  SOURCES
  src/psengine/PSUtil.java 
  src/psengine/util/LibraryLoader.java 
  src/psengine/util/SimpleTimer.java 
  INCLUDE_JARS
  PSEngineGenerated.jar)