#ifndef H_DSA
#define H_DSA

#include "PlanDatabaseDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
#include "SolverDefs.hh"
#include "SAVH_ResourceDefs.hh"

using namespace EUROPA::SOLVERS;

namespace EUROPA {
  namespace DSA {

    class ResultSet{
    public:
      virtual const std::string& toXML() const = 0;
    };

    class DSA {
    public:
      class StringResultSet: public ResultSet {
      public:
	StringResultSet(){}
	virtual ~StringResultSet(){}
	std::string& str() {return m_str;}
	const std::string& toXML() const {return m_str;}

      private:
	std::string m_str;
      };

      static DSA& instance(){
	static DSA sl_instance;
	return sl_instance;
      }
      
      void load(const char* model);

      void addPlan(const char* txSource,bool interpreted);

      const ResultSet& getComponents();
      const ResultSet& getActions(int componentKey);
      const ResultSet& getAction(int actionKey);
      const ResultSet& getMaster(int actionKey);
      const ResultSet& getChildActions(int actionKey);
      const ResultSet& getConditions(int actionKey);
      const ResultSet& getEffects(int actionKey);
      const ResultSet& getComponentForAction(int actionKey);

      const ResultSet& getResources();
      const ResultSet& getResourceCapacityProfile(int resourceKey);
      const ResultSet& getResourceUsageProfile(int resourceKey);

      const SolverId& getSolver(){return m_solver;}
      void solverConfigure(const char* source, int horizonStart, int horizonEnd);
      void solverSolve(int maxSteps, int maxDepth);
      void solverStep();
      void solverReset();
      void solverClear();

    private:
      DSA();
      void init();
      void unload();
      void loadModelLibrary(const char* model);
      void writeSolverState();
      const ResultSet& makeTokenCollection(const TokenSet& tokens);
      const ResultSet& makeObjectCollection(const ObjectSet& objects) const;
      const ResultSet& getObjectsByType(const std::string& type) const;
      const std::string makeCapacityProfile(const SAVH::ResourceId& res) const; 
      const std::string makeUsageProfile(const SAVH::ResourceId& res) const;

      ConstraintEngineId m_ce; /*!< A Constraint Engine for propagation of relations */
      PlanDatabaseId m_db; /*!< A PlanDatabase as central state representation */
      RulesEngineId m_re; /*!< A Rules Engine to enforce model rules. */
      SolverId m_solver; /*!< At most, one solver allowed */
      void* m_libHandle;
    };

  }
}
#endif
