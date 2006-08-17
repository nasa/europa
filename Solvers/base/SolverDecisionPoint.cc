#include "SolverDecisionPoint.hh"
#include "Debug.hh"
#include "PlanDatabase.hh"
#include "DbClient.hh"
#include "Solver.hh"

namespace EUROPA {
  namespace SOLVERS {
    DecisionPoint::DecisionPoint(const DbClientId& client, unsigned int entityKey, const LabelStr& explanation) 
      : Entity(), m_client(client),  m_entityKey(entityKey), m_id(this), m_explanation(explanation), m_isExecuted(false), m_initialized(false) {}

    DecisionPoint::~DecisionPoint() {m_id.remove();}

    const DecisionPointId& DecisionPoint::getId() const {return m_id;}

    void DecisionPoint::initialize(){
      if(!m_initialized){
        handleInitialize();
        m_initialized = true;
      }
    }

    void DecisionPoint::execute(){
      static unsigned int sl_counter(0);
      checkError(isInitialized(), "Trying to execute an uninitialized decision. This is a bug in the Solver.");
      checkError(!isExecuted(), "Cannot execute if already executed. This indicates a bug in the Solver.");
      checkError(hasNext(), "Tried to execute past available choices. This indicates a bug in the Solver.");
      debugMsg("DecisionPoint:execute", sl_counter++ << ": Executing current decision. " << toString());
      handleExecute();
      debugMsg("DecisionPoint:execute", sl_counter++ << ": Executed current decision. " << toString());
      m_isExecuted = true;
    }

    void DecisionPoint::undo(){
      debugMsg("DecisionPoint:undo", "Undoing current decision.");
      checkError(isExecuted(), "Cannot undo if not executed already:" << toString());
      handleUndo();
      m_isExecuted = false;
    }

    bool DecisionPoint::isExecuted() const {return m_isExecuted;}

    bool DecisionPoint::canUndo() const{
      return(Entity::getEntity(m_entityKey).isId() && isExecuted());
    }

    bool DecisionPoint::isInitialized() const {
      return m_initialized;
    }
  }
}
