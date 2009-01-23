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

protected:
    EngineId m_engine;
};


class NddlSymbolTable : public EvalContext
{
public:
    NddlSymbolTable(const PlanDatabaseId& pdb);
    virtual ~NddlSymbolTable();

    virtual void* getElement(const char* name) const;

    PlanDatabaseId& getPlanDatabase();

    AbstractDomain* getVarType(const char* name) const;

    void addError(const std::string& msg);
    std::string getErrors() const;

protected:
    PlanDatabaseId m_planDatabase;
    std::vector<std::string> m_errors;
};


class ExprTypedef : public Expr
{
public:
    ExprTypedef(const char* name, AbstractDomain* type);
    virtual ~ExprTypedef();

    virtual DataRef eval(EvalContext& context) const;
    virtual std::string toString() const;

protected:
    LabelStr m_name;
    AbstractDomain* m_type;
};

class ExprVarDeclaration : public Expr
{
public:
    ExprVarDeclaration(const char* name, AbstractDomain* type, Expr* initValue);
    virtual ~ExprVarDeclaration();

    virtual DataRef eval(EvalContext& context) const;
    virtual std::string toString() const;

protected:
    LabelStr m_name;
    AbstractDomain* m_type;
    Expr* m_initValue;
};

class ExprAssignment : public Expr
{
public:
    ExprAssignment(Expr* lhs, Expr* rhs);
    virtual ~ExprAssignment();

    Expr* getLhs() { return m_lhs; }
    Expr* getRhs() { return m_rhs; }

    virtual DataRef eval(EvalContext& context) const;
    virtual std::string toString() const;

protected:
    Expr* m_lhs;
    Expr* m_rhs;
};

}


#endif /* NDDLINTERPRETER_H_ */
