#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "LockManager.hh"

int main(int argc, const char ** argv) {
  LockManager::connect();
  LockManager::lock();

  const char* plannerLibPath;
  const char* initialStatePath;
  const char* modelLibPath;
  const char* destPath;
  char* error_msg;
  char* status_msg;
  void* libHandle;
  int retStatus;
  int lastStepCompleted;
  int (*fcn_initModel)(const char*, const char*, const char*);
  int (*fcn_completeRun)();
  int (*fcn_terminateRun)();
  int (*fcn_getStatus)();

  switch (argc) {
    case 1:
    case 2:
    case 3:
    case 4:
      printf("Usage: runPlannerControlTest planner model transactions destination\n\nwhere:\n");
      printf("   planner        Planner library name\n");
      printf("   model          Model library path\n");
      printf("   transactions   Initial state XML file path\n");
      printf("   destination    Planner output destination path\n");
      exit(1);
    case 5:
      plannerLibPath = argv[1];
      modelLibPath = argv[2];
      initialStatePath = argv[3];
      destPath = argv[4];
      break;
    default:
      printf("Too many arguments\n");
      exit(1);
  }

  // load the CBPlannerControl shared library
  printf("runPlannerControlTest:Loading planner shared library file: %s\n", plannerLibPath);
  fflush(stdout);

  //load model library using full path
  libHandle = dlopen(plannerLibPath, RTLD_LAZY);

  if (!libHandle) {
    error_msg = dlerror();
    printf("Error during dlopen() of %s:", plannerLibPath);
    if (error_msg) {
      printf("%s:\n", error_msg);
    } else {
      printf("\n");
    }
    exit(1);
  }

  //locate the initModel function
  fcn_initModel = (int (*)(const char*, const char*, const char*))dlsym(libHandle, "initModel");
  if (!fcn_initModel) {
    error_msg = dlerror();
    printf("dlsym: Error locating EUROPA::initModel:");
    if (error_msg) {
      printf("%s:\n", error_msg);
    } else {
      printf("\n");
    }
    exit(1);
  } 

  // call the EUROPA::initModel function
  try {
    retStatus = (*fcn_initModel)(modelLibPath, initialStatePath, destPath);
  } 
  catch (...) {
    printf("runPlannerControlTest.cc:Unexpected exception in initModel function\n");
    exit(1);
  }
  if (retStatus) {
    printf("Planner initialization failed with return status %d:\n", retStatus);
    exit(1);
  }
    
  //locate the completeRun function
  fcn_completeRun = (int (*)())dlsym(libHandle, "completeRun");
  if (!fcn_completeRun) {
    error_msg = dlerror();
    printf("dlsym: Error locating EUROPA::completeRun:");
    if (error_msg) {
      printf("%s:\n", error_msg);
    } else {
      printf("\n");
    }
    exit(1);
  } 

  // call the completeRun function
  try {
    lastStepCompleted  = (*fcn_completeRun)();
  } 
  catch (...) {
    printf("runPlannerControlTest.cc:Unexpected exception in completeRun function\n");
    exit(1);
  }
  printf("Model ran for %d steps\n", lastStepCompleted);

  //locate the 'getStatus' function in the library
  fcn_getStatus = (int (*)())dlsym(libHandle, "getStatus");
  if (!fcn_getStatus) {
    error_msg = dlerror();
    printf("dlsym: Error locating EUROPA::completeRun:");
    if (error_msg) {
      printf("%s:\n", error_msg);
    } else {
      printf("\n");
    }
    exit(1);
  } 

  // call the getStatus function
  try {
    retStatus  = (*fcn_getStatus)();
  } 
  catch (...) {
    printf("runPlannerControlTest.cc:Unexpected exception in getStatus function\n");
    exit(1);
  }

  if (retStatus == 0) {
     status_msg = "In Progress";
  } else if (retStatus == 1) {
     status_msg = "Timeout Reached";
  } else if (retStatus == 2) {
     status_msg = "Found Plan";
  } else if (retStatus == 3) {
     status_msg = "Search Exhausted";
  } else if (retStatus == 4) {
     status_msg = "Initially Inconsistant";
  } else {
     status_msg = "Unknown";
  }

  printf("runPlannerControlTest: CB Planner run completed. Planner Staus: %s.\n", status_msg);
  fflush(stdout);

  //locate the 'terminateRun' function in the library
  fcn_terminateRun = (int (*)())dlsym(libHandle, "terminateRun");
  //printf("Returned from (int (*)())dlsym(libHandle, terminateRun)\n");
  if (!fcn_terminateRun) {
    error_msg = dlerror();
    printf("dlsym: Error locating EUROPA::terminateRun:");
    if (error_msg) {
      printf("%s:\n", error_msg);
    } else {
      printf("\n");
    }
    exit(1);
  } 

  // call the terminateRun function
  try {
    retStatus  = (*fcn_terminateRun)();
  } 
  catch (...) {
    printf("runPlannerControlTest.cc:Unexpected exception in getStatus function\n");
    exit(1);
  }
  printf("Model Library Unloaded\n");
  fflush(stdout);
  exit(0);
}

