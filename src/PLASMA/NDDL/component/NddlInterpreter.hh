/*
 * NddlInterpreter.h
 *
 *  Created on: Jan 21, 2009
 *      Author: javier
 */

#ifndef NDDLINTERPRETER_H_
#define NDDLINTERPRETER_H_

#include "Interpreter.hh"

namespace EUROPA {


class NddlInterpreter : public LanguageInterpreter
{
public:
    NddlInterpreter(EngineId& engine);
    virtual ~NddlInterpreter();
    virtual std::string interpret(std::istream& input, const std::string& source);

    std::string getFilename(const std::string& f);

protected:
    EngineId m_engine;

    std::vector<std::string> getIncludePath();
};

// An Interpreter that just returns the AST
class NddlToASTInterpreter : public NddlInterpreter
{
public:
    NddlToASTInterpreter(EngineId& engine);
    virtual ~NddlToASTInterpreter();
    virtual std::string interpret(std::istream& input, const std::string& source);
};

class NddlSymbolTable : public EvalContext
{
public:
    NddlSymbolTable(const EngineId& engine);
    NddlSymbolTable(NddlSymbolTable* parent);
    virtual ~NddlSymbolTable();

    NddlSymbolTable* getParentST();

    const PlanDatabaseId& getPlanDatabase() const;

    virtual DataTypeId getDataType(const char* name) const;
    AbstractDomain* makeNumericDomainFromLiteral(const std::string& type,const std::string& value);

    // Error reporting methods
    void reportError(void* treeWalker, const std::string& msg);
    void addError(const std::string& msg);
    std::string getErrors() const;

    // EvalContext methods
    virtual ConstrainedVariableId getVar(const char* name);
    virtual void* getElement(const char* name) const;

    // Enum support methods
    void addEnumValues(const char* enumName,const std::vector<std::string>& values);
    bool isEnumValue(const char* value) const;
    Expr* makeEnumRef(const char* value) const;

protected:
    NddlSymbolTable* m_parentST;

    EngineId m_engine;
    std::vector<std::string> m_errors;
    std::map<std::string,std::string> m_enumValues; // Hack to keep track of enum values for the time being

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

protected:
    ObjectType* m_objectType; // Object type being declared
};

}


#endif /* NDDLINTERPRETER_H_ */
