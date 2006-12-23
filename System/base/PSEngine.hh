#ifndef _H_PSEngine
#define _H_PSEngine

#include "Id.hh"

#define Instant int
#define PSEntityKey int

namespace EUROPA {
	
	class PSEntity;
	class PSObject;
	class PSResource;
	class PSResourceProfile;
	class PSSolver;
	class PSToken;
	class PSVariable;
	class PSVarValue;

    // TODO: use IDs?
    // Need to be able to make memory management easy for the client
	typedef Id<PSObject>          PSObjectId;
	typedef Id<PSResource>        PSResourceId;
	typedef Id<PSResourceProfile> PSResourceProfileId;
	typedef Id<PSSolver>          PSSolverId;
	typedef Id<PSToken>           PSTokenId;
	typedef Id<PSVariable>        PSVariableId;

    // TODO: flesh this out, make it easy to wrap
    template<class T>
    class PSList
    {
      public:
    	int size() const { return m_elements.size(); }
    	T& get(int idx) { return m_elements[idx]; }
     	
      protected:
        std::vector<T> m_elements;    	
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
	    
	    void start();
	    void shutdown();
	     
		// Loads a planning model in binary format
		void loadModel(const std::string& modelFileName);
        		
		void executeTxns(const std::string& xmlTxnSource); // TODO: fold XML into executeScript?
		void executeScript(const std::string& language, const std::string& script);
	
		PSList<PSObjectId> getObjectsByType(const std::string& objectType);
		PSObjectId getObjectByKey(PSEntityKey id);
		
		PSList<PSResourceId> getResourcesByType(const std::string& resourceType);
		PSResourceId getResourceByKey(PSEntityKey id);
		
		const PSList<PSTokenId>& getTokens();    	 
		PSTokenId getTokenByKey(PSEntityKey id);	
		
		PSSolverId createSolver(const std::string& configurationFile);		
	};		

    class PSEntity
    {
      public: 
        PSEntityKey getKey() const;
        const std::string& getName() const;
	    const std::string& getType() const;
    };	
    
    class PSObject : public PSEntity
    {
       public:
	     const PSList<PSVariableId>& getMemberVariables();
	     PSVariableId getMemberVariable(const std::string& name);

    	 const PSList<PSTokenId>& getTokens();	    	 
    };

    class PSResource : public PSEntity
    {
    	public:
          PSResourceProfileId getCapacity();
          PSResourceProfileId getUsage();        	  
    };   
    
    class PSResourceProfile
    {
        PSList<Instant> getTimes();
        double getLowerBound(Instant time);
        double getUpperBound(Instant time);    	
    };
    
    class PSSolver
    {
      public:
		void step();
		void solve(int maxSteps,int maxDepth);
		void reset();
	    void destroy();
	    
		int getStepCount();
		int getDepth();
			
		bool isExhausted();
		bool isTimedOut();	
		bool isConstraintConsistent();
	
		// TODO: relationship between flaws and open decisions?
		// Can we call getFlaws() and get something different from open decisions?
		bool hasFlaws();	
		
	    int getOpenDecisionCnt();	
	    PSList<std::string> getOpenDecisions();	
		const std::string& getLastExecutedDecision();	
		
	    // Configuration
		// TODO: should horizon start and end be part of configuration?
	    const std::string& getConfigFilename();	
	    int getHorizonStart();
	    int getHorizonEnd();
	    
	    void configure(const std::string& configFilename, int horizonStart, int horizonEnd);
    };

    class PSToken : public PSEntity
    {	    
	    PSObjectId getOwner(); 
	    
	    // TODO: Add setStatus(int newStatus)?; ask Mike Iatauro
	    // TODO: getStatus()? -> MERGED, ACTIVE, REJECTED, etc
	    
	    double getViolation();
	    const std::string& getViolationExpl();
	    
	    PSList<PSVariableId> getParameters();
	    PSVariableId getParameter(const std::string& name);
	    
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
    };
    
    enum PSVarType {OBJECT,STRING,INTEGER,DOUBLE,BOOLEAN};
    
    class PSVariable
    {
      public:
	    const std::string& getName();
	    
	    bool isEnumerated();
	    bool isInterval();
	
	    PSVarType getType(); 
	    
	    bool isSingleton();
	
	    PSVarValue getSingletonValue();    // Call to get value if isSingleton()==true 
	
	    PSList<PSVarValue> getValues();  // if isSingleton()==false && isEnumerated() == true
	
	    double getLowerBound();  // if isSingleton()==false && isInterval() == true
	    double getUpperBound();  // if isSingleton()==false && isInterval() == true
	    
	    void specifyValue(PSVarValue v);
	    
	    std::string toString();	    
    };
    
    class PSVarValue
    {
      public:
       
        PSVarType getType() const;
        
        PSObjectId          asObject();
        int                 asInt();
        double              asDouble();
        bool                asBoolean();
        const std::string&  asString();
    };                
}	

#endif // _H_PSEngine