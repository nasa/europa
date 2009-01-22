/*
 * NddlInterpreter.cpp
 *
 *  Created on: Jan 21, 2009
 *      Author: javier
 */

#include "NddlInterpreter.hh"

#include "EnumeratedTypeFactory.hh"
#include "IntervalTypeFactory.hh"

namespace EUROPA {

NddlSymbolTable::NddlSymbolTable(const PlanDatabaseId& pdb)
    : EvalContext(NULL)
    , m_planDatabase(pdb)
{
}

NddlSymbolTable::~NddlSymbolTable()
{
}

void* NddlSymbolTable::getElement(const char* name) const
{
    std::string str(name);
    if (str == "PlanDatabase")
        return (PlanDatabase*)m_planDatabase;

    return NULL;
}

PlanDatabaseId& NddlSymbolTable::getPlanDatabase() { return m_planDatabase; }


AbstractDomain* NddlSymbolTable::getVarType(const char* name) const
{
    CESchemaId ces = m_planDatabase->getSchema()->getCESchema();

    if (!ces->isType(name)) {
        return NULL;
        debugMsg("NddlInterpreter:SymbolTable","Unknown type " << name);
    }
    else
        return (AbstractDomain*)&(ces->baseDomain(name)); // TODO: deal with this ugly cast
}

ExprTypedef::ExprTypedef(const char* name, AbstractDomain* type)
    : m_name(name)
    , m_type(type)
{
}

ExprTypedef::~ExprTypedef()
{
}

DataRef ExprTypedef::eval(EvalContext& context) const
{
    const char* name = m_name.c_str();
    const AbstractDomain& domain = *m_type;

    debugMsg("NddlInterpreter:typedef","Defining type:" << name);

    TypeFactory* factory = NULL;
    if (m_type->isEnumerated())
      factory = new EnumeratedTypeFactory(name,name,domain);
    else
      factory = new IntervalTypeFactory(name,domain);

    debugMsg("NddlInterpreter:typedef"
            , "Created type factory " << name <<
              " with base domain " << domain.toString());

    // TODO: this is what the code generator does for every typedef, it doesn't seem right for interval types though
    PlanDatabase* pdb = (PlanDatabase*)context.getElement("PlanDatabase");
    pdb->getSchema()->addEnum(name);
    pdb->getSchema()->getCESchema()->registerFactory(factory->getId());

    return DataRef::null;
}

std::string ExprTypedef::toString() const
{
    std::ostringstream os;

    os << "TYPEDEF:" << m_type->toString() << " -> " << m_name.toString();

    return os.str();
}

ExprVarDeclaration::ExprVarDeclaration(const char* name, AbstractDomain* type, Expr* initValue)
    : m_name(name)
    , m_type(type)
    , m_initValue(initValue)
{
}

ExprVarDeclaration::~ExprVarDeclaration()
{
}

DataRef ExprVarDeclaration::eval(EvalContext& context) const
{
    // TODO: Implement this
    return DataRef::null;
}

std::string ExprVarDeclaration::toString() const
{
    std::ostringstream os;

    os << m_type->toString() << " " << m_name.toString();
    if (m_initValue != NULL)
        os << " " << m_initValue->toString();

    return os.str();
}

ExprAssignment::ExprAssignment(Expr* lhs, Expr* rhs)
    : m_lhs(lhs)
    , m_rhs(rhs)
{
}

ExprAssignment::~ExprAssignment()
{
}

DataRef ExprAssignment::eval(EvalContext& context) const
{
    // TODO: Implement this
    return DataRef::null;
}

std::string ExprAssignment::toString() const
{
    std::ostringstream os;

    os << "{ " << m_lhs->toString() << " = " << (m_rhs != NULL ? m_rhs->toString() : "NULL") << "}";

    return os.str();
}

}
