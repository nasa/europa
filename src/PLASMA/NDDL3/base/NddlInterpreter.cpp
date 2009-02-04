/*
 * NddlInterpreter.cpp
 *
 *  Created on: Jan 21, 2009
 *      Author: javier
 */

#include "NddlInterpreter.hh"

#include "NDDL3Lexer.h"
#include "NDDL3Parser.h"
#include "NDDL3Tree.h"

#include "Debug.hh"
#include "EnumeratedTypeFactory.hh"
#include "IntervalTypeFactory.hh"

namespace EUROPA {

// TODO: keep using pdbClient?
const DbClientId& getPDB(EvalContext& context)
{
    // TODO: Add this behavior to EvalContext instead?
    PlanDatabase* pdb = (PlanDatabase*)context.getElement("PlanDatabase");
    check_error(pdb != NULL,"Could not find Plan Database in context to evaluate asingment");
    return pdb->getClient();
}


NddlInterpreter::NddlInterpreter(EngineId& engine)
    : m_engine(engine)
{
}

NddlInterpreter::~NddlInterpreter()
{
}

pANTLR3_INPUT_STREAM getInputStream(std::istream& input, const std::string& source)
{
    if (source == "<eval>") {
        // TODO: this is kind of a hack, see if it can be made more robust & efficient
        std::istringstream* is = dynamic_cast<std::istringstream*>(&input);
        std::string strInput = is->str(); // This makes a copy of the original string that could be avoided

        return antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8)strInput.c_str(),(ANTLR3_UINT32)strInput.size(),(pANTLR3_UINT8)source.c_str());
    }
    else {
        return antlr3AsciiFileStreamNew((pANTLR3_UINT8)source.c_str());
    }
}

std::string NddlInterpreter::interpret(std::istream& ins, const std::string& source)
{
    pANTLR3_INPUT_STREAM input = getInputStream(ins,source);

    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);

    // Build he AST
    NDDL3Parser_nddl_return result = parser->nddl(parser);
    if (parser->pParser->rec->state->errorCount > 0) {
        debugMsg("NddlInterpreter:interpret","The parser returned " << parser->pParser->rec->state->errorCount << " errors");
    }
    else {
        debugMsg("NddlInterpreter:interpret","NDDL AST:\n" << result.tree->toStringTree(result.tree)->chars);
    }

    // Walk the AST to create nddl expr to evaluate
    pANTLR3_COMMON_TREE_NODE_STREAM nodeStream = antlr3CommonTreeNodeStreamNewTree(result.tree, ANTLR3_SIZE_HINT);
    pNDDL3Tree treeParser = NDDL3TreeNew(nodeStream);

    NddlSymbolTable symbolTable(((PlanDatabase*)m_engine->getComponent("PlanDatabase"))->getId());
    treeParser->SymbolTable = &symbolTable;

    treeParser->nddl(treeParser);

    // Free everything
    treeParser->free(treeParser);
    nodeStream->free(nodeStream);

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);

    return symbolTable.getErrors();
}


NddlSymbolTable::NddlSymbolTable(const PlanDatabaseId& pdb)
    : EvalContext(NULL)
    , m_planDatabase(pdb)
{
}

NddlSymbolTable::~NddlSymbolTable()
{
}

void NddlSymbolTable::addError(const std::string& msg)
{
    m_errors.push_back(msg);
}

std::string NddlSymbolTable::getErrors() const
{
    std::ostringstream os;

    for (unsigned int i=0; i<m_errors.size(); i++)
        os << m_errors[i] << std::endl;

    return os.str();
}

void* NddlSymbolTable::getElement(const char* name) const
{
    std::string str(name);
    if (str == "PlanDatabase")
        return (PlanDatabase*)m_planDatabase;

    return EvalContext::getElement(name);
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

ConstrainedVariableId NddlSymbolTable::getVar(const char* name)
{
    if (m_planDatabase->isGlobalVariable(name))
        return m_planDatabase->getGlobalVariable(name);
    else
        return ConstrainedVariableId::noId();
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
    const DbClientId& pdb = getPDB(context);

    ConstrainedVariableId v = pdb->createVariable(
            m_type->getTypeName().c_str(),
            *m_type,
            m_name.c_str()
    );

    return DataRef(v);
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
    DataRef lhs = m_lhs->eval(context);

    if (m_rhs != NULL) {
        DataRef rhs = m_rhs->eval(context);
        const DbClientId& pdb = getPDB(context);

        if (rhs.getValue()->lastDomain().isSingleton()) {
            pdb->specify(lhs.getValue(),rhs.getValue()->lastDomain().getSingletonValue());
        }
        else {
            pdb->restrict(lhs.getValue(),rhs.getValue()->lastDomain());
            // TODO: this behavior seems more reasonable, specially to support violation reporting
            // lhs.getValue()->getCurrentDomain().equate(rhs.getValue()->getCurrentDomain());
        }
    }

    return lhs;
}

std::string ExprAssignment::toString() const
{
    std::ostringstream os;

    os << "{ " << m_lhs->toString() << " = " << (m_rhs != NULL ? m_rhs->toString() : "NULL") << "}";

    return os.str();
}


ExprObjectTypeDeclaration::ExprObjectTypeDeclaration(const ObjectTypeId& objType)
    : m_objType(objType)
{
}

ExprObjectTypeDeclaration::~ExprObjectTypeDeclaration()
{
}

DataRef ExprObjectTypeDeclaration::eval(EvalContext& context) const
{
    PlanDatabase* pdb = (PlanDatabase*)context.getElement("PlanDatabase");
    pdb->getSchema()->declareObjectType(m_objType->getName());
    return DataRef::null;
}

std::string ExprObjectTypeDeclaration::toString() const
{
    std::ostringstream os;

    os << "{class " << m_objType->getName().c_str() << "}";

    return os.str();
}


ExprObjectTypeDefinition::ExprObjectTypeDefinition(const ObjectTypeId& objType)
    : m_objType(objType)
{
}

ExprObjectTypeDefinition::~ExprObjectTypeDefinition()
{
}

DataRef ExprObjectTypeDefinition::eval(EvalContext& context) const
{
    PlanDatabase* pdb = (PlanDatabase*)context.getElement("PlanDatabase");
    pdb->getSchema()->registerObjectType(m_objType);
    return DataRef::null;
}

std::string ExprObjectTypeDefinition::toString() const
{
    std::ostringstream os;

    os << m_objType->toString();

    return os.str();
}


}
