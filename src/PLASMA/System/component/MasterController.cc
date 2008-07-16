#include "MasterController.hh"
#include "PlanDatabase.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "ResourcePropagator.hh"
#include "DefaultPropagator.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "PlanDatabaseWriter.hh"
#include "SolverPartialPlanWriter.hh"
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"

// Registration support
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "NddlDefs.hh"
#include "ObjectFactory.hh"
#include "ConstraintFactory.hh"
#include "Constraints.hh"
#include "Rule.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "floatType.hh"
#include "intType.hh"

// Utilities
#include "Debug.hh"
#include "Error.hh"
#include "Utils.hh"
#include "Pdlfcn.hh"
#include <fstream>
#include <sstream>

namespace EUROPA {

  MasterControllerFactory* MasterController::s_factory = NULL;
  MasterController* MasterController::s_instance = NULL;

  /** IMPLEMENTATION FOR C-CALL of JNI INTERFACE **/

  int initModel(const char* libPath, const char* initialStatePath, const char* destPath, 
		const char* plannerConfigPath, const char** sourcePaths, const int numPaths){

    int retStatus;

    std::string msgStr = std::string("initModel:\n") +
      "   libPath = " + libPath + "\n" + 
      "   initialStatePath = " + initialStatePath + "\n" + 
      "   destPath = " + destPath + "\n" + 
      "   plannerConfigPath = " + plannerConfigPath + "\n" + 
      "   sources = " + EUROPA::toString(numPaths);
 
    MasterController::logMsg(msgStr);

    /*
     * get full library name from model name parameter
     */
    try {
      //enable EUROPA exceptions
      Error::doThrowExceptions();

      MasterController *controller = MasterController::instance();

      // Load the model
      controller->loadModel(libPath);

      // Initialize the system
      retStatus = controller->loadInitialState(plannerConfigPath, initialStatePath, destPath, sourcePaths, numPaths);
    }
    catch (Error e) {
      MasterController::logMsg("Exception in MasterController.cc:initModel()");
      throw;
    }

    return retStatus;
  }


  int getStatus(void) {
    debugMsg("JNI:getStatus", "Calling for controller status.");

    int retStatus;
    try {
      retStatus = MasterController::instance()->getStatus();
    }
    catch (Error e) {
      MasterController::logMsg("Exception in MasterController.cc:getgetStatus()");
      throw;
      e.display();
    }

    return retStatus;
  }

  int writeStep(int step_num){
    debugMsg("JNI:writeStep", "Skipping to step " << step_num);

    int retStatus;
    const unsigned int stepNum = (unsigned int) step_num;

    try {
      MasterController* controller = MasterController::instance();
      bool ok = controller->getStatus() == MasterController::IN_PROGRESS &&  controller->getStepCount() < stepNum;
      while(ok){
	controller->next();
	ok = controller->getStatus() == MasterController::IN_PROGRESS &&  controller->getStepCount() < stepNum;
	if(ok)
	  controller->writeStatistics();
      }
      
      // Now output at step_num
      controller->write();
      retStatus = controller->getStatus();
    }
    catch (Error e) {
      MasterController::logMsg("Exception in MasterController.cc:getStatus()");
      throw;
      e.display();
    }

    return retStatus;
  }

  /**
   * Implementation delegates to writeStep, called for each step in num_steps
   */
  int writeNext(int num_steps){
    debugMsg("JNI:writeNext", "Writing next " << num_steps << " steps");

    int currentStepCount = MasterController::instance()->getStepCount();
    const int finalStep =  currentStepCount + num_steps;
    while(currentStepCount++ <  finalStep && 
	  writeStep(currentStepCount) == MasterController::IN_PROGRESS);

    return MasterController::instance()->getStatus();
  }

  int completeRun(void){
    debugMsg("JNI:completeRun", "Completing remaining steps.");

    writeStep(PLUS_INFINITY);

    debugMsg("JNI:completeRun", MasterController::toString(MasterController::instance()->getPlanDatabase()));

    return MasterController::instance()->getStatus();
  }

  int terminateRun(void){
    debugMsg("JNI:terminateRun", "Terminating.");
    MasterController::terminate();
    return 0;
  }

  void enableDebugMsg(const char* file, const char* pattern){
    MasterController::logMsg(std::string("Enable file ") + file + " and pattern " + pattern);
    DebugMessage::enableMatchingMsgs(std::string(file), std::string(pattern));
  }

  void disableDebugMsg(const char* file, const char* pattern){
    MasterController::logMsg(std::string("Disable file ") + file + " and pattern " + pattern);
    DebugMessage::disableMatchingMsgs(std::string(file), std::string(pattern));
  }

  const char* getOutputLocation(void){
    return MasterController::instance()->getDestination().c_str();
  }

  int  getNumTransactions(void){
    checkError(ALWAYS_FAILS, "No longer supported.");
    return 0;
  }

  int  getMaxLengthTransactions(void){
    checkError(ALWAYS_FAILS, "No longer supported.");
    return 0;
  }

  const char** getTransactionNameStrs(void){
    checkError(ALWAYS_FAILS, "No longer supported.");
    return 0;
  }

  void getTransactionFilterStates(int* states, int numType){
    checkError(ALWAYS_FAILS, "No longer supported.");
  }

  void setTransactionFilterStates(int* states, int numType){
    checkError(ALWAYS_FAILS, "No longer supported.");
  }

  /** MASTER CONTROLLER IMPLEMENTATION **/
  MasterController* MasterController::instance(){

    if(s_instance == NULL)
      s_instance = createInstance();

    checkError(s_instance != NULL, "Failed to allocate a MasterController.");

    return s_instance;
  }

  MasterController* MasterController::createInstance() 
  {
      if (s_factory != NULL)
          return s_factory->createInstance(); 
      
      return NULL;
  }      
  
  MasterController::MasterController()
      : m_stepCount(0)
      , m_status(IN_PROGRESS)
      , m_libHandle(NULL)
      , m_debugStream(NULL)
  {
      doStart();
  }

  MasterController::~MasterController()
  {
    if(m_debugStream != NULL){
      m_debugStream->flush(); 
      m_debugStream->close(); 
      delete m_debugStream;
    }

    doShutdown();
    
    // Finally, unload the model
    unloadModel();
  }

  void MasterController::loadModel(const char* libPath){
    logMsg("Opening library");

    try {
      check_error(m_libHandle == NULL, "Model already loaded.");
      m_libHandle = p_dlopen(libPath, RTLD_NOW);
      if (!m_libHandle) {
        check_error_variable(const char* error_msg = p_dlerror());
        check_error(!error_msg, error_msg); 
      }
    }
    catch (Error e) {
      logMsg("Unexpected exception attempting p_dlopen()");
      e.display();
      throw;
    }

    logMsg("Loading function pointer");

    //locate the NDDL 'loadSchema' function in the library and check for errors
    SchemaId (*fcn_loadSchema)(const SchemaId&);   //function pointer to NDDL::loadSchema()
    try {
      fcn_loadSchema = (SchemaId (*)(const SchemaId&))p_dlsym(m_libHandle, "loadSchema");
      if (!fcn_loadSchema) {
        check_error_variable(const char* error_msg = p_dlerror());
        check_error(!error_msg, error_msg); 
      }
    }
    catch (Error e) {
      logMsg("Unexpected exception attempting p_dlsym()");
      e.display();
      throw;
    }

    // call the NDDL::loadSchema function
    logMsg("Calling NDDL:loadSchema");
    try {
        SchemaId schema = ((Schema*)getComponent("Schema"))->getId();
      (*fcn_loadSchema)(schema);
    }
    catch (Error e) {
      logMsg("Unexpected exception in NDDL::loadSchema()");
      e.display();
      throw;
    }
  }

  int MasterController::loadInitialState(const char* configPath, 
					 const char* initialStatePath,
					 const char* destination,
					 const char** sourcePaths, 
					 const int numPaths){
    logMsg("loadInitialState:setting up debug stream");

    std::string debugDest = std::string(destination) + "/DEBUG_FILE";
    m_debugStream = new std::ofstream(debugDest.c_str(), std::ios_base::out);
    if(!m_debugStream->good()) {
      logMsg(std::string("loadInitialState: can't open the debug file ") + debugDest);
      return INITIALLY_INCONSISTENT;
    }

    DebugMessage::setStream(*m_debugStream);

    logMsg("loadInitialState: Allocating components");


    logMsg("loadInitialState: Configuring Partial PlanWriter");
    m_ppw = new SOLVERS::PlanWriter::PartialPlanWriter(getPlanDatabase(), getConstraintEngine(), getRulesEngine());
    m_ppw->setDest(destination);
    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 1;
    SOLVERS::PlanWriter::PartialPlanWriter::writeStep = 1;

    // Obtain the client to play transactions on.
    DbClientId client = getPlanDatabase()->getClient();

    // Construct player
    DbClientTransactionPlayer player(client);

    // Open transaction source and play transactions
    logMsg(std::string("loadInitialState: Reading initial state from ") + initialStatePath);
    std::ifstream in(initialStatePath);
    check_error(in, "Invalid transaction source '" + std::string(initialStatePath) + "'.");
    player.play(in);

    // Provide a hook to handle custom configuration of solvers
    logMsg("loadInitialState: Calling configureSolvers");
    configureSolvers(configPath);

    if(!getConstraintEngine()->propagate()){
      logMsg("loadInitialState: Found to be initially inconsistent");
      m_status = INITIALLY_INCONSISTENT;
    }
    else {
      logMsg("loadInitialState: Successfully loaded");
      m_status = IN_PROGRESS;
    }

    write();

    return m_status;
  }

  void MasterController::unloadModel(){
    logMsg("In MasterController unloadModel");

    if (m_libHandle) {
      logMsg("In MasterController: Closing");

      if (p_dlclose(m_libHandle)) {
        check_error_variable(const char* error_msg = p_dlerror());
        try {
          check_error(!error_msg, error_msg); 
        }
        catch (Error e) {
          logMsg("Unexpected exception in unloadModel()");
          e.display();
          throw;
        }
      }
      m_libHandle = 0;
    }
  }

  void MasterController::terminate(){
    delete s_instance;
    s_instance = NULL;
  }


  MasterController::Status MasterController::getStatus(){
    return m_status;
  }

  const std::string MasterController::getDestination() const { 
    checkError(m_ppw != NULL, "No Partial Plan Writer allocated.");
    return m_ppw->getDest();
  }

  unsigned int MasterController::getStepCount() const {
    return m_stepCount;
  }

  void MasterController::next() {
    checkError(m_status == IN_PROGRESS, "Cannot advance if we are done.");
    m_stepCount++;
    m_ppw->incrementStep();
    m_status = handleNext();
  }

  void MasterController::write(){
    debugMsg("MasterController:write", "Step " << m_stepCount);
    if(!getConstraintEngine()->provenInconsistent())
      m_ppw->write();
  }

  void MasterController::writeStatistics(){
    debugMsg("MasterController:Statistics", "Step " << m_stepCount);
    if(!getConstraintEngine()->provenInconsistent())
      m_ppw->writeStatistics();
  }

  void MasterController::logMsg(std::string msg){
    std::cout << "EUROPA:" << msg << std::endl;
    fflush(stdout);
  }

  std::string MasterController::toString(const PlanDatabaseId& planDatabase){
    std::stringstream ss;
    if(!planDatabase->getConstraintEngine()->provenInconsistent())
      PlanDatabaseWriter::write(planDatabase, ss);
    return ss.str();
  }

  std::string MasterController::extractPath(const char* configPath){
    LabelStr lblStr(configPath);
    int numElements = lblStr.countElements("/");
    LabelStr suffix = lblStr.getElement(numElements-1, "/");

    std::string configStr(configPath);
    int pos = configStr.find(suffix.toString());
    return configStr.substr(0, pos);
  }
}
