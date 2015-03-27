#ifndef H_EuropaEngine
#define H_EuropaEngine

#include "Engine.hh"
#include "Module.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"
#include "tinyxml.h"
#include "Logger.hh"

namespace EUROPA {
//namespace System { //TODO: mcr

  class EuropaEngine : public EngineBase
  {
    public:
        LOGGER_CLASS_INSTANCE()
        //static Logger &LOGGER;

        EuropaEngine();
        virtual ~EuropaEngine();

        virtual ConstraintEngineId getConstraintEngine() const;
        virtual PlanDatabaseId     getPlanDatabase() const;
        virtual RulesEngineId      getRulesEngine() const;

        virtual const ConstraintEngine* getConstraintEnginePtr() const;
        virtual const PlanDatabase*     getPlanDatabasePtr() const;
        virtual const RulesEngine*      getRulesEnginePtr() const;

        // TODO: remains of the old Assemblies, these are only used by test code, should be dropped, eventually.
        virtual bool playTransactions(const char* txSource, const char* language="nddl-xml-txn");
        virtual bool plan(const char* txSource, const char* config, const char* language="nddl-xml-txn");
        virtual void write(std::ostream& os) const;
        virtual unsigned long getTotalNodesSearched() const;
        virtual unsigned long getDepthReached() const;
        static const char* TX_LOG();

    protected:
        virtual void initializeModules();
    	virtual void createModules();

        unsigned long m_totalNodes;
        unsigned long m_finalDepth;
  };
}

#endif
