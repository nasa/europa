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

    Expr* result=NULL;
    parser->nddl(parser,result);

    EvalContext globalCtx(NULL);
    result->eval(globalCtx);

    delete result;

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);
}


