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

	typedef Id<PSObject>          PSObjectId;
	typedef Id<PSResource>        PSResourceId;
	typedef Id<PSResourceProfile> PSResourceProfileId;
	typedef Id<PSSolver>          PSSolverId;
	typedef Id<PSToken>           PSTokenId;

    // TODO: flesh this out, make it easy to wrap
    template<class T>
    class List
    {
      public:
    	int size() const;
    	T& get(int idx);
    }
    
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
	
		List<PSObjectId> getObjectsByType(const std::string& objectType);
		PSObjectId getObjectByKey(PSEntityKey id);
		
		List<PSResourceId> getResourcesByType(const std::string& resourceType);
		PSResourceId getResourceByKey(PSEntityKey id);
		
		const List<PSTokenId>& getTokens();    	 
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
	     const List<PSVariableId>& getMemberVariables();
	     PSVariableId getMemberVariable(const std::string& name);

    	 const List<PSTokenId>& getTokens();	    	 
    };

    class PSResource : public PSEntity
    {
    	public:
          PSResourceProfileId getCapacity();
          PSResourceProfileId getUsage();        	  
    };   
    
    class PSResourceProfile
    {
        List<Instant> getTimes();
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
			
		boolean isExhausted();
		boolean isTimedOut();	
		boolean isConstraintConsistent();
	
		// TODO: relationship between flaws and open decisions?
		// Can we call getFlaws() and get something different from open decisions?
		boolean hasFlaws();	
		
	    int getOpenDecisionCnt();	
	    List<std::string> getOpenDecisions();	
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
	    
	    List<PSVariableId> getParameters();
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
	    String getName();
	    
	    bool isEnumerated();
	    bool isInterval();
	
	    PSVarType getType(); // nddl type
	    
	    boolean isSingleton();
	
	    PSVarValue getSingletonValue();    // Call to get value if isSingleton()==true 
	
	    List<PSVarValue> getValues();  // if isSingleton()==false && isEnumerated() == true
	
	    double getLowerBound();  // if isSingleton()==false && isInterval() == true
	    double getUpperBound();  // if isSingleton()==false && isInterval() == true
	    
	    void specifyValue(PSVarValue v);
	    
	    std::string toString();	    
    };
    
    class PSVarValue
    {
      public:
       
        PSVarType getType const;
        
        PSObjectId          asObject();
        int                 asInt();
        double              asDouble();
        bool                asBoolean();
        const std::string&  asString();
    };                
}	

#endif // _H_PSEngine