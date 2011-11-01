%module(directors="1") PSEngineInterface
%include "std_string.i"

%rename(executeScript_internal) executeScript;

%{
#include "PSEngine.hh"
#include "Error.hh"
#include "StringErrorStream.hh"
#include "PSPlanDatabaseListener.hh"
#include "PSConstraintEngineListener.hh"
#include "NddlInterpreter.hh"
%}

%rename(PSException) Error;  // Our Error C++ class is wrapped instead as PSException
%typemap(javabase) Error "java.lang.RuntimeException";  // extends RuntimeException
%typemap(javacode) Error %{  // copied verbatim into the java code (so java's standard getMessage function is available)
  public String getMessage() {
    return getMsg();
  }
%}

%typemap(javabase) EUROPA::PSLanguageExceptionList "java.lang.RuntimeException";  // extends RuntimeException
%typemap(javacode) EUROPA::PSLanguageExceptionList %{  // copied verbatim into the java code
  public String getMessage() {
    return "Got " + getExceptionCount() + " errors";
  }
%}


// Specific exception handlers should be defined before the catch-all version
%exception executeScript {
  try {
     $action
  } catch (const Error &er) {
		jclass excepClass = jenv->FindClass("psengine/PSLanguageExceptionList");
		if (excepClass == NULL)
			return $null;

		jmethodID excepConstructor = jenv->GetMethodID(excepClass, "<init>", "(JZ)V");
		if(excepConstructor == NULL)
			return $null;
			
		EUROPA::PSLanguageException exc(er.getFile().c_str(), er.getLine(), 0, 0,
			er.getMsg().c_str());
		std::vector<EUROPA::PSLanguageException> list;
		list.push_back(exc);
		
		jthrowable excep = static_cast<jthrowable> (jenv->NewObject(excepClass, excepConstructor, new EUROPA::PSLanguageExceptionList(list), true));
		if(excep == NULL)
			return $null;
		else
			jenv->Throw(excep);

		return $null;
  } catch (const EUROPA::PSLanguageExceptionList &e) {
		jclass excepClass = jenv->FindClass("psengine/PSLanguageExceptionList");
		if (excepClass == NULL)
			return $null;

		jmethodID excepConstructor = jenv->GetMethodID(excepClass, "<init>", "(JZ)V");
		if(excepConstructor == NULL)
			return $null;

		jthrowable excep = static_cast<jthrowable> (jenv->NewObject(excepClass, excepConstructor, new EUROPA::PSLanguageExceptionList(e), true));
		if(excep == NULL)
			return $null;
		else
			jenv->Throw(excep);

		return $null;
   }
}


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

class StringErrorStream {
public:
  static void setErrorStreamToString();
  static std::string retrieveString();
};

namespace EUROPA {

  class PSLanguageException {
  public:
	const std::string& getFileName() const;
	int getLine() const;
	int getOffset() const;
	int getLength() const;
	const std::string& getMessage() const;
  protected:
    // Prevent auto-generation
  	PSLanguageException(const char *fileName, int line, int offset, int length,
			const char *message);  
  };
  
  class PSLanguageExceptionList {
  public:
    int getExceptionCount() const;
    const PSLanguageException& getException(int index) const;
  protected:
    // Prevent auto-generation
	PSLanguageExceptionList(const std::vector<PSLanguageException>& exceptions);
  };


  typedef long TimePoint;
  typedef int PSEntityKey;

  class PSConstraint;
  class PSObject;
  class PSSolver;
  class PSToken;
  class PSVariable;
  class PSVarValue;
  class PSDataType;
  class PSTokenType;
  class PSObjectType;
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
  %template(PSDataTypeList) PSList<PSDataType*>;
  %template(PSTokenTypeList) PSList<PSTokenType*>;
  %template(PSObjectTypeList) PSList<PSObjectType*>;
  //%template(PSValueList) PSList<PSVarValue>;

  // using template instantiation to get the right results.
  %rename(PSStringList) PSList<std::string>;
  class PSList<std::string> {
  public:
    int size() const;
    std::string get(int idx);
  };

  %rename(PSTimePointList) PSList<long>;
  class PSList<long> {
  public:
    int size() const;
    int get(int idx);
  };

  %rename(PSIntList) PSList<int>;
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

%typemap(javacode) PSEngine %{
  public String executeScript(String language, String script, boolean isFile) throws PSLanguageExceptionList
  {
      String retval = "";
      
      try {
          retval = executeScript_internal(language,script,isFile);
      }
      catch (PSLanguageExceptionList e) {
        // This exception derives from RuntimeException and needs not be declared
        throw e;
      } catch(Exception e) {
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
      void readFromXML(const char* file_name, bool isFile);
      void writeFromXML(const char* file_name);
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
    PSEntityKey getEntityKey() const;
    const std::string& getEntityName() const;
    const std::string& getEntityType() const;

    std::string toString() const;
    std::string toLongString() const;

  protected:
    PSEntity(); //protected constructors prevent wrapper generation
  };

  class PSDataType
  {
  public:
	  const std::string& getNameString() const;
  protected:
      PSDataType();
  };
  
  class PSTokenType
  {
  public:
	  const std::string& getName() const;
	  PSList<std::string> getParameterNames() const;
	  PSDataType* getParameterType(int index) const;
	  PSDataType* getParameterType(const std::string& name) const;
  protected:
      PSTokenType();
  };  

  class PSObjectType
  {
  public:
	  const std::string& getNameString() const;
	  const std::string& getParentName() const;
	  PSList<std::string> getMemberNames() const;
	  PSDataType* getMemberTypeRef(const std::string& name) const;
	  PSList<PSTokenType*> getPredicates() const;
  protected:
      PSObjectType();
  };

  class PSSchema
  {
  public:
	  ~PSSchema();
	  PSList<std::string> getAllPredicates() const;
	  PSList<std::string> getMembers(const std::string& objectType) const;
	  bool hasMember(const std::string& parentType, const std::string& memberName) const;
	  
	  PSList<PSObjectType*> getAllPSObjectTypes() const;

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
    void destroy();
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
    std::string getFullTokenType() const;

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
    PSResourceProfile* getCapacity();
    PSResourceProfile* getUsage();
    PSResourceProfile* getLimits();
    PSResourceProfile* getFDLevels();
    PSResourceProfile* getVDLevels();

    PSList<PSEntityKey> getOrderingChoices(TimePoint t);

    static PSResource* asPSResource(PSObject* obj);

  protected:
    PSResource();
  };


  class PSResourceProfile
  {
  public:
    PSList<TimePoint> getTimes();
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
