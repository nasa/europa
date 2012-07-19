#ifndef _H_PlannerControlIntf
#define _H_PlannerControlIntf

/**
 * @file PlannerControlIntf.hh
 * @author Patrick Daley
 * @brief Defines the planner control interface for controlling the planner
 * from an external application such as a Java client JNI adapter.
 * @date October, 2004
 */

#ifdef __cplusplus
extern "C" {
#endif

namespace EUROPA {

  /**
   * @brief Java Planner Control JNI adapter will call this method to load 
   * and initialize model to the point that it is ready to accept a call 
   * to writeStep().
   * 
   * @param libPath fully qulified path to model library
   * @param initialStatePath fully qulified path to initial transactions file
   * @param destPath fully qulified path to output destination directory
   * @param plannerConfig fully qualified path to a planner config file
   * @param sourcePaths the paths to search for model source files
   * @param numPaths the number of paths in sourcePaths
   */
  int initModel(const char* libPath, const char* initalStatePath, const char* destPath, const char* plannerConfig,
		const char** sourcePaths, const int numPaths);

  /**
   * @brief Method to retrieve the the current running state of the planner
   * @return plannerStatus
   */
  int getStatus(void);

  /**
   * @brief Java Planner Control JNI adapter will call this method to record a
   * specific planner step.
   * 
   * @param step_num step number of step to record
   * @return lastStepCompleted may be different then step_num if plan completes 
   * or terminates early
   */
  int writeStep(int step_num);

  /**
   * @brief Java Planner Control JNI adapter will call this method to record a
   * sequence of one or more planner steps.
   * 
   * @param num_steps number of steps to record
   * @return lastStepCompleted 
   */
  int writeNext(int num_steps);

  /**
   * @brief Java Planner Control JNI adapter will call this method to complete
   * the planner run 
   * 
   * @return lastStepCompleted 
   */
  int completeRun(void);

  /**
   * @brief Java Planner Control JNI adapter will call this method to terminate
   * the planner run and unload the model.
   * 
   * @return 0
   */
  int terminateRun(void);

  /**
   * @brief Java Planner Control JNI adapter will call this method to retrieve the
   * fully qualified path to the model output directory
   * 
   * @return destPath location of output for the planner run
   */
  const char* getOutputLocation(void);


  /**
   * @brief Java Planner Control JNI adapter will call this method to retrieve the
   * number of transaction name strings that can be filtered
   * 
   * @return numTypes number of transaction types
   */
  int  getNumTransactions(void);
  
  /**
   * @brief Java Planner Control JNI adapter will call this method to retrieve the
   * length of the longest transaction name string
   * 
   * @return typeLength length of the longest transaction type
   */
  int  getMaxLengthTransactions(void);
  
  /**
   * @brief Java Planner Control JNI adapter will call this method to retrieve the
   * array of all transaction name strings
   * 
   * @return types[] array of transaction types
   */
  const char** getTransactionNameStrs(void);


  /**
   * @brief Java Planner Control JNI adapter will call this method to get the filter
   * state for all transaction types 
   * 
   * @param states[] array of all filter states
   * @param numType number of filter states in the array
   * @note updated filter states are returned in states[] provided by caller
   */
  void getTransactionFilterStates(int* states, int numType);

  /**
   * @brief Java Planner Control JNI adapter will call this method to set the filter
   * state for all transaction types
   * 
   * @param states[] array of all filter states
   * @param numType number of filter states in the array
   */
  void setTransactionFilterStates(int* states, int numType);

  void enableDebugMsg(const char* file, const char* pattern);
  void disableDebugMsg(const char* file, const char* pattern);

}

#ifdef __cplusplus
}
#endif
#endif

