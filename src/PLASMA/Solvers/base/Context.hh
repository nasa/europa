#ifndef H_Context
#define H_Context

#include "SolverDefs.hh"

namespace EUROPA {
  namespace SOLVERS {
    
    class Context {
    public:
      Context(const std::string& name);
      ~Context();
      double get(const std::string& key) const;
      void remove(const std::string& key);
      void put(const std::string& key, const double value) {m_map[key] = value;}
      const std::string& getName() const {return m_name;}
      ContextId getId() const {return m_id;}
    protected:
    private:
      ContextId m_id;
      const std::string m_name;
      std::map<std::string, double> m_map;
    };
  }
}

#endif
