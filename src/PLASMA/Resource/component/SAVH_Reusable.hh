#ifndef _H_SAVH_Reusable
#define _H_SAVH_Reusable

#include "SAVH_ResourceDefs.hh"
#include "SAVH_Resource.hh"
#include <map>
#include <vector>

namespace EUROPA {
  namespace SAVH {
    class Reusable : public Resource {
    public:
      //initial capacity is the upper limit, maxInstConsumption == maxInstProduction, maxConsumption == maxProduction
      Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, const LabelStr& detectorName, const LabelStr& profileName,
	       double initCapacityLb = 0, double initCapacityUb = 0, double lowerLimit = MINUS_INFINITY,
	       double maxInstConsumption = PLUS_INFINITY,
	       double maxConsumption = PLUS_INFINITY);
      Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open);
      Reusable(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open);
      virtual ~Reusable();

      virtual void getOrderingChoices(const TokenId& token,
				      std::vector<std::pair<TokenId, TokenId> >& results,
				      unsigned int limit = PLUS_INFINITY);

    protected:
      void addToProfile(const TokenId& tok);
      void removeFromProfile(const TokenId& tok);

      std::map<TokenId, std::pair<TransactionId, TransactionId> > m_tokensToTransactions;
    };

    typedef Id<Reusable> ReusableId;

    class UnaryTimeline : public Reusable {
    public:
      UnaryTimeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open = false);
      UnaryTimeline(const ObjectId& parent, const LabelStr& type, const LabelStr& name, bool open = false);
      virtual ~UnaryTimeline();
    };

    class CBReusable;
    class Uses;

    typedef Id<CBReusable> CBReusableId;
    typedef Id<Uses> UsesId;

    class CBReusable : public Resource {
    public:
      //initial capacity is the upper limit, maxInstConsumption == maxInstProduction, maxConsumption == maxProduction
      CBReusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, const LabelStr& detectorName, const LabelStr& profileName,
           double initCapacityLb = 0, double initCapacityUb = 0, double lowerLimit = MINUS_INFINITY,
           double maxInstConsumption = PLUS_INFINITY,
           double maxConsumption = PLUS_INFINITY);
      CBReusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open);
      CBReusable(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open);
      virtual ~CBReusable();

      virtual void notifyViolated(const InstantId inst, Resource::ProblemType problem);
      virtual void notifyNoLongerViolated(const InstantId inst);
      virtual void notifyFlawed(const InstantId inst);
      virtual void notifyNoLongerFlawed(const InstantId inst);

    protected:
      void addToProfile(const ConstraintId& c);
      void removeFromProfile(const ConstraintId& c);
      std::set<UsesId> getConstraintsForInstant(const InstantId& instant);

      void addToProfile(TransactionId& t);
      void removeFromProfile(TransactionId& t);

      // TODO: only needed for backwards compatibility with Resource API, rework hierarchy to fix this.
      void addToProfile(const TokenId& tok);
      void removeFromProfile(const TokenId& tok);

      std::map<UsesId, std::pair<TransactionId, TransactionId> > m_constraintsToTransactions;

      friend class Uses;
    };

    class Uses : public Constraint
    {
    public:
      Uses(const LabelStr& name,
           const LabelStr& propagatorName,
           const ConstraintEngineId& constraintEngine,
           const std::vector<ConstrainedVariableId>& scope);

      static const LabelStr& CONSTRAINT_NAME();
      static const LabelStr& PROPAGATOR_NAME();

      static const int RESOURCE_VAR = 0;
      static const int QTY_VAR = 1;
      static const int START_VAR = 2;
      static const int END_VAR = 3;

      virtual std::string getViolationExpl() const;

      const TransactionId& getTransaction(int var) const;

    protected:
      virtual void handleDiscard();

      virtual void notifyViolated();
      virtual void notifyNoLongerViolated();

      virtual void notifyViolated(Resource::ProblemType problem, const InstantId inst);
      virtual void notifyNoLongerViolated(const InstantId inst);

      CBReusableId m_resource;
      std::vector<TransactionId> m_txns;
      std::map<InstantId,Resource::ProblemType> m_violationProblems; // instant->problem map

    private:
      virtual bool canIgnore(const ConstrainedVariableId& variable,
             int argIndex,
             const DomainListener::ChangeType& changeType);

      virtual void handleExecute();

      friend class CBReusable;
    };

  }
}

#endif
