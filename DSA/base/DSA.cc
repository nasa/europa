#ifndef H_DSA
#define H_DSA

#include "JNI.h"
#include "DSA.hh"
#include "Nddl.hh"
#include "Schema.hh"
#include "Debug.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "ConstraintLibrary.hh"
#include "Rule.hh"
#include "NddlDefs.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "ConstraintEngine.hh"
#include "STNTemporalAdvisor.hh"
#include "DefaultPropagator.hh"
#include "TemporalPropagator.hh"
#include "ResourcePropagator.hh"
#include "DbClient.hh"
#include "DbClientTransactionPlayer.hh"
#include "Solver.hh"
#include "Filters.hh"
#include "Error.hh"

#include <dlfcn.h>
#include <fstream>
#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif
  
  JNIEXPORT void JNICALL Java_dsa_JNI_load(JNIEnv * env, jclass, jstring model){
    const char* modelStr = env->GetStringUTFChars(model, NULL);
    EUROPA::DSA::DSA::instance().load(modelStr);
  }
  
  JNIEXPORT void JNICALL Java_dsa_JNI_loadTransactions(JNIEnv * env, jclass, jstring txSource){
    const char* txSourceStr = env->GetStringUTFChars(txSource, NULL);
    EUROPA::DSA::DSA::instance().loadTransactions(txSourceStr);
  }

  JNIEXPORT void JNICALL Java_dsa_JNI_getComponents(JNIEnv *, jclass){
    EUROPA::DSA::DSA::instance().queryGetComponents();
  }

  JNIEXPORT void JNICALL Java_dsa_JNI_solverConfigure(JNIEnv * env, jclass, jstring solverCfg, jint horizonStart, jint horizonEnd){
    const char* solverCfgStr = env->GetStringUTFChars(solverCfg, NULL);
    EUROPA::DSA::DSA::instance().solverConfigure(solverCfgStr, (int) horizonStart, (int) horizonEnd);
  }

  JNIEXPORT void JNICALL Java_dsa_JNI_solverSolve(JNIEnv *, jclass, jint maxSteps, jint maxDepth){
    EUROPA::DSA::DSA::instance().solverSolve((int) maxSteps, (int) maxDepth);
  }

  JNIEXPORT void JNICALL Java_dsa_JNI_solverStep(JNIEnv *, jclass){
    EUROPA::DSA::DSA::instance().solverStep();
  }

  JNIEXPORT void JNICALL Java_dsa_JNI_solverReset(JNIEnv *, jclass){
    EUROPA::DSA::DSA::instance().solverReset();
  }

  JNIEXPORT void JNICALL Java_dsa_JNI_solverClear(JNIEnv *, jclass){
    EUROPA::DSA::DSA::instance().solverClear();
  }


#ifdef __cplusplus
}
#endif


namespace EUROPA {
  namespace DSA{

    const LabelStr LOAD_TRANSACTIONS("LOAD_TRANSACTIONS");

    DSA::DSA(){
      m_libHandle = NULL;
      Error::doThrowExceptions();
    }

    void DSA::load(const char* modelStr){
      unload();
      loadModelLibrary(modelStr);
      init();
    }

    void DSA::loadTransactions(const char* txSource){
      // Obtain the client to play transactions on.
      DbClientId client = m_db->getClient();

      // Construct player
      DbClientTransactionPlayer player(client);

      // Open transaction source and play transactions
      std::ifstream in(txSource);
      checkError(in, "Invalid transaction source '" + std::string(txSource) + "'.");
      player.play(in);
    }

    void DSA::queryGetComponents(){
      checkError(m_db.isValid(), "No good database");

      std::ofstream os("RESPONSE_FILE");

      os << "<COLLECTION>" << std::endl;

      const ObjectSet& objects = m_db->getObjects();
      for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
	ObjectId object = *it;
	if(Schema::instance()->isA(object->getType(), "Timeline"))
	  os << "   <Component key=\"" << object->getKey() << "\" name=\"" << object->getName().toString() << "\"/>" << std::endl;
      }

      os << "</COLLECTION>" << std::endl;
      os.close();
    }


    void DSA::solverConfigure(const char* source, int horizonStart, int horizonEnd){
      std::ifstream in(source);
      checkError(in, "Invalid configuration source '" + std::string(source) + "'.");

      if(m_solver.isId()){
	delete (Solver*) m_solver;
	m_solver = SolverId::noId();
      }

      TiXmlDocument doc(source);
      doc.LoadFile();
      m_solver = (new EUROPA::SOLVERS::Solver(m_db, *(doc.RootElement())))->getId();

      SOLVERS::HorizonFilter::getHorizon() = IntervalDomain(horizonStart, horizonEnd);
    }

    void DSA::solverSolve(int maxSteps, int maxDepth){
      checkError(m_solver.isId(), "Solver has not been allocated.");
      m_solver->solve(maxSteps, maxDepth);
      writeSolverState();
    }

    void DSA::solverStep(){
      checkError(m_solver.isId(), "Solver has not been allocated.");
      m_solver->step();
      writeSolverState();
    }

    void DSA::solverReset(){
      checkError(m_solver.isId(), "Solver has not been allocated.");
      m_solver->reset();
      writeSolverState();
    }

    void DSA::solverClear(){
      checkError(m_solver.isId(), "Solver has not been allocated.");
      m_solver->clear();
      writeSolverState();
    }

    void DSA::loadModelLibrary(const char* modelStr){
      m_libHandle = dlopen(modelStr, RTLD_NOW);

      //locate the NDDL 'loadSchema' function in the library and check for errors
      EUROPA::SchemaId (*fcn_load) ();
      try {
	fcn_load = (EUROPA::SchemaId (*)())dlsym(m_libHandle, "loadSchema");
	if (!fcn_load) {
	  const char* error_msg = dlerror();
	  check_error(!error_msg, error_msg); 
	}
      }
      catch (Error e) {
	e.display();
	return;
      }

      // call the NDDL::loadSchema function
      try {
	(*fcn_load)();
      }
      catch (Error e) {
	e.display();
      }
    }

    void DSA::unload(){
      if(m_solver.isId()){
	delete (Solver*) m_solver;
	m_solver = SolverId::noId();
      }

      if(m_db.isId()){
	// Indicate a mode swith to purging to avoid propagation of deletion and removal
	// messages. Makes for much more efficient deletion
	Entity::purgeStarted();
	delete (RulesEngine*) m_re;
	delete (PlanDatabase*) m_db;
	delete (ConstraintEngine*) m_ce;

	// Return to standard behavior for deletion
	Entity::purgeEnded();

	m_db = PlanDatabaseId::noId();
	m_ce = ConstraintEngineId::noId();
	m_re = RulesEngineId::noId();
      }

      ObjectFactory::purgeAll();
      TokenFactory::purgeAll();
      ConstraintLibrary::purgeAll();
      Rule::purgeAll();
      uninitNDDL();

      if(m_libHandle != NULL){
	dlclose(m_libHandle);
	m_libHandle = NULL;
      }
    }

    void DSA::init(){
      checkError(m_libHandle != NULL, "Should have a model loaded");
      checkError(m_db.isNoId(), "Should not have a database instance");
      initNDDL();

      // Allocate the Constraint Engine
      m_ce = (new ConstraintEngine())->getId();

      // Allocate the plan database
      m_db = (new PlanDatabase(m_ce, Schema::instance()))->getId();

      // Construct propagators - order of introduction determines order of propagation.
      // Note that propagators will subsequently be managed by the constraint engine
      new DefaultPropagator(LabelStr("Default"), m_ce);
      new TemporalPropagator(LabelStr("Temporal"), m_ce);
      new ResourcePropagator(LabelStr("Resource"), m_ce, m_db);

      // Link up the Temporal Advisor in the PlanDatabase so that it can use the temporal
      // network for determining temporal distances between time points.
      PropagatorId temporalPropagator = m_ce->getPropagatorByName(LabelStr("Temporal"));
      m_db->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());

      // Allocate the rules engine to process rules
      m_re = (new RulesEngine(m_db))->getId();
    }

    void DSA::writeSolverState(){
      // Now output the state in the RESPONSE_FILE
      std::ofstream os("RESPONSE_FILE");

      os << "<SOLVER_STATE hasFlaws=\"" << !m_solver->noMoreFlaws() << 
	"\" isTimedOut=\"" << m_solver->isTimedOut() << 
	"\" isExhausted=\"" << m_solver->isExhausted() << 
	"\" stepCount=\"" << m_solver->getStepCount() << 
	"\" depth=\"" << m_solver->getDepth() << "\"/>";

      os.close();
    }
  }
}

#endif
