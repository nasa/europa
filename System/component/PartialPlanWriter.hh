#ifndef PARTIALPLANWRITER_HH
#define PARTIALPLANWRITER_HH

#include <fstream>

#include <list>

#include "../ConstraintEngine/ConstraintEngineListener.hh"
#include "../ConstraintEngine/DomainListener.hh"
#include "../ConstraintEngine/Id.hh"
#include "../ConstraintEngine/VariableChangeListener.hh"
#include "../PlanDatabase/PlanDatabaseListener.hh"
#include "../Resource/Resource.hh"
#include "../Resource/ResourceDefs.hh"
#include "../Resource/Transaction.hh"

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

    class PartialPlanWriter: public PlanDatabaseListener, public ConstraintEngineListener/*, 
											   public VariableChangeListener*/ {
    public:
      PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &);
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
      std::ofstream *transOut, *statsOut;
      std::string dest;
      void outputObject(const ObjectId &, const int, std::ofstream &, std::ofstream &);
      void outputToken(const TokenId &, const int, const int, const int, const int, 
                       const ObjectId &, std::ofstream &, std::ofstream &, std::ofstream &);
      void outputEnumVar(const Id< TokenVariable<EnumeratedDomain> > &, const int,
			 const int, std::ofstream &);
      void outputIntVar(const Id< TokenVariable<IntervalDomain> > &, const int,
                        const int, std::ofstream &);
      void outputIntIntVar(const Id< TokenVariable<IntervalIntDomain> >&, const int,
			   const int, std::ofstream &);
      void outputObjVar(const Id< TokenVariable<ObjectSet> > &, const int, const int,
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
    };
  }
}
#endif
