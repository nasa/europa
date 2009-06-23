%module(directors="1") PSEngineInterface
%include "std_string.i"

%rename(executeScript_internal) executeScript;

%{
#include "PSEngine.hh"
#include "Error.hh"
#include "PSPlanDatabaseListener.hh"
#include "PSConstraintEngineListener.hh"
%}

%rename(PSException) Error;  // Our Error C++ class is wrapped instead as PSException
%typemap(javabase) Error "java.lang.RuntimeException";  // extends RuntimeException
%typemap(javacode) Error %{  // copied verbatim into the java code (so java's standard getMessage function is available)
  public String getMessage() {
    return getMsg();
  }
%}

// Generic exception handling will wrap all functions with the following try/catch block:
// copied from http://www.swig.org/Doc1.3/Java.html#exception_handling
%exception {
	try {
		$action
	}
	catch (Error& e) {
		// TODO: There's probably a better way to refer to both package and class name here.
		jclass excepClass = jenv->FindClass("psengine/PSException");
		if (excepClass == NULL)
			return $null;

		jmethodID excepConstructor = jenv->GetMethodID(excepClass, "<init>", "(JZ)V");
		if(excepConstructor == NULL)
			return $null;

		jthrowable excep = static_cast<jthrowable> (jenv->NewObject(excepClass, excepConstructor, new Error(e), true));
		if(excep == NULL)
			return $null;
		else
			jenv->Throw(excep);

		return $null;
	}
}


%typemap(javabody) SWIGTYPE %{
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected $javaclassname(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  public static long getCPtr($javaclassname obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

%}

class Error {
public:
  std::string getMsg();
  std::string getFile();
  std::string getCondition();
  int getLine();
  std::string getType();
private:
  Error();
};

namespace EUROPA {

  typedef int TimePoint;
  typedef int PSEntityKey;

  class PSConstraint;
  class PSObject;
  class PSSolver;
  class PSToken;
  class PSVariable;
  class PSVarValue;
  class PSPlanDatabaseClient;
  class PSPlanDatabaseListener;
  class PSConstraintEngineListener;
  class PSSchema;

  template<class T>
  class PSList {
  public:
    PSList();
    int size() const;
    T& get(int idx);
    void push_back(const T& value);
    void remove(const T& value);
    void clear();
  };

  %template(PSObjectList) PSList<PSObject*>;
  %template(PSTokenList) PSList<PSToken*>;
  %template(PSVariableList) PSList<PSVariable*>;
  %template(PSConstraintList) PSList<PSConstraint*>;
  //%template(PSValueList) PSList<PSVarValue>;

  // using template instantiation to get the right results.
  %rename(PSStringList) PSList<std::string>;
  class PSList<std::string> {
  public:
    int size() const;
    std::string get(int idx);
  };

  %rename(PSTimePointList) PSList<int>;
  class PSList<int> {
  public:
    int size() const;
    int get(int idx);
  };

  %rename(PSValueList) PSList<EUROPA::PSVarValue>;
  class PSList<EUROPA::PSVarValue> {
  public:
    int size() const;
    PSVarValue get(int idx);
  };

// If the output language is java, add a method to the PSEngine
// that can get at the NDDL implementation.
%typemap(javacode) PSEngine %{
  protected Class getClassForName(String name)
  {
      try {
          return Class.forName(name);
      }
      catch (Exception e) {
          try {
              return ClassLoader.getSystemClassLoader().loadClass(name);
          }
          catch (Exception ex) {
              throw new RuntimeException("Failed to load class "+name,ex);
          }
      }
  }

  public String executeScript(String language, java.io.Reader reader) throws PSException {
    String retval = "";
    
    String txns = null;
    if(language.equalsIgnoreCase("nddl2")) {
      try {
        Class nddlClass = getClassForName("nddl.Nddl");
        Class[] parameters = new Class[]{java.io.Reader.class, boolean.class};
        Object[] arguments = new Object[]{reader, new Boolean(false)};
        txns = (String) nddlClass.getMethod("nddlToXML", parameters).invoke(null, arguments);
      }
      catch(NoSuchMethodException ex) {
        System.err.println("Cannot execute NDDL source: Unexpected NDDL implementation (nddlToXML not found).");
        throw new RuntimeException(ex);
      }
      catch(IllegalAccessException ex) {
        System.err.println("Cannot execute NDDL source: Unexpected NDDL implementation (access modifiers too restrictive on parse method).");
        throw new RuntimeException(ex);
      }
      catch(java.lang.reflect.InvocationTargetException ex) {
        System.err.println("Cannot execute NDDL source: exception during parsing: ");
        System.err.println(ex.getCause().getClass().getName() + ": "+ ex.getCause().getMessage());
        throw new RuntimeException(ex.getCause());
      }
      catch(RuntimeException ex) {
        System.err.println("Cannot execute NDDL source: exception during parsing: ");
        System.err.println(ex.getClass().getName() + ": "+ ex.getMessage());
        throw ex;
      }
    }

    if(txns != null) {
      retval = executeScript_internal("nddl-xml",txns,false /*isFile*/);
    }
    else {
      throw new RuntimeException("Failed to create transactions from "+language+" source.");
    }
    
    return retval;
  }

  protected NddlInterpreter nddlInterpreter_=null;

  public NddlInterpreter getNddlInterpreter()
  {
      if (nddlInterpreter_ == null)
          nddlInterpreter_ = new NddlInterpreter(this);

      return nddlInterpreter_;
  }

  public String executeScript(String language, String script, boolean isFile) throws PSException
  {
      String retval = "";
      
      try {
        if (language.equalsIgnoreCase("nddl2")) {
            if (isFile)
                getNddlInterpreter().source(script);
            else
                getNddlInterpreter().eval(script);
        }
        else {
            retval = executeScript_internal(language,script,isFile);
        }
      }
      catch(Exception e) {
          e.printStackTrace();
          throw new RuntimeException("Failed to execute "+language+" script "+script,e);
      }
      
      return retval;
  }

%}

  class EngineConfig
  {
    public:
      const std::string& getProperty(const std::string& name) const;
      void setProperty(const std::string& name,const std::string& value);
  };


%nodefaultctor PSEngine;

  class PSEngine {
  public:
    static PSEngine* makeInstance();

    void start();
    void shutdown();

    EngineConfig* getConfig();

    void loadModule(const std::string& moduleFileName);
    std::string executeScript(const std::string& language, const std::string& script, bool isFile);

    PSList<PSObject*> getObjects();
    PSList<PSObject*> getObjectsByType(const std::string& objectType);
    PSObject* getObjectByKey(PSEntityKey id);
    PSObject* getObjectByName(const std::string& name);

    PSPlanDatabaseClient* getPlanDatabaseClient();
    void addPlanDatabaseListener(PSPlanDatabaseListener& listener);
    void addConstraintEngineListener(PSConstraintEngineListener& listener);
    std::string planDatabaseToString();

    PSSchema* getPSSchema();

    PSList<PSVariable*> getGlobalVariables();
    PSVariable* getVariableByKey(PSEntityKey id);
    PSVariable* getVariableByName(const std::string& name);

    PSList<PSToken*> getTokens();
    PSToken* getTokenByKey(PSEntityKey id);

    bool getAutoPropagation() const;
    void setAutoPropagation(bool v);
    bool propagate();

    bool getAllowViolations() const;
    void setAllowViolations(bool v);

    double getViolation() const;
    PSList<std::string> getViolationExpl() const;
	PSList<PSConstraint*> getAllViolations() const;

    PSSolver* createSolver(const std::string& configurationFile);
  };

  class PSEntity
  {
  public:
    PSEntityKey getKey() const;
    const std::string& getEntityName() const;
    const std::string& getEntityType() const;

    std::string toString() const;
    std::string toLongString() const;

  protected:
    PSEntity(); //protected constructors prevent wrapper generation
  };

  class PSSchema
  {
  public:
	  ~PSSchema();
	  PSList<std::string> getAllPredicates() const;
	  PSList<std::string> getMembers(const std::string& objectType) const;
	  bool hasMember(const std::string& parentType, const std::string& memberName) const;

  protected:
	  PSSchema();
  };

  class PSObject : public PSEntity
  {
  public:
    std::string getObjectType() const;
    PSList<PSVariable*> getMemberVariables();
    PSVariable* getMemberVariable(const std::string& name);
    PSList<PSToken*> getTokens();
    void addPrecedence(PSToken* pred,PSToken* succ);
    void removePrecedence(PSToken* pred,PSToken* succ);
    PSVarValue asPSVarValue() const;

    static PSObject* asPSObject(PSEntity* entity);

    ~PSObject();
  protected:
    PSObject();
  };

  class PSSolver
  {
  public:
    void step();
    void solve(int maxSteps,int maxDepth);
    bool backjump(unsigned int stepCount);
    void reset();
    void reset(unsigned int depth);

    int getStepCount();
    int getDepth();

    bool isExhausted();
    bool isTimedOut();
    bool isConstraintConsistent();

    bool hasFlaws();

    int getOpenDecisionCnt();
    PSList<std::string> getFlaws();
    std::string getLastExecutedDecision();

    const std::string& getConfigFilename();
    int getHorizonStart();
    int getHorizonEnd();

    void configure(int horizonStart, int horizonEnd);
  protected:
    PSSolver();
  };

  enum PSTokenState { INACTIVE,ACTIVE,MERGED,REJECTED };

  class PSToken : public PSEntity
  {
  public:
    std::string getTokenType() const;

    bool isFact();
    bool isIncomplete();

    PSObject* getOwner();

    PSToken* getMaster();
    PSList<PSToken*> getSlaves();

    PSToken* getActive() const;
    PSList<PSToken*> getMerged() const;

    PSTokenState getTokenState() const;
    PSVariable* getStart();
    PSVariable* getEnd();
    PSVariable* getDuration();

    double getViolation() const;
    std::string getViolationExpl() const;

    PSList<PSVariable*> getParameters();
    PSVariable* getParameter(const std::string& name);

    void activate();
    void reject();
    void merge(PSToken* activeToken);
    void cancel();

    PSList<PSToken*> getCompatibleTokens(unsigned int limit, bool useExactTest);

    std::string toString();

  protected:
    PSToken();
  };

  enum PSVarType {INTEGER,DOUBLE,BOOLEAN,STRING,OBJECT};

  class PSVariable : public PSEntity
  {
  public:

    PSVarType getType();
    PSEntity* getParent();

    bool isSingleton();
    PSVarValue getSingletonValue();    // Call to get value if isSingleton()==true

    bool isInterval();
    double getLowerBound();  // if isSingleton()==false && isInterval() == true
    double getUpperBound();  // if isSingleton()==false && isInterval() == true

    bool isEnumerated();
    PSList<PSVarValue> getValues();  // if isSingleton()==false && isEnumerated() == true
    PSList<PSConstraint*> getConstraints();


    void specifyValue(PSVarValue& v);
    void reset();

    double getViolation() const;
    std::string getViolationExpl() const;

    std::string toString();
  protected:
    PSVariable();
  };

  class PSVarValue
  {
  public:
  	PSVarValue(const double val, const PSVarType type);
    static PSVarValue getInstance(const std::string& val);
    static PSVarValue getInstance(int val);
    static PSVarValue getInstance(double val);
    static PSVarValue getInstance(bool val);
    static PSVarValue getObjectInstance(double obj);

    PSVarType getType() const;

    PSEntity*           asObject() const;
    int                 asInt() const;
    double              asDouble() const;
    bool                asBoolean() const;
    const std::string&  asString() const;

    std::string toString() const;

  protected:
    PSVarValue();
  };

  class PSConstraint : public PSEntity
  {
    public:
      bool isActive() const;
      void deactivate();
      void undoDeactivation();

      double getViolation() const;
      std::string getViolationExpl() const;

      std::string toString() const;
      PSList<PSVariable*> getVariables() const;

    protected:
      PSConstraint();
  };

  // TODO: move this to a separate file?
  class PSResource;
  class PSResourceProfile;

  class PSResource : public PSObject
  {
  public:
    PSResourceProfile* getLimits();
    PSResourceProfile* getLevels();

    PSList<PSEntityKey> getOrderingChoices(TimePoint t);

    static PSResource* asPSResource(PSObject* obj);

  protected:
    PSResource();
  };


  class PSResourceProfile
  {
  public:
    const PSList<TimePoint>& getTimes();
    double getLowerBound(TimePoint time);
    double getUpperBound(TimePoint time);
  protected:
    PSResourceProfile();
  };

  class PSPlanDatabaseClient
  {
    public:
      PSVariable* createVariable(const std::string& typeName, const std::string& name, bool isTmpVar) = 0;
      void deleteVariable(PSVariable* var) = 0;

      PSObject* createObject(const std::string& type, const std::string& name) = 0;
      void deleteObject(PSObject* obj) = 0;

      PSToken* createToken(const std::string& predicateName, bool rejectable, bool isFact) = 0;
      void deleteToken(PSToken* token) = 0;

      void constrain(PSObject* object, PSToken* predecessor, PSToken* successor) = 0;
      void free(PSObject* object, PSToken* predecessor, PSToken* successor) = 0;
      void activate(PSToken* token) = 0;
      void merge(PSToken* token, PSToken* activeToken) = 0;
      void reject(PSToken* token) = 0;
      void cancel(PSToken* token) = 0;

      PSConstraint* createConstraint(const std::string& name, PSList<PSVariable*>& scope) = 0;
      void deleteConstraint(PSConstraint* constr) = 0;

      void specify(PSVariable* variable, double value) = 0;
      void reset(PSVariable* variable) = 0;

      void close(PSVariable* variable) = 0;
      void close(const std::string& objectType) = 0;
      void close() = 0;
  };

// generate directors for all virtual methods in class Foo
// (enables calls from C++ to inherited java code)
  %feature("director") PSPlanDatabaseListener;

  class PSPlanDatabaseListener
  {
  public:
	  virtual ~PSPlanDatabaseListener();
	  virtual void notifyAdded(PSObject* obj);
	  virtual void notifyRemoved(PSObject* object);
	  virtual void notifyActivated(PSToken* token);
	  virtual void notifyDeactivated(PSToken* token);
	  virtual void notifyMerged(PSToken* token);
	  virtual void notifySplit(PSToken* token);
	  virtual void notifyRejected(PSToken* token);
	  virtual void notifyAdded(PSObject* object, PSToken* token);
	  virtual void notifyRemoved(PSObject* object, PSToken* token);
  };

// generate directors for all virtual methods in class Foo
// (enables calls from C++ to inherited java code)
  %feature("director") PSConstraintEngineListener;




  class PSConstraintEngineListener
  {
  public:
	enum PSChangeType { UPPER_BOUND_DECREASED, LOWER_BOUND_INCREASED,  BOUNDS_RESTRICTED,
                                     VALUE_REMOVED, RESTRICT_TO_SINGLETON,  SET_TO_SINGLETON,  RESET,
                      	 			 RELAXED, CLOSED, OPENED,  EMPTIED,  LAST_CHANGE_TYPE};

    virtual ~PSConstraintEngineListener();
	virtual void notifyViolationAdded(PSConstraint* constraint);
	virtual void notifyViolationRemoved(PSConstraint* constraint);
    virtual void notifyChanged(PSVariable* variable, PSChangeType changeType);
  };


}
