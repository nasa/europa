#include "PLASMAPerformanceConstraint.hh"

// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "ResourceOpenDecisionManager.hh"

#include "PlanDatabaseWriter.hh"
#include "Constraints.hh"
#include "LoadInitModel.hh"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

namespace EUROPA {
  int loadInitModel(const char* libPath, const char* initialStatePath) {

    int retStatus;
    const char* error_msg;
    void* libHandle;
    SchemaId (*fcn_loadSchema)();   //function pointer to NDDL::loadSchema()
    SchemaId schema;
    bool replay = false;
    SamplePlanDatabase *db1;
    DbClientId client;


    //printf("LoadInitModel:dlopen() file %s\n", libPath);
    //printf("Initial Transactions file %s\n", initialStatePath);
    //fflush(stdout);

    //load model library using full path
    try {
      libHandle = dlopen(libPath, RTLD_LAZY);
      if (!libHandle) {
        error_msg = dlerror();
        printf("Error during dlopen() of %s:\n", libPath);
        check_always(!error_msg, error_msg); 
      }
    }
    catch (Error e) {
      printf("Unexpected exception attempting dlopen()\n");
      fflush(stdout);
      e.display();
      throw;
    }

    //save the handle for unloading the library later
    accessLibHandle() = libHandle;

    // initialize constraint factories
    try {
      SamplePlanDatabase::initialize();
    }
    catch (Error e) {
      printf("Unexpected exception initializing constraint factories\n");
      fflush(stdout);
      e.display();
      throw;
    }

    //locate the NDDL 'loadSchema' function in the library and check for errors
    try {
      fcn_loadSchema = (SchemaId (*)())dlsym(libHandle, "loadSchema");
      //printf("Returned from (SchemaId (*)())dlsym(libHandle, loadSchema)\n");
      if (!fcn_loadSchema) {
        error_msg = dlerror();
        printf("dlsym: Error locating NDDL::loadSchema:\n");
        check_always(!error_msg, error_msg); 
      } 
    }
    catch (Error e) {
      printf("Unexpected exception attempting dlsym()\n");
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
      db1 = new SamplePlanDatabase(schema, replay);
    }
    catch (Error e) {
      printf("Unexpected exception creating SamplePlanDatabase \n");
      fflush(stdout);
      e.display();
      throw;
    }
    //save pointer for all functions that access SamplePlanDatabase
    accessSamplePlanDB() = db1;

    printf("SamplePlanDatabase created\n");
    fflush(stdout);

    // Set ResourceOpenDecisionManager
    try {
      check_always(initialStatePath, "Initial transaction file not specified"); 
      DecisionManagerId local_dm = db1->planner->getDecisionManager();
      ResourceOpenDecisionManagerId local_rodm = (new ResourceOpenDecisionManager(local_dm))->getId();
      local_dm->setOpenDecisionManager( local_rodm );

      client = db1->planDatabase->getClient();

      DbClientTransactionPlayer player(client);
      std::ifstream in(initialStatePath);
      player.play(in);
      
      bool prop_status = client->propagate();
      check_always(prop_status, "Loading initial state led to inconsistancy");
    }
    catch (Error e) {
      printf("Unexpected exception while initializing model\n");
      fflush(stdout);
      e.display();
      throw;
    }
    printf("Plan database initialized with initial transactions\n");
    fflush(stdout);

    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    try {
      ObjectId world = client->getObject("world");
      check_error(world.isValid());
      ConstrainedVariableId horizonStart = world->getVariable("world.m_horizonStart");
      check_error(horizonStart.isValid());
      ConstrainedVariableId horizonEnd = world->getVariable("world.m_horizonEnd");
      check_error(horizonEnd.isValid());
      int start = (int) horizonStart->baseDomain().getSingletonValue();
      int end = (int) horizonEnd->baseDomain().getSingletonValue();
      db1->horizon->setHorizon(start, end);
      //create and run planner
      ConstrainedVariableId maxPlannerSteps = world->getVariable("world.m_maxPlannerSteps");
      check_error(maxPlannerSteps.isValid());
      int steps = (int) maxPlannerSteps->baseDomain().getSingletonValue();
      printf("Max number of steps in this plan is %d\n", steps);
      fflush(stdout);
      retStatus = db1->planner->initRun(steps);
    }
    catch (Error e) {
      printf("Unexpected exception initializing planner\n");
      fflush(stdout);
      e.display();
      throw;
    }

    printf("CB Planner initRun() completed. Planner ready to step. Staus is %d\n", retStatus);
    fflush(stdout);

    return retStatus;
  }

  void unloadModel() {

    const char* error_msg;
    void* modelLibHandle = accessLibHandle();

    if (modelLibHandle) {
      if (dlclose(modelLibHandle)) {
        error_msg = dlerror();
        printf("Error during dlclose():\n");
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
