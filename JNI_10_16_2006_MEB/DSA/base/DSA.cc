#include "DSA.hh"
#include "JNI.h"
#include "Nddl.hh"
#include "Schema.hh"
#include "Debug.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "ConstraintLibrary.hh"
#include "Rule.hh"
#include "NddlDefs.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"
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

// JRB
#include "Constraints.hh"
#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_IncrementalFlowProfile.hh" 
#include "ResourceThreatDecisionPoint.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_Resource.hh"

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

    ss << "<SOLVER_STATE"
       << " hasFlaws=\"" << !solver->noMoreFlaws() << "\""
       << " isTimedOut=\"" << solver->isTimedOut() << "\""
       << " isExhausted=\"" << solver->isExhausted() <<  "\""
       << " isConstraintConsistent=\"" << solver->isConstraintConsistent() <<  "\""
       << " stepCount=\"" << solver->getStepCount() << "\""
       << " depth=\"" << solver->getDepth() << "\"" 
       << " lastExecutedDecision=\"" << solver->getLastExecutedDecision() << "\"" 
       << "/>";

    return env->NewStringUTF(ss.str().c_str());
  }

  jstring makeReply(JNIEnv * env, const EUROPA::DSA::ResultSet& rs){
    return env->NewStringUTF(rs.toXML().c_str());
  }

  JNIEXPORT void JNICALL Java_dsa_impl_JNI_load(JNIEnv * env, jclass klass, jstring model){
    const char* modelStr = env->GetStringUTFChars(model, NULL);
    EUROPA::DSA::DSA::instance().load(modelStr);

    // Invoke call back api - just a test for now. Eventually implement a listener on events to do this
    jmethodID method = env->GetStaticMethodID(klass, "handleCallBack", "()V");
    checkError(method != NULL, "No Method found");
    env->CallStaticVoidMethod(klass, method);
  }
  
  JNIEXPORT void JNICALL Java_dsa_impl_JNI_addPlan(JNIEnv * env, jclass, jstring txSource){
    const char* txSourceStr = env->GetStringUTFChars(txSource, NULL);
    EUROPA::DSA::DSA::instance().addPlan(txSourceStr);
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getComponents(JNIEnv * env, jclass){
    return(makeReply(env, EUROPA::DSA::DSA::instance().getComponents()));
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getActions(JNIEnv * env, jclass, jint componentKey){
    return(makeReply(env, EUROPA::DSA::DSA::instance().getActions(componentKey)));
  }

  // TODO: returning a collection to reuse existing code, optimize.
  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getAction(JNIEnv * env, jclass, jint actionKey){
    return(makeReply(env, EUROPA::DSA::DSA::instance().getAction(actionKey)));
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getConditions(JNIEnv * env, jclass, jint actionKey){
    return(0);
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getEffects(JNIEnv * env, jclass, jint actionKey){
    return(0);
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getChildActions(JNIEnv * env, jclass, jint actionKey){
    return(0);
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getResources(JNIEnv * env, jclass){
    return(makeReply(env, EUROPA::DSA::DSA::instance().getResources()));
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getResourceCapacityProfile(JNIEnv * env, jclass, jint resourceKey){
    return(makeReply(env, EUROPA::DSA::DSA::instance().getResourceCapacityProfile(resourceKey)));
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_getResourceUsageProfile(JNIEnv * env, jclass, jint resourceKey){
    return(makeReply(env, EUROPA::DSA::DSA::instance().getResourceUsageProfile(resourceKey)));
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_solverConfigure(JNIEnv * env, jclass, jstring solverCfg, jint horizonStart, jint horizonEnd){
    const char* solverCfgStr = env->GetStringUTFChars(solverCfg, NULL);
    EUROPA::DSA::DSA::instance().solverConfigure(solverCfgStr, (int) horizonStart, (int) horizonEnd);
    return makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver());
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_solverSolve(JNIEnv * env, jclass, jint maxSteps, jint maxDepth){
    EUROPA::DSA::DSA::instance().solverSolve((int) maxSteps, (int) maxDepth);
    return makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver());
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_solverStep(JNIEnv * env, jclass){
    EUROPA::DSA::DSA::instance().solverStep();
    return makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver());
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_solverReset(JNIEnv * env, jclass){
    EUROPA::DSA::DSA::instance().solverReset();
    return makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver());
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_solverClear(JNIEnv * env, jclass){
    EUROPA::DSA::DSA::instance().solverClear();
    return makeSolverState(env, EUROPA::DSA::DSA::instance().getSolver());
  }

  JNIEXPORT jstring JNICALL Java_dsa_impl_JNI_solverGetOpenDecisions(JNIEnv * env, jclass){
    std::string retval = EUROPA::DSA::DSA::instance().getSolver()->printOpenDecisions();
    return env->NewStringUTF(retval.c_str());
  }


#ifdef __cplusplus
}
#endif

namespace EUROPA {
  namespace DSA {


    DSA::DSA(){
      m_libHandle = NULL;

	 REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);
	 REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile);
	 REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreatDecisionPoint);
    }

    const ResultSet& DSA::getComponents()
    {
    	return getObjectsByType("Timeline");    	
    }

    void printAllObjectTypes()
    {
    	std::cout << "All Object Types:" << std::endl;
    	const std::set<LabelStr>& ot = Schema::instance()->getAllObjectTypes();
    	for(std::set<LabelStr>::const_iterator it = ot.begin(); it != ot.end(); ++it) {
    		std::cout << it->toString() << std::endl;
    	}
    }
    
    void printAncestors(const LabelStr& objectType)
    {
    	std::cout << "Ancestors for : " << objectType.toString() << std::endl;
    	const std::vector<LabelStr>& ot = Schema::instance()->getAllObjectTypes(objectType);
    	for(std::vector<LabelStr>::const_iterator it = ot.begin(); it != ot.end(); ++it) {
    		std::cout << it->toString() << std::endl;
    	}
    }
    
    const ResultSet& DSA::getResources()
    {
    	return getObjectsByType("Reusable");
    }

    const ResultSet& DSA::getObjectsByType(const std::string& type) const
    {
      static StringResultSet sl_resultSet;

      //std::cout << "Getting Objects of type : " << type << std::endl; 
      checkError(m_db.isValid(), "No good database");

      std::stringstream ss;

      ss << "<COLLECTION>" << std::endl;

      const ObjectSet& objects = m_db->getObjects();
      for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
	      ObjectId object = *it;
     	  //std::cout << "Object:" << object->getName().toString() << " type:" << object->getType().toString() << std::endl;
	      if(Schema::instance()->isA(object->getType(), type.c_str())) {
	      	  //std::cout << "Object:" << object->getName().toString() << " type:" << object->getType().toString() << std::endl;
	          ss << "   <Resource key=\"" << object->getKey() 
	                        << "\" name=\"" << object->getName().toString() 
	                        << "\"/>" 
	                        << std::endl;
	      }
      }

      ss << "</COLLECTION>" << std::endl;

      sl_resultSet.str() = ss.str();

      return sl_resultSet;
    }
    
    const ResultSet& DSA::getResourceCapacityProfile(int resourceKey)
    {
      static StringResultSet sl_resultSet;
      
      checkError(m_db.isValid(), "No good database");
      EntityId entity = Entity::getEntity(resourceKey);
      if (!SAVH::ResourceId::convertable(entity))
         std::cout << entity->getName().toString() << std::endl;
         
      checkError(entity.isValid() && SAVH::ResourceId::convertable(entity), "No resource for key [" << resourceKey << "]");
      SAVH::ResourceId res = (SAVH::ResourceId) entity;
      
      std::stringstream ss;
      ss << makeCapacityProfile(res);

      sl_resultSet.str() = ss.str();

      return sl_resultSet;
    }

    const ResultSet& DSA::getResourceUsageProfile(int resourceKey)
    {
      static StringResultSet sl_resultSet;
      
      checkError(m_db.isValid(), "No good database");
      EntityId entity = Entity::getEntity(resourceKey);
      checkError(entity.isValid() && SAVH::ResourceId::convertable(entity), "No resource for key [" << resourceKey << "]");
      SAVH::ResourceId res = (SAVH::ResourceId) entity;
      
      std::stringstream ss;
      ss << makeUsageProfile(res);

      sl_resultSet.str() = ss.str();

      return sl_resultSet;
    }

    /*
     *  TODO: hacking it to be a constant range for the entire timeline for now
     */
    const std::string DSA::makeCapacityProfile(const SAVH::ResourceId& res) const
    {
      checkError(m_db.isValid(), "No good database");

      std::stringstream ss;

      double lb = res->getLowerLimit();
      double ub = res->getUpperLimit();
      
      SAVH::ProfileIterator it(res->getProfile());     
      int start = it.getStartTime();
      int end = it.getEndTime();
      
      ss << "<COLLECTION>" << std::endl;
      
	  ss << "   <ProfileEntry " 
	     << "time=\"" << start << "\" " 
	     << "lb=\"" << lb << "\" " 
	     << "ub=\"" << ub << "\"" 
	     << "/>" << std::endl;
	     	
	  ss << "   <ProfileEntry " 
	     << "time=\"" << end << "\" " 
	     << "lb=\"" << lb << "\" " 
	     << "ub=\"" << ub << "\"" 
	     << "/>" << std::endl;
	     	
      ss << "</COLLECTION>" << std::endl;

      return ss.str();
    }


    /*
     *  TODO: this is probably not usage right now, but level. need to fix it
     */
    const std::string DSA::makeUsageProfile(const SAVH::ResourceId& res) const
    {
      checkError(m_db.isValid(), "No good database");

      std::stringstream ss;

      ss << "<COLLECTION>" << std::endl;
      SAVH::ProfileIterator it(res->getProfile());     
      for(; !it.done(); it.next()){
		ss << "   <ProfileEntry " 
		   << "time=\"" << it.getTime() << "\" " 
		   << "lb=\"" << it.getLowerBound() << "\" " 
		   << "ub=\"" << it.getUpperBound() << "\"" 
		   << "/>" << std::endl;	
      }
      ss << "</COLLECTION>" << std::endl;

      return ss.str();
    }


    int asInt(double d)
    {
    	if (d == PLUS_INFINITY)
    	    return INT_MAX;
    	else if (d == MINUS_INFINITY)
    	    return INT_MIN;
    	
    	return (int)d;
    }
    
    const ResultSet& DSA::getActions(int componentKey){
      checkError(m_db.isValid(), "No good database");
      EntityId entity = Entity::getEntity(componentKey);
      checkError(entity.isValid() && ObjectId::convertable(entity), "No component for key [" << componentKey << "]");
      ObjectId component = (ObjectId) entity;
      const TokenSet& tokens = component->getTokens();
      return makeTokenCollection(tokens);
    }


    const ResultSet& DSA::getAction(int actionKey)
    {
      checkError(m_db.isValid(), "No good database");
      EntityId entity = Entity::getEntity(actionKey);
      checkError(entity.isValid() && TokenId::convertable(entity), "No action for key [" << actionKey << "]");
      TokenId action = (TokenId) entity;
      TokenSet tokens;
      tokens.insert(action);
      return makeTokenCollection(tokens);
    }


    const ResultSet& DSA::makeTokenCollection(const TokenSet& tokens) 
    {
      static StringResultSet sl_resultSet;

      checkError(m_db.isValid(), "No good database");

      std::stringstream ss;

      ss << "<COLLECTION>" << std::endl;

      for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
		TokenId token = *it;
		ss << "   <Token key=\"" << token->getKey() << "\" " 
		  "type=\""        << token->getPredicateName().toString() << "\"" <<
		  "startLb=\""     << asInt(token->getStart()->lastDomain().getLowerBound()) << "\"" <<
		  "startUb=\""     << asInt(token->getStart()->lastDomain().getUpperBound()) << "\"" <<
		  "endLb=\""       << asInt(token->getEnd()->lastDomain().getLowerBound()) << "\"" <<
		  "endUb=\""       << asInt(token->getEnd()->lastDomain().getUpperBound()) << "\"" <<
		  "durationLb=\""  << asInt(token->getDuration()->lastDomain().getLowerBound()) << "\"" <<
		  "durationUb=\""  << asInt(token->getDuration()->lastDomain().getUpperBound()) << "\"" << ">" << std::endl;
	
		const std::vector<ConstrainedVariableId>& params = token->getParameters();
		for(std::vector<ConstrainedVariableId>::const_iterator it = params.begin(); it != params.end(); ++it){
		  ConstrainedVariableId var = *it;
		  ss << "<Parameter name=\"" << var->getName().toString() << "\" value=\"" << var->lastDomain().toString() << "\"/>" << std::endl;
		}
	
		ss << "</Token>" << std::endl;
      }

      ss << "</COLLECTION>" << std::endl;

      sl_resultSet.str() = ss.str();

      return sl_resultSet;
    }

    void DSA::load(const char* modelStr){
      unload();

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
      initConstraintLibrary();
      
      // Allocate the Constraint Engine
      m_ce = (new ConstraintEngine())->getId();

      // Allocate the plan database
      m_db = (new PlanDatabase(m_ce, Schema::instance()))->getId();

      // Construct propagators - order of introduction determines order of propagation.
      // Note that propagators will subsequently be managed by the constraint engine
      new DefaultPropagator(LabelStr("Default"), m_ce);
      new TemporalPropagator(LabelStr("Temporal"), m_ce);
      new ResourcePropagator(LabelStr("Resource"), m_ce, m_db);
      new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_ce);


      // Link up the Temporal Advisor in the PlanDatabase so that it can use the temporal
      // network for determining temporal distances between time points.
      PropagatorId temporalPropagator = m_ce->getPropagatorByName(LabelStr("Temporal"));
      m_db->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());

      // Allocate the rules engine to process rules
      m_re = (new RulesEngine(m_db))->getId();
    }


    void DSA::addPlan(const char* txSource){
      checkError(m_libHandle, "Must have a model loaded.");

      if(m_db.isNoId())
	init();

      checkError(m_db.isValid(), "Invalid database instance.");

      DbClientId client = m_db->getClient();

      // Construct player
      DbClientTransactionPlayer player(client);

      // Open transaction source and play transactions
      debugMsg("DSA:addPlan", "Reading initial state from " << txSource);
      std::ifstream in(txSource);
      checkError(in, "Invalid transaction source '" + std::string(txSource) + "'.");
      player.play(in);
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
    }

    void DSA::solverStep(){
      checkError(m_solver.isId(), "Solver has not been allocated.");
      m_solver->step();
    }

    void DSA::solverReset(){
      checkError(m_solver.isId(), "Solver has not been allocated.");
      m_solver->reset();
    }

    void DSA::solverClear(){
      checkError(m_solver.isId(), "Solver has not been allocated.");
      m_solver->clear();
    }
  }
}
