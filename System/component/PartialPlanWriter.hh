#ifndef PARTIALPLANWRITER_HH
#define PARTIALPLANWRITER_HH

#include <fstream>

#include <list>

#include "../ConstraintEngine/ConstraintEngineListener.hh"
#include "../ConstraintEngine/DomainListener.hh"
#include "../ConstraintEngine/Entity.hh"
#include "../ConstraintEngine/Id.hh"
#include "../ConstraintEngine/VariableChangeListener.hh"
#include "../PlanDatabase/PlanDatabaseListener.hh"
#include "../Resource/Resource.hh"
#include "../Resource/ResourceDefs.hh"
#include "../Resource/Transaction.hh"
#include "../RulesEngine/RulesEngine.hh"
#include "../RulesEngine/RulesEngineDefs.hh"
#include "../RulesEngine/RulesEngineListener.hh"
#include "../RulesEngine/RuleInstance.hh"

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
      ~PartialPlanWriter(void);
      void notifyPropagationCompleted(void);
      void write(void);
    private:
      long long int seqId, ppId;
      int numTokens, numConstraints, numVariables, numTransactions;
      int stepsPerWrite, transactionId, nstep, tokenRelationId, writeCounter, noWrite;
      ConstraintEngineId *ceId;
      PlanDatabaseId *pdbId;
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
      const std::string getUpperBoundStr(IntervalDomain &dom) const;
      const std::string getLowerBoundStr(IntervalDomain &dom) const;
      const std::string getEnumerationStr(EnumeratedDomain &dom) const;
      const std::string getVarInfo(const ConstrainedVariableId &) const;
      const std::string getLongVarInfo(const ConstrainedVariableId &) const;

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

      /****From RulesEngineListener****/
      void notifyExecuted(const RuleInstanceId &);
      void notifyUndone(const RuleInstanceId &);

      class PPWPlanDatabaseListener;
      class PPWConstraintEngineListener;
      class PPWRulesEngineListener;

      friend class PPWPlanDatabaseListener;
      friend class PPWConstraintEngineListener;
      friend class PPWRulesEngineListener;

      class PPWPlanDatabaseListener : public PlanDatabaseListener {
      public:
        PPWPlanDatabaseListener(const PlanDatabaseId &planDb, PartialPlanWriter *planWriter)
          : PlanDatabaseListener(planDb), ppw(planWriter) {
          std::cout << "PPWPlanDatbaseListener: " << getId() << std::endl;
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
          std::cout << "PPWConstraintEngineListener: " << getId() << std::endl;
        }
        void notifyPropagationCompleted(void){ppw->notifyPropagationCompleted();}
      protected:
      private:
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
          std::cout << "PPWRulesEngineListener: " << getId() << std::endl;
        }
      protected:
      private:
        void notifyExecuted(const RuleInstanceId &rule) {ppw->notifyExecuted(rule);}
        void notifyUndone(const RuleInstanceId &rule) {ppw->notifyUndone(rule);}
        PartialPlanWriter *ppw;
      };
    };
  }
}
#endif
