#include "MasterController.hh"
#include "PlanDatabase.hh"
#include "ConstraintEngine.hh"
#include "Rule.hh"
#include "RulesEngine.hh"
#include "PlanDatabaseWriter.hh"
#include "SolverPartialPlanWriter.hh"
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"


// Utilities
//#include "Debug.hh"
#include "Error.hh"
#include "Utils.hh"
#include "Pdlfcn.hh"
#include <fstream>
#include <sstream>

namespace EUROPA {
//namespace System { //TODO mcr - a note for coming back through with namespaces

  MasterControllerFactory* MasterController::s_factory = NULL;
  MasterController* MasterController::s_instance = NULL;

  //Logger  &MasterController::LOGGER = Logger::getInstance( "EUROPA::System::MasterController", Logger::DEBUG );
  LOGGER_CLASS_INSTANCE_IMPL( MasterController, "EUROPA::System::MasterController", DEBUG )


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

    //MasterController::LOGGER << Logger::DEBUG << msgStr;
    LOGGER_CLASS_DEBUG_MSG( MasterController, DEBUG, msgStr )

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
      //MasterController::LOGGER << Logger::ERROR << "Exception in MasterController.cc:initModel()";
      LOGGER_CLASS_DEBUG_MSG( MasterController, ERROR, "Exception in MasterController.cc:initModel()")
      throw;
    }

    return retStatus;
  }


  int getStatus(void) {
    //debugMsg("JNI:getStatus", "Calling for controller status.");
    //MasterController::LOGGER << Logger::DEBUG << "JNI:getStatus " << "Calling for controller status.";
    LOGGER_CLASS_DEBUG_MSG( MasterController, DEBUG, "JNI:getStatus " << "Calling for controller status.") 

    int retStatus;
    try {
      retStatus = MasterController::instance()->getStatus();
    }
    catch (Error e) {
      //MasterController::logMsg("Exception in MasterController.cc:getgetStatus()");
      LOGGER_CLASS_DEBUG_MSG( MasterController, ERROR, "Exception in MasterController.cc:getgetStatus()") 
      throw;  //TODO - mcr huh!? the following line is unreachable
      e.display(); //TODO - mcr error printing
    }

    return retStatus;
  }

  int writeStep(int step_num){
    //debugMsg("JNI:writeStep", "Skipping to step " << step_num);
    //MasterController::LOGGER << Logger::DEBUG << "JNI:writeStep " << "Skipping to step " << step_num;
    LOGGER_CLASS_DEBUG_MSG( MasterController, DEBUG, "JNI:writeStep " << "Skipping to step " << step_num)

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
      //MasterController::logMsg("Exception in MasterController.cc:getStatus()");
      //MasterController::LOGGER << Logger::ERROR << "Exception in MasterController.cc:getStatus()";
      LOGGER_CLASS_DEBUG_MSG( MasterController, ERROR, "Exception in MasterController.cc:getStatus()" )
	throw;
      e.display(); //TODO - mcr error printing
    }

    return retStatus;
  }

  /**
   * Implementation delegates to writeStep, called for each step in num_steps
   */
  int writeNext(int num_steps){
    //debugMsg("JNI:writeNext","Writing next " << num_steps << " steps";
    //MasterController::LOGGER << Logger::DEBUG << "JNI:writeNext " << "Writing next " << num_steps << " steps";
    LOGGER_CLASS_DEBUG_MSG( MasterController, DEBUG, "JNI:writeNext " << "Writing next " << num_steps << " steps" )
    int currentStepCount = MasterController::instance()->getStepCount();
    const int finalStep =  currentStepCount + num_steps;
    while(currentStepCount++ <  finalStep &&
	  writeStep(currentStepCount) == MasterController::IN_PROGRESS);

    return MasterController::instance()->getStatus();
  }

  int completeRun(void){
    //debugMsg("JNI:completeRun", "Completing remaining steps.");
    //MasterController::LOGGER << Logger::DEBUG << "Completing remaining steps.";
    LOGGER_CLASS_DEBUG_MSG( MasterController, DEBUG, "Completing remaining steps." )
      
    writeStep(PLUS_INFINITY);

    //debugMsg("JNI:completeRun", MasterController::toString(MasterController::instance()->getPlanDatabase()));
    LOGGER_CLASS_DEBUG_MSG( MasterController, DEBUG,  MasterController::toString(MasterController::instance()->getPlanDatabase()))

    return MasterController::instance()->getStatus();
  }

  int terminateRun(void){
    //debugMsg("JNI:terminateRun", "Terminating.");
    LOGGER_CLASS_DEBUG_MSG( MasterController, DEBUG, "JNI:terminateRun " << "Terminating." )
    MasterController::terminate();
    return 0;
  }

//   void enableDebugMsg(const char* file, const char* pattern){
//     MasterController::logMsg(std::string("Enable file ") + file + " and pattern " + pattern);
//     DebugMessage::enableMatchingMsgs(std::string(file), std::string(pattern));
//   }

//   void disableDebugMsg(const char* file, const char* pattern){
//     MasterController::logMsg(std::string("Disable file ") + file + " and pattern " + pattern);
//     DebugMessage::disableMatchingMsgs(std::string(file), std::string(pattern));
//   }

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
    //logMsg("Opening library");
    //LOGGER << Logger::DEBUG << "Opening library";
    LOGGER_DEBUG_MSG( DEBUG, "Opening library" )

    try {
      check_error(m_libHandle == NULL, "Model already loaded.");
      m_libHandle = p_dlopen(libPath, RTLD_NOW);
      if (!m_libHandle) {
        check_error_variable(const char* error_msg = p_dlerror());
        check_error(!error_msg, error_msg);
      }
    }
    catch (Error e) {
      //LOGGER << Logger::ERROR << "Unexpected exception attempting p_dlopen()";
      LOGGER_DEBUG_MSG( ERROR, "Unexpected exception attempting p_dlopen()" )
      e.display(); //TODO - mcr error printing
      throw;
    }

    //logMsg("Loading function pointer");
    //LOGGER << Logger::DEBUG << "Loading function pointer";
    LOGGER_DEBUG_MSG( DEBUG, "Loading function pointer" )

    //locate the NDDL 'loadSchema' function in the library and check for errors
    SchemaId (*fcn_loadSchema)(const SchemaId&,const RuleSchemaId&);   //function pointer to NDDL::loadSchema()
    try {
      fcn_loadSchema = (SchemaId (*)(const SchemaId&,const RuleSchemaId&))p_dlsym(m_libHandle, "loadSchema");
      if (!fcn_loadSchema) {
        check_error_variable(const char* error_msg = p_dlerror());
        check_error(!error_msg, error_msg);
      }
    }
    catch (Error e) {
      //LOGGER << Logger::ERROR << "Unexpected exception attempting p_dlsym()";
      LOGGER_DEBUG_MSG( ERROR, "Unexpected exception attempting p_dlsym()" )
      e.display(); //TODO - mcr error printing
      throw;
    }

    // call the NDDL::loadSchema function
    //LOGGER << Logger::DEBUG << "Calling NDDL:loadSchema";
    LOGGER_DEBUG_MSG( DEBUG, "Calling NDDL:loadSchema" )
    try {
        SchemaId schema = ((Schema*)getComponent("Schema"))->getId();
        RuleSchemaId ruleSchema = ((RuleSchema*)getComponent("RuleSchema"))->getId();
      (*fcn_loadSchema)(schema,ruleSchema);
    }
    catch (Error e) {
      //LOGGER << Logger::ERROR << "Unexpected exception in NDDL::loadSchema()";
      LOGGER_DEBUG_MSG( ERROR, "Unexpected exception in NDDL::loadSchema()" )
      e.display(); //TODO - mcr error printing
      throw;
    }
  }

  int MasterController::loadInitialState(const char* configPath,
					 const char* initialStatePath,
					 const char* destination,
					 const char** sourcePaths,
					 const int numPaths){
    //LOGGER.log(Logger::DEBUG, "loadInitialState:setting up debug stream");
    LOGGER_DEBUG_MSG( DEBUG,  "loadInitialState:setting up debug stream" ) 

    std::string debugDest = std::string(destination) + "/DEBUG_FILE";
    m_debugStream = new std::ofstream(debugDest.c_str(), std::ios_base::out);
    if(!m_debugStream->good()) {
      //LOGGER.log(Logger::DEBUG, std::string("loadInitialState: can't open the debug file ") + debugDest);
      LOGGER_DEBUG_MSG( DEBUG, "loadInitialState: can't open the debug file " << debugDest)
      return INITIALLY_INCONSISTENT;
    }

    DebugMessage::setStream(*m_debugStream);

    //LOGGER.log(Logger::DEBUG, "loadInitialState: Allocating components");
    LOGGER_DEBUG_MSG(DEBUG, "loadInitialState: Allocating components")


    //LOGGER.log(Logger::DEBUG, "loadInitialState: Configuring Partial PlanWriter");
    LOGGER_DEBUG_MSG( DEBUG, "loadInitialState: Configuring Partial PlanWriter" )
    m_ppw = new SOLVERS::PlanWriter::PartialPlanWriter(getPlanDatabase(), getConstraintEngine(), getRulesEngine());
    m_ppw->setDest(destination);
    SOLVERS::PlanWriter::PartialPlanWriter::noFullWrite = 1;
    SOLVERS::PlanWriter::PartialPlanWriter::writeStep = 1;

    // Obtain the client to play transactions on.
    DbClientId client = getPlanDatabase()->getClient();

    // Construct player
    DbClientTransactionPlayer player(client);

    // Open transaction source and play transactions
    //LOGGER.log(Logger::DEBUG, std::string("loadInitialState: Reading initial state from ") + initialStatePath);
    LOGGER_DEBUG_MSG( DEBUG, "loadInitialState: Reading initial state from " << initialStatePath)
    std::ifstream in(initialStatePath);
    check_error(in, "Invalid transaction source '" + std::string(initialStatePath) + "'.");
    player.play(in);

    // Provide a hook to handle custom configuration of solvers
    //LOGGER.log(Logger::DEBUG, "loadInitialState: Calling configureSolvers");
    LOGGER_DEBUG_MSG( DEBUG, "loadInitialState: Calling configureSolvers" )
    configureSolvers(configPath);

    if(!getConstraintEngine()->propagate()){
      //LOGGER.log(Logger::DEBUG, "loadInitialState: Found to be initially inconsistent");
      LOGGER_DEBUG_MSG( DEBUG, "loadInitialState: Found to be initially inconsistent" ) 
      m_status = INITIALLY_INCONSISTENT;
    }
    else {
      //LOGGER.log(Logger::DEBUG, "loadInitialState: Successfully loaded");
      LOGGER_DEBUG_MSG( DEBUG, "loadInitialState: Successfully loaded" )
      m_status = IN_PROGRESS;
    }

    write();

    return m_status;
  }

  void MasterController::unloadModel(){
    //LOGGER.log(Logger::DEBUG, "In MasterController unloadModel");
    LOGGER_DEBUG_MSG( DEBUG, "In MasterController unloadModel" )

    if (m_libHandle) {
      //LOGGER.log(Logger::DEBUG, "In MasterController: Closing");
      LOGGER_DEBUG_MSG( DEBUG, "In MasterController: Closing" )
      if (p_dlclose(m_libHandle)) {
        check_error_variable(const char* error_msg = p_dlerror());
        try {
          check_error(!error_msg, error_msg);
        }
        catch (Error e) {
          //LOGGER.log(Logger::DEBUG, "Unexpected exception in unloadModel()");
	  LOGGER_DEBUG_MSG( DEBUG, "Unexpected exception in unloadModel()" )
          e.display();//TODO - mcr error printing
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
    //debugMsg("MasterController:write", "Step " << m_stepCount);
    //LOGGER << Logger::DEBUG << "write() Step " << m_stepCount;
    LOGGER_DEBUG_MSG( DEBUG, "write() Step " << m_stepCount )
    if(!getConstraintEngine()->provenInconsistent())
      m_ppw->write();
  }

  void MasterController::writeStatistics(){
    //debugMsg("MasterController:Statistics", "Step " << m_stepCount);
    //LOGGER << Logger::DEBUG << "writeStatistics() Step " << m_stepCount;
    LOGGER_DEBUG_MSG( DEBUG, "writeStatistics() Step " << m_stepCount )
    if(!getConstraintEngine()->provenInconsistent())
      m_ppw->writeStatistics();
  }

//     void MasterController::logMsg(std::string msg){
//       std::cout << "EUROPA:" << msg << std::endl;
//       fflush(stdout);
//     }

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
