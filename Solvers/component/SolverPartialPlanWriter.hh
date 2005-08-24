#ifndef PARTIALPLANWRITER_HH
#define PARTIALPLANWRITER_HH

#include "LabelStr.hh"
#include "PlanDatabaseDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
#include "ResourceDefs.hh"
#include "DomainListener.hh"
#include "PlanDatabaseListener.hh"
#include "ConstraintEngineListener.hh"
#include "RulesEngineListener.hh"
#include "SearchListener.hh"
#include "Solver.hh"

#include <set>
#include <map>
#include <vector>

namespace EUROPA {
  namespace SOLVERS {
    namespace PlanWriter {
      class Transaction {
      public :
	Transaction(const LabelStr& type, int key, const LabelStr& src, int transId, long long int seqid, int nstep,
		    const std::string &info)
	  : m_type(type), m_key(key), m_src(src), m_id(transId), m_stepNum(nstep),
	    m_seqId(seqid), m_info(info) {}
	Transaction(const Transaction &other) : m_type(other.m_type), m_key(other.m_key), m_src(other.m_src),
						m_id(other.m_id), m_stepNum(other.m_stepNum),
						m_seqId(other.m_seqId), m_info(other.m_info) {}
	void write(std::ostream &out, long long int ppId) const;
	static void addType(const LabelStr& type);
	static void addTransaction(const LabelStr& trans, const LabelStr& type, const bool allow = false);
	static void allowTransaction(const LabelStr& trans);
	static void disallowTransaction(const LabelStr& trans);
	static bool isAllowed(const LabelStr& trans);
	static bool isRegistered(const LabelStr& trans);
	static void allowAllTransactions();
	static void disallowAllTransactions();
	static int registeredTransactionCount();
	static int maxTransactionLength();
	static const char** getNameStrs();
	static bool* getAllowStates();
	static void setAllowStates(bool *state, int numTrans); //set all states
      private:
	LabelStr m_type;
	int m_key;
	LabelStr m_src;
	int m_id, m_stepNum;
	long long int m_seqId;
	std::string m_info;
	static std::set<LabelStr> s_types;
	static std::set<LabelStr> s_trans;
	static std::map<LabelStr, bool> s_allowed;
	static std::map<LabelStr, LabelStr> s_transToType;
      };
    
      class PartialPlanWriter {
      public:
	PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &);
	PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &, 
			  const RulesEngineId &);
	PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId&,
			  const RulesEngineId &, SOLVERS::SolverId&);
	virtual ~PartialPlanWriter();
	virtual void write();
	virtual void initTransactions();
	void marksStep(const LabelStr& trans);
	void unmarksStep(const LabelStr& trans);
	bool isStep(const LabelStr& trans);

	//EUROPA JNI runtime interface functions
	std::string getDest(void);
	void setDest(std::string destPath);
	int getNumTransactions(void);
	int getMaxLengthTransactions(void);
	const char** getTransactionNameStrs(void);
	bool* getTransactionFilterStates(void);
	void setTransactionFilterStates(bool*, int);
	static int noFullWrite, writeStep;
      protected:
	virtual bool parseSection(std::ifstream& configFile);
	inline long long int getPPId(void){return ppId;}
	long long int ppId;
	std::list<Transaction> *transactionList;
	std::string dest;
      private:
	class PPWPlanDatabaseListener;
	class PPWConstraintEngineListener;
	class PPWRulesEngineListener;
	class PPWSearchListener;

	bool destAlreadyInitialized;
	long long int seqId;
	int numTokens, numConstraints, numVariables, numTransactions;
	int stepsPerWrite, transactionId, nstep, writeCounter, maxChoices;
	double m_writing;

	ConstraintEngineId ceId;
	PlanDatabaseId pdbId;
	RulesEngineId reId;

	PlanDatabaseListenerId dbl;
	ConstraintEngineListenerId cel;
	RulesEngineListenerId rel;
	SOLVERS::SearchListenerId sl;

	std::ofstream *transOut, *statsOut, *ruleInstanceOut;
	std::list<std::string> sourcePaths;
	std::vector<LabelStr> stepTransactions;

	void allocateListeners();
	void initOutputDestination();
	void parseConfigFile(std::ifstream &);
	void parseGeneralConfigSection(std::ifstream&);
	void parseTransactionConfigSection(std::ifstream&);
	void parseRuleConfigSection(std::ifstream&);
	void commonInit(const PlanDatabaseId& db, const ConstraintEngineId& ce, const RulesEngineId& re);
	void outputObject(const ObjectId &, const int, std::ofstream &, std::ofstream &);
	void outputToken(const TokenId &, const int, const int, const int, const int, 
			 const ObjectId &, std::ofstream &, std::ofstream &);
	void outputStateVar(const Id<TokenVariable<StateDomain> >&, const int, const int,
			    std::ofstream &varOut);
	void outputEnumVar(const Id< TokenVariable<EnumeratedDomain> > &, const int,
			   const int, std::ofstream &);
	void outputIntVar(const Id< TokenVariable<IntervalDomain> > &, const int,
			  const int, std::ofstream &);
	void outputIntIntVar(const Id< TokenVariable<IntervalIntDomain> >&, const int,
			     const int, std::ofstream &);
	void outputObjVar(const ObjectVarId &, const int, const int,
			  std::ofstream &);
	void outputConstrVar(const ConstrainedVariableId &, const int, const int, 
			     std::ofstream &);
	void outputConstraint(const ConstraintId &, std::ofstream &, std::ofstream &);
	void outputInstant(const InstantId &, const int, std::ofstream &);
	void outputRuleInstance(const RuleInstanceId &, std::ofstream &, std::ofstream & , std::ofstream &);
	void buildSlaveAndVarSets(std::set<TokenId> &, std::set<ConstrainedVariableId> &, 
				  const RuleInstanceId &);
	void outputTransactions(std::ofstream *);
	void writeStatsAndTransactions(void);
	void collectStats(void);
	void condWrite(const LabelStr& trans);
	const std::string getUpperBoundStr(IntervalDomain &dom) const;
	const std::string getLowerBoundStr(IntervalDomain &dom) const;
	const std::string getEnumerationStr(EnumeratedDomain &dom) const;
	const std::string getVarInfo(const ConstrainedVariableId &) const;
	const bool isCompatGuard(const ConstrainedVariableId &) const;

	/****From PlanDatabaseListener****/
    
	void notifyAdded(const ObjectId &); //OBJECT_CREATED
	void notifyRemoved(const ObjectId &); //OBJECT_DELETED
	void notifyAdded(const TokenId &); //TOKEN_CREATED
	void notifyAdded(const ObjectId &, const TokenId &);  //TOKEN_ADDED_TO_OBJECT
	void notifyClosed(const TokenId &); //TOKEN_CLOSED
	void notifyActivated(const TokenId &); //TOKEN_ACTIVATED
	void notifyDeactivated(const TokenId &); //TOKEN_DEACTIVATED
	void notifyMerged(const TokenId &); //TOKEN_MERGED
	void notifySplit(const TokenId &); //TOKEN_SPLIT
	void notifyRejected(const TokenId &); //TOKEN_REJECTED
	void notifyReinstated(const TokenId &); //TOKEN_REINSTATED
	void notifyRemoved(const TokenId &); //TOKEN_DELETED
	void notifyRemoved(const ObjectId &, const TokenId &); //TOKEN_REMOVED
	void notifyConstrained(const ObjectId &, const TokenId &, const TokenId &); //TOKEN_INSERTED
	void notifyFreed(const ObjectId &, const TokenId &); //TOKEN_FREED
      

	/****From ConstraintEngineListener****/
	void notifyAdded(const ConstraintId &); //CONSTRAINT_CREATED
	void notifyRemoved(const ConstraintId &); //CONSTRAINT_DELETED
	void notifyExecuted(const ConstraintId &); //CONSTRAINT_EXECUTED
	void notifyAdded(const ConstrainedVariableId &); //VARIABLE_CREATED
	void notifyRemoved(const ConstrainedVariableId &); //VARIABLE_DELETED
	void notifyChanged(const ConstrainedVariableId &, const DomainListener::ChangeType &);
	void notifyPropagationCommenced(void);
	void notifyPropagationPreempted(void);
	void notifyPropagationCompleted(void);

	/****From RulesEngineListener****/
	void notifyExecuted(const RuleInstanceId &);
	void notifyUndone(const RuleInstanceId &);

	/****From SearchListener****/
	void notifyStepSucceeded();
	void notifyStepFailed();
	void notifyCompleted();
	void notifyExhausted();
	void notifyTimedOut();

	friend class PPWPlanDatabaseListener;
	friend class PPWConstraintEngineListener;
	friend class PPWRulesEngineListener;
	friend class PPWSearchListener;

	class PPWPlanDatabaseListener : public PlanDatabaseListener {
	public:
	  PPWPlanDatabaseListener(const PlanDatabaseId &planDb, PartialPlanWriter *planWriter)
	    : PlanDatabaseListener(planDb), ppw(planWriter) {
	  }
	  virtual ~PPWPlanDatabaseListener() {}
	protected:
	private:
	  void notifyAdded(const ObjectId &o){ppw->notifyAdded(o);}
	  void notifyRemoved(const ObjectId &o){ppw->notifyRemoved(o);}
	  void notifyAdded(const TokenId &t){ppw->notifyAdded(t);}
	  void notifyAdded(const ObjectId &o, const TokenId &t){ppw->notifyAdded(o,t);}
	  void notifyClosed(const TokenId &t){ppw->notifyClosed(t);}
	  void notifyActivated(const TokenId &t){ppw->notifyActivated(t);}
	  void notifyDeactivated(const TokenId &t){ppw->notifyDeactivated(t);}
	  void notifyMerged(const TokenId &t){ppw->notifyMerged(t);}
	  void notifySplit(const TokenId &t){ppw->notifySplit(t);}
	  void notifyRejected(const TokenId &t){ppw->notifyRejected(t);}
	  void notifyReinstated(const TokenId &t){ppw->notifyReinstated(t);}
	  void notifyRemoved(const TokenId &t){ppw->notifyRemoved(t);}
	  void notifyRemoved(const ObjectId &o, const TokenId &t){ppw->notifyRemoved(o,t);}
	  void notifyConstrained(const ObjectId &o, const TokenId &t1, const TokenId &t2)
	  {ppw->notifyConstrained(o,t1,t2);}
	  void notifyFreed(const ObjectId &o, const TokenId &t){ppw->notifyFreed(o,t);}

	  PartialPlanWriter *ppw;
	};
      
	class PPWConstraintEngineListener : public ConstraintEngineListener {
	public:
	  PPWConstraintEngineListener(const ConstraintEngineId &ceId, 
				      PartialPlanWriter *planWriter) : 
	    ConstraintEngineListener(ceId), ppw(planWriter) {
	  }
	protected:
	private:
	  void notifyPropagationCommenced(void){ppw->notifyPropagationCommenced();}
	  void notifyPropagationPreempted(void){ppw->notifyPropagationPreempted();}
	  void notifyPropagationCompleted(void){ppw->notifyPropagationCompleted();}
	  void notifyAdded(const ConstraintId &c){ppw->notifyAdded(c);}
	  void notifyRemoved(const ConstraintId &c){ppw->notifyRemoved(c);}
	  void notifyExecuted(const ConstraintId &c){ppw->notifyExecuted(c);}
	  void notifyAdded(const ConstrainedVariableId &v){ppw->notifyAdded(v);}
	  void notifyRemoved(const ConstrainedVariableId &v){ppw->notifyRemoved(v);}
	  void notifyChanged(const ConstrainedVariableId &v, const DomainListener::ChangeType &t)
	  {ppw->notifyChanged(v,t);}

	  PartialPlanWriter *ppw;
	};

	class PPWRulesEngineListener : public RulesEngineListener {
	public:
	  PPWRulesEngineListener(const RulesEngineId &reId, PartialPlanWriter *planWriter) :
	    RulesEngineListener(reId), ppw(planWriter) {
	  }
	protected:
	private:
	  void notifyExecuted(const RuleInstanceId &rule) {ppw->notifyExecuted(rule);}
	  void notifyUndone(const RuleInstanceId &rule) {ppw->notifyUndone(rule);}
	  PartialPlanWriter *ppw;
	};

	class PPWSearchListener : public SOLVERS::SearchListener {
	public:
	  PPWSearchListener(SOLVERS::SolverId& solver, PartialPlanWriter* planWriter) 
	    : SearchListener(), m_solver(solver), ppw(planWriter) {
	    m_solver->addListener(getId());
	  }
	  ~PPWSearchListener(){m_solver->removeListener(getId());};
	  void notifyStepSucceeded() {ppw->notifyStepSucceeded();}
	  void notifyStepFailed() {ppw->notifyStepFailed();}
	  void notifyCompleted() {ppw->notifyCompleted();}
	  void notifyExhausted() {ppw->notifyExhausted();}
	  void notifyTimedOut() {ppw->notifyTimedOut();}
	protected:
	private:
	  SOLVERS::SolverId m_solver;
	  PartialPlanWriter *ppw;
	};

      public:
	//TRANSACTIONS
	static const LabelStr OBJECT_CREATED;
	static const LabelStr OBJECT_DELETED;
	static const LabelStr TOKEN_CREATED; 
	static const LabelStr TOKEN_ADDED_TO_OBJECT; 
	static const LabelStr TOKEN_CLOSED; 
	static const LabelStr TOKEN_ACTIVATED; 
	static const LabelStr TOKEN_DEACTIVATED; 
	static const LabelStr TOKEN_MERGED; 
	static const LabelStr TOKEN_SPLIT;
	static const LabelStr TOKEN_REJECTED; 
	static const LabelStr TOKEN_REINSTATED; 
	static const LabelStr TOKEN_DELETED; 
	static const LabelStr TOKEN_REMOVED; 
	static const LabelStr TOKEN_INSERTED; 
	static const LabelStr TOKEN_FREED; 
	static const LabelStr CONSTRAINT_CREATED; 
	static const LabelStr CONSTRAINT_DELETED;
	static const LabelStr CONSTRAINT_EXECUTED; 
	static const LabelStr VAR_CREATED; 
	static const LabelStr VAR_DELETED; 
	static const LabelStr VAR_DOMAIN_RELAXED;
	static const LabelStr VAR_DOMAIN_RESTRICTED; 
	static const LabelStr VAR_DOMAIN_SPECIFIED; 
	static const LabelStr VAR_DOMAIN_RESET; 
	static const LabelStr VAR_DOMAIN_EMPTIED; 
	static const LabelStr VAR_DOMAIN_UPPER_BOUND_DECREASED; 
	static const LabelStr VAR_DOMAIN_LOWER_BOUND_INCREASED; 
	static const LabelStr VAR_DOMAIN_BOUNDS_RESTRICTED;
	static const LabelStr VAR_DOMAIN_VALUE_REMOVED;
	static const LabelStr VAR_DOMAIN_RESTRICT_TO_SINGLETON;
	static const LabelStr VAR_DOMAIN_SET; 
	static const LabelStr VAR_DOMAIN_SET_TO_SINGLETON; 
	static const LabelStr VAR_DOMAIN_CLOSED; 
	static const LabelStr RULE_EXECUTED;
	static const LabelStr RULE_UNDONE; 
	static const LabelStr PROPAGATION_COMMENCED; 
	static const LabelStr PROPAGATION_COMPLETED; 
	static const LabelStr PROPAGATION_PREEMPTED; 

	static const LabelStr STEP_SUCCEEDED;
	static const LabelStr STEP_FAILED;
	static const LabelStr PLAN_FOUND;
	static const LabelStr SEARCH_EXHAUSTED;
	static const LabelStr TIMEOUT_REACHED;

	//transaction types
	static const LabelStr CREATION;
	static const LabelStr DELETION;
	static const LabelStr ADDITION;
	static const LabelStr REMOVAL;
	static const LabelStr CLOSURE;
	static const LabelStr RESTRICTION;
	static const LabelStr RELAXATION;
	static const LabelStr EXECUTION;
	static const LabelStr SPECIFICATION;
	static const LabelStr UNDO;
	static const LabelStr NONE;

	//transaction sources
	static const LabelStr SYSTEM;
	static const LabelStr USER;
	static const LabelStr UNKNOWN;

	static const LabelStr ERROR;
      };
    }
  }
}

#endif
