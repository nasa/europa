#include "PlannerControlIntf.hh"
#include "PLASMAPerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"

#include "PlanDatabaseWriter.hh"
#include "LoadInitModel.hh"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

namespace PLASMA {

  extern "C"

  const char* SPDstr = "SamplePlanDatabase not initialized";

  int initModel(const char* libPath, const char* initialState, const char* destPath) {

    int retStatus;

    /*
     * get full library name from model name parameter
     */
    try {
      //enable PLASMA exceptions
      Error::doThrowExceptions();
      retStatus = loadInitModel(libPath, initialState);

      /*
       * now that PPW is initialized set the output destination path 
       */
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      db1->writer->setDest(destPath);
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:initModel()\n");
      fflush(stdout);
      throw;
    }
    return retStatus;
  }

  int getStatus(void) {

    int retStatus;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      retStatus = db1->planner->getStatus();
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:getStatus()\n");
      fflush(stdout);
      throw;
      e.display();
    }
    return retStatus;
  }

  int writeStep(int step_num) {

    int nextPlannerStep;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      nextPlannerStep = db1->planner->writeStep(step_num);
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:writeStep()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    //returns last step completed
    return nextPlannerStep-1;
  }

  int writeNext(int num_steps) {

    int nextPlannerStep;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      nextPlannerStep = db1->planner->writeNext(num_steps);
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:writeNext()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    //returns last step completed
    return nextPlannerStep-1;
  }

  int completeRun(void) {

    int lastStepCompleted;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      lastStepCompleted = db1->planner->completeRun();

      PlanDatabaseWriter::write(db1->planDatabase, std::cout);
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:completeRun()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    printf("completePlannerRun: Finished \n");
    fflush(stdout);
    return lastStepCompleted;
  }

  int terminateRun(void) {

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      db1->terminate();
      delete(db1);
      //unload dynamic library
      unloadModel();
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:terminateRun()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    printf("terminatePlannerRun: Model unloaded \n");
    fflush(stdout);

    return 0;
  }

  const char* getOutputLocation(void) {

    std::string dest;
    const char* dPath;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      dest = db1->writer->getDest();
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:getOutputLocation()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    dPath = dest.c_str();
    return dPath; 
  }

  int  getNumTransactions(void) {

    int numTypes;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      numTypes = db1->writer->getNumTransactions();
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:getNumTransactions()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    return numTypes;
  }

  int  getMaxLengthTransactions(void) {

    int typeLength;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      typeLength = db1->writer->getMaxLengthTransactions();
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:getMaxLengthTransactions()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    return typeLength;
  }

  const char** getTransactionNameStrs(void) {

    const char** types;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      types = db1->writer->getTransactionNameStrs();
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:getTransactionNameStrs()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    return types;
  }

  void getTransactionFilterStates(int* states, int numTypes) {

    bool* filterState;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      filterState = db1->writer->getTransactionFilterStates();
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:getTransactionFilterStates()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    for (int i = 0; i < numTypes; i++) {
      states[i] = (filterState[i]) ? 1 : 0;
    }
  }

  void setTransactionFilterStates(int* states, int numTypes) {

    bool* filterState;

    try {
      SamplePlanDatabase *db1 = accessSamplePlanDB();
      check_always(db1, SPDstr);
      filterState = new bool[numTypes];
      for (int i = 0; i < numTypes; i++) {
        filterState[i] = (states[i]) ? true : false;
      }
      db1->writer->setTransactionFilterStates(filterState, numTypes);
    }
    catch (Error e) {
      printf("Exception in CBPlannerControl.cc:setTransactionFilterStates()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    delete[] filterState;
  }
}
