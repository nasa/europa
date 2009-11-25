#ifndef _H_Context
#define _H_Context

#include "SolverDefs.hh"
#include "LabelStr.hh"

namespace EUROPA {
  namespace SOLVERS {
    
    class Context {
    public:
      Context(const LabelStr& name);
      ~Context();
      double get(const LabelStr& key) const;
      void remove(const LabelStr& key);
      void put(const LabelStr& key, const double value) {m_map[key] = value;}
      const LabelStr& getName() const {return m_name;}
      ContextId getId() const {return m_id;}
    protected:
    private:
      ContextId m_id;
      const LabelStr m_name;
      std::map<edouble, double> m_map;
    };
  }
}

#endif
