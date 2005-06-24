#include "PLASMAPerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "PlannerControlAssembly.hh"

// Support for planner
//not needed
//#include "CBPlanner.hh"
//#include "DecisionPoint.hh"
//#include "ResourceOpenDecisionManager.hh"

#include "PlanDatabaseWriter.hh"
#include "Constraints.hh"
#include "LoadInitModel.hh"
#include "Pdlfcn.hh"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace EUROPA {
  int loadInitModel(const char* libPath, const char* initialStatePath, const char* plannerConfigPath) {

    int retStatus;
    const char* error_msg;
    void* libHandle;
    SchemaId (*fcn_loadSchema)();   //function pointer to NDDL::loadSchema()
    SchemaId schema;
    PlannerControlAssembly *db1;
    DbClientId client;


    //printf("LoadInitModel:p_dlopen() file %s\n", libPath);
    //printf("Initial Transactions file %s\n", initialStatePath);
    //fflush(stdout);

    //load model library using full path
    try {
      libHandle = p_dlopen(libPath, RTLD_NOW);
      if (!libHandle) {
        error_msg = p_dlerror();
        printf("Error during p_dlopen() of %s:\n", libPath);
        check_always(!error_msg, error_msg); 
      }
    }
    catch (Error e) {
      printf("Unexpected exception attempting p_dlopen()\n");
      fflush(stdout);
      e.display();
      throw;
    }

    //save the handle for unloading the library later
    accessLibHandle() = libHandle;

    // initialize constraint factories
    try {
      PlannerControlAssembly::initialize();
    }
    catch (Error e) {
      printf("Unexpected exception initializing constraint factories\n");
      fflush(stdout);
      e.display();
      throw;
    }

    //locate the NDDL 'loadSchema' function in the library and check for errors
    try {
      fcn_loadSchema = (SchemaId (*)())p_dlsym(libHandle, "loadSchema");
      //printf("Returned from (SchemaId (*)())p_dlsym(libHandle, loadSchema)\n");
      if (!fcn_loadSchema) {
        error_msg = p_dlerror();
        printf("p_dlsym: Error locating NDDL::loadSchema:\n");
        check_always(!error_msg, error_msg); 
      } 
    }
    catch (Error e) {
      printf("Unexpected exception attempting p_dlsym()\n");
      fflush(stdout);
      e.display();
      throw;
    }

    // call the NDDL::loadSchema function
    try {
      schema = (*fcn_loadSchema)();
    }
    catch (Error e) {
      printf("Unexpected exception in NDDL::loadSchema()\n");
      fflush(stdout);
      e.display();
      throw;
    }
    //printf("Returned from calling the NDDL::loadSchema function\n");
    //fflush(stdout);

    try {
      db1 = new PlannerControlAssembly(schema);
    }
    catch (Error e) {
      printf("Unexpected exception creating PlannerControlAssembly \n");
      fflush(stdout);
      e.display();
      throw;
    }
    //save pointer for all functions that access the assembly
    accessAssembly() = db1;

    printf("PlannerControlAssembly created\n");
    fflush(stdout);

    // Initialize plan given the transactions in initialStatePath.
    try {
      retStatus = db1->initPlan(initialStatePath, plannerConfigPath);
    }
    catch (Error e) {
      printf("Unexpected exception initializing planner\n");
      fflush(stdout);
      e.display();
      throw;
    }

    printf("initPlan() completed. Planner ready to step. Staus is %d\n", retStatus);
    fflush(stdout);

    return retStatus;
  }

  void unloadModel() {

    const char* error_msg;
    void* modelLibHandle = accessLibHandle();

    if (modelLibHandle) {
      if (p_dlclose(modelLibHandle)) {
        error_msg = p_dlerror();
        printf("Error during p_dlclose():\n");
        try {
          check_always(!error_msg, error_msg); 
        }
        catch (Error e) {
          printf("Unexpected exception in unloadModel()\n");
          fflush(stdout);
          e.display();
          throw;
        }
      }
      accessLibHandle() = 0;
    }
  }
}
