#include "PlanDatabaseDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
#include "SolverDefs.hh"

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

      void addPlan(const char* txSource);

      void configureSolver(const char* source, int horizonStart, int horizonEnd);

      const SolverId& getSolver(){return m_solver;}

      const ResultSet& queryGetComponents();

    private:
      DSA();
      void init();
      void unload();
      void loadModelLibrary(const char* model);

      ConstraintEngineId m_ce; /*!< A Constraint Engine for propagation of relations */
      PlanDatabaseId m_db; /*!< A PlanDatabase as central state representation */
      RulesEngineId m_re; /*!< A Rules Engine to enforce model rules. */
      SolverId m_solver; /*!< At most, one solver allowed */
      void* m_libHandle;
    };

  }
}
