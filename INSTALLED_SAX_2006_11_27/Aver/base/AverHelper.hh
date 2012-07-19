#ifndef _H_AverHelper
#define _H_AverHelper

/**
 * @file   AverHelper.hh
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @brief A set of utility methods for compiling and interpreting Aver and a class for representing compiled Aver.
 * @date   Thu Jan 13 09:14:57 2005
 * @ingroup Aver
 */

#include "AbstractDomain.hh"
#include "EnumeratedDomain.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "tinyxml.h"
#include <stack>

namespace EUROPA {

  class AverExecutionPath;
  typedef Id<AverExecutionPath> AverExecutionPathId;

  /**
   *@class DomainContainer
   *@brief A class to provide garbage collection to Aver.
   *
   *Aver uses a stack of pointers to AbstractDomains for the arguments to its functions.  Because there's no way to tell if a domain was allocated
   *by Aver or is from within EUROPA, a domain's pointer may be removed from the stack and go out of scope, but the memory may erroneously remain
   *allocated.  The DomainContainer class provides a small wrapper around AbstractDomain pointers so that the memory can be properly freed.
   */
  class DomainContainer {
  public:
    /**
     *@brief Constructor
     *@param dom the AbstractDomain this instance contains
     *@param remove if true, the domain will be deleted when the container is.
     *@param dieEarly if true, the container and domain will be deleted as early as possible.
     */
    DomainContainer(AbstractDomain* dom, bool remove = true, bool dieEarly = true);

    /**
     *@brief Destructor.  Will delete the contained domain if constructed to do so.
     */
    ~DomainContainer();
    
    AbstractDomain& operator*() const {return *m_dom;}
    /**
     *@brief convenience operator for accessing the domain pointer.
     */
    operator AbstractDomain* () const { return m_dom;}

    /**
     *@brief convenience operator for accessing the domain pointer.
     */
    AbstractDomain* operator->() const {return m_dom;}

    bool earlyDeath() const {return m_dieEarly;}

  protected:
  private:
    AbstractDomain* m_dom; /*!< A pointer to the contained domain. */
    bool m_remove;  /*!< If true, the domain will be deleted with the container. */
    bool m_dieEarly; /*!< If true, the container will be deleted as early as possible. */
    int m_id; /*!< An identifier for the container.  Used in debugging. */
    static int s_lastId;
  };
  
  /**
   * @class AverExecutionPath
   * @brief Represents a single Aver instruction.  Execution paths are represented as linked lists of AverExecutionPaths.
   * Each AverExecutionPath contains a single Aver instruction and all of the data associated with it.  AverExecutionPaths are strung
   * together into linked lists that are held by Assertions and handed off to interpretation functions in the AverHelper at execution
   * time.
   */
  class AverExecutionPath {
  public:
    enum Actions {
      //standard functions
      COUNT = 0, /**< The 'Count' Aver function.*/
      QUERY_TOKENS, /**< The 'Tokens' function.  This queries all tokens, which are then trimmed with later functions. */
      QUERY_OBJECTS, /**< The 'Objects' function.  This queries all objects, which are then trimmed with later functions. */
      PROPERTY, /**< The 'Property' function. */
      ENTITY, /**< The 'Entity' function. */
      TRANSACTIONS, /**< The 'Transactions' function. */

      //trim functions
      TRIM_TOKENS_BY_PRED, /**< Trim tokens by predicate. */
      TRIM_TOKENS_BY_VAR, /**< Trim tokens by a named parameter variable. */
      TRIM_TOKENS_BY_START, /**< Trim tokens by start variable. */
      TRIM_TOKENS_BY_END, /**< Trim tokens by end variable. */
      TRIM_TOKENS_BY_STATUS, /**< Trim tokens by state variable. */
      TRIM_TOKENS_BY_PATH, /**< Trim tokens by path. */
      TRIM_OBJECTS_BY_NAME, /**< Trim objects by name. */
      TRIM_OBJECTS_BY_VAR, /**< Trim objects by a named member variable. */

      //boolean comparison functions
      BOOL_OP, /**< Evaluate a boolean operation. */

      //simple actions
      NO_OP, /**< No operation. */
      PUSH_DOMAIN, /**< Push a domain onto the stack. */
      PUSH_OP, /**< Push an operator onto the stack. */

      NUMACTIONS /**< Dummy to represent the number of Aver instructions */
    };

    /**
     *@brief Constructor for operations with no arguments or which expect all arguments to be on the stack.
     *@param action the action that this ExecutionPath represents.
     */
    AverExecutionPath(const Actions action);

    /**
     *@brief Constructor for operations with a domain argument.
     *@param action the action that this ExecutionPath represents.
     *@param dom the container of the domain argument to the action.
     */
    AverExecutionPath(const Actions action, DomainContainer* dom);


    /**
     * @brief Constructor for operations with an operator argument
     * @param action the action that this ExecutionPath represents.
     * @param op the operator argument to the action.
     */
    AverExecutionPath(const Actions action, const std::string& op);

    /**
     * @brief Destructor.  Calls the destructor of the next member in the Path.
     */
    ~AverExecutionPath();

    /**
     *@brief Returns the Id representing this AverExecutionPath.
     *@return the Id representing this AverExecutionPath.
     */
    AverExecutionPathId getId() const {return m_id;}

    /**
     *@brief Sets the next operation in the path.
     *@param next Id representing the next operation in the path.
     */
    void setNext(const AverExecutionPathId& next) {m_next = next;}
    
    /**
     *@brief Gets the next operation in the path.
     *@return the next operation in the path.
     */
    AverExecutionPathId next() const {return m_next;}

    /**
     *@brief Gets the domain for this action, if any.
     *@return the container of the domain for this action.
     */
    DomainContainer* domain() const {return m_dom;}

    /**
     *@brief Gets the Action that this ExecutionPath represents.
     *@return the action.
     */
    Actions action() const {return m_action;}

    /**
     *@brief Gets the operator for this action, if any.
     *@return the operator for this action.
     */
    std::string op() const {return m_op;}

    /**
     *@brief Gets the last action linked to this ExecutionPath.
     *@return the Id for the last action linked to this ExecutionPath.
     */
    AverExecutionPathId end() const;

    /**
     *@brief Utility function for printing ExecutionPaths to a stream.
     *@param os the stream to recieve output.
     */
    void operator >> (std::ostream& os) const;
  protected:
  private:
    AverExecutionPathId m_id; /*!< The id for this ExecutionPath */
    Actions m_action; /*!< The action that this ExecutionPath represents. */
    DomainContainer* m_dom; /*!< The container of the domain argument to the action, if any. */
    std::string m_op; /*!< The operator argument to the action, if any. */
    AverExecutionPathId m_next; /*!< The next element in the ExecutionPath. */
    static const std::string s_actNames[NUMACTIONS]; /*!< A set of string representations of the names of the actions.  Used for debugging.*/
  };

  /**
   *@class AverHelper
   *@brief A set of static functions to compile Aver XML into instructions and execute those instructions.
   *
   *This is only a class because the op and domain stacks need to be hidden.
   */
  class AverHelper {
  public:
    /**
     *@brief Constructs a EUROPA 2 domain object from an XML representation.
     *@param dom the XML representation of the domain.
     *@param dieEarly if true, Aver will delete the returned DomainContainer as early as possible.
     *@return a pointer to the container of the evaluated domain.
     */
    static DomainContainer* evaluateDomain(const TiXmlElement* dom, bool dieEarly = true); //tested

    /**
     *@brief Evaluates a boolean function on two domains and returns the result.
     *@param op the boolean operation to evaluate.
     *@param d1 the LHS argument to the operation.
     *@param d2 the RHS argument to the operation.
     *@return the boolean value of op applied to d1 and d2.
     */
    static bool evaluateBoolean(const std::string& op, const AbstractDomain& d1,
                                const AbstractDomain& d2); //tested

    /**
     *@brief Compile an XML representation of an assertion into an AverExecutionPath.
     *@param assn the XML assertion
     *@return the Id of the first instruction in the execution path.
     */
    static AverExecutionPathId buildExecutionPath(const TiXmlElement* assn); //tested

    /**
     *@brief Execute a set of Aver instructions and return their boolean result.
     *@param exec the path to execute.
     *@return bool the result of execution.
     */
    static bool execute(const AverExecutionPathId& exec);

    /**
     *@brief Sets the database on which queries can be made.
     *@param db the PlanDatabase.
     */
    static void setDb(const PlanDatabaseId& db) {s_db = db;}
    //static AverExecutionPathId setup(const PlanDatabaseId& db, const std::string& xml);

    /**
     *@brief Destructor.  This has to be public to make BeOS happy.
     */
    ~AverHelper() {}
  protected:
    friend class AverHelperTest;
    //  private:
    AverHelper() {}

    /**
     *@brief Internal function for compiling the query parts of an assertion.  Called from buildExecutionPath.
     *@param xml the XML representing the queries involved in an assertion.
     *@return an Id representing the compiled path.
     */
    static AverExecutionPathId _buildExecutionPath(const TiXmlElement* xml); //tested

    /**
     *@brief Internal function for compiling the parts of a Tokens, Objects, or Transactions query that specify properties on the queried
     * objects.
     *@param xml the XML for the speficiation section.
     *@param action the trim action to be taken
     *@return the ExecutionPath for the trimming of a general Tokens query.
     */
    static AverExecutionPathId buildTrimPath(const TiXmlElement* xml, 
                                             const AverExecutionPath::Actions action); //tested

    /**
     *@brief Internal function for compiling Aver xml and suffixing it to the given action.
     *@param action the action that needs to appear at the front of the path.
     *@param xml the XML to compile
     *@return the Id for the compiled path.
     */
    static AverExecutionPathId buildPathAtEnd(const AverExecutionPath::Actions action,
                                              const TiXmlElement* xml); //tested

    /**
     *@brief Internal function for compiling Aver xml and prefixing it with the given action.
     *@param action the action that needs to appear at the end of the path.
     *@param xml the XML to compile
     *@return the Id for the compiled path.
     */
    static AverExecutionPathId buildPathAtFront(const AverExecutionPath::Actions action,
                                                const TiXmlElement* xml); //tested

    /**
     *@brief Internal function for compiling the Entity query.
     *@param xml the XML to compile.
     *@return the Id for the compiled path.
     */
    static AverExecutionPathId buildEntityPath(const TiXmlElement* xml); //tested

    /**
     *@brief Internal function for creating a EUROPA 2 enumerated domain object from its XML representation.
     *@param dom the XML representation of the enumerated domain.
     *@param dieEarly if true, Aver will delete the returned DomanContainer as early as possible.
     *@return a pointer to the container of the EnumeratedDomain object.
     */
    static DomainContainer* evaluateEnumDomain(const TiXmlElement* dom, bool dieEarly = true); //tested

    /**
     *@brief Internal function for creating a EUROPA 2 interval domain object from its XML representation.
     *@param dom the XML representation of the interval domain.
     *@param dieEarly if true, Aver will delete the returned DomanContainer as early as possible.
     *@return a pointer to the container of the IntervalDomain object.
     */
    static DomainContainer* evaluateIntDomain(const TiXmlElement* dom, bool dieEarly = true); //tested

    /**
     *@brief Internal function for getting the largest value in any domain.
     *@param dom the Domain whose greatest value is to be gotten.
     *@return the greatest value in dom.
     */
    static double getGreatest(const AbstractDomain& dom); //tested

    /**
     *@brief Internal function for getting the smallest value in any domain.
     *@param dom the Domain whose least value is to be gotten.
     *@return the least value in dom.
     */
    static double getLeast(const AbstractDomain& dom); //tested

    /**
     *@brief The implementation of the Count function in Aver.  Expects its argument on the stack and places the count on the stack in
     *an EnumeratedDomain.
     *@param _ (on the domain stack) an EnumeratedDomain to count.
     *@return (on the domain stack) a singleton EnumeratedDomain containing the count.
     */
    static void count(); //tested

    /**
     *@brief The implementation of the Tokens function in Aver.  This queries all tokens, which are trimmed by later functions.  Places
     * its return value (the set of token ids) on the stack.
     *@return (on the domain stack) an EnumeratedDomain containing the set of all TokenIds.
     */
    static void queryTokens(); //tested

    /**
     *@brief The implementation of the Objects function in Aver.  This queries all objects, which are trimmed by later functions.  Places
     * its return value (the set of object ids) on the stack.
     *@return (on the domain stack) an EnumeratedDomain containing the set of all ObjectIds.
     */
    static void queryObjects(); //tested

    /**
     *@brief The implementation of the Property function in Aver.  Expects it's arguments on the stack and places it's return value on the 
     *stack.
     *@param _ (on the domain stack) an EnumeratedDomain to be operated on to specify the name of the Property
     *@param _ (on the op stack) a string representing the boolean operator for the name specification
     *@param _ (on the domain stack) an EnumeratedDomain (currently required to be singleton) of TokenIds or ObjectIds whose named property
     * is to be gotten.
     *@return (on the domain stack) the domain of the specified property.
     */
    static void property(); //tested

    /**
     *@brief The implementation of the Entity function in Aver.
     *@param _ (on the domain stack) a singleton domain containing the index into the second domain argument.
     *@param _ (on the domain stack) an domain whose ith element is to be gotten.
     *@return (on the domain stack) a singleton EnumeratedDomain containing the ith element.
     */
    static void entity(); //tested

    /**
     *@brief The implementation of the predicate specification section of a Tokens query in Aver.
     *@param _ (on the op stack) the comparison operator between the Tokens' predicates and the argument domain
     *@param _ (on the domain stack) an EnumeratedDomain containing the strings to which the Tokens' predicates will be compared.
     *@param _ (on the domain stack) a domain containing the TokenIds to be trimmed.
     *@return (on the domain stack) a domain containing the TokenIds whose predicates pass the boolean test.
     */
    static void trimTokensByPredicate(); //tested
    
    /**
     *@brief The implementation of the variable specification section of a Tokens query in Aver.
     *@param _ (on the op stack) the comparison operator between the Tokens' variable names and the name argument domain.
     *@param _ (on the op stack) the comparison operator between the Tokens' variables' domains and the argument domain.
     *@param _ (on the domain stack) a domain containing the strings to which the Tokens' variable names will be compared.
     *@param _ (on the domain stack) the domain to which the Tokens' variables' domains will be compared.
     *@param _ (on the domain stack) a domain containing the TokenIds to be trimmed.
     *@return (on the domain stack) a domain containing the TokenIds whose variables pass the boolean test.
     */
    static void trimTokensByVariable(); //tested

    /**
     *@brief The implementation of the start variable specification section of a Tokens query in Aver.
     *@param _ (on the op stack) the comparison operator between the Tokens' start variables' domains and the argument domain.
     *@param _ (on the domain stack) the domain to which the Tokens' start variables' domains will be compared.
     *@param _ (on the domain stack) a domain containin the TokenIds to be trimmed.
     *@return (on the domain stack) a domain containing the TokenIds whose start variables pass the boolean test.
     */
    static void trimTokensByStart(); //tested

    /**
     *@brief The implementation of the end variable specification section of a Tokens query in Aver.
     *@param _ (on the op stack) the comparison operator between the Tokens' end variables' domains and the argument domain.
     *@param _ (on the domain stack) the domain to which the Tokens' end variables' domains will be compared.
     *@param _ (on the domain stack) a domain containin the TokenIds to be trimmed.
     *@return (on the domain stack) a domain containing the TokenIds whose end variables pass the boolean test.
     */
    static void trimTokensByEnd(); //tested

    /**
     *@brief The implementation of the status variable specification section of a Tokens query in Aver.
     *@param _ (on the op stack) the comparison operator between the Tokens' status variables' domains and the argument domain.
     *@param _ (on the domain stack) the domain to which the Tokens' status variables' domains will be compared.
     *@param _ (on the domain stack) a domain containin the TokenIds to be trimmed.
     *@return (on the domain stack) a domain containing the TokenIds whose status variables pass the boolean test.
     */
    static void trimTokensByStatus(); //tested

    /**
     *@brief The implementation of the name specification section of an Objects query in Aver.
     *@param _ (on the op stack) the comparison operator between the Objects' names and the argument domain
     *@param _ (on the domain stack) an EnumeratedDomain containing the strings to which the Objects' names will be compared.
     *@param _ (on the domain stack) a domain containing the ObjectIds to be trimmed.
     *@return (on the domain stack) a domain containing the ObjectIds whose names pass the boolean test.
     */    
    static void trimObjectsByName(); //tested

    /**
     *@brief The implementation of the variable specification section of a Objects query in Aver.
     *@param _ (on the op stack) the comparison operator between the Objects' variable names and the name argument domain.
     *@param _ (on the op stack) the comparison operator between the Objects' variables' domains and the argument domain.
     *@param _ (on the domain stack) a domain containing the strings to which the Objects' variable names will be compared.
     *@param _ (on the domain stack) the domain to which the Objects' variables' domains will be compared.
     *@param _ (on the domain stack) a domain containing the ObjectIds to be trimmed.
     *@return (on the domain stack) a domain containing the ObjectIds whose variables pass the boolean test.
     */
    static void trimObjectsByVariable(); //tested

    /**
     *@brief The implementation of the path specification section of a Tokens query in Aver. (NOT CURRENTLY IMPLEMENTED).
     */
    static void trimTokensByPath(); //unimplemented right now

    /**
     *@brief Evaluates a boolean operation on two domains and returns the result.
     *@param _ (on the op stack) the boolean operator to be evaluated.
     *@param _ (on the domain stack) the RHS of the statement.
     *@param _ (on the domain stack) the LHS of the statement.
     *@return the result of the boolean evaluation
     */
    static bool boolOp(); //tested

    /**
     *@brief Converts a TokenSet into an EnumeratedDomain.
     *@param tokens the set of TokenIds
     *@return the domain containing the set of TokenIds as doubles
     */
    static DomainContainer* tokenSetToDomain(const TokenSet& tokens); //tested
    
    /**
     *@brief Converts an EnumeratedDomain into a TokenSet.
     *@param dom the domain containing the set of TokenIds as doubles.
     *@return the set of TokenIds.
     */
    static TokenSet domainToTokenSet(const DomainContainer* dom); //tested

    /**
     *@brief Converts a ObjectSet into an EnumeratedDomain.
     *@param tokens the set of ObjectIds
     *@return the domain containing the set of ObjectIds as doubles
     */
    static DomainContainer* objectSetToDomain(const ObjectSet& objs); //tested

    /**
     *@brief Converts an EnumeratedDomain into a ObjectSet.
     *@param dom the domain containing the set of ObjectIds as doubles.
     *@return the set of ObjectIds.
     */
    static ObjectSet domainToObjectSet(const DomainContainer* dom); //tested

    static PlanDatabaseId s_db; /*!< The database on which Token and Object queries are made. */
    //static std::stack<AbstractDomain*> s_domStack; /*!< A stack for domains. */
    static std::stack<DomainContainer*> s_domStack; /*!< A stack for domains. */
    static std::stack<std::string> s_opStack; /*!< A stack for operators. */
    //static const bool debug = true;
    static const bool debug = false; /*!< A debug flag for VERY verbose output. */
  };
}

#endif
