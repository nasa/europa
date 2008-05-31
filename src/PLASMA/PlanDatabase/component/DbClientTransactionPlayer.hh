#ifndef _H_DbClientTransactionPlayer
#define _H_DbClientTransactionPlayer

#include "PlanDatabaseDefs.hh"
#include "TypeFactory.hh"
#include <iostream>
#include <map>
#include <list>
#include <set>


/**
 * @file DbClientTransactionPlayer
 * @brief Main interface for playing transactions.
 * @note Necessary for copy. replay, and possibly recovery.
 */

namespace EUROPA {

	class TiXmlElement;

  class DbClientTransactionPlayer {
  public:
    DbClientTransactionPlayer(const DbClientId & client);
    virtual ~DbClientTransactionPlayer();

    /**
     * @brief Play all transactions from an input stream
     * @param is a stream of xml-based transactions.
     */
    void play(std::istream& is);

    /**
     * @brief Play all transactions from a given TransactionLog
     * @param txLog the source log which has all transactions in memory
     */
    void play(const DbClientTransactionLogId& txLog);

    /**
     * @brief Play the inverses of transactions from an input stream.
     * @param is a stream of xml-based transactions
     * @param breakpoint If true, stop at the first "breakpoint" transaction
     */
    void rewind(std::istream& is, bool breakpoint = false);

    /**
     * @brief Play the inverses of transactions from a TransactionLog, in reverse order,
     *        popping reverted transactions.
     * @param txLog The source log which has all transactions in memory.
     * @param breakpoint If true, stop at the first "breakpoint" transaction
     */
    void rewind(const DbClientTransactionLogId& txLog, bool breakpoint = false);

    void setFilter(const std::set<std::string>& filters);
    static const std::set<std::string>& MODEL_TRANSACTIONS();
    static const std::set<std::string>& STATE_TRANSACTIONS();
    static const std::set<std::string>& NO_TRANSACTIONS();
  protected:
    typedef std::multimap<std::pair<ConstrainedVariableId, ConstrainedVariableId>, ConstraintId> TemporalRelations;

    bool transactionMatch(const TiXmlElement& trans, const std::string& name) const;
    bool transactionFiltered(const TiXmlElement& trans) const;
    virtual void processTransaction(const TiXmlElement & element);
    template<typename Iterator>
    void processTransactionInverse(const TiXmlElement& element,
				   Iterator start, Iterator end);

    // These are handled by code-generation
    virtual void playDeclareClass(const TiXmlElement &)       {} 
    virtual void playDefineClass(const TiXmlElement &)        {} 
    virtual void playDefineCompat(const TiXmlElement &)       {}
    virtual void playDefineEnumeration(const TiXmlElement &)  {}
    virtual void playDefineType(const TiXmlElement &)         {}
    // end code-generation

    void playVariableCreated(const TiXmlElement & element);
    void playVariableDeleted(const TiXmlElement& element); //P
    template <typename Iterator>
    void playVariableUndeleted(const TiXmlElement& element, Iterator start, Iterator end);
    void playObjectCreated(const TiXmlElement & element);
    void playObjectDeleted(const TiXmlElement& element);
    template <typename Iterator>
    void playObjectUndeleted(const TiXmlElement& element, Iterator start, Iterator end);
    void playTokenCreated(const TiXmlElement & element);
    void playTokenDeleted(const TiXmlElement& element);
    template <typename Iterator>
    void playTokenUndeleted(const TiXmlElement& element, Iterator start, Iterator end); 
    void playFactCreated(const TiXmlElement & element);
    void playConstrained(const TiXmlElement & element);
    void playFreed(const TiXmlElement & element);
    template <typename Iterator>
    void playUnfreed(const TiXmlElement& element, Iterator start, Iterator end);
    void playActivated(const TiXmlElement & element);
    void playMerged(const TiXmlElement & element);
    void playRejected(const TiXmlElement & element);
    void playCancelled(const TiXmlElement & element);
    template <typename Iterator>
    void playUncancelled(const TiXmlElement& element, Iterator start, Iterator end);
    void playVariableSpecified(const TiXmlElement & element);
    void playVariableAssigned(const TiXmlElement & element);
    void playVariableRestricted(const TiXmlElement & element);
    void playVariableReset(const TiXmlElement & element);
    template <typename Iterator>
    void playVariableUnreset(const TiXmlElement& element, Iterator start, Iterator end);
    void playInvokeConstraint(const TiXmlElement & element);
    void playUninvokeConstraint(const TiXmlElement& element);
    template <typename Iterator>
    void playUninvokeConstraint(const TiXmlElement& element, Iterator start, Iterator end);
    template <typename Iterator>
    void playReinvokeConstraint(const TiXmlElement& element, Iterator start, Iterator end);
    void playInvokeTransaction(const TiXmlElement & element);
    template <typename Iterator>
    void playUninvokeTransaction(const TiXmlElement& element, Iterator start, Iterator end);
    void playTemporalRelationCreated(const TiXmlElement& element);
    void playTemporalRelationDeleted(const TiXmlElement& element);
    TemporalRelations::iterator getTemporalConstraint(const ConstrainedVariableId& fvar,
						      const ConstrainedVariableId& svar,
						      const std::string& name);
    void deleteTemporalConstraint(TemporalRelations::iterator it);
    void removeTemporalConstraint(const ConstrainedVariableId& fvar,
				  const ConstrainedVariableId& svar,
				  const std::string& name);
    void getElementsFromConstrain(const TiXmlElement& elem, ObjectId& obj, TokenId& pred,
				  TokenId& succ);

    const CESchemaId& getCESchema() const;
    const SchemaId& getSchema() const;
    
    DbClientId m_client;
    int m_objectCount;
    int m_varCount;
    std::set<std::string> m_filters;
    std::map<std::string, TokenId> m_tokens;
    std::map<std::string, ConstrainedVariableId> m_variables;
    TemporalRelations m_relations;
    //These two seem not to be used ~MJI
//     std::list<std::string> m_enumerations;
//     std::list<std::string> m_classes;

  //! string input functions

    /** 
     * @brief read a value string as a variable identifier
     */
    ConstrainedVariableId parseVariable(const char * varString);

    /** 
     * @brief read a value string as a token identifier
     */
    TokenId parseToken(const char * tokString);

  //! XML input functions

    /** 
     * @brief create an abstract domain as represented by an xml element
     */
    const AbstractDomain * xmlAsAbstractDomain(const TiXmlElement & element,
					       const char * name = NULL,
					       const char* typeName = NULL);

    /** 
     * @brief create an interval domain as represented by an xml element
     */
    IntervalDomain * xmlAsIntervalDomain(const TiXmlElement & element,
					 const char* typeName = NULL);

    /** 
     * @brief create an enumerated domain as represented by an xml element
     */
    EnumeratedDomain * xmlAsEnumeratedDomain(const TiXmlElement & element,
					     const char* typeName = NULL);

    /** 
     * @brief return a value as represented by an xml element
     */
    double xmlAsValue(const TiXmlElement & value, const char * name = NULL);

    /** 
     * @brief return a variable as represented by an xml element
     */
    ConstrainedVariableId xmlAsVariable(const TiXmlElement & variable);

    /** 
     * @brief return a token as represented by an xml element
     */
    TokenId xmlAsToken(const TiXmlElement & token);

    /**
     * @brief return a newly created variable as represented by an xml element
     */
    ConstrainedVariableId xmlAsCreateVariable(const char * type, const char * name, const TiXmlElement * value);

    /**
     * @brief create a new token
     */
    TokenId createToken(const char* name,const char* type,bool rejectable,bool isFact);

    const char* getObjectAndType(DbClientId& client, const char* predicate,ObjectId& object) const;
    
#define construct_constraint(relation, ftoken, fvar, stoken, svar){\
        std::vector<ConstrainedVariableId> variables;\
        variables.push_back( ftoken##_token->fvar());\
        variables.push_back( stoken##_token->svar());\
        m_relations.insert(std::make_pair(std::make_pair(variables[0], variables[1]), m_client->createConstraint(#relation, variables))); \
        }
    
  };
}

#endif // _H_DbClientTransactionPlayer
