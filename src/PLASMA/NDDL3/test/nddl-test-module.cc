#include "nddl-test-module.hh"

#include "Interpreter.hh"

#include "NDDL3Lexer.h"
#include "NDDL3Parser.h"
#include "NDDL3Tree.h"

using namespace EUROPA;

void NDDLModuleTests::syntaxTests()
{
    std::string filename="parser.nddl";

    pANTLR3_INPUT_STREAM input = antlr3AsciiFileStreamNew((pANTLR3_UINT8)filename.c_str());
    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);

    NDDL3Parser_nddl_return result = parser->nddl(parser);

    if (parser->pParser->rec->state->errorCount > 0)
      std::cerr << "The parser returned " << parser->pParser->rec->state->errorCount << " errors" << std::endl;
    else
      std::cout << "NDDL AST:\n" << result.tree->toStringTree(result.tree)->chars << std::endl;

    pANTLR3_COMMON_TREE_NODE_STREAM nodeStream = antlr3CommonTreeNodeStreamNewTree(result.tree, ANTLR3_SIZE_HINT);
    pNDDL3Tree treeParser = NDDL3TreeNew(nodeStream);

    Expr* treeResult = NULL;
    treeParser->nddl(treeParser,treeResult);
    if (treeResult == NULL)
        std::cerr << "ERROR: the tree walk returned NULL:";
    else
        std::cout << "Result of tree walk:\n" << treeResult->toString() << std::endl;

    treeParser->free(treeParser);
    nodeStream->free(nodeStream);

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);
}


