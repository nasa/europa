#ifdef __BEOS__
#include <Path.h>
#endif

#include "PartialPlanWriter.hh"

#include "../ConstraintEngine/Constraint.hh"
#include "../ConstraintEngine/ConstraintEngine.hh"
#include "../ConstraintEngine/ConstraintEngineDefs.hh"
#include "../ConstraintEngine/ConstrainedVariable.hh"
#include "../ConstraintEngine/Domain.hh"
#include "../ConstraintEngine/EnumeratedDomain.hh"
#include "../ConstraintEngine/IntervalDomain.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "../ConstraintEngine/LabelStr.hh"
#include "../ConstraintEngine/Variable.hh"

#include "../PlanDatabase/PlanDatabase.hh"
#include "../PlanDatabase/PlanDatabaseDefs.hh"
#include "../PlanDatabase/Object.hh"
#include "../PlanDatabase/Timeline.hh"
#include "../PlanDatabase/Token.hh"
#include "../PlanDatabase/TokenVariable.hh"

// #include "../Resource/Resource.hh"
// #include "../Resource/ResourceDefs.hh"
// #include "../Resource/Transaction.hh"

#include <exception>
#include <iostream>
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

#define FatalError(s) { std::cerr << "At " << __FILE__ << ":" << __PRETTY_FUNCTION__ << ", line " << __LINE__ << std::endl; std::cerr << (s) << std::endl; exit(-1);}
#define FatalErrno(){FatalError(strerror(errno))}

const char *envStepsPerWrite = "PPW_WRITE_NSTEPS";

const char *envAltWriteDest = "PPW_WRITE_DEST";

const char *envPPWNoWrite = "PPW_DONT_WRITE";

enum transactionTypes {OBJECT_CREATED = 0, OBJECT_DELETED, TOKEN_CREATED, TOKEN_ADDED_TO_OBJECT, 
                       TOKEN_CLOSED, TOKEN_ACTIVATED, TOKEN_DEACTIVATED, TOKEN_MERGED, TOKEN_SPLIT,
                       TOKEN_REJECTED, TOKEN_REINSTATED, TOKEN_DELETED, TOKEN_REMOVED, 
                       TOKEN_INSERTED, TOKEN_FREED, CONSTRAINT_CREATED, CONSTRAINT_DELETED,
                       CONSTRAINT_EXECUTED, VAR_CREATED, VAR_DELETED, VAR_DOMAIN_RELAXED,
                       VAR_DOMAIN_RESTRICTED, VAR_DOMAIN_SPECIFIED, VAR_DOMAIN_RESET, 
                       VAR_DOMAIN_EMPTIED, VAR_DOMAIN_UPPER_BOUND_DECREASED, 
                       VAR_DOMAIN_LOWER_BOUND_INCREASED, VAR_DOMAIN_BOUNDS_RESTRICTED,
                       VAR_DOMAIN_VALUE_REMOVED, VAR_DOMAIN_RESTRICT_TO_SINGLETON,
                       VAR_DOMAIN_SET, VAR_DOMAIN_SET_TO_SINGLETON, VAR_DOMAIN_CLOSED, ERROR};

const int transactionTotal = ERROR + 1;

const char *transactionTypeNames[transactionTotal] = { 
  "OBJECT_CREATED", "OBJECT_DELETED", "TOKEN_CREATED", "TOKEN_ADDED_TO_OBJECT", "TOKEN_CLOSED",
    "TOKEN_ACTIVATED", "TOKEN_DEACTIVATED", "TOKEN_MERGED", "TOKEN_SPLIT", "TOKEN_REJECTED",
    "TOKEN_REINSTATED", "TOKEN_DELETED", "TOKEN_REMOVED", "TOKEN_INSERTED", "TOKEN_FREED",
    "CONSTRAINT_CREATED", "CONSTRAINT_DELETED", "CONSTRAINT_EXECUTED", "VARIABLE_CREATED",
    "VARIABLE_DELETED", "VARIABLE_DOMAIN_RELAXED", "VARIABLE_DOMAIN_RESTRICTED", 
    "VARIABLE_DOMAIN_SPECIFIED", "VARIABLE_DOMAIN_RESET", "VARIABLE_DOMAIN_EMPTIED", 
    "VARIABLE_DOMAIN_UPPER_BOUND_DECREASED", "VARIABLE_DOMAIN_LOWER_BOUND_INCREASED", 
    "VARIABLE_DOMAIN_BOUNDS_RESTRICTED", "VARIABLE_DOMAIN_VALUE_REMOVED", 
    "VARIABLE_DOMAIN_RESTRICT_TO_SINGLETON", "VARIABLE_DOMAIN_SET", 
    "VARIABLE_DOMAIN_SET_TO_SINGLETON", "VARIABLE_DOMAIN_CLOSED", "ERROR"};

const char *sourceTypeNames[3] = {"SYSTEM", "USER", "UNKNOWN"};

enum sourceTypes {SYSTEM = 0, USER, UNKNOWN};

const std::string DURATION_VAR("DURATION_VAR");
const std::string END_VAR("END_VAR");
const std::string START_VAR("START_VAR");
const std::string STATE_VAR("STATE_VAR");
const std::string OBJECT_VAR("OBJECT_VAR");
const std::string PARAMETER_VAR("PARAMETER_VAR");
const std::string MEMBER_VAR("MEMBER_VAR");

const std::string tokenVarTypes[7] = 
{STATE_VAR, OBJECT_VAR, DURATION_VAR, START_VAR, END_VAR, PARAMETER_VAR, MEMBER_VAR};

enum varTypes {I_STATE = 0, I_OBJECT, I_DURATION, I_START, I_END, I_PARAMETER, I_MEMBER};
enum objectTypes {O_OBJECT = 0, O_TIMELINE, O_RESOURCE};
enum tokenTypes {T_INTERVAL = 0, T_TRANSACTION};

#define TAB "\t"
#define COLON ":"
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
const std::string PARTIAL_PLAN(".partialPlan");
const std::string OBJECTS(".objects");
const std::string TOKENS(".tokens");
const std::string TOKEN_RELATIONS(".tokenRelations");
const std::string VARIABLES(".variables");
const std::string CONSTRAINTS(".constraints");
const std::string CONSTRAINT_VAR_MAP(".constraintVarMap");
const std::string INSTANTS(".instants");
const std::string E_DOMAIN("E");
const std::string I_DOMAIN("I");
const std::string CAUSAL("CAUSAL");
const std::string ENUM_DOMAIN("EnumeratedDomain");
const std::string INT_DOMAIN("IntervalDomain");

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

namespace Prototype {
  namespace PlanWriter {
    inline long long int timeval2Id(const struct timeval &currTime) {
      return (((long long int) currTime.tv_sec) * 1000) + (currTime.tv_usec / 1000);
    }

    PartialPlanWriter::PartialPlanWriter(const PlanDatabaseId &planDb, 
					 const ConstraintEngineId &ceId2) 
      : PlanDatabaseListener(planDb), ConstraintEngineListener(ceId2) {
      nstep = 0;
      struct timeval currTime;
      if(gettimeofday(&currTime, NULL)) {
	FatalError("Failed to get current time.");
      }
      seqId = timeval2Id(currTime);
      pdbId = const_cast<PlanDatabaseId *> (&planDb);
      ceId = const_cast<ConstraintEngineId *> (&ceId2);
      transactionId = writeCounter = 0;
      transactionList = new std::list<Transaction>();
    
      char *spw = getenv(envStepsPerWrite);
      if(spw == NULL) {
	stepsPerWrite = 0;
      }
      else {
	stepsPerWrite = (int) strtol(spw, (char **) NULL, 10);
	if((stepsPerWrite == 0 && errno == EINVAL) || stepsPerWrite == INT_MAX) {
	  FatalErrno();
	}
      }
      dest = "./plans";
      char *altDest = getenv(envAltWriteDest);
      if(altDest != NULL) {
	dest = std::string(altDest);
      }

      char *dontWrite = getenv(envPPWNoWrite);
      if(dontWrite == NULL) {
	noWrite = 0;
      }
      else {
	noWrite = (int) strtol(spw, (char **) NULL, 10);
	if((noWrite == 0  && errno == EINVAL) || stepsPerWrite == INT_MAX) {
	  FatalErrno();
	}
      }
    
      char *destBuf = new char[PATH_MAX];
      if(realpath(dest.c_str(), destBuf) == NULL && stepsPerWrite != 0) {
	if(mkdir(destBuf, 0777) && errno != EEXIST) {
	  std::cerr << "Failed to make destination directory " << dest << std::endl;
	  FatalErrno();
	}
      }
      realpath(dest.c_str(), destBuf);
      dest = destBuf;
      delete [] destBuf;
      dest += "/";

      char timestr[NBBY * sizeof(seqId) * 28/93 + 4];
      sprintf(timestr, "%lld", seqId);
      std::string modelName = (*pdbId)->getSchema()->getName().toString();
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
	  std::cerr << "Failed to make directory " << dest << std::endl;
	  FatalErrno();
	}
	dest += seqName;
	dest += timestr;
	if(mkdir(dest.c_str(), 0777) && errno != EEXIST) {
	  std::cerr << "Failed to make directory " << dest << std::endl;
	  FatalErrno();
	}
	std::string ppStats(dest + PARTIAL_PLAN_STATS);
	std::string ppTransactions(dest + TRANSACTIONS);
	std::string seqStr(dest + SEQUENCE);
	std::ofstream seqOut(seqStr.c_str());
	if(!seqOut) {
	  std::cerr << "Failed to open " << seqStr << std::endl;
	  FatalErrno();
	}
	seqOut << dest << TAB << seqId << std::endl;
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
  
    PartialPlanWriter::~PartialPlanWriter(void) {
      if(stepsPerWrite) {
        //write();
	transOut->close();
	statsOut->close();
	delete transOut;
	delete statsOut;
      }
    }
  
    void PartialPlanWriter::write(void) {
      if(!(*ceId)->constraintConsistent()) {
	FatalError("Attempted to write inconsistant database.");
      }
      if(!transOut || !statsOut)
        return;

      ppId = 0LL;
      struct timeval currTime;
      if(gettimeofday(&currTime, NULL)) {
	FatalError("Failed to get current time.");
      }
      ppId = timeval2Id(currTime);

      numTokens = numVariables = numConstraints = numTransactions = 0;
      tokenRelationId = 1;

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

      ppOut << stepnum << TAB << ppId << TAB << (*pdbId)->getSchema()->getName().toString()
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

      std::string ppTokRel = ppDest + SLASH + stepnum + TOKEN_RELATIONS;
      std::ofstream tokRelOut(ppTokRel.c_str());
      if(!tokRelOut) {
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

      const std::set<ConstraintId> &constraints = (*ceId)->getConstraints();
      numConstraints = constraints.size();
      for(std::set<ConstraintId>::const_iterator it = constraints.begin();
	  it != constraints.end(); ++it) {
	outputConstraint(*it, constrOut, cvmOut);
      }

      ObjectSet objects((*pdbId)->getObjects());
      TokenSet tokens((*pdbId)->getTokens());
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
                        tokRelOut, varOut);
            tokens.erase(token);
            TokenSet::const_iterator mergedTokenIterator = 
              token->getMergedTokens().begin();
            for(;mergedTokenIterator != token->getMergedTokens().end(); ++mergedTokenIterator) {
              slotOrder++;
              outputToken(*mergedTokenIterator, T_INTERVAL, slotId, slotIndex, slotOrder,
                          (ObjectId &) tId, tokOut, tokRelOut, varOut);
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
          objOut << rId->getHorizonStart() << COMMA << rId->getHorizonEnd() << COMMA
                 << rId->getInitialCapacity() << COMMA << rId->getLimitMin() << COMMA
                 << rId->getLimitMax() << COMMA;
          std::list<TransactionId> resTrans;
          rId->getTransactions(resTrans, MINUS_INFINITY, PLUS_INFINITY);
          for(std::list<TransactionId>::iterator transIt = resTrans.begin();
              transIt != resTrans.end(); ++transIt) {
            TransactionId trans = *transIt;
            outputToken(trans, T_TRANSACTION, 0, 1, 0, rId, tokOut, tokRelOut, varOut);
            tokens.erase(trans);
          }
          std::list<InstantId> insts;
          rId->getInstants(insts, MINUS_INFINITY, PLUS_INFINITY);
          for(std::list<InstantId>::iterator instIt = insts.begin();
              instIt != insts.end(); ++instIt) {
            InstantId inst = *instIt;
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
	outputToken(token, T_INTERVAL, 0, 0, 0, ObjectId::noId(), tokOut, tokRelOut, varOut);
      }

      (*statsOut) << seqId << TAB << ppId << TAB << nstep << TAB << numTokens << TAB << numVariables
		  << TAB << numConstraints << TAB << numTransactions << std::endl;
      statsOut->flush();
      for(std::list<Transaction>::iterator it = transactionList->begin();
          it != transactionList->end(); ++it) {
	(*it).write((*transOut), ppId);
      }
      objOut.close();
      tokOut.close();
      tokRelOut.close();
      varOut.close();
      constrOut.close();
      cvmOut.close();
      nstep++;
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
                                        std::ofstream &tokRelOut, std::ofstream &varOut) {
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
      if(token->getMaster().isValid()) {
	tokRelOut << ppId << TAB << token->getMaster()->getKey() << TAB 
		  << token->getKey() << TAB << CAUSAL << TAB << tokenRelationId << std::endl;
	tokOut << tokenRelationId << TAB;
	tokenRelationId++;
      }
      else {
	tokOut << SNULL << TAB;
      }

      outputObjVar(token->getObject(), token->getKey(), I_OBJECT, varOut);
      outputIntIntVar(token->getStart(), token->getKey(), I_START, varOut);
      outputIntIntVar(token->getEnd(), token->getKey(), I_END, varOut);
      outputIntIntVar(token->getDuration(), token->getKey(), I_DURATION, varOut);
      outputEnumVar(token->getState(), token->getKey(), I_STATE, varOut);

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
        //std::cerr << "=====>" << trans->getMin() << "->" << trans->getMax() << std::endl;
        //std::cerr << "=====>" << (double) MINUS_INFINITY << "=>" << (double) PLUS_INFINITY << std::endl;
        tokOut << trans->getMin() << COMMA << trans->getMax();
      }
      /*ExtraInfo: SlotOrder*/
      else {
        tokOut << slotOrder;
      }
      
      tokOut << std::endl;
      numTokens++;
    }
  
    void PartialPlanWriter::outputEnumVar(const Id<TokenVariable<EnumeratedDomain> >& enumVar, 
					  //const TokenId &tokId, const int type, 
                                          const int parentId, const int type,
					  std::ofstream &varOut) {
      numVariables++;

      varOut << enumVar->getKey() << TAB << ppId << TAB << /*tokId->getKey()*/parentId << TAB 
	     << enumVar->getName().toString() << TAB;

      varOut << ENUM_DOMAIN << TAB << getEnumerationStr((EnumeratedDomain &) enumVar->lastDomain()) 
	     << TAB << SNULL << TAB << SNULL << TAB << SNULL << TAB;

      varOut << tokenVarTypes[type] << std::endl;
    }
  
    void PartialPlanWriter::outputIntVar(const Id<TokenVariable<IntervalDomain> >& intVar,
					 //const TokenId &tokId, const int type, 
                                         const int parentId, const int type,
					 std::ofstream &varOut) {
      numVariables++;
      varOut << intVar->getKey() << TAB << ppId << TAB << /*tokId->getKey()*/parentId << TAB 
	     << intVar->getName().toString() << TAB;

      varOut << INT_DOMAIN << TAB << SNULL << TAB << REAL_SORT << TAB 
	     << getLowerBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB
	     << getUpperBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB;

      varOut << tokenVarTypes[type] << std::endl;
    }
  
    void PartialPlanWriter::outputIntIntVar(const Id<TokenVariable<IntervalIntDomain> >& intVar,
					    //const TokenId &tokId, const int type, 
                                            const int parentId, const int type,
					    std::ofstream &varOut) {
      numVariables++;

      varOut << intVar->getKey() << TAB << ppId << TAB << /*tokId->getKey()*/parentId << TAB 
	     << intVar->getName().toString() << TAB;
    
      varOut << INT_DOMAIN << TAB << SNULL << TAB << INTEGER_SORT << TAB 
	     << getLowerBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB
	     << getUpperBoundStr((IntervalDomain &)intVar->lastDomain()) << TAB;
    
      varOut << tokenVarTypes[type] << std::endl;
    }

    void PartialPlanWriter::outputObjVar(const ObjectVarId& objVar,
					 //const TokenId &tokId, const int type, 
                                         const int parentId, const int type,
					 std::ofstream &varOut) {
      numVariables++;

      varOut << objVar->getKey() << TAB << ppId << TAB << /*tokId->getKey()*/parentId << TAB 
	     << objVar->getName().toString() << TAB;

      varOut << ENUM_DOMAIN << TAB;

      std::list<ObjectId> objects;
      objVar->getDerivedDomain().getValues(objects);
      for(std::list<ObjectId>::iterator it = objects.begin(); it != objects.end(); ++it) {
	varOut << (*it)->getName().toString() << " ";
      }

      varOut << TAB << SNULL << TAB << SNULL << TAB << SNULL << TAB;

      varOut << tokenVarTypes[type] << std::endl;
    }
  
    void PartialPlanWriter::outputConstrVar(const ConstrainedVariableId &otherVar, 
					    //const TokenId &tokId, const int type, 
                                            const int parentId, const int type,
					    std::ofstream &varOut) {
      numVariables++;

      varOut << otherVar->getKey() << TAB << ppId << TAB << /*tokId->getKey()*/parentId << TAB 
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
	std::cerr << " I don't know what my domain is!" << std::endl;
	exit(-1);
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

    const std::string PartialPlanWriter::getUpperBoundStr(IntervalDomain &dom) const {
      if(dom.isNumeric()) {
        if((int) dom.getUpperBound() == PLUS_INFINITY)
          return PINFINITY;
        else if((int) dom.getUpperBound() == MINUS_INFINITY)
          return NINFINITY;
        else {
          std::stringstream stream;
          stream << (int) dom.getUpperBound();
          return std::string(stream.str());
        }
      }
      else if(LabelStr::isString((int)dom.getUpperBound())) {
        LabelStr label((int)dom.getUpperBound());
        return label.toString();
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
        stream << (int) dom.getLowerBound();
        return std::string(stream.str());
      }
      else if(LabelStr::isString((int)dom.getLowerBound())) {
        LabelStr label((int)dom.getLowerBound());
        return label.toString();
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
        else {
          stream << ObjectId(*it)->getName().toString() << " ";
        }
      }
      if(streamIsEmpty(stream)) {
	return "bugStr";
      }
      return std::string(stream.str());
    }
  
    /****From PlanDatabaseListener****/

    void PartialPlanWriter::notifyAdded(const ObjectId &objId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(OBJECT_CREATED, objId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               objId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRemoved(const ObjectId &objId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(OBJECT_DELETED, objId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               objId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyAdded(const TokenId &tokId) {
      if(stepsPerWrite) {
	transactionList->push_back(Transaction(TOKEN_CREATED, tokId->getKey(), UNKNOWN,
					       transactionId++, seqId, nstep, 
					       tokId->getPredicateName().toString()));
	numTransactions++;
      }
    }

    void PartialPlanWriter::notifyAdded(const ObjectId &objId, const TokenId &tokId) {
      if(stepsPerWrite) {
 	transactionList->push_back(Transaction(TOKEN_ADDED_TO_OBJECT, tokId->getKey(), UNKNOWN,
 					       transactionId++, seqId, nstep, 
 					       tokId->getPredicateName().toString()));
 	numTransactions++;
      }
    }

    void PartialPlanWriter::notifyClosed(const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_CLOSED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyActivated(const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_ACTIVATED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyDeactivated(const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_DEACTIVATED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyMerged(const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_MERGED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifySplit(const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_SPLIT, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRejected(const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_REJECTED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyReinstated(const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_REINSTATED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }


    void PartialPlanWriter::notifyConstrained(const ObjectId &objId, const TokenId &tokId,
                                              const TokenId &successor) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_INSERTED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
      }
    }

    void PartialPlanWriter::notifyRemoved(const TokenId &tokId) {
      if(stepsPerWrite) {
	transactionList->push_back(Transaction(TOKEN_DELETED, tokId->getKey(), UNKNOWN, 
                                               transactionId++, seqId, nstep, 
                                               tokId->getPredicateName().toString()));
	numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRemoved(const ObjectId &objId, const TokenId &tokId) {
      if(stepsPerWrite) {
	transactionList->push_back(Transaction(TOKEN_REMOVED, tokId->getKey(), UNKNOWN, 
                                               transactionId++, seqId, nstep, 
                                               tokId->getPredicateName().toString()));
	numTransactions++;
      }
    }

    void PartialPlanWriter::notifyFreed(const ObjectId &objId, const TokenId &tokId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(TOKEN_FREED, tokId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               tokId->getPredicateName().toString()));
        numTransactions++;
      }
    }

    /****From ConstraintEngineListener****/
  
    void PartialPlanWriter::notifyAdded(const ConstraintId &constrId) {
      if(stepsPerWrite) {
	transactionList->push_back(Transaction(CONSTRAINT_CREATED, constrId->getKey(), UNKNOWN,
					       transactionId++, seqId, nstep, 
					       constrId->getName().toString()));
	numTransactions++;
      }
    }

    void PartialPlanWriter::notifyRemoved(const ConstraintId &constrId) {
      if(stepsPerWrite) {
	transactionList->push_back(Transaction(CONSTRAINT_DELETED, constrId->getKey(), UNKNOWN,
					       transactionId++, seqId, nstep, 
					       constrId->getName().toString()));
	numTransactions++;
      }
    }

    void PartialPlanWriter::notifyExecuted(const ConstraintId &constrId) {
      if(stepsPerWrite) {
        transactionList->push_back(Transaction(CONSTRAINT_EXECUTED, constrId->getKey(), UNKNOWN,
                                               transactionId++, seqId, nstep,
                                               constrId->getName().toString()));
        numTransactions++;
      }
    }

    void PartialPlanWriter::notifyAdded(const ConstrainedVariableId &varId) {
      if(stepsPerWrite) {
	transactionList->push_back(Transaction(VAR_CREATED, varId->getKey(), UNKNOWN,
					       transactionId++, seqId, nstep, 
					       getVarInfo(varId)));
      }
    }

    void PartialPlanWriter::notifyRemoved(const ConstrainedVariableId &varId) {
      if(stepsPerWrite) {
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
	  //VAR_DOMAIN_RELAXED
	  transactionList->push_back(Transaction(VAR_DOMAIN_RELAXED, varId->getKey(), SYSTEM,
						 transactionId++, seqId, nstep, 
						 getLongVarInfo(varId)));
	  break;
	case DomainListener::RESET:
	  transactionList->push_back(Transaction(VAR_DOMAIN_RESET, varId->getKey(), USER,
						 transactionId++, seqId, nstep, 
						 getLongVarInfo(varId)));
	  break;
	case DomainListener::VALUE_REMOVED:
          transactionList->push_back(Transaction(VAR_DOMAIN_VALUE_REMOVED, varId->getKey(), 
                                                 UNKNOWN, transactionId++, seqId, nstep,
                                                 getLongVarInfo(varId)));
          break;
	case DomainListener::BOUNDS_RESTRICTED:
          transactionList->push_back(Transaction(VAR_DOMAIN_BOUNDS_RESTRICTED, varId->getKey(),
                                                 UNKNOWN, transactionId++, seqId, nstep,
                                                 getLongVarInfo(varId)));
          break;
	case DomainListener::LOWER_BOUND_INCREASED:
          transactionList->push_back(Transaction(VAR_DOMAIN_LOWER_BOUND_INCREASED, varId->getKey(),
                                                 UNKNOWN, transactionId++, seqId, nstep,
                                                 getLongVarInfo(varId)));
          break;
	case DomainListener::UPPER_BOUND_DECREASED:
          transactionList->push_back(Transaction(VAR_DOMAIN_UPPER_BOUND_DECREASED, varId->getKey(),
                                                 UNKNOWN, transactionId++, seqId, nstep,
                                                 getLongVarInfo(varId)));
          break;
	case DomainListener::RESTRICT_TO_SINGLETON:
	  //VAR_DOMAIN_RESTRICTED
	  transactionList->push_back(Transaction(VAR_DOMAIN_RESTRICT_TO_SINGLETON, varId->getKey(),
                                                 SYSTEM, transactionId++, seqId, nstep, 
						 getLongVarInfo(varId)));
	  break;
	case DomainListener::SET:
          transactionList->push_back(Transaction(VAR_DOMAIN_SET, varId->getKey(), UNKNOWN,
                                                 transactionId++, seqId, nstep,
                                                 getLongVarInfo(varId)));
          break;
	case DomainListener::SET_TO_SINGLETON:
	  //VAR_DOMAIN_SPECIFIED
	  transactionList->push_back(Transaction(VAR_DOMAIN_SET_TO_SINGLETON, varId->getKey(),
                                                 USER, transactionId++, seqId, nstep, 
						 getLongVarInfo(varId)));
	  break;
	case DomainListener::EMPTIED:
	  //VAR_DOMAIN_EMPTIED
	  transactionList->push_back(Transaction(VAR_DOMAIN_EMPTIED, varId->getKey(), SYSTEM,
						 transactionId++, seqId, nstep, 
						 getLongVarInfo(varId)));
	  break;
	case DomainListener::CLOSED:
          transactionList->push_back(Transaction(VAR_DOMAIN_CLOSED, varId->getKey(), SYSTEM,
                                                 transactionId++, seqId, nstep, 
                                                 getLongVarInfo(varId)));
          break;
	default:
	  break;
	}
      }
    }
  
    void PartialPlanWriter::notifyPropagationCompleted(void) {
      writeCounter++;
      if(writeCounter == stepsPerWrite && noWrite == 0) {
	write();
	transactionList->clear();
	numTransactions = 0;
	writeCounter = 0;
      }
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

    const std::string PartialPlanWriter::getLongVarInfo(const ConstrainedVariableId &varId) const {
      std::string retval = getVarInfo(varId);

      varId->lastDomain();
      if(varId->lastDomain().isEnumerated()) {
	retval += E_DOMAIN + COMMA + 
	  getEnumerationStr((EnumeratedDomain &)varId->lastDomain()) + COMMA;
      }
      else if(varId->lastDomain().isInterval()) {
	retval += I_DOMAIN + COMMA + 
	  getLowerBoundStr((IntervalDomain &)varId->lastDomain()) + SPACE;
	retval += 
	  getUpperBoundStr((IntervalDomain &)varId->lastDomain()) + COMMA;
      }
      if(varId->specifiedDomain().isEnumerated()) {
	retval += E_DOMAIN + COMMA + 
	  getEnumerationStr((EnumeratedDomain &)varId->specifiedDomain()) + COMMA;
      }
      else if(varId->specifiedDomain().isInterval()) {
	retval += I_DOMAIN + COMMA + getLowerBoundStr((IntervalDomain &)varId->specifiedDomain()) + 
	  SPACE;
	retval += getUpperBoundStr((IntervalDomain &)varId->specifiedDomain()) + COMMA;
      }
      return std::string(retval);
    }

    void Transaction::write(std::ostream &out, long long int ppId) const {
      out << transactionTypeNames[transactionType] << TAB << objectKey << TAB
	  << sourceTypeNames[source] << TAB << id << TAB << stepNum << TAB
	  << sequenceId << TAB << ppId << TAB << info << std::endl;
    }
  }
}
