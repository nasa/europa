#ifdef __BEOS__
#include <Path.h>
#endif

#include "PartialPlanWriter.hh"

//#include "ConstrainedVariableDecisionPoint.hh"
//#include "DecisionManager.hh"
//#include "ObjectDecisionPoint.hh"
//#include "TokenDecisionPoint.hh"
//#include "ResourceFlawDecisionPoint.hh"
#include "Constraint.hh"
#include "ConstraintEngine.hh"
#include "ConstraintEngineDefs.hh"
#include "ConstrainedVariable.hh"
#include "EnumeratedDomain.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "LabelStr.hh"
#include "Variable.hh"

#include "PlanDatabase.hh"
#include "PlanDatabaseDefs.hh"
#include "Object.hh"
#include "Timeline.hh"
#include "Token.hh"
#include "TokenVariable.hh"

#include "Rule.hh"
#include "RulesEngine.hh"
#include "RulesEngineDefs.hh"
#include "RuleInstance.hh"

#include "Resource.hh"
#include "Instant.hh"

#include "Debug.hh"

#include <algorithm>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <stdexcept>
#include <set>
#include <typeinfo>
#include <vector>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define FatalError(cond, msg...){Error(cond, msg, __FILE__, __LINE__).handleAssert();}
#define FatalErrno(){FatalError("Condition", strerror(errno))}
#define FatalErr(s) {std::cerr << (s) << std::endl; FatalErrno(); }

#define IN_NO_SECTION 0
#define IN_GENERAL_SECTION 1
#define IN_TRANSACTION_SECTION 2

const char *envPPWConfigFile = "PPW_CONFIG";

namespace EUROPA {
  namespace PlanWriter {

    std::set<LabelStr> Transaction::s_types;
    std::set<LabelStr> Transaction::s_trans;
    std::map<LabelStr, bool> Transaction::s_allowed;
    std::map<LabelStr, LabelStr> Transaction::s_transToType;

    const LabelStr PartialPlanWriter::OBJECT_CREATED("OBJECT_CREATED");
    const LabelStr PartialPlanWriter::OBJECT_DELETED("OBJECT_DELETED");
    const LabelStr PartialPlanWriter::TOKEN_CREATED("TOKEN_CREATED");
    const LabelStr PartialPlanWriter::TOKEN_ADDED_TO_OBJECT("TOKEN_ADDED_TO_OBJECT"); 
    const LabelStr PartialPlanWriter::TOKEN_CLOSED("TOKEN_CLOSED"); 
    const LabelStr PartialPlanWriter::TOKEN_ACTIVATED("TOKEN_ACTIVATED"); 
    const LabelStr PartialPlanWriter::TOKEN_DEACTIVATED("TOKEN_DEACTIVATED"); 
    const LabelStr PartialPlanWriter::TOKEN_MERGED("TOKEN_MERGED"); 
    const LabelStr PartialPlanWriter::TOKEN_SPLIT("TOKEN_SPLIT");
    const LabelStr PartialPlanWriter::TOKEN_REJECTED("TOKEN_REJECTED"); 
    const LabelStr PartialPlanWriter::TOKEN_REINSTATED("TOKEN_REINSTATED"); 
    const LabelStr PartialPlanWriter::TOKEN_DELETED("TOKEN_DELETED"); 
    const LabelStr PartialPlanWriter::TOKEN_REMOVED("TOKEN_REMOVED"); 
    const LabelStr PartialPlanWriter::TOKEN_INSERTED("TOKEN_INSERTED"); 
    const LabelStr PartialPlanWriter::TOKEN_FREED("TOKEN_FREED"); 
    const LabelStr PartialPlanWriter::CONSTRAINT_CREATED("CONSTRAINT_CREATED"); 
    const LabelStr PartialPlanWriter::CONSTRAINT_DELETED("CONSTRAINT_DELETED");
    const LabelStr PartialPlanWriter::CONSTRAINT_EXECUTED("CONSTRAINT_EXECUTED"); 
    const LabelStr PartialPlanWriter::VAR_CREATED("VARIABLE_CREATED"); 
    const LabelStr PartialPlanWriter::VAR_DELETED("VARIABLE_DELETED"); 
    const LabelStr PartialPlanWriter::VAR_DOMAIN_RELAXED("VARIABLE_DOMAIN_RELAXED");
    const LabelStr PartialPlanWriter::VAR_DOMAIN_RESTRICTED("VARIABLE_DOMAIN_RESTRICTED");
    const LabelStr PartialPlanWriter::VAR_DOMAIN_SPECIFIED("VARIABLE_DOMAIN_SPECIFIED");
    const LabelStr PartialPlanWriter::VAR_DOMAIN_RESET("VARIABLE_DOMAIN_RESET"); 
    const LabelStr PartialPlanWriter::VAR_DOMAIN_EMPTIED("VARIABLE_DOMAIN_EMPTIED"); 
    const LabelStr PartialPlanWriter::VAR_DOMAIN_UPPER_BOUND_DECREASED("VARIABLE_DOMAIN_UPPER_BOUND_DECREASED"); 
    const LabelStr PartialPlanWriter::VAR_DOMAIN_LOWER_BOUND_INCREASED("VARIABLE_DOMAIN_LOWER_BOUND_INCREASED"); 
    const LabelStr PartialPlanWriter::VAR_DOMAIN_BOUNDS_RESTRICTED("VARIABLE_DOMAIN_BOUNDS_RESTRICTED");
    const LabelStr PartialPlanWriter::VAR_DOMAIN_VALUE_REMOVED("VARIABLE_DOMAIN_VALUE_REMOVED");
    const LabelStr PartialPlanWriter::VAR_DOMAIN_RESTRICT_TO_SINGLETON("VARIABLE_DOMAIN_RESTRICT_TO_SINGLETON");
    const LabelStr PartialPlanWriter::VAR_DOMAIN_SET("VARIABLE_DOMAIN_SET"); 
    const LabelStr PartialPlanWriter::VAR_DOMAIN_SET_TO_SINGLETON("VARIABLE_DOMAIN_SET_TO_SINGLETON"); 
    const LabelStr PartialPlanWriter::VAR_DOMAIN_CLOSED("VARIABLE_DOMAIN_CLOSED"); 
    const LabelStr PartialPlanWriter::RULE_EXECUTED("RULE_EXECUTED");
    const LabelStr PartialPlanWriter::RULE_UNDONE("RULE_UNDONE"); 
    const LabelStr PartialPlanWriter::PROPAGATION_COMMENCED("PROPAGATION_COMMENCED"); 
    const LabelStr PartialPlanWriter::PROPAGATION_COMPLETED("PROPAGATION_COMPLETED"); 
    const LabelStr PartialPlanWriter::PROPAGATION_PREEMPTED("PROPAGATION_PREEMPTED"); 
    const LabelStr PartialPlanWriter::ERROR("ERROR");

    const LabelStr PartialPlanWriter::STEP_SUCCEEDED("STEP_SUCCEEDED");
    const LabelStr PartialPlanWriter::STEP_FAILED("STEP_FAILED");
    const LabelStr PartialPlanWriter::PLAN_FOUND("PLAN_FOUND");
    const LabelStr PartialPlanWriter::SEARCH_EXHAUSTED("SEARCH_EXHAUSTED");
    const LabelStr PartialPlanWriter::TIMEOUT_REACHED("TIMEOUT_REACHED");
    

    const LabelStr PartialPlanWriter::CREATION("CREATION");
    const LabelStr PartialPlanWriter::DELETION("DELETION");
    const LabelStr PartialPlanWriter::ADDITION("ADDITION");
    const LabelStr PartialPlanWriter::REMOVAL("REMOVAL");
    const LabelStr PartialPlanWriter::CLOSURE("CLOSURE");
    const LabelStr PartialPlanWriter::RESTRICTION("RESTRICTION");
    const LabelStr PartialPlanWriter::RELAXATION("RELAXATION");
    const LabelStr PartialPlanWriter::EXECUTION("EXECUTION");
    const LabelStr PartialPlanWriter::SPECIFICATION("SPECIFICATION");
    const LabelStr PartialPlanWriter::UNDO("UNDO");
    const LabelStr PartialPlanWriter::NONE("NONE");

    const LabelStr PartialPlanWriter::SYSTEM("SYSTEM");
    const LabelStr PartialPlanWriter::USER("USER");
    const LabelStr PartialPlanWriter::UNKNOWN("UNKNOWN");

    const std::string DURATION_VAR("DURATION_VAR");
    const std::string END_VAR("END_VAR");
    const std::string START_VAR("START_VAR");
    const std::string STATE_VAR("STATE_VAR");
    const std::string OBJECT_VAR("OBJECT_VAR");
    const std::string PARAMETER_VAR("PARAMETER_VAR");
    const std::string MEMBER_VAR("MEMBER_VAR");
    const std::string RULE_VAR("RULE_VAR");

    const std::string tokenVarTypes[8] = 
      {STATE_VAR, OBJECT_VAR, DURATION_VAR, START_VAR, END_VAR, PARAMETER_VAR, MEMBER_VAR, RULE_VAR};

    enum varTypes {I_STATE = 0, I_OBJECT, I_DURATION, I_START, I_END, I_PARAMETER, I_MEMBER, I_RULE};
    enum objectTypes {O_OBJECT = 0, O_TIMELINE, O_RESOURCE};
    enum tokenTypes {T_INTERVAL = 0, T_TRANSACTION};
    enum decisionTypes {D_OBJECT = 0, D_TOKEN, D_VARIABLE, D_RESOURCE, D_ERROR};

#define SEQ_COL_SEP (unsigned char) 0x1e
#define SEQ_LINE_SEP (unsigned char) 0x1f
    const std::string TAB("\t");
    const std::string COLON(":");
    const std::string SNULL("\\N");
    const std::string CONSTRAINT_TOKEN("constraintToken");
    const std::string COMMA(",");
    const std::string SLASH("/");
    const std::string SPACE(" ");
    const std::string TEMPORAL("TEMPORAL");
    const std::string ATEMPORAL("ATEMPORAL");
    const std::string VAR_TEMP_CONSTR("variableTempConstr");
    const std::string UNARY_TEMP_CONSTR("unaryTempConstr");
    const std::string FIXED_TEMP_CONSTR("fixedTempConstr");
    const std::string UNARY_CONSTR("unaryConstr");
    const std::string EQ_CONSTR("equalityConstr");
    const std::string BUG_CONSTR("bugConstr");
    const std::string STRUE("true");
    const std::string SFALSE("false");
    const std::string PINFINITY("Infinity");
    const std::string NINFINITY("-Infinity");
    const std::string INTEGER_SORT("INTEGER_SORT");
    const std::string REAL_SORT("REAL_SORT");
    const std::string STEP("step");
    const std::string PARTIAL_PLAN_STATS("/partialPlanStats");
    const std::string TRANSACTIONS("/transactions");
    const std::string SEQUENCE("/sequence");
    const std::string RULES("/rules");
    const std::string PARTIAL_PLAN(".partialPlan");
    const std::string OBJECTS(".objects");
    const std::string TOKENS(".tokens");
    const std::string RULE_INSTANCES(".ruleInstances");
    const std::string RULE_INSTANCE_SLAVE_MAP(".ruleInstanceSlaveMap");
    const std::string VARIABLES(".variables");
    const std::string CONSTRAINTS(".constraints");
    const std::string CONSTRAINT_VAR_MAP(".constraintVarMap");
    const std::string INSTANTS(".instants");
    const std::string DECISIONS(".decisions");
    const std::string E_DOMAIN("E");
    const std::string I_DOMAIN("I");
    const std::string CAUSAL("CAUSAL");
    const std::string ENUM_DOMAIN("EnumeratedDomain");
    const std::string INT_DOMAIN("IntervalDomain");
    const std::string GENERAL_CONFIG_SECTION("GeneralConfigSection:");
    const std::string TRANSACTION_CONFIG_SECTION("TransactionConfigSection:");
    const std::string RULE_CONFIG_SECTION("RuleConfigSection:");
    const std::string SOURCE_PATH("SourcePath");
    const std::string AUTO_WRITE("AutoWrite");
    const std::string STEPS_PER_WRITE("StepsPerWrite");
    const std::string WRITE_FINAL_STEP("WriteFinalStep");
    const std::string WRITE_DEST("WriteDest");
    const std::string MAX_CHOICES("MaxChoices");

#ifdef __BEOS__
#define NBBY 8
    static char *realpath(const char *path, char *resolved_path) {
      BPath tempPath(path,NULL,true);
      if (tempPath.Path() == NULL) {
        return NULL;
      }
      strcpy(resolved_path,tempPath.Path());
      return resolved_path;
    }
#endif

    inline long long int timeval2Id(const struct timeval &currTime) {
      return (((long long int) currTime.tv_sec) * 1000) + (currTime.tv_usec / 1000);
    }

    /* These are static to make them visible in GDB out of their normal scope.
     */
    int PartialPlanWriter::noFullWrite(0);
    int PartialPlanWriter::writeStep(0);

    PartialPlanWriter::PartialPlanWriter(const PlanDatabaseId& planDb,
                                         const ConstraintEngineId& ceId2,
                                         const RulesEngineId& reId2,
                                         SOLVERS::SolverId& solver) {
      commonInit(planDb, ceId2, reId2);
      sl = (new PPWSearchListener(solver, this))->getId();
      unmarksStep(PROPAGATION_PREEMPTED);
      unmarksStep(PROPAGATION_COMPLETED);
      marksStep(STEP_SUCCEEDED);
      marksStep(STEP_FAILED);
      marksStep(PLAN_FOUND);
      marksStep(SEARCH_EXHAUSTED);
      marksStep(TIMEOUT_REACHED);
    }

    PartialPlanWriter::PartialPlanWriter(const PlanDatabaseId &planDb,
                                         const ConstraintEngineId &ceId2,
                                         const RulesEngineId &reId2) {
      commonInit(planDb, ceId2, reId2);//, CBPlannerId::noId());
    }

    PartialPlanWriter::PartialPlanWriter(const PlanDatabaseId &planDb, 
                                         const ConstraintEngineId &ceId2) {
      commonInit(planDb, ceId2, RulesEngineId::noId());//, CBPlannerId::noId());
    }

    void PartialPlanWriter::allocateListeners() {
      dbl = (new PPWPlanDatabaseListener(pdbId, this))->getId();
      cel = (new PPWConstraintEngineListener(ceId, this))->getId();
      if (!reId.isNoId())
        rel = (new PPWRulesEngineListener(reId, this))->getId();
    }

    void PartialPlanWriter::commonInit(const PlanDatabaseId &planDb,
                                       const ConstraintEngineId &ceId2,
                                       const RulesEngineId& _reId) {
      reId = _reId;
      nstep = 0;
      destAlreadyInitialized = false;
      m_writing = false;
      struct timeval currTime;
      if(gettimeofday(&currTime, NULL)) {
        FatalError("gettimeofday()", "Failed to get current time.");
      }
      seqId = timeval2Id(currTime);
      pdbId = planDb;
      ceId = ceId2;
      transactionId = writeCounter = numTransactions = 0;
      transactionList = new std::list<Transaction>();
      stepsPerWrite = 0;
      dest = "./plans";
      noFullWrite = 1;
      writeStep = 0;

      initTransactions();

      //add default directories to search for model files
      sourcePaths.push_back(".");
      sourcePaths.push_back("..");
      char *configPath = getenv(envPPWConfigFile);

      // If null, then also test for existence of a default config file - PlanWorks.cfg
      if(configPath == NULL){
        std::ifstream config("PlanWorks.cfg");
        if (config.good())
          configPath = "PlanWorks.cfg";
      }

      if (configPath == NULL || configPath[0] == '\0') {
        debugMsg("PartialPlanWriter",  "Warning: PPW_CONFIG not set or is empty.");
        debugMsg("PartialPlanWriter",  "   PartialPlanWriter will not write.");
        debugMsg("PartialPlanWriter",  "    Under client control, some step 0 transactions will not be logged.");
        stepsPerWrite = 0;
        noFullWrite = 1;
        writeStep = 0;
        //initialize as enabled in case under client control
        //does not affect normal operation since stepsPerWrite = 0
        Transaction::allowAllTransactions();
        return;
      }
			
      char *configBuf = new char[PATH_MAX + 100];
      if (configBuf == 0)
        FatalErr("No memory for PPW_CONFIG");
      if (realpath(configPath, configBuf) == NULL) {
        std::cerr << "Failed to get config file " << configPath << std::endl;
        FatalErrno();
      }

      // std::cerr << "PPW DEBUG:constructing:configFile; configBuf = " << configBuf << std::endl;
      std::ifstream configFile(configBuf);
      if (!configFile) {
        std::cerr << "Failed to open config file " << configBuf << std::endl;
        FatalErrno();
      }
      std::string buf;

      parseConfigFile(configFile);

      if (stepsPerWrite != 0) {
        allocateListeners();
      }

    }

    void PartialPlanWriter::initOutputDestination() {
      debugMsg("PartialPlanWriter:initOutputDestination", "Running...");

      char *destBuf = new char[PATH_MAX];
      if(realpath(dest.c_str(), destBuf) == NULL && stepsPerWrite != 0) {
        debugMsg("PartialPlanWriter:initOutputDestination", "Creating dest dir '" << destBuf << "'");
        if(mkdir(destBuf, 0777) && errno != EEXIST) {
          std::cerr << "Failed to make destination directory " << dest << std::endl;
          FatalErrno();
        }
      }
      realpath(dest.c_str(), destBuf);
      dest = destBuf;
      delete [] destBuf;

      char timestr[NBBY * sizeof(seqId) * 28/93 + 4];
      sprintf(timestr, "%lld", seqId);
      std::string modelName = pdbId->getSchema()->getName().toString();
      {
        std::string::size_type tempIndex = modelName.rfind('/');
        if(tempIndex > 0 && tempIndex < modelName.length()) {
          modelName = modelName.substr(tempIndex);
        }
      }

      std::string seqName = modelName;
      std::string::size_type extStart = seqName.find('.');
      seqName = seqName.substr(0, extStart);

      if(stepsPerWrite) {
        if(mkdir(dest.c_str(), 0777) && errno != EEXIST) {
          /*
           * Failing with return code = -1, errno = 0 when dir already 
           * exists.  Should be returning errno = EEXIST. 
           * Check errno is non-zero before call to FatalErrno()
           */
          if (errno) {
            std::cerr << "Failed to make directory " << dest << std::endl;
            FatalErrno();
          }
        }
        if(seqName[0] != '/')
          dest += "/";
        dest += seqName;
        dest += timestr;

        if(mkdir(dest.c_str(), 0777) && errno != EEXIST) {
          std::cerr << "Failed to make directory " << dest << std::endl;
          FatalErrno();
        }
        std::string ppStats(dest + PARTIAL_PLAN_STATS);
        std::string ppTransactions(dest + TRANSACTIONS);
        std::string seqRules(dest + RULES);
        std::string seqStr(dest + SEQUENCE);
        std::ofstream seqOut(seqStr.c_str());
        if(!seqOut) {
          std::cerr << "Failed to open " << seqStr << std::endl;
          FatalErrno();
        }
        seqOut << dest << SEQ_COL_SEP << seqId << SEQ_COL_SEP;// << std::endl;
        
        std::ofstream rulesOut(seqRules.c_str());
        if(!rulesOut) {
          std::cerr << "Failed to open " << seqRules << std::endl;
          FatalErrno();
        }
      
        std::set<std::string> modelFiles;
        char realModelPaths[PATH_MAX];
        bool foundModelPath = false;
        for(std::multimap<double, RuleId>::const_iterator it = Rule::getRules().begin(); 
            it != Rule::getRules().end(); ++it) {
          std::string ruleSrc = ((*it).second)->getSource().toString();
          if(ruleSrc == "noSrc")
            continue;
          std::string modelFile = ruleSrc.substr(1, ruleSrc.rfind(",")-1);
          std::string lineNumber = ruleSrc.substr(ruleSrc.rfind(","), ruleSrc.size()-1);
          lineNumber.replace(lineNumber.rfind('"'), 1, "\0");
          foundModelPath = false;
          for(std::list<std::string>::iterator pathIt = sourcePaths.begin();
              pathIt != sourcePaths.end(); ++pathIt) {
            std::string modelPath = (*pathIt) + "/" + modelFile;
            if(realpath(modelPath.c_str(), realModelPaths) == NULL) {
              continue;
            }
            modelPath = realModelPaths;
            modelFiles.insert(modelPath);
            foundModelPath = true;
            rulesOut << seqId << TAB << (*it).second->getKey() << TAB << modelPath << lineNumber 
                     << std::endl;
            break;
          }
          if (foundModelPath == false) {
            std::cerr << "Warning: PPW could not find path to model file for rule " 
                      << (*it).second->getKey() << std::endl;
            std::cerr << "         Check configuration of RuleConfigSection in PlanWorks.cfg " << std::endl;
          }
        }
        {
          std::ostream_iterator<unsigned char> out(seqOut);
          for(std::set<std::string>::const_iterator it = modelFiles.begin(); 
              it != modelFiles.end(); ++it) {
            seqOut << "--begin " << *it << std::endl;
            std::ifstream modelIn((*it).c_str());
            modelIn.unsetf(std::ios::skipws);
            std::copy(std::istream_iterator<unsigned char>(modelIn), 
                      std::istream_iterator<unsigned char>(), out);
          }
        }
        seqOut << SEQ_LINE_SEP;
        seqOut.close();

        transOut = new std::ofstream(ppTransactions.c_str());
        if(!(*transOut)) {
          FatalErrno();
        }
        statsOut = new std::ofstream(ppStats.c_str());
        if(!(*statsOut)) {
          FatalErrno();
        }
      }
    }

    void PartialPlanWriter::initTransactions() {
      Transaction::addType(CREATION);
      Transaction::addType(DELETION);
      Transaction::addType(ADDITION);
      Transaction::addType(REMOVAL);
      Transaction::addType(CLOSURE);
      Transaction::addType(RESTRICTION);
      Transaction::addType(RELAXATION);
      Transaction::addType(EXECUTION);
      Transaction::addType(SPECIFICATION);
      Transaction::addType(UNDO);
      Transaction::addType(NONE);

      Transaction::addTransaction(OBJECT_CREATED, CREATION);
      Transaction::addTransaction(OBJECT_DELETED, DELETION);
      Transaction::addTransaction(TOKEN_CREATED, CREATION);
      Transaction::addTransaction(TOKEN_ADDED_TO_OBJECT, ADDITION);
      Transaction::addTransaction(TOKEN_CLOSED, CLOSURE);
      Transaction::addTransaction(TOKEN_ACTIVATED, NONE);
      Transaction::addTransaction(TOKEN_DEACTIVATED, NONE);
      Transaction::addTransaction(TOKEN_MERGED, NONE);
      Transaction::addTransaction(TOKEN_SPLIT, NONE);
      Transaction::addTransaction(TOKEN_REJECTED, NONE);
      Transaction::addTransaction(TOKEN_REINSTATED, NONE);
      Transaction::addTransaction(TOKEN_DELETED, DELETION);
      Transaction::addTransaction(TOKEN_REMOVED, REMOVAL);
      Transaction::addTransaction(TOKEN_INSERTED, NONE);
      Transaction::addTransaction(TOKEN_FREED, NONE);
      Transaction::addTransaction(CONSTRAINT_CREATED, CREATION); 
      Transaction::addTransaction(CONSTRAINT_DELETED, DELETION);
      Transaction::addTransaction(CONSTRAINT_EXECUTED, EXECUTION);
      Transaction::addTransaction(VAR_CREATED, CREATION);
      Transaction::addTransaction(VAR_DELETED, DELETION);
      Transaction::addTransaction(VAR_DOMAIN_RELAXED, RELAXATION);
      Transaction::addTransaction(VAR_DOMAIN_RESTRICTED, RESTRICTION);
      Transaction::addTransaction(VAR_DOMAIN_SPECIFIED, SPECIFICATION);
      Transaction::addTransaction(VAR_DOMAIN_RESET, RELAXATION);
      Transaction::addTransaction(VAR_DOMAIN_EMPTIED, RESTRICTION);
      Transaction::addTransaction(VAR_DOMAIN_UPPER_BOUND_DECREASED, RESTRICTION); 
      Transaction::addTransaction(VAR_DOMAIN_LOWER_BOUND_INCREASED, RESTRICTION);
      Transaction::addTransaction(VAR_DOMAIN_BOUNDS_RESTRICTED,RESTRICTION);
      Transaction::addTransaction(VAR_DOMAIN_VALUE_REMOVED,RESTRICTION);
      Transaction::addTransaction(VAR_DOMAIN_RESTRICT_TO_SINGLETON, RESTRICTION);
      Transaction::addTransaction(VAR_DOMAIN_SET, SPECIFICATION);
      Transaction::addTransaction(VAR_DOMAIN_SET_TO_SINGLETON, SPECIFICATION);
      Transaction::addTransaction(VAR_DOMAIN_CLOSED, CLOSURE);
      Transaction::addTransaction(RULE_EXECUTED,EXECUTION);
      Transaction::addTransaction(RULE_UNDONE, UNDO);
      Transaction::addTransaction(PROPAGATION_COMMENCED, NONE);
      Transaction::addTransaction(PROPAGATION_COMPLETED, NONE);
      Transaction::addTransaction(PROPAGATION_PREEMPTED, NONE);
      Transaction::addTransaction(STEP_SUCCEEDED, NONE);
      Transaction::addTransaction(STEP_FAILED, NONE);
      Transaction::addTransaction(PLAN_FOUND, NONE);
      Transaction::addTransaction(SEARCH_EXHAUSTED, NONE);
      Transaction::addTransaction(TIMEOUT_REACHED, NONE);

      marksStep(PROPAGATION_COMPLETED);
      marksStep(PROPAGATION_PREEMPTED);
    }
  
    PartialPlanWriter::~PartialPlanWriter(void) {
      if (!dbl.isNoId())
        delete (PlanDatabaseListener*) dbl;
      if (!cel.isNoId())
        delete (ConstraintEngineListener*) cel;
      if (!rel.isNoId())
        delete (RulesEngineListener*) rel;
      //incredibly bizarre... something's deleting the search listener
      //before this, and I can't breakpoint where it's happening.
      //~MJI
      //if(!sl.isNoId())
      //  delete (SOLVERS::SearchListener*) sl;

      if(stepsPerWrite) {
        if(destAlreadyInitialized) {
          transOut->close();
          statsOut->close();
          delete transOut;
          delete statsOut;
        }
        delete transactionList;
      }
    }

    // accessor to get output destination full path
    std::string PartialPlanWriter::getDest(void) {
      return dest;
    }

    /*
     * accessor to set and init output destination path and files
     *
     * This should only be called before calling write() or
     * writeStatsAndTransactions()
     */
    void PartialPlanWriter::setDest(std::string destPath) {
      /*
       * initialize write controls
       * initialize the directories and files for the
       * specified output destination.
       */
      if(!destAlreadyInitialized) {
        /*
         *If stepsPerWrite is still 0 after commonInit has run
         *then allocateListeners() has not been called yet 
         *Call now since Planner Control needs them
         */
        if (stepsPerWrite == 0) {
          allocateListeners();
        }
        dest = destPath;
        noFullWrite = 1;   // do not write every step
        stepsPerWrite = 1; // enable write in one step increments
        writeStep = 1;     // enable one step client control of write
        initOutputDestination();
        destAlreadyInitialized = true;
      } else {
        std::cerr << "Destination directory already initialized to " << dest << std::endl; 
        std::cerr << "Failed to initialize destination directory to " << destPath << std::endl;
        FatalErrno();
      }
    }

    int PartialPlanWriter::getNumTransactions() {
      return Transaction::registeredTransactionCount();
    }

    int PartialPlanWriter::getMaxLengthTransactions() {
      return Transaction::maxTransactionLength();
    }

    const char** PartialPlanWriter::getTransactionNameStrs() {
      return Transaction::getNameStrs();
    }
     
    bool* PartialPlanWriter::getTransactionFilterStates() {
      return Transaction::getAllowStates();
    }

    void PartialPlanWriter::setTransactionFilterStates(bool* transFilterStates, 
                                                       int numTrans) {
      Transaction::setAllowStates(transFilterStates, numTrans);
    }

    void PartialPlanWriter::write(void) {

      /*
       * init output destination files if this has not been done
       * This is also called in WriteStatsAndTransactions() to cover
       * cases where the first step is not written.
       */

      // std::cerr << " PartialPlanWriter::write() called " << std::endl;
      check_error(!m_writing, "PartialPlanWriter attempted to write while writing.");
      m_writing = true;
      debugMsg("PartialPlanWriter:write", "Writing step " << nstep);

      if(!destAlreadyInitialized) {
        initOutputDestination();
        destAlreadyInitialized = true;
      }
      if(!transOut || !statsOut)
        return;
      ppId = 0LL;
      struct timeval currTime;
      if(gettimeofday(&currTime, NULL)) {
        FatalError("gettimeofday()", "Failed to get current time.");
      }
      ppId = timeval2Id(currTime);

      numTokens = numVariables = numConstraints = 0;

      char stepstr[NBBY * sizeof(nstep) * 28/93 + 4];
      sprintf(stepstr, "%d", nstep);
    
      std::string stepnum(STEP + stepstr);

      std::string ppDest = dest + SLASH + stepnum;
      if(mkdir(ppDest.c_str(), 0777) && errno != EEXIST) {
        std::cerr << "Failed to create " << ppDest << std::endl;
        FatalErrno();
      }

      std::string ppPartialPlan = ppDest + SLASH + stepnum + PARTIAL_PLAN;
      std::ofstream ppOut(ppPartialPlan.c_str());
      if(!ppOut) {
        FatalErrno();
      }

      ppOut << stepnum << TAB << ppId << TAB << pdbId->getSchema()->getName().toString()
						<< TAB << seqId << std::endl;
      ppOut.close();

      std::string ppObj = ppDest + SLASH + stepnum + OBJECTS;
      std::ofstream objOut(ppObj.c_str());
      if(!objOut) {
        FatalErrno();
      }

      std::string ppTok = ppDest + SLASH + stepnum + TOKENS;
      std::ofstream tokOut(ppTok.c_str());
      if(!tokOut) {
        FatalErrno();
      }

      std::string ppRuleInstances = ppDest + SLASH + stepnum + RULE_INSTANCES;
      std::ofstream ruleInstanceOut(ppRuleInstances.c_str());
      if(!ruleInstanceOut) {
        FatalErrno();
      }

      std::string ppRISM = ppDest + SLASH + stepnum + RULE_INSTANCE_SLAVE_MAP;
      std::ofstream rismOut(ppRISM.c_str());
      if(!rismOut) {
        FatalErrno();
      }

      std::string ppVars = ppDest + SLASH + stepnum + VARIABLES;
      std::ofstream varOut(ppVars.c_str());
      if(!varOut) {
        FatalErrno();
      }

      std::string ppConstrs = ppDest + SLASH + stepnum + CONSTRAINTS;
      std::ofstream constrOut(ppConstrs.c_str());
      if(!constrOut) {
        FatalErrno();
      }

      std::string ppCVM = ppDest + SLASH + stepnum + CONSTRAINT_VAR_MAP;
      std::ofstream cvmOut(ppCVM.c_str());
      if(!cvmOut) {
        FatalErrno();
      }

      std::string ppInsts = ppDest + SLASH + stepnum + INSTANTS;
      std::ofstream instsOut(ppInsts.c_str());
      if(!instsOut) {
        FatalErrno();
      }

      std::string ppDecs = ppDest + SLASH + stepnum + DECISIONS;
      std::ofstream decsOut(ppDecs.c_str());
      if(!decsOut) {
        FatalErrno();
      }

      const ConstraintSet &constraints = ceId->getConstraints();
      numConstraints = constraints.size();
      for(ConstraintSet::const_iterator it = constraints.begin(); it != constraints.end(); ++it) {
        outputConstraint(*it, constrOut, cvmOut);
      }

      ObjectSet objects(pdbId->getObjects());
      TokenSet tokens(pdbId->getTokens());
      int slotId = 1000000;
      for(ObjectSet::iterator objectIterator = objects.begin();
          objectIterator != objects.end(); ++objectIterator) {
        const ObjectId &objId = *objectIterator;
        if(TimelineId::convertable(objId)) {
          outputObject(objId, O_TIMELINE, objOut, varOut);
          TimelineId &tId = (TimelineId &) objId;
          const std::list<TokenId>& orderedTokens = tId->getTokenSequence();
          int slotIndex = 0;
          int emptySlots = 0;
          for(std::list<TokenId>::const_iterator tokenIterator = orderedTokens.begin();
              tokenIterator != orderedTokens.end(); ++tokenIterator) {
            int slotOrder = 0;
            const TokenId &token = *tokenIterator;
            outputToken(token, T_INTERVAL, slotId, slotIndex, slotOrder, (ObjectId) tId, tokOut,
                        varOut);
            tokens.erase(token);
            TokenSet::const_iterator mergedTokenIterator = 
              token->getMergedTokens().begin();
            for(;mergedTokenIterator != token->getMergedTokens().end(); ++mergedTokenIterator) {
              slotOrder++;
              outputToken(*mergedTokenIterator, T_INTERVAL, slotId, slotIndex, slotOrder,
                          (ObjectId &) tId, tokOut, varOut);
              tokens.erase(*mergedTokenIterator);
            }
            slotId++;
            slotIndex++;
            ++tokenIterator;
            /*ExtraData: empty slot info*/
            if(tokenIterator != orderedTokens.end()) {
              const TokenId &nextToken = *tokenIterator;
              if(token->getEnd()->lastDomain() != nextToken->getStart()->lastDomain()) {
                objOut << slotId << COMMA << slotIndex << COLON;
                emptySlots++;
                slotId++;
                slotIndex++;
              }
            }
            --tokenIterator;
          }
          if(!emptySlots)
            objOut << SNULL;
          objOut << std::endl;
        }
        else if(ResourceId::convertable(objId)) {
          outputObject(objId, O_RESOURCE, objOut, varOut);

          ResourceId &rId = (ResourceId &) objId;
          
          /*ExtraData: resource info*/
          objOut << MINUS_INFINITY << COMMA << PLUS_INFINITY << COMMA
                 << rId->getInitialCapacity() << COMMA << rId->getLimitMin() << COMMA
                 << rId->getLimitMax() << COMMA;

          //const std::set<TransactionId>& resTrans = rId->getTransactions();
          std::set<TransactionId> resTrans;
          rId->getTransactions(resTrans, MINUS_INFINITY, PLUS_INFINITY, false);
          for(std::set<TransactionId>::iterator transIt = resTrans.begin();
              transIt != resTrans.end(); ++transIt) {
            TransactionId trans = *transIt;
            outputToken(trans, T_TRANSACTION, 0, 1, 0, rId, tokOut, varOut);
            tokens.erase(trans);
          }

          const std::map<int, InstantId>& insts = rId->getInstants();
          for(std::map<int,InstantId>::const_iterator instIt = insts.begin();
              instIt != insts.end(); ++instIt) {
            InstantId inst = (*instIt).second;
            outputInstant(inst, rId->getKey(), instsOut);
            objOut << inst->getKey() << COMMA;
          }
          objOut << std::endl;
        }
        else {
          outputObject(objId, O_OBJECT, objOut, varOut);
          /*ExtraData: NULL*/
          objOut << SNULL << std::endl;
        }
      }
      for(TokenSet::iterator tokenIterator = tokens.begin(); 
          tokenIterator != tokens.end(); ++tokenIterator) {
				TokenId token = *tokenIterator;
				check_error(token.isValid());
				outputToken(token, T_INTERVAL, 0, 0, 0, ObjectId::noId(), tokOut, varOut);
      }



      std::set<RuleInstanceId> ruleInst = reId->getRuleInstances();
      for(std::set<RuleInstanceId>::const_iterator it = ruleInst.begin();
          it != ruleInst.end(); ++it) {
        RuleInstanceId ri = *it;
        outputRuleInstance(ri, ruleInstanceOut, varOut, rismOut);
      }
			
      collectStats(); // this call will overwrite incremental counters for tokens, variables, and constraints
      (*statsOut) << seqId << TAB << ppId << TAB << nstep << TAB << numTokens << TAB << numVariables
                  << TAB << numConstraints << TAB << numTransactions << std::endl;
      statsOut->flush();

      outputTransactions(transOut);

      objOut.close();
      tokOut.close();
      ruleInstanceOut.close();
      rismOut.close();
      varOut.close();
      constrOut.close();
      cvmOut.close();
      instsOut.close();
      decsOut.close();
      m_writing = false;
    }

    /* writeStatsAndTransactions() is called at the end of each step 
       instead of write() when step data is written for only the 
       final step. this ensures that transaction and statistics info
       is written for all steps.
    */
    void PartialPlanWriter::writeStatsAndTransactions(void) {
      if(!destAlreadyInitialized) {
        initOutputDestination();
        destAlreadyInitialized = true;
      }
      if(!transOut)
        return;

      ppId = 0LL;
      struct timeval currTime;
      if(gettimeofday(&currTime, NULL)) {
        FatalError("gettimeofday()", "Failed to get current time.");
      }
      ppId = timeval2Id(currTime);

      collectStats();
      (*statsOut) << seqId << TAB << ppId << TAB << nstep << TAB << numTokens << TAB << numVariables
                  << TAB << numConstraints << TAB << numTransactions << std::endl;
      statsOut->flush();

      outputTransactions(transOut);
    }


    /* collects all but numTransactions which are
       collected incrementally
    */
    void PartialPlanWriter::collectStats(void) {
      TokenSet tokens(pdbId->getTokens());
      numTokens = tokens.size();
      ConstrainedVariableSet variables = ceId->getVariables();
      numVariables = variables.size();
      const ConstraintSet &constraints = ceId->getConstraints();
      numConstraints = constraints.size();
    }

    void PartialPlanWriter::outputTransactions(std::ofstream *transOut) {
      for(std::list<Transaction>::iterator it = transactionList->begin();
          it != transactionList->end(); ++it) {
        (*it).write((*transOut), ppId);
      }
    }

    void PartialPlanWriter::outputObject(const ObjectId &objId, const int type,
                                         std::ofstream &objOut, std::ofstream &varOut) {
      int parentKey = -1;
      if(!objId->getParent().isNoId())
        parentKey = objId->getParent()->getKey();
      objOut << objId->getKey() << TAB << type << TAB << parentKey << TAB
             << ppId << TAB << objId->getName().toString() << TAB;
      /*ChildObjectIds*/
      if(objId->getComponents().empty()) {
        objOut << SNULL << TAB;
      }
      else {
        for(ObjectSet::const_iterator childIt = objId->getComponents().begin();
            childIt != objId->getComponents().end(); ++childIt) {
          ObjectId child = *childIt;
          objOut << child->getKey() << COMMA;
        }
        objOut << TAB;
      }
      /*end ChildObjectIds*/
      /*VariableIds*/
      if(objId->getVariables().empty()) {
        objOut << SNULL << TAB;
      }
      else {
        for(std::vector<ConstrainedVariableId>::const_iterator varIt = 
              objId->getVariables().begin(); varIt != objId->getVariables().end(); ++varIt) {
          ConstrainedVariableId var = *varIt;
          objOut << var->getKey() << COMMA;
          outputConstrVar(var, objId->getKey(), I_MEMBER, varOut);
        }
        objOut << TAB;
      }
      /*end VariableIds*/
      /*TokenIds*/
      if(objId->getTokens().empty()) {
        objOut << SNULL << TAB;
      }
      else {
        for(TokenSet::const_iterator tokIt = objId->getTokens().begin();
            tokIt != objId->getTokens().end(); ++tokIt) {
          TokenId token = *tokIt;
          objOut << token->getKey() << COMMA;
        }
        objOut << TAB;
      }
      /*end TokenIds*/
    }

    void PartialPlanWriter::outputToken(const TokenId &token, const int type, const int slotId, 
                                        const int slotIndex, const int slotOrder, 
                                        const ObjectId &tId, std::ofstream &tokOut, 
                                        std::ofstream &varOut) {
      check_error(token.isValid());
      if(token->isIncomplete()) {
        std::cerr << "Token " << token->getKey() << " is incomplete.  Skipping. " << std::endl;
        return;
      }
      if(!tId.isNoId()) {
        tokOut << token->getKey() << TAB << type << TAB << slotId << TAB << slotIndex << TAB 
               << ppId << TAB << 0 << TAB << 1 << TAB << token->getStart()->getKey() << TAB 
               << token->getEnd()->getKey() << TAB << token->getDuration()->getKey() << TAB 
               << token->getState()->getKey() << TAB << token->getPredicateName().toString() 
               << TAB << tId->getKey() << TAB << tId->getName().toString() << TAB 
               << token->getObject()->getKey() << TAB;
      }
      else {
        tokOut << token->getKey() << TAB << type << TAB << SNULL << TAB << SNULL << TAB << ppId 
               << TAB << 1 << TAB << 1 << TAB << token->getStart()->getKey() << TAB 
               << token->getEnd()->getKey() << TAB << token->getDuration()->getKey() << TAB 
               << token->getState()->getKey() << TAB << token->getPredicateName().toString() 
               << TAB << SNULL << TAB << SNULL << TAB << token->getObject()->getKey() << TAB;
      }
      outputObjVar(token->getObject(), token->getKey(), I_OBJECT, varOut);
      outputIntIntVar(token->getStart(), token->getKey(), I_START, varOut);
      outputIntIntVar(token->getEnd(), token->getKey(), I_END, varOut);
      outputIntIntVar(token->getDuration(), token->getKey(), I_DURATION, varOut);
      //outputEnumVar(token->getState(), token->getKey(), I_STATE, varOut);
      outputStateVar(token->getState(), token->getKey(), I_STATE, varOut);

      std::string paramVarIds;
      char paramIdStr[NBBY * sizeof(int) * 28/93 + 4];
      for(std::vector<ConstrainedVariableId>::const_iterator paramVarIterator = 
            token->getParameters().begin();
          paramVarIterator != token->getParameters().end(); ++paramVarIterator) {
        ConstrainedVariableId varId = *paramVarIterator;
        check_error(varId.isValid());
        outputConstrVar(varId, token->getKey(), I_PARAMETER, varOut);
        memset(paramIdStr, '\0', NBBY * sizeof(int) * 28/93 + 4);
        sprintf(paramIdStr, "%d", varId->getKey());
        paramVarIds += std::string(paramIdStr) + COLON;
      }
      if(paramVarIds == "") {
        tokOut << SNULL << TAB;
      }
      else {
        tokOut << paramVarIds << TAB;
      }
      /*ExtraInfo: QuantityMin:QuantityMax*/
      if(type == T_TRANSACTION) {
        TransactionId trans = (TransactionId) token;
        tokOut << trans->getMin() << COMMA << trans->getMax();
      }
      /*ExtraInfo: SlotOrder*/
      else {
        tokOut << slotOrder;
      }
      
      tokOut << std::endl;
      numTokens++;
    }
  
    void PartialPlanWriter::outputStateVar(const Id<TokenVariable<StateDomain> >& stateVar,
                                           const int parentId, const int type,
                                           std::ofstream &varOut) {
      numVariables++;
      varOut << stateVar->getKey() << TAB << ppId << TAB << parentId << TAB 
						 << stateVar->getName().toString() << TAB;

      varOut << ENUM_DOMAIN << TAB;
      StateDomain &dom = (StateDomain &) stateVar->lastDomain();
      std::list<double> vals;
      dom.getValues(vals);
      for(std::list<double>::const_iterator it = vals.begin(); it != vals.end(); ++it){
        LabelStr strVal(*it);
        varOut << strVal.toString() << " ";
      }
      varOut << TAB;
      varOut << SNULL << TAB << SNULL << TAB << SNULL << TAB;

      varOut << tokenVarTypes[type] << std::endl;
    }

    void PartialPlanWriter::outputEnumVar(const Id<TokenVariable<EnumeratedDomain> >& enumVar, 
                                          const int parentId, const int type,
																					std::ofstream &varOut) {
      numVariables++;
      varOut << enumVar->getKey() << TAB << ppId << TAB << parentId << TAB 
						 << enumVar->getName().toString() << TAB;

      varOut << ENUM_DOMAIN << TAB;

      varOut << getEnumerationStr((EnumeratedDomain &)enumVar->lastDomain()) << TAB;
      varOut << SNULL << TAB << SNULL << TAB << SNULL << TAB;

      varOut << tokenVarTypes[type] << std::endl;
    }
  
    void PartialPlanWriter::outputIntVar(const Id<TokenVariable<IntervalDomain> >& intVar,
                                         const int parentId, const int type,
																				 std::ofstream &varOut) {
      numVariables++;
      varOut << intVar->getKey() << TAB << ppId << TAB << parentId << TAB 
						 << intVar->getName().toString() << TAB;

      varOut << INT_DOMAIN << TAB << SNULL << TAB << REAL_SORT << TAB 
						 << getLowerBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB
						 << getUpperBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB;

      varOut << tokenVarTypes[type] << std::endl;
    }
  
    void PartialPlanWriter::outputIntIntVar(const Id<TokenVariable<IntervalIntDomain> >& intVar,
                                            const int parentId, const int type,
																						std::ofstream &varOut) {
      numVariables++;

      varOut << intVar->getKey() << TAB << ppId << TAB << parentId << TAB 
						 << intVar->getName().toString() << TAB;

      varOut << INT_DOMAIN << TAB << SNULL << TAB << INTEGER_SORT << TAB 
						 << getLowerBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB
						 << getUpperBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB;
    
      varOut << tokenVarTypes[type] << std::endl;
    }

    void PartialPlanWriter::outputObjVar(const ObjectVarId& objVar,
                                         const int parentId, const int type,
																				 std::ofstream &varOut) {
      numVariables++;

      varOut << objVar->getKey() << TAB << ppId << TAB << parentId << TAB 
						 << objVar->getName().toString() << TAB;

      varOut << ENUM_DOMAIN << TAB;

      std::list<double> objects;
      ((ObjectDomain &)objVar->lastDomain()).getValues(objects);
      for(std::list<double>::iterator it = objects.begin(); it != objects.end(); ++it) {
				varOut << ((ObjectId)(*it))->getName().toString() << " ";
      }

      varOut << TAB << SNULL << TAB << SNULL << TAB << SNULL << TAB;

      varOut << tokenVarTypes[type] << std::endl;
    }
  
    void PartialPlanWriter::outputConstrVar(const ConstrainedVariableId &otherVar, 
                                            const int parentId, const int type,
																						std::ofstream &varOut) {
      numVariables++;

      varOut << otherVar->getKey() << TAB << ppId << TAB << parentId << TAB 
						 << otherVar->getName().toString() << TAB;

      if(otherVar->lastDomain().isEnumerated()) {
				varOut << ENUM_DOMAIN << TAB << getEnumerationStr((EnumeratedDomain &)otherVar->lastDomain()) 
							 << TAB << SNULL << TAB << SNULL << TAB << SNULL << TAB;
      }
      else if(otherVar->lastDomain().isInterval()) {
				varOut << INT_DOMAIN << TAB << SNULL << TAB << REAL_SORT << TAB 
							 << getLowerBoundStr((IntervalDomain &)otherVar->lastDomain()) << TAB 
							 << getUpperBoundStr((IntervalDomain &)otherVar->lastDomain()) << TAB;
      }
      else {
        FatalError("otherVar->lastDomain()isEnumerated() || otherVar->lastDomain().isInterval()",
                   "I don't know what my domain is!");
      }
      varOut << tokenVarTypes[type] << std::endl;
    }

    void PartialPlanWriter::outputConstraint(const ConstraintId &constrId, std::ofstream &constrOut, 
																						 std::ofstream &cvmOut) {
      constrOut << constrId->getKey() << TAB << ppId << TAB << constrId->getName().toString() 
								<< TAB << ATEMPORAL << std::endl;
      std::vector<ConstrainedVariableId>::const_iterator it =
				constrId->getScope().begin();
      for(; it != constrId->getScope().end(); ++it) {
				cvmOut << constrId->getKey() << TAB << (*it)->getKey() << TAB << ppId << std::endl;
      }
    }

    void PartialPlanWriter::outputRuleInstance(const RuleInstanceId &ruleId,
                                               std::ofstream &ruleInstanceOut,
                                               std::ofstream &varOut,
                                               std::ofstream &rismOut) {

      ruleInstanceOut << ruleId->getKey() << TAB << ppId << TAB << seqId
                      << TAB << ruleId->getRule()->getKey()
                      << TAB << ruleId->getToken()->getKey() << TAB;

      /*SlaveTokenIds*/
      std::set<TokenId> slaves;
      std::set<ConstrainedVariableId> vars;
      buildSlaveAndVarSets(slaves, vars, ruleId);
      if(slaves.empty()) {
        ruleInstanceOut << SNULL << TAB;
      }
      else {
        for(std::set<TokenId>::const_iterator it = slaves.begin();
            it != slaves.end(); ++it) {
          TokenId slaveToken = *it;
          if(slaveToken.isValid())
            ruleInstanceOut << slaveToken->getKey() << COMMA;
          rismOut << ruleId->getKey() <<TAB << slaveToken->getKey() << TAB << ppId << std::endl;
        }
        ruleInstanceOut << TAB;
      }

      /* gaurd and local variables */
      if(vars.empty()) {
        ruleInstanceOut << SNULL;
      }
      else {
        for(std::set<ConstrainedVariableId>::const_iterator it = vars.begin();
            it != vars.end(); ++it) {
          ConstrainedVariableId localVar = *it;
          ruleInstanceOut << localVar->getKey() << COMMA;
          outputConstrVar(localVar, ruleId->getKey(), I_RULE, varOut);
        }
      }
      ruleInstanceOut << std::endl;
    }

    void PartialPlanWriter::buildSlaveAndVarSets(std::set<TokenId> &tokSet,
                                                 std::set<ConstrainedVariableId> &varSet,
                                                 const RuleInstanceId &ruleId) {
      std::vector<TokenId> tokVec = ruleId->getSlaves();
      for(std::vector<TokenId>::const_iterator tempIt = tokVec.begin(); tempIt != tokVec.end(); ++tempIt)
        if(!((*tempIt).isNoId()))
          tokSet.insert(*tempIt);

      for(std::vector<ConstrainedVariableId>::const_iterator varIt = ruleId->getVariables().begin();
          varIt != ruleId->getVariables().end(); ++varIt) {
        ConstrainedVariableId var = *varIt;
        if(RuleInstanceId::convertable(var->getParent())) {
          varSet.insert(var);
        }
      }
      for(std::vector<RuleInstanceId>::const_iterator ruleIt = ruleId->getChildRules().begin();
          ruleIt != ruleId->getChildRules().end(); ++ruleIt) {
        RuleInstanceId rid = *ruleIt;
        buildSlaveAndVarSets(tokSet, varSet, rid);
      }
    }

    void PartialPlanWriter::outputInstant(const InstantId &instId, const int resId, 
                                          std::ofstream &instOut) {
      instOut << ppId << TAB << resId << TAB << instId->getKey() << TAB << instId->getTime() 
              << TAB << instId->getLevelMin() << TAB << instId->getLevelMax() << TAB;
      const TransactionSet &transactions = instId->getTransactions();
      for(TransactionSet::const_iterator transIt = transactions.begin();
          transIt != transactions.end(); ++transIt) {
        TransactionId trans = *transIt;
        instOut << trans->getKey() << COMMA;
      }
      instOut << std::endl;
    }

//     void PartialPlanWriter::outputDecision(const DecisionPointId &dp, std::ofstream &decOut) {
//       int type = 0;
//       int isUnit = 0;
//       if((!dp.isValid()) || dp.isNoId())
//         return;
//       if(ObjectDecisionPointId::convertable(dp))
//         type = D_OBJECT;
//       else if(TokenDecisionPointId::convertable(dp)) {
//         type = D_TOKEN;
//         TokenDecisionPointId &tdp = (TokenDecisionPointId &) dp;
//         if(tdp->getToken()->getState()->lastDomain().isSingleton())
//           isUnit = 1;
//       }
//       else if(ConstrainedVariableDecisionPointId::convertable(dp)) {
//         type = D_VARIABLE;
//         ConstrainedVariableDecisionPointId &vdp = (ConstrainedVariableDecisionPointId &)dp;
//         if((isCompatGuard(vdp->getVariable()) &&
//             !vdp->getVariable()->specifiedDomain().isSingleton()) ||
//            !vdp->getVariable()->lastDomain().isSingleton())
//           isUnit = 1;
//       }
//       else if(ResourceFlawDecisionPointId::convertable(dp))
//         type = D_RESOURCE;
//       else
//         type = D_ERROR;
		
//       decOut << ppId << TAB << dp->getKey() << TAB << type << TAB << dp->getEntityKey() << TAB << isUnit << TAB;

//       decOut << std::endl;
//     }

    const std::string PartialPlanWriter::getUpperBoundStr(IntervalDomain &dom) const {
      if(dom.isNumeric()) {
        if((int) dom.getUpperBound() == PLUS_INFINITY)
          return PINFINITY;
        else if((int) dom.getUpperBound() == MINUS_INFINITY)
          return NINFINITY;
        else {
          std::stringstream stream;
          stream <<  dom.getUpperBound();
          return std::string(stream.str());
        }
      }
      else if(LabelStr::isString((int)dom.getUpperBound())) {
        LabelStr label((int)dom.getUpperBound());
        return label.toString();
      }
      else if(dom.isBool()) {
        if (dom.getUpperBound() == 0)
          return std::string("false");
        return std::string("true");
      }
      else {
        return ObjectId(dom.getUpperBound())->getName().toString();
      }
      return std::string("");
    }

    const std::string PartialPlanWriter::getLowerBoundStr(IntervalDomain &dom) const {
      
      if(dom.isNumeric()) {
        if((int)dom.getLowerBound() == PLUS_INFINITY)
          return PINFINITY;
        else if((int) dom.getLowerBound() == MINUS_INFINITY)
          return NINFINITY;
        std::stringstream stream;
        stream << dom.getLowerBound();
        return std::string(stream.str());
      }
      else if(LabelStr::isString((int)dom.getLowerBound())) {
        LabelStr label((int)dom.getLowerBound());
        return label.toString();
      }
      else if(dom.isBool()) {
        if (dom.getLowerBound() == 0)
          return std::string("false");
        return std::string("true");
      }
      else {
        return ObjectId(dom.getLowerBound())->getName().toString();
      }
      return std::string("");
    }
  
    const std::string PartialPlanWriter::getEnumerationStr(EnumeratedDomain &edom) const {
      std::stringstream stream;
      std::list<double> enumeration;
      EnumeratedDomain dom(edom);
      if(dom.isOpen()) {
        dom.close();
      }
      if(dom.isInfinite()) {
        return "-Infinity +Infinity";
      }
      if(dom.isEmpty()) {
        return "empty";
      }
      else {
        dom.getValues(enumeration);
      }
      for(std::list<double>::iterator it = enumeration.begin(); it != enumeration.end(); ++it) {
        if(dom.isNumeric()) {
          if((int) (*it) == PLUS_INFINITY)
            stream << PINFINITY;
          else if((int) (*it) == MINUS_INFINITY)
            stream << NINFINITY;
          else
            stream << (int)(*it) << " ";
        }
        else if(LabelStr::isString(*it)) {
          LabelStr label(*it);
          stream << label.toString() << " ";
        }
        else if(dom.isBool()) {
          if ((*it) == 0)
            stream << "false" << " ";
          stream << "true" << " ";
        }
        else {
          stream << ObjectId(*it)->getName().toString() << " ";
        }
      }
      if(streamIsEmpty(stream)) {
				return "bugStr";
      }
      return std::string(stream.str());
    }

    const bool PartialPlanWriter::isCompatGuard(const ConstrainedVariableId &var) const {
      std::set<ConstraintId> constrs;
      var->constraints(constrs);
      for(std::set<ConstraintId>::const_iterator it = constrs.begin();
          it != constrs.end(); ++it)
        if((*it)->getName() == LabelStr("RuleVariableListener"))
          return true;
      return false;
    }

    void PartialPlanWriter::condWrite(const LabelStr& trans) {
      if(isStep(trans)) {
        writeCounter++;
        if(noFullWrite == 0) {
          if(writeCounter >= stepsPerWrite) {
            write();
            nstep++;
            transactionList->clear();
            numTransactions = 0;
            writeCounter = 0;
          }
        }
        else {
          if(writeStep == 1) {
            writeStatsAndTransactions();
            transactionList->clear();
            numTransactions = 0;
            writeCounter = 0;
            nstep++;
          }
        }
      }
    }
  
    /****From PlanDatabaseListener****/

    void PartialPlanWriter::notifyAdded(const ObjectId &objId) {
      if(stepsPerWrite && Transaction::isAllowed(OBJECT_CREATED)) {
        transactionList->push_back(Transaction(OBJECT_CREATED, objId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               objId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRemoved(const ObjectId &objId) {
      if(stepsPerWrite && Transaction::isAllowed(OBJECT_DELETED)) {        
        transactionList->push_back(Transaction(OBJECT_DELETED, objId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               objId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyAdded(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_CREATED)) {
        transactionList->push_back(Transaction(TOKEN_CREATED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep, 
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyAdded(const ObjectId &objId, const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_ADDED_TO_OBJECT)) {
        transactionList->push_back(Transaction(TOKEN_ADDED_TO_OBJECT, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep, 
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyClosed(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_CLOSED)) {
        transactionList->push_back(Transaction(TOKEN_CLOSED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyActivated(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_ACTIVATED)) {
        transactionList->push_back(Transaction(TOKEN_ACTIVATED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyDeactivated(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_DEACTIVATED)) {
        transactionList->push_back(Transaction(TOKEN_DEACTIVATED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyMerged(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_MERGED)) {
        transactionList->push_back(Transaction(TOKEN_MERGED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifySplit(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_SPLIT)) {
        transactionList->push_back(Transaction(TOKEN_SPLIT, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRejected(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_REJECTED)) {
        transactionList->push_back(Transaction(TOKEN_REJECTED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyReinstated(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_REINSTATED)) {
        transactionList->push_back(Transaction(TOKEN_REINSTATED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }


    void PartialPlanWriter::notifyConstrained(const ObjectId &objId, const TokenId &tokId,
                                              const TokenId &successor) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_INSERTED)) {
        transactionList->push_back(Transaction(TOKEN_INSERTED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
      }
    }

    void PartialPlanWriter::notifyRemoved(const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_DELETED)) {
				transactionList->push_back(Transaction(TOKEN_DELETED, tokId->getKey(), UNKNOWN, 
                                               transactionId++, seqId, nstep, 
                                               tokId->getPredicateName().toString()));
				numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRemoved(const ObjectId &objId, const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_REMOVED)) {
				transactionList->push_back(Transaction(TOKEN_REMOVED, tokId->getKey(), UNKNOWN, 
                                               transactionId++, seqId, nstep, 
                                               tokId->getPredicateName().toString()));
				numTransactions++;
      }
    }

    void PartialPlanWriter::notifyFreed(const ObjectId &objId, const TokenId &tokId) {
      if(stepsPerWrite && Transaction::isAllowed(TOKEN_FREED)) {
        transactionList->push_back(Transaction(TOKEN_FREED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    /****From ConstraintEngineListener****/
  
    void PartialPlanWriter::notifyAdded(const ConstraintId &constrId) {
      if(stepsPerWrite && Transaction::isAllowed(CONSTRAINT_CREATED)) {
        transactionList->push_back(Transaction(CONSTRAINT_CREATED, constrId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep, 
                                               constrId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRemoved(const ConstraintId &constrId) {
      if(stepsPerWrite && Transaction::isAllowed(CONSTRAINT_DELETED)) {
        transactionList->push_back(Transaction(CONSTRAINT_DELETED, constrId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep, 
                                               constrId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyExecuted(const ConstraintId &constrId) {
      if(stepsPerWrite && Transaction::isAllowed(CONSTRAINT_EXECUTED)) {
        transactionList->push_back(Transaction(CONSTRAINT_EXECUTED, constrId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               constrId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyAdded(const ConstrainedVariableId &varId) {
      if(stepsPerWrite && Transaction::isAllowed(VAR_CREATED)) {
        transactionList->push_back(Transaction(VAR_CREATED, varId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep, 
                                               getVarInfo(varId)));
      }
    }

    void PartialPlanWriter::notifyRemoved(const ConstrainedVariableId &varId) {
      if(stepsPerWrite && Transaction::isAllowed(VAR_DELETED)) {
        transactionList->push_back(Transaction(VAR_DELETED, varId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep, 
                                               getVarInfo(varId)));
      }
    }

    void PartialPlanWriter::notifyChanged(const ConstrainedVariableId &varId, 
                                          const DomainListener::ChangeType &changeType) {
      if(stepsPerWrite) {
        switch(changeType) {
        case DomainListener::RELAXED:
          if(Transaction::isAllowed(VAR_DOMAIN_RELAXED))
            transactionList->push_back(Transaction(VAR_DOMAIN_RELAXED, varId->getKey(), SYSTEM,
                                                   transactionId++, seqId, nstep, getVarInfo(varId)));
          break;
        case DomainListener::RESET:
          if(Transaction::isAllowed(VAR_DOMAIN_RESET))
            transactionList->push_back(Transaction(VAR_DOMAIN_RESET, varId->getKey(), USER,
                                                   transactionId++, seqId, nstep, getVarInfo(varId)));
          break;
        case DomainListener::VALUE_REMOVED:
          if(Transaction::isAllowed(VAR_DOMAIN_VALUE_REMOVED))
            transactionList->push_back(Transaction(VAR_DOMAIN_VALUE_REMOVED, varId->getKey(), 
                                                   UNKNOWN, transactionId++, seqId, nstep, 
                                                   getVarInfo(varId)));
          break;
        case DomainListener::BOUNDS_RESTRICTED:
          if(Transaction::isAllowed(VAR_DOMAIN_BOUNDS_RESTRICTED))
            transactionList->push_back(Transaction(VAR_DOMAIN_BOUNDS_RESTRICTED, varId->getKey(),
                                                   UNKNOWN, transactionId++, seqId, nstep, 
                                                   getVarInfo(varId)));
          break;
        case DomainListener::LOWER_BOUND_INCREASED:
          if(Transaction::isAllowed(VAR_DOMAIN_LOWER_BOUND_INCREASED))
            transactionList->push_back(Transaction(VAR_DOMAIN_LOWER_BOUND_INCREASED, varId->getKey(),
                                                   UNKNOWN, transactionId++, seqId, nstep, 
                                                   getVarInfo(varId)));
          break;
        case DomainListener::UPPER_BOUND_DECREASED:
          if(Transaction::isAllowed(VAR_DOMAIN_UPPER_BOUND_DECREASED))
            transactionList->push_back(Transaction(VAR_DOMAIN_UPPER_BOUND_DECREASED, varId->getKey(),
                                                   UNKNOWN, transactionId++, seqId, nstep, 
                                                   getVarInfo(varId)));
          break;
        case DomainListener::RESTRICT_TO_SINGLETON:
          if(Transaction::isAllowed(VAR_DOMAIN_RESTRICT_TO_SINGLETON))
            transactionList->push_back(Transaction(VAR_DOMAIN_RESTRICT_TO_SINGLETON, varId->getKey(),
                                                   SYSTEM, transactionId++, seqId, nstep, 
                                                   getVarInfo(varId)));
          break;
        case DomainListener::SET:
          if(Transaction::isAllowed(VAR_DOMAIN_SET))
            transactionList->push_back(Transaction(VAR_DOMAIN_SET, varId->getKey(), UNKNOWN,
                                                   transactionId++, seqId, nstep, getVarInfo(varId)));
          break;
        case DomainListener::SET_TO_SINGLETON:
          if(Transaction::isAllowed(VAR_DOMAIN_SET_TO_SINGLETON))
            transactionList->push_back(Transaction(VAR_DOMAIN_SET_TO_SINGLETON, varId->getKey(),
                                                   USER, transactionId++, seqId, nstep, getVarInfo(varId)));
          break;
        case DomainListener::EMPTIED:
          if(Transaction::isAllowed(VAR_DOMAIN_EMPTIED))
            transactionList->push_back(Transaction(VAR_DOMAIN_EMPTIED, varId->getKey(), SYSTEM,
                                                   transactionId++, seqId, nstep, getVarInfo(varId)));
          break;
        case DomainListener::CLOSED:
          if(Transaction::isAllowed(VAR_DOMAIN_CLOSED))
            transactionList->push_back(Transaction(VAR_DOMAIN_CLOSED, varId->getKey(), SYSTEM,
                                                   transactionId++, seqId, nstep, getVarInfo(varId)));
          break;
        default:
          break;
        }
      }
    }

    void PartialPlanWriter::notifyExecuted(const RuleInstanceId &ruleId) {
      if(stepsPerWrite && Transaction::isAllowed(RULE_EXECUTED)) {
        std::stringstream info;
        info << ruleId->getRule()->getKey() << COMMA << ruleId->getToken()->getKey();
        transactionList->push_back(Transaction(RULE_EXECUTED, ruleId->getKey(), SYSTEM,
                                               transactionId++, seqId, nstep,
                                               std::string(info.str())));
        numTransactions++;
      }
    }
    
    void PartialPlanWriter::notifyUndone(const RuleInstanceId &ruleId) {
      if(stepsPerWrite && Transaction::isAllowed(RULE_UNDONE)) {
        std::stringstream info;
        info << ruleId->getRule()->getKey() << COMMA << ruleId->getToken()->getKey();
        transactionList->push_back(Transaction(RULE_UNDONE, ruleId->getKey(), SYSTEM,
                                               transactionId++, seqId, nstep,
                                               std::string(info.str())));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyPropagationCommenced(void) {
      if(stepsPerWrite && Transaction::isAllowed(PROPAGATION_COMMENCED)) {
        transactionList->push_back(Transaction(PROPAGATION_COMMENCED, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyPropagationCompleted(void) {
      if(stepsPerWrite && Transaction::isAllowed(PROPAGATION_COMPLETED)) {
        transactionList->push_back(Transaction(PROPAGATION_COMPLETED, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
      condWrite(PROPAGATION_COMPLETED);
    }
  
    void PartialPlanWriter::notifyPropagationPreempted(void) {
      if(stepsPerWrite && Transaction::isAllowed(PROPAGATION_PREEMPTED)) {
        transactionList->push_back(Transaction(PROPAGATION_PREEMPTED, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
      condWrite(PROPAGATION_PREEMPTED);
    }

    void PartialPlanWriter::notifyStepSucceeded() {
      if(stepsPerWrite && Transaction::isAllowed(STEP_SUCCEEDED)) {
        transactionList->push_back(Transaction(STEP_SUCCEEDED, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
      condWrite(STEP_SUCCEEDED);
    }

    void PartialPlanWriter::notifyStepFailed() {
      if(stepsPerWrite && Transaction::isAllowed(STEP_FAILED)) {
        transactionList->push_back(Transaction(STEP_FAILED, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
      condWrite(STEP_FAILED);
    }

    void PartialPlanWriter::notifyCompleted() {
      if(stepsPerWrite && Transaction::isAllowed(PLAN_FOUND)) {
        transactionList->push_back(Transaction(PLAN_FOUND, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
      condWrite(PLAN_FOUND);
    }

    void PartialPlanWriter::notifyExhausted() {
      if(stepsPerWrite && Transaction::isAllowed(SEARCH_EXHAUSTED)) {
        transactionList->push_back(Transaction(SEARCH_EXHAUSTED, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
      condWrite(SEARCH_EXHAUSTED);
    }
    
    void PartialPlanWriter::notifyTimedOut() {
      if(stepsPerWrite && Transaction::isAllowed(TIMEOUT_REACHED)) {
        transactionList->push_back(Transaction(TIMEOUT_REACHED, -1, SYSTEM,
                                               transactionId++, seqId, nstep, SNULL));
        numTransactions++;
      }
      condWrite(TIMEOUT_REACHED);
    }

    const std::string PartialPlanWriter::getVarInfo(const ConstrainedVariableId &varId) const {
      std::string type, paramName, predName, retval;
      if(varId->getIndex() >= I_STATE && varId->getIndex() <= I_END) {
				type = tokenVarTypes[varId->getIndex()];
      }
      else {
				type = PARAMETER_VAR;
				paramName = varId->getName().toString();
      }
      if(TokenId::convertable(varId->getParent())) {
				predName = ((TokenId &)varId->getParent())->getPredicateName().toString();
      }
      else if(ObjectId::convertable(varId->getParent())) {
        predName = ((ObjectId &)varId->getParent())->getName().toString();
      }
      else {
				predName = "UNKNOWN VARIABLE PARENT";
      }
      retval = type + COMMA + predName + COMMA + paramName + COMMA;
      return retval;
    }

    void PartialPlanWriter::parseConfigFile(std::ifstream& configFile) {
      while(parseSection(configFile)){}
      if(!configFile.eof())
        FatalError("!configFile.eof()", "Error parsing config file.");
    }

    bool PartialPlanWriter::parseSection(std::ifstream& configFile) {
      char buf[PATH_MAX];
      bool retval = false;
      while(!configFile.eof()) {
        configFile.getline(buf, PATH_MAX);
        //std::cerr << "DEBUG:reading buf: " << buf << std::endl;
        if(buf[0] == '#' || buf[0] == ' ' || buf[0] == '\n')
          continue;
        std::string line = buf;

        debugMsg("PartialPlanWriter:parseSection", "Parsing line '" << line << "'");
        if(line == GENERAL_CONFIG_SECTION) {
          parseGeneralConfigSection(configFile);
          retval = true;
        }
        else if(line == TRANSACTION_CONFIG_SECTION) {
          parseTransactionConfigSection(configFile);
          retval = true;
        }
        else if(line == RULE_CONFIG_SECTION) {
          parseRuleConfigSection(configFile);
          retval = true;
        }
        else
          return false;
      }
      return retval;
    }

    void PartialPlanWriter::parseGeneralConfigSection(std::ifstream& configFile) {
      char buf[PATH_MAX];
      while(!configFile.eof()) {
        configFile.getline(buf, PATH_MAX);
        //        std::cerr << "DEBUG:reading buf: " << buf << std::endl;
        if(buf[0] == '#' || buf[0] == ' ' || buf[0] == '\n')
          continue;
        std::string line = buf;
        debugMsg("PartialPlanWriter:parseGeneralConfigSection", "Parsing line '" << line << "'");
        if(line.find(AUTO_WRITE) != std::string::npos) {
          std::string autoWrite = line.substr(line.find("=")+1);
          //	  std::cerr << " autoWrite " << autoWrite << std::endl;
          noFullWrite = (autoWrite.find("1") != std::string::npos ? 0 : 1);
        }
        else if(line.find(STEPS_PER_WRITE) != std::string::npos) {
          std::string spw = line.substr(line.find("=")+1);
          stepsPerWrite = strtol(spw.c_str(), NULL, 10);
          if(stepsPerWrite < 0)
            FatalError("stepsPerWrite < 0", "StepsPerWrite must be a non-negative value");
          if(stepsPerWrite == LONG_MAX || stepsPerWrite == LONG_MIN)
            FatalErrno();
        }
        else if(line.find(WRITE_FINAL_STEP) != std::string::npos) {
          std::string wfs = line.substr(line.find("=")+1);
          writeStep = (wfs.find("0") != std::string::npos ? 0 : 1);
        }
        else if(line.find(WRITE_DEST) != std::string::npos) {
          std::string wd = line.substr(line.find("=")+1);
          dest = wd;
        }
        else if(line.find(MAX_CHOICES) != std::string::npos) {
          std::string mc = line.substr(line.find("=")+1);
          maxChoices = strtol(mc.c_str(), NULL, 10);
          if(maxChoices < 0)
            FatalError("maxChoices < 0", "MaxChoices must be a non-negative value");
          if(maxChoices == LONG_MAX || maxChoices == LONG_MIN)
            FatalErrno();
        }
        else {
          for(int i = strlen(buf); i >= 0; i--)
            configFile.putback(buf[i]);
          return;
        }
      }
    }

    void PartialPlanWriter::parseTransactionConfigSection(std::ifstream& configFile) {
      char buf[PATH_MAX];
      while(!configFile.eof()) {
        configFile.getline(buf, PATH_MAX);
        //std::cerr << "DEBUG:reading buf: " << buf << std::endl;
        if(buf[0] == '#' || buf[0] == ' ' || buf[0] == '\n')
          continue;
        std::string line = buf;
        debugMsg("PartialPlanWriter:parseTransactionConfigSection", 
                 "Parsing line '" << buf << "'");
        if(line.find("Section") != std::string::npos) {
          for(int i = strlen(buf); i >= 0; i--)
            configFile.putback(buf[i]);
          return;
        }
        if(Transaction::isRegistered(LabelStr(line)))
          Transaction::allowTransaction(LabelStr(line));
        else {
          debugMsg("PartialPlanWriter:parseTransactionConfigSection",
                   "Putting back line '" << line << "'");
          for(int i = strlen(buf); i >= 0; i--)
            configFile.putback(buf[i]);
          return;
        }
      }
    }

    void PartialPlanWriter::parseRuleConfigSection(std::ifstream& configFile) {
      char buf[PATH_MAX];
      while(!configFile.eof()) {
        configFile.getline(buf, PATH_MAX);
        //std::cerr << "DEBUG:reading buf: " << buf << std::endl;
        if(buf[0] == '#' || buf[0] == ' ' || buf[0] == '\n')
          continue;
        std::string line = buf;
        debugMsg("PartialPlanWriter:parseRuleConfigSection",
                 "Parsing line '" << line << "'");
        if(line.find(SOURCE_PATH) != std::string::npos) {
          std::string path = line.substr(line.find("=")+1);
          sourcePaths.push_back(path);
        }
        else {
          for(int i = strlen(buf); i >= 0; i--)
            configFile.putback(buf[i]);
          return;
        }
      }
    }

    void PartialPlanWriter::marksStep(const LabelStr& trans) {
      stepTransactions.push_back(trans);
    }
    void PartialPlanWriter::unmarksStep(const LabelStr& trans) {
      std::vector<LabelStr>::iterator it = stepTransactions.begin();
      for(; it != stepTransactions.end() && (*it) != trans; ++it){}
      if(it != stepTransactions.end())
        stepTransactions.erase(it);
    }
    bool PartialPlanWriter::isStep(const LabelStr& trans) {
      std::vector<LabelStr>::const_iterator it = stepTransactions.begin();
      for(; it != stepTransactions.end() && (*it) != trans; ++it){}
      return it != stepTransactions.end();
    }

    void Transaction::addType(const LabelStr& type) {
      s_types.insert(type);
    }

    void Transaction::addTransaction(const LabelStr& trans, const LabelStr& type, const bool allow) {
      //this message unnecessarily unhelpful because of odd compiler problems
      check_error(s_types.find(type) != s_types.end(), "Can't add transaction of unknown type.");
      s_trans.insert(trans);
      s_transToType[trans] = type;
      s_allowed[trans] = allow;
    }

    bool Transaction::isAllowed(const LabelStr& trans) {
      check_error(s_trans.find(trans) != s_trans.end(), "Can't check allowance of unknown transaction");
      return s_allowed[trans];
    }

    bool Transaction::isRegistered(const LabelStr& trans) {
      return s_trans.find(trans) != s_trans.end();
    }

    void Transaction::allowTransaction(const LabelStr& trans) {
      check_error(s_trans.find(trans) != s_trans.end(), "Can't allow unknown transaction");
      s_allowed[trans] = true;
    }

    void Transaction::disallowTransaction(const LabelStr& trans) {
      check_error(s_trans.find(trans) != s_trans.end(), "Can't disallow unknown transaction");
      s_allowed[trans] = false;
    }

    void Transaction::allowAllTransactions() {
      for(std::map<LabelStr, bool>::iterator it = s_allowed.begin(); it != s_allowed.end(); ++it)
        (*it).second = true;
    }

    void Transaction::disallowAllTransactions() {
      for(std::map<LabelStr, bool>::iterator it = s_allowed.begin(); it != s_allowed.end(); ++it)
        (*it).second = false;
    }

    int Transaction::registeredTransactionCount() {
      return s_trans.size();
    }

    int Transaction::maxTransactionLength() {
      int maxLength = 0;
      for(std::set<LabelStr>::const_iterator it = s_trans.begin(); it != s_trans.end(); ++it)
        maxLength = ((*it).toString().length() > (unsigned int) maxLength ? (*it).toString().length() : maxLength);
      return maxLength;
    }

    //potential memory leak.  user has to delete the array.
    const char** Transaction::getNameStrs() {
      const char** retval = new const char* [s_trans.size()];
      int i = 0;
      for(std::set<LabelStr>::const_iterator it = s_trans.begin(); it != s_trans.end(); ++it, i++)
        retval[i] = (*it).c_str();
      return retval;
    }

    //potential memory leak.  user has to delete the array.
    bool* Transaction::getAllowStates() {
      bool* retval = new bool[s_trans.size()];
      int i = 0;
      for(std::set<LabelStr>::const_iterator it = s_trans.begin(); it != s_trans.end(); ++it, i++)
        retval[i] = s_allowed[(*it)];
      return retval;
    }

    void Transaction::setAllowStates(bool* state, int numTrans) {
      check_error(numTrans = s_trans.size(), "setAllowStates must be used for all transactions.");
      int i = 0;
      for(std::set<LabelStr>::const_iterator it = s_trans.begin(); it != s_trans.end(); ++it, i++)
        s_allowed[(*it)] = state[i];
    }

    void Transaction::write(std::ostream &out, long long int ppId) const {
      out << m_type.toString() << TAB << Transaction::s_transToType[m_type].toString()
          << TAB << m_key << TAB << m_src.toString() << TAB << m_id << TAB << m_stepNum 
          << TAB << m_seqId << TAB << ppId << TAB << m_info << std::endl;
    }
  }
}
