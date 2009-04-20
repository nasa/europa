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
    virtual ~NddlSymbolTable();

    virtual void* getElement(const char* name) const;

    const PlanDatabaseId& getPlanDatabase() const;

    AbstractDomain* getVarType(const char* name) const;
    AbstractDomain* makeNumericDomainFromLiteral(const std::string& type,const std::string& value);

    void addError(const std::string& msg);
    std::string getErrors() const;

    // EvalContext methods
    virtual ConstrainedVariableId getVar(const char* name);

    // Enum support methods
    void addEnumValues(const char* enumName,const std::vector<std::string>& values);
    bool isEnumValue(const char* value) const;
    Expr* makeEnumRef(const char* value) const;

    void setCurrentObjectType(ObjectType* ot);

protected:
    EngineId m_engine;
    ObjectType* m_currentObjectType; // Object type being declared, needed to manage self-refenreces
    std::vector<std::string> m_errors;

    // Hack to keep track of enum values for the time being
    std::map<std::string,std::string> m_enumValues;
};

}


#endif /* NDDLINTERPRETER_H_ */
