#ifndef _H_PSEngine
#define _H_PSEngine

#include "Id.hh"
#include "Entity.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "SolverDefs.hh"
#include "RulesEngineDefs.hh"
#include "DbClientTransactionPlayer.hh"
#include "TransactionInterpreter.hh"


/*
  Changes by Mike (01/03/2007):
  1) got rid of Id types.  They create a necessity for a double-wrapping.
  2) changed "getCapacity" and "getUsage" to "getLimits" and "getLevels".  This is more in
     line with internal usage and is also much clearer
  3) changed "getTokens" in PSObject to return a pointer to a list, since the list changes.
     as a result, we may want to have the PSList destructor destroy the things it wraps.
  4) changed Instant and PSEntityKey into typedefs rather than #defines because it plays havoc
     with other code
  5) re-named "Instant" to "TimePoint" so as not to clash with existing Instant definition in 
     Resource.
 */

namespace EUROPA {

  typedef int TimePoint;
  typedef int PSEntityKey;
  
  // TODO: flesh this out, make it easy to wrap
  template<class T>
  class PSList
  {
  public:
    int size() const { return m_elements.size(); }
    T& get(int idx) { return m_elements[idx]; }
    void push_back(const T& value) {m_elements.push_back(value);}
  protected:
    std::vector<T> m_elements;    	
  };

  class PSObject;
  class PSToken;
  class PSSolver;
  class PSVariable;
  class PSVarValue;
    

  class ObjectWrapperGenerator {
  public:
    virtual ~ObjectWrapperGenerator() {}
    virtual PSObject* wrap(const ObjectId& obj) = 0;
  };

  class PSLanguageInterpreter {
  public:
    virtual ~PSLanguageInterpreter() {}
    virtual std::string interpret(const std::string& script) = 0;
  };

  /* Wihtout a query interface PSObjects and PSResources may need to :
   * - have lazy instanciation (otherwise calls that return collections of objects may end up being very expensive)
   * - deal gracefully with deletions/changes
   * lazy evaluation gets tricky with multi-threading
   */
  // TODO: Simplest option would be support for multi-threading with query language and non-lazy instanciation
  // see how close we can get to that in first round.
  // Initially we may want to keep it simple and do non-lazy instanciation even without queries and take potential performance hit
  // TODO: Add change notification for variables and resource profiles? 
  class PSEngine 
  {
  public:
    PSEngine();
    virtual ~PSEngine();
	    
    virtual void start();
    virtual void shutdown();
	     
    // Loads a planning model in binary format
    void loadModel(const std::string& modelFileName);
        		
    void executeTxns(const std::string& xmlTxnSource,bool isFile,bool useInterpreter); // TODO: fold XML into executeScript?
    std::string executeScript(const std::string& language, const std::string& script);
	
    PSList<PSObject*> getObjectsByType(const std::string& objectType);
    PSObject* getObjectByKey(PSEntityKey id);
		
    PSList<PSToken*> getTokens();    	 
    PSToken* getTokenByKey(PSEntityKey id);	
		
	PSList<PSVariable*> getGlobalVariables();
		
    PSSolver* createSolver(const std::string& configurationFile);
    std::string planDatabaseToString();
    
    double getViolation() const;
    
    static void addObjectWrapperGenerator(const LabelStr& type,
					  ObjectWrapperGenerator* wrapper);
    
    static void addLanguageInterpreter(const LabelStr& langauge,
				       PSLanguageInterpreter* interpreter);
  protected:
    virtual void initDatabase();
    static ObjectWrapperGenerator* getObjectWrapperGenerator(const LabelStr& type);

    DbClientTransactionPlayerId m_interpTransactionPlayer;
    DbClientTransactionPlayerId m_transactionPlayer;
    ConstraintEngineId m_constraintEngine;
    PlanDatabaseId m_planDatabase;
    RulesEngineId m_rulesEngine;

    static std::map<double, PSLanguageInterpreter*>& getLanguageInterpreters();
    static std::map<double, ObjectWrapperGenerator*>& getObjectWrapperGenerators();    
  };

  class PSEntity
  {
  public: 
    virtual ~PSEntity() {}
    
    PSEntityKey getKey() const;
    const std::string& getName() const;
    virtual const std::string& getEntityType() const;

    virtual std::string toString();

  protected:
    PSEntity(const EntityId& entity);

  private:
    EntityId m_entity;
  };	
    
  class PSObject : public PSEntity
  {
  public:
    virtual ~PSObject();
    
    virtual const std::string& getEntityType() const;
    
    std::string getObjectType() const; 

    const PSList<PSVariable*>& getMemberVariables();
    PSVariable* getMemberVariable(const std::string& name);

    PSList<PSToken*> getTokens();
    
  protected:
    friend class PSEngine;
    friend class PSToken;
    friend class PSVarValue;
    friend class BaseObjectWrapperGenerator;
    PSObject(const ObjectId& obj);
    
  private:
    ObjectId m_obj;
    PSList<PSVariable*> m_vars;
  };
    
  class PSSolver
  {
  public:
    void step();
    //Solver::solve returns a bool to determine if a solution was found.  Should this perhaps
    //do the same?  ~MJI
    void solve(int maxSteps,int maxDepth);
    void reset();
    //What is this supposed to do? ~MJI
    void destroy();
	    
    int getStepCount();
    int getDepth();
			
    bool isExhausted();
    bool isTimedOut();	
    //The Solver always leaves the database in a consistent state.  What does this do?
    bool isConstraintConsistent();
	
    // TODO: relationship between flaws and open decisions?
    // Can we call getFlaws() and get something different from open decisions?
    bool hasFlaws();	
		
    int getOpenDecisionCnt();	
    //What are these strings supposed to look like? ~MJI
    PSList<std::string> getFlaws();	
    std::string getLastExecutedDecision();	
		
    // Configuration
    // TODO: should horizon start and end be part of configuration?
    const std::string& getConfigFilename();	
    int getHorizonStart();
    int getHorizonEnd();
	    
    //it isn't currently possible to configure a solver from an XML file after construction
    void configure(int horizonStart, int horizonEnd);
  protected:
    friend class PSEngine;
    PSSolver(const SOLVERS::SolverId& solver, const std::string& configFilename);
  private:
    SOLVERS::SolverId m_solver;
    std::string m_configFile;
  };

  class PSToken : public PSEntity
  {	    
  public:
    virtual ~PSToken() {}

    virtual const std::string& getEntityType() const;

    std::string getTokenType() const; 

    bool isFact(); 
    
    PSObject* getOwner(); 
	    
	PSToken* getMaster();
	
	PSList<PSToken*> getSlaves();
	    
    // TODO: Add setStatus(int newStatus)?; ask Mike Iatauro
    // TODO: getStatus()? -> MERGED, ACTIVE, REJECTED, etc
	    
    //What does this do? ~MJI
    double getViolation();
    const std::string& getViolationExpl();
	    
    //Traditionally, the temporal variables and the object and state variables aren't 
    //considered "parameters".  I'm putting them in for the moment, but clearly the token
    //interface has the least thought put into it. ~MJI
    const PSList<PSVariable*>& getParameters();
    PSVariable* getParameter(const std::string& name);
	    
    virtual std::string toString();

    /*
      static final double NO_CONFLICT=0.0;
      static final double HARD_CONFLICT=Double.MAX_VALUE;
    */
	    
    // Parameters available for all tokens
    /*
      static final const std::string& START="start";
      static final const std::string& END="end";
      static final const std::string& DURATION="duration";
    */    
  protected:
    friend class PSEngine;
    friend class PSObject;
    PSToken(const TokenId& tok);

    TokenId m_tok;
    PSList<PSVariable*> m_vars;
  };
    
  enum PSVarType {OBJECT,STRING,INTEGER,DOUBLE,BOOLEAN};
  
  class PSVariable : public PSEntity
  {
  public:
    virtual ~PSVariable(){}
	    
    virtual const std::string& getEntityType() const;

    PSVarType getType(); // Data Type 

    bool isEnumerated();
    bool isInterval();

    bool isNull();      // iif CurrentDomain is empty and the variable hasn't been specified    
    bool isSingleton();
		   
    PSVarValue getSingletonValue();    // Call to get value if isSingleton()==true 
	
    PSList<PSVarValue> getValues();  // if isSingleton()==false && isEnumerated() == true
	
    double getLowerBound();  // if isSingleton()==false && isInterval() == true
    double getUpperBound();  // if isSingleton()==false && isInterval() == true
	    
    void specifyValue(PSVarValue& v);
	
	double getViolation() const;
	    
    virtual std::string toString();
  protected:
    friend class PSEngine;
    friend class PSObject;
    friend class PSToken;

    PSVariable(const ConstrainedVariableId& var);
  private:
    ConstrainedVariableId m_var;
    PSVarType m_type;
  };
    
  class PSVarValue
  {
  public:
       
    PSVarType getType() const;
        
    PSObject*           asObject() const;
    int                 asInt() const;
    double              asDouble() const;
    bool                asBoolean() const;
    const std::string&  asString() const; 
      
    std::string toString() const;
          
    static PSVarValue getInstance(std::string val) {return PSVarValue((double)LabelStr(val), STRING);}
    static PSVarValue getInstance(int val) {return PSVarValue((double)val, INTEGER);}
    static PSVarValue getInstance(double val) {return PSVarValue((double)val, DOUBLE);}
    static PSVarValue getInstance(bool val) {return PSVarValue((double)val, BOOLEAN);}
            
  protected:
    friend class PSVariable;
    PSVarValue(const double val, const PSVarType type);
  private:
    double m_val;
    PSVarType m_type;
  };                
}	

#endif // _H_PSEngine
