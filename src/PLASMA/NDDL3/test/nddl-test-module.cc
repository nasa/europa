#include "nddl-test-module.hh"
#include "NDDL3Parser.h"
#include "NDDL3Lexer.h"

void NDDLModuleTests::syntaxTests()
{
    std::string filename="parser.nddl";

    pANTLR3_INPUT_STREAM input = antlr3AsciiFileStreamNew((pANTLR3_UINT8)filename.c_str());
    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);

    NDDL3Parser_nddl_return result = parser->nddl(parser);

    if (parser->pParser->rec->state->errorCount > 0) {
      fprintf(stderr, "The parser returned %d errors.\n", parser->pParser->rec->state->errorCount);
    }
    else {
      printf("NDDL AST: \n%s\n\n", result.tree->toStringTree(result.tree)->chars);
    }

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);
}


