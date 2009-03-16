#ifndef _H_EuropaEngine
#define _H_EuropaEngine

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

        virtual ConstraintEngineId& getConstraintEngine();
        virtual PlanDatabaseId&     getPlanDatabase();
        virtual RulesEngineId&      getRulesEngine();

        virtual const ConstraintEngine* getConstraintEnginePtr() const;
        virtual const PlanDatabase*     getPlanDatabasePtr() const;
        virtual const RulesEngine*      getRulesEnginePtr() const;

        // TODO: remains of the old Assemblies, these are only used by test code, should be dropped, eventually.
        virtual bool playTransactions(const char* txSource, bool interp = false);
        virtual bool plan(const char* txSource, const char* config, bool interp = false);
        virtual bool plan(const char* txSource, const TiXmlElement& config, bool interp = false);
        virtual void write(std::ostream& os) const;
        virtual unsigned int getTotalNodesSearched() const;
        virtual unsigned int getDepthReached() const;
        static const char* TX_LOG();

    protected:
        virtual void initializeModules();
    	virtual void createModules();

        unsigned int m_totalNodes;
        unsigned int m_finalDepth;
  };
}

#endif
