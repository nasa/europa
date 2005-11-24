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

namespace EUROPA {
  namespace PlanWriter {
    class PartialPlanWriter {
    public:
      PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &);
      PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &, 
                        const RulesEngineId &);
      PartialPlanWriter(const PlanDatabaseId &, const ConstraintEngineId &,
                        const RulesEngineId &, const CBPlannerId &);
      virtual ~PartialPlanWriter(void);
      void write(void);

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
      class PPWPlannerListener;

      bool havePlanner;
      bool destAlreadyInitialized;
      long long int seqId;
      int numTokens, numConstraints, numVariables;
      int stepsPerWrite, nstep, writeCounter, maxChoices;
      ConstraintEngineId ceId;
      PlanDatabaseId pdbId;
      RulesEngineId reId;
      CBPlannerId plId;
      ConstraintEngineListenerId cel;
      DecisionManagerListenerId pl;
      std::ofstream *statsOut, *ruleInstanceOut;
      std::list<std::string> sourcePaths;
      void allocateListeners();
      void initOutputDestination();
      void parseConfigFile(std::ifstream &);
      void parseGeneralConfigSection(std::ifstream&);
      void parseRuleConfigSection(std::ifstream&);
      void commonInit(const PlanDatabaseId &, const ConstraintEngineId &, const RulesEngineId&, const CBPlannerId&);
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
      void outputDecision(const DecisionPointId &, std::ofstream &);
      void writeStats(void);
      void collectStats(void);
      const std::string getUpperBoundStr(IntervalDomain &dom) const;
      const std::string getLowerBoundStr(IntervalDomain &dom) const;
      const std::string getEnumerationStr(EnumeratedDomain &dom) const;
      const std::string getVarInfo(const ConstrainedVariableId &) const;
      const bool isCompatGuard(const ConstrainedVariableId &) const;

      void notifyPropagationPreempted(void);
      void notifyPropagationCompleted(void);

      /****From DecisionManagerListener****/
      void notifyAssignNextFailed(const DecisionPointId &dec);
      void notifyAssignNextSucceeded(const DecisionPointId &dec);
      void notifyAssignCurrentFailed(const DecisionPointId &dec);
      void notifyAssignCurrentSucceeded(const DecisionPointId &dec);
      void notifySearchFinished();
      void notifyPlannerTimeout();

      friend class PPWConstraintEngineListener;
      friend class PPWPlannerListener;

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

      class PPWPlannerListener : public DecisionManagerListener {
      public:
        PPWPlannerListener(const CBPlannerId &plannerId, PartialPlanWriter *planWriter) :
          DecisionManagerListener(plannerId->getDecisionManager()), ppw(planWriter) {
        }
        void notifyAssignNextFailed(const DecisionPointId &dec){ppw->notifyAssignNextFailed(dec);}
        void notifyAssignNextSucceeded(const DecisionPointId &dec){ppw->notifyAssignNextSucceeded(dec);}
        void notifyAssignCurrentFailed(const DecisionPointId &dec){ppw->notifyAssignCurrentFailed(dec);}
        void notifyAssignCurrentSucceeded(const DecisionPointId &dec){ppw->notifyAssignCurrentSucceeded(dec);}
        void notifyPlannerTimeout(){ppw->notifyPlannerTimeout();}
        void notifySearchFinished(){ppw->notifySearchFinished();}
      protected:
      private:
        PartialPlanWriter *ppw;
      };
    };
  }
}
#endif
