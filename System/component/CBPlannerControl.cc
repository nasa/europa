#include "PlannerControlIntf.hh"
#include "PLASMAPerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "PlannerControlAssembly.hh"

// Support for planner
#include "Horizon.hh"
#include "CBPlanner.hh"
#include "DecisionPoint.hh"

#include "PlanDatabaseWriter.hh"
#include "LoadInitModel.hh"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

namespace EUROPA {

  extern "C"

  const char* SAstr = "PlannerControlAssembly not initialized";

  int initModel(const char* libPath, const char* initialState, const char* destPath) {

    int retStatus;

    /*
     * get full library name from model name parameter
     */
    try {
      //enable EUROPA exceptions
      Error::doThrowExceptions();
      retStatus = loadInitModel(libPath, initialState);

      /*
       * now that PPW is initialized set the output destination path 
       */
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      db1->getWriter()->setDest(destPath);
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      retStatus = db1->getPlanner()->getStatus();
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      nextPlannerStep = db1->getPlanner()->writeStep(step_num);
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      nextPlannerStep = db1->getPlanner()->writeNext(num_steps);
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      lastStepCompleted = db1->getPlanner()->completeRun();

      PlanDatabaseWriter::write(db1->getPlanDatabase(), std::cout);
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
      PlannerControlAssembly *db1 = accessAssembly();
      if (db1) {
        if (db1->getWriter()) {
          delete(db1->getWriter());
        }
        if (db1->getPlanner()) {
          delete((CBPlanner*) db1->getPlanner());
        }
        if (db1->getHorizon()) {
          delete((Horizon*) db1->getHorizon());
        }
        db1->terminate();
        delete(db1);
      }
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      dest = db1->getWriter()->getDest();
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      numTypes = db1->getWriter()->getNumTransactions();
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      typeLength = db1->getWriter()->getMaxLengthTransactions();
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      types = db1->getWriter()->getTransactionNameStrs();
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      filterState = db1->getWriter()->getTransactionFilterStates();
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
      PlannerControlAssembly *db1 = accessAssembly();
      check_always(db1, SAstr);
      filterState = new bool[numTypes];
      for (int i = 0; i < numTypes; i++) {
        filterState[i] = (states[i]) ? true : false;
      }
      db1->getWriter()->setTransactionFilterStates(filterState, numTypes);
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
