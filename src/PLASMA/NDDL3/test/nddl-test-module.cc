#include "nddl-test-module.hh"

#include "NddlInterpreter.hh"

#include "NDDL3Lexer.h"
#include "NDDL3Parser.h"
#include "NDDL3Tree.h"

#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"

#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleNddl.hh"

using namespace EUROPA;


class NddlTestEngine : public EngineBase
{
  public:
    NddlTestEngine();
    virtual ~NddlTestEngine();

  protected:
    virtual void createModules();
};

NddlTestEngine::NddlTestEngine()
{
    createModules();
    doStart();
}

NddlTestEngine::~NddlTestEngine()
{
    doShutdown();
}

void NddlTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
    addModule((new ModuleRulesEngine())->getId());
    addModule((new ModuleTemporalNetwork())->getId());
    addModule((new ModuleNddl())->getId());
}

void NDDLModuleTests::syntaxTests()
{
    std::string filename="parser.nddl";

    NddlTestEngine engine;

    pANTLR3_INPUT_STREAM input = antlr3AsciiFileStreamNew((pANTLR3_UINT8)filename.c_str());
    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);

    // Build he AST
    NDDL3Parser_nddl_return result = parser->nddl(parser);
    if (parser->pParser->rec->state->errorCount > 0)
      std::cerr << "The parser returned " << parser->pParser->rec->state->errorCount << " errors" << std::endl;
    else
      std::cout << "NDDL AST:\n" << result.tree->toStringTree(result.tree)->chars << std::endl;

    // Walk the AST to create nddl expr to evaluate
    pANTLR3_COMMON_TREE_NODE_STREAM nodeStream = antlr3CommonTreeNodeStreamNewTree(result.tree, ANTLR3_SIZE_HINT);
    pNDDL3Tree treeParser = NDDL3TreeNew(nodeStream);

    NddlSymbolTable symbolTable(((PlanDatabase*)engine.getComponent("PlanDatabase"))->getId());
    treeParser->SymbolTable = &symbolTable;

    Expr* treeResult = treeParser->nddl(treeParser);
    if (treeResult == NULL)
        std::cerr << "ERROR: the tree walk returned NULL:";
    else
        std::cout << "Result of tree walk:\n" << treeResult->toString() << std::endl;

    // TODO: evaluate nddl expr
    // EvalContext context;
    // treeResult->eval(context);

    treeParser->free(treeParser);
    nodeStream->free(nodeStream);

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);
}


