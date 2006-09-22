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

/**
 * The JNI binding support below should probably be factored into a separate compilation unit if
 * we generalize to handle a variety of integration models.
 */

#ifdef __cplusplus
extern "C" {
#endif

  jstring makeSolverState(JNIEnv * env, const EUROPA::SOLVERS::SolverId& solver){
    std::stringstream ss;

    ss << "<SOLVER_STATE hasFlaws=\"" << !solver->noMoreFlaws() << 
      "\" isTimedOut=\"" << solver->isTimedOut() << 
      "\" isExhausted=\"" << solver->isExhausted() << 
      "\" stepCount=\"" << solver->getStepCount() << 
      "\" depth=\"" << solver->getDepth() << "\"/>";

    return env->NewStringUTF(ss.str().c_str());
  }

  jstring makeReply(JNIEnv * env, const EUROPA::DSA::ResultSet& rs){
    return env->NewStringUTF(rs.toXML().c_str());
  }

  JNIEXPORT void JNICALL Java_dsa_JNI_load(JNIEnv * env, jclass, jstring model){
    const char* modelStr = env->GetStringUTFChars(model, NULL);
    EUROPA::DSA::DSA::instance().load(modelStr);
  }
  
  JNIEXPORT void JNICALL Java_dsa_JNI_addPlan(JNIEnv * env, jclass, jstring txSource){
    const char* txSourceStr = env->GetStringUTFChars(txSource, NULL);
    EUROPA::DSA::DSA::instance().addPlan(txSourceStr);
  }

  JNIEXPORT jstring JNICALL Java_dsa_JNI_getComponents(JNIEnv * env, jclass){
    return(makeReply(env, EUROPA::DSA::DSA::instance().queryGetComponents()));
  }

  JNIEXPORT jstring JNICALL Java_dsa_JNI_solverConfigure(JNIEnv * env, jclass, jstring solverCfg, jint horizonStart, jint horizonEnd){
    const char* solverCfgStr = env->GetStringUTFChars(solverCfg, NULL);
    EUROPA::DSA::DSA::instance().configureSolver(solverCfgStr, (int) horizonStart, (int) horizonEnd);
    return(makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver()));
  }

  JNIEXPORT jstring JNICALL Java_dsa_JNI_solverSolve(JNIEnv * env, jclass klass, jint maxSteps, jint maxDepth){

    EUROPA::DSA::DSA::instance().getSolver()->solve((int) maxSteps, (int) maxDepth);

    // Invoke call back api - just a test for now. Eventually implement a listener on events to do this
    jmethodID method = env->GetStaticMethodID(klass, "handleCallBack", "()V");
    checkError(method != NULL, "No Method found");

    env->CallStaticVoidMethod(klass, method); 

    return(makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver()));
  }

  JNIEXPORT jstring JNICALL Java_dsa_JNI_solverStep(JNIEnv * env, jclass){
    EUROPA::DSA::DSA::instance().getSolver()->step();
    return(makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver()));
  }

  JNIEXPORT jstring JNICALL Java_dsa_JNI_solverReset(JNIEnv * env, jclass){
    EUROPA::DSA::DSA::instance().getSolver()->reset();
    return(makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver()));
  }

  JNIEXPORT jstring JNICALL Java_dsa_JNI_solverClear(JNIEnv * env, jclass){
    EUROPA::DSA::DSA::instance().getSolver()->clear();
    return(makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver()));
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

    void DSA::addPlan(const char* txSource){
      // Obtain the client to play transactions on.
      DbClientId client = m_db->getClient();

      // Construct player
      DbClientTransactionPlayer player(client);

      // Open transaction source and play transactions
      std::ifstream in(txSource);
      checkError(in, "Invalid transaction source '" + std::string(txSource) + "'.");
      player.play(in);
    }

    const ResultSet& DSA::queryGetComponents(){
      static StringResultSet sl_resultSet;

      checkError(m_db.isValid(), "No good database");

      std::stringstream ss;

      ss << "<COLLECTION>" << std::endl;

      const ObjectSet& objects = m_db->getObjects();
      for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
	ObjectId object = *it;
	if(Schema::instance()->isA(object->getType(), "Timeline"))
	  ss << "   <Component key=\"" << object->getKey() << "\" name=\"" << object->getName().toString() << "\"/>" << std::endl;
      }

      ss << "</COLLECTION>" << std::endl;

      sl_resultSet.str() = ss.str();

      return sl_resultSet;
    }

    void DSA::configureSolver(const char* source, int horizonStart, int horizonEnd){
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
  }
}

#endif
