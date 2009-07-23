/*
 * NddlInterpreter.h
 *
 *  Created on: Jan 21, 2009
 *      Author: javier
 */

#ifndef NDDLINTERPRETER_H_
#define NDDLINTERPRETER_H_

#include "Interpreter.hh"
#include <antlr3interfaces.h>

namespace EUROPA {

class NddlSymbolTable : public EvalContext
{
public:
    NddlSymbolTable(const EngineId& engine);
    NddlSymbolTable(NddlSymbolTable* parent);
    virtual ~NddlSymbolTable();

    NddlSymbolTable* getParentST();

    const PlanDatabaseId& getPlanDatabase() const;

    virtual void addLocalVar(const char* name,const DataTypeId& type);
    virtual void addLocalToken(const char* name,const TokenFactoryId& type);

    virtual DataTypeId getDataType(const char* name) const;
    virtual ObjectTypeId getObjectType(const char* name) const;
    virtual TokenFactoryId getTokenType(const char* name) const;

    virtual DataTypeId getTypeForVar(const char* name);
    virtual DataTypeId getTypeForVar(const char* qualifiedName,std::string& errorMsg);

    virtual TokenFactoryId getTypeForToken(const char* name);
    virtual TokenFactoryId getTypeForToken(const char* qualifiedName,std::string& errorMsg);

    AbstractDomain* makeNumericDomainFromLiteral(const std::string& type,const std::string& value);

    // Error reporting methods
    void reportError(void* treeWalker, const std::string& msg);
    void addError(const std::string& msg);
    std::string getErrors() const;

    // EvalContext methods
    virtual ConstrainedVariableId getVar(const char* name);
    virtual TokenId getToken(const char* name);
    virtual void* getElement(const char* name) const;

    // Enum support methods
    void addEnumValues(const char* enumName,const std::vector<std::string>& values);
    bool isEnumValue(const char* value) const;
    Expr* makeEnumRef(const char* value) const;

protected:
    NddlSymbolTable* m_parentST;

    EngineId m_engine;
    std::vector<std::string> m_errors;
    std::map<std::string,DataTypeId> m_localVars;
    std::map<std::string,TokenFactoryId> m_localTokens;

    // Hack to keep track of enum values for the time being
    // TODO!: drop this as it won't work over multiple invocations of the interpreter
    // which will use different Symbol Tables. This must come from the Schema
    std::map<std::string,std::string> m_enumValues;

    const EngineId& engine() const;
    std::vector<std::string>& errors();
    const std::vector<std::string>& errors() const;
    std::map<std::string,std::string>& enumValues();
    const std::map<std::string,std::string>& enumValues() const;
};

class NddlClassSymbolTable : public NddlSymbolTable
{
public:
    NddlClassSymbolTable(NddlSymbolTable* parent, ObjectType* ot);
    virtual ~NddlClassSymbolTable();

    virtual DataTypeId getDataType(const char* name) const;
    virtual ObjectTypeId getObjectType(const char* name) const;

    virtual DataTypeId getTypeForVar(const char* varName);

protected:
    ObjectType* m_objectType; // Object type being declared
};

class NddlTokenSymbolTable : public NddlSymbolTable
{
public:
    NddlTokenSymbolTable(NddlSymbolTable* parent, const TokenFactoryId& tt, const ObjectTypeId& ot);
    virtual ~NddlTokenSymbolTable();

    virtual DataTypeId getTypeForVar(const char* varName);
    virtual TokenFactoryId getTokenType(const char* name) const;

    virtual TokenFactoryId getTypeForToken(const char* name);

protected:
    TokenFactoryId m_tokenType;
    ObjectTypeId m_objectType;
};

class NddlInterpreter : public LanguageInterpreter
{
public:
    NddlInterpreter(EngineId& engine);
    virtual ~NddlInterpreter();
    virtual std::string interpret(std::istream& input, const std::string& source);

    std::string getFilename(const std::string& f);
    bool queryIncludeGuard(const std::string& f);
    void addInclude(const std::string &f);

    std::vector<std::string> getIncludePath();
    void addInputStream(pANTLR3_INPUT_STREAM in);
    char* createImplicitVariable();
protected:
    EngineId m_engine;
    std::vector<std::string> m_filesread;
    unsigned int m_varcount;
    std::vector<pANTLR3_INPUT_STREAM> m_inputstreams;

};

// An Interpreter that just returns the AST
class NddlToASTInterpreter : public NddlInterpreter
{
public:
    NddlToASTInterpreter(EngineId& engine);
    virtual ~NddlToASTInterpreter();
    virtual std::string interpret(std::istream& input, const std::string& source);
};

}


#endif /* NDDLINTERPRETER_H_ */
