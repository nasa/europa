#ifndef PARTIALPLANWRITER_HH
#define PARTIALPLANWRITER_HH

#include <fstream>

#include <list>

#include "CBPlanner.hh"
#include "DecisionManagerListener.hh"
#include "DecisionPoint.hh"
#include "ConstraintEngineListener.hh"
#include "DomainListener.hh"
#include "Entity.hh"
#include "Id.hh"
#include "VariableChangeListener.hh"
#include "PlanDatabaseListener.hh"
#include "Resource.hh"
#include "ResourceDefs.hh"
#include "Transaction.hh"
#include "RulesEngine.hh"
#include "RulesEngineDefs.hh"
#include "RulesEngineListener.hh"
#include "RuleInstance.hh"

namespace Prototype {
  namespace PlanWriter {
    class Transaction {
    public :
      Transaction(int type, int key, int source2, int id2, long long int seqid, int nstep,
		  const std::string &info2)
	: transactionType(type), objectKey(key), source(source2), id(id2), stepNum(nstep),
	  sequenceId(seqid), info(info2) {}
      Transaction(const Transaction &other) : transactionType(other.transactionType), 
	objectKey(other.objectKey), source(other.source), id(other.id), stepNum(other.stepNum),
	sequenceId(other.sequenceId), info(other.info) {}
      void write(std::ostream &out, long long int ppId) const;
    private:
      int transactionType, objectKey, source, id, stepNum;
      long long int sequenceId;
      std::string info;
    };

    class PartialPlanWriter {
    public:
      PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &);
      PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &, 
                        const RulesEngineId &);
			PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &,
												const RulesEngineId &, const CBPlannerId &);
      ~PartialPlanWriter(void);
      //void notifyPropagationCompleted(void);
      void write(void);
    private:
			bool havePlanner;
      long long int seqId, ppId;
      int numTokens, numConstraints, numVariables, numTransactions;
      int stepsPerWrite, transactionId, nstep, tokenRelationId, writeCounter, noWrite, maxChoices;
      ConstraintEngineId *ceId;
      PlanDatabaseId *pdbId;
			RulesEngineId *reId;
			CBPlannerId *plId;
      std::list<Transaction> *transactionList;
      std::ofstream *transOut, *statsOut, *ruleMapOut;
      std::string dest;
      void commonInit(const PlanDatabaseId &, const ConstraintEngineId &);
      void outputObject(const ObjectId &, const int, std::ofstream &, std::ofstream &);
      void outputToken(const TokenId &, const int, const int, const int, const int, 
                       const ObjectId &, std::ofstream &, std::ofstream &, std::ofstream &);
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
			void outputDecision(const DecisionPointId &, std::ofstream &);
      const std::string getUpperBoundStr(IntervalDomain &dom) const;
      const std::string getLowerBoundStr(IntervalDomain &dom) const;
      const std::string getEnumerationStr(EnumeratedDomain &dom) const;
      const std::string getVarInfo(const ConstrainedVariableId &) const;
			const std::string getChoiceInfo(void) const;

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


			/****From DecisionManagerListener****/
			void notifyAssignNextStarted(const DecisionPointId &dec);
			void notifyAssignNextFailed(const DecisionPointId &dec);
			void notifyAssignNextSucceeded(const DecisionPointId &dec);
			void notifyAssignCurrentStarted(const DecisionPointId &dec);
			void notifyAssignCurrentFailed(const DecisionPointId &dec);
			void notifyAssignCurrentSucceeded(const DecisionPointId &dec);
			void notifyRetractStarted(const DecisionPointId &dec);
			void notifyRetractFailed(const DecisionPointId &dec);
			void notifyRetractSucceeded(const DecisionPointId &dec);

      class PPWPlanDatabaseListener;
      class PPWConstraintEngineListener;
      class PPWRulesEngineListener;
			class PPWPlannerListener;

      friend class PPWPlanDatabaseListener;
      friend class PPWConstraintEngineListener;
      friend class PPWRulesEngineListener;
			friend class PPWPlannerListener;

      class PPWPlanDatabaseListener : public PlanDatabaseListener {
      public:
        PPWPlanDatabaseListener(const PlanDatabaseId &planDb, PartialPlanWriter *planWriter)
          : PlanDatabaseListener(planDb), ppw(planWriter) {
        }
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

			class PPWPlannerListener : public DecisionManagerListener {
			public:
				PPWPlannerListener(const CBPlannerId &plannerId, PartialPlanWriter *planWriter) :
					DecisionManagerListener(plannerId->getDecisionManager()), ppw(planWriter) {
				}
				void notifyAssignNextStarted(const DecisionPointId &dec){ppw->notifyAssignNextStarted(dec);}
				void notifyAssignNextFailed(const DecisionPointId &dec){ppw->notifyAssignNextFailed(dec);}
				void notifyAssignNextSucceeded(const DecisionPointId &dec){ppw->notifyAssignNextSucceeded(dec);}
				void notifyAssignCurrentStarted(const DecisionPointId &dec){ppw->notifyAssignCurrentStarted(dec);}
				void notifyAssignCurrentFailed(const DecisionPointId &dec){ppw->notifyAssignCurrentFailed(dec);}
				void notifyAssignCurrentSucceeded(const DecisionPointId &dec){ppw->notifyAssignCurrentSucceeded(dec);}
				void notifyRetractStarted(const DecisionPointId &dec){ppw->notifyRetractStarted(dec);}
				void notifyRetractFailed(const DecisionPointId &dec){ppw->notifyRetractFailed(dec);}
				void notifyRetractSucceeded(const DecisionPointId &dec){ppw->notifyRetractSucceeded(dec);}				
			protected:
			private:
				PartialPlanWriter *ppw;
			};
    };
  }
}
#endif
