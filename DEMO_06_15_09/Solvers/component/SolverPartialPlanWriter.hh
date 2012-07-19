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

      class PartialPlanWriter {
      public:
	PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &);
	PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &, 
			  const RulesEngineId &);
	PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId&,
			  const RulesEngineId &, SOLVERS::SolverId&);
	virtual ~PartialPlanWriter();
	virtual void write();
	virtual void writeStatistics();
	void marksStep(const LabelStr& trans);
	void unmarksStep(const LabelStr& trans);
	bool isStep(const LabelStr& trans);
	void incrementStep();

	//EUROPA JNI runtime interface functions
	std::string getDest(void);
	void setDest(std::string destPath);
	void addSourcePath(const char* path);
	static int noFullWrite, writeStep;
      protected:
	virtual bool parseSection(std::ifstream& configFile);
	inline long long int getPPId(void){return ppId;}
	long long int ppId;
	std::string dest;
      private:
	class PPWConstraintEngineListener;
	class PPWSearchListener;

	bool destAlreadyInitialized;
	long long int seqId;
	int numTokens, numConstraints, numVariables;
	int stepsPerWrite, nstep, writeCounter, maxChoices;
	double m_writing;
	std::vector<LabelStr> stepTransactions;

	ConstraintEngineId ceId;
	PlanDatabaseId pdbId;
	RulesEngineId reId;

	ConstraintEngineListenerId cel;
	SOLVERS::SearchListenerId sl;

	std::ofstream *statsOut, *ruleInstanceOut;
	std::list<std::string> sourcePaths;

	void allocateListeners();
	void initOutputDestination();
	void parseConfigFile(std::ifstream &);
	void parseGeneralConfigSection(std::ifstream&);
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
	void writeStats(void);
	void collectStats(void);
	void condWrite(const LabelStr& trans);
	const std::string getUpperBoundStr(IntervalDomain &dom) const;
	const std::string getLowerBoundStr(IntervalDomain &dom) const;
	const std::string getEnumerationStr(EnumeratedDomain &dom) const;
	const std::string getVarInfo(const ConstrainedVariableId &) const;
	const bool isCompatGuard(const ConstrainedVariableId &) const;

	/****From ConstraintEngineListener****/
	void notifyPropagationPreempted(void);
	void notifyPropagationCompleted(void);

	/****From SearchListener****/
	void notifyStepSucceeded();
	void notifyStepFailed();
	void notifyRetractSucceeded();
	void notifyRetractNotDone();
	void notifyCompleted();
	void notifyExhausted();
	void notifyTimedOut();

	friend class PPWConstraintEngineListener;
	friend class PPWSearchListener;

	class PPWConstraintEngineListener : public ConstraintEngineListener {
	public:
	  PPWConstraintEngineListener(const ConstraintEngineId &ceId, 
				      PartialPlanWriter *planWriter) : 
	    ConstraintEngineListener(ceId), ppw(planWriter) {
	  }
	protected:
	private:
	  void notifyPropagationPreempted(void){ppw->notifyPropagationPreempted();}
	  void notifyPropagationCompleted(void){ppw->notifyPropagationCompleted();}

	  PartialPlanWriter *ppw;
	};

	class PPWSearchListener : public SOLVERS::SearchListener {
	public:
	  PPWSearchListener(SOLVERS::SolverId& solver, PartialPlanWriter* planWriter) 
	    : SearchListener(), m_solver(solver), ppw(planWriter) {
	    m_solver->addListener(getId());
	  }
	  ~PPWSearchListener(){m_solver->removeListener(getId());};
	  void notifyStepSucceeded(DecisionPointId& dp) {ppw->notifyStepSucceeded();}
	  void notifyStepFailed(DecisionPointId& dp) {ppw->notifyStepFailed();}
	  void notifyRetractSucceeded(DecisionPointId& dp) {ppw->notifyRetractSucceeded();}
	  void notifyRetractNotDone(DecisionPointId& dp) {ppw->notifyRetractNotDone();}
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
	static const LabelStr PROPAGATION_COMMENCED; 
	static const LabelStr PROPAGATION_COMPLETED; 
	static const LabelStr PROPAGATION_PREEMPTED; 

	static const LabelStr STEP_SUCCEEDED;
	static const LabelStr RETRACT_SUCCEEDED;
	static const LabelStr RETRACT_FAILED;
	static const LabelStr STEP_FAILED;
	static const LabelStr PLAN_FOUND;
	static const LabelStr SEARCH_EXHAUSTED;
	static const LabelStr TIMEOUT_REACHED;

	static const LabelStr ERROR;
      };
    }
  }
}

#endif
