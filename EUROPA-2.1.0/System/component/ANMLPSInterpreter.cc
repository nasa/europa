#include "ANMLPSInterpreter.hh"
#include "ANMLParser.hpp"
#include "ANML2NDDL.hpp"

namespace EUROPA {
  std::string ANMLPSInterpreter::interpret(const std::string& script) {
    antlr::RefAST ast;
    // create and invoke parser
    ast = ANMLParser::eval(script);
    // create translator
    ANML2NDDL treeParser(m_anmlTranslator);
    // pass AST to translator
    treeParser.anml(ast);
    // return result.
    return m_anmlTranslator.toString();

  }

  // TODO: Do this through configuration with dynamic loading instead
  class ANMLPSInterpreterLocalStatic {
  public:
    ANMLPSInterpreterLocalStatic() {
      PSEngineImpl::addLanguageInterpreter("anml", new ANMLPSInterpreter());
    }
  };
  
  namespace ANMLInterpreter {
    ANMLPSInterpreterLocalStatic s_localStatic;
  }
}
