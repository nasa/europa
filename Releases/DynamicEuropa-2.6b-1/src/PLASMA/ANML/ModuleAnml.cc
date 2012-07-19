#include "ModuleAnml.hh"
#include "ANMLTranslator.hh"
#include "ANMLParser.hpp"
#include "ANML2NDDL.hpp"

namespace EUROPA {

  static bool & anmlInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleAnml::ModuleAnml()
      : Module("ANML")
  {	  
  }

  ModuleAnml::~ModuleAnml()
  {	  
  }  
    
  void ModuleAnml::initialize()
  {
	  if (anmlInitialized())
		  return;
	  
	  anmlInitialized() = true;
  }  

  void ModuleAnml::uninitialize()
  {
	  if (!anmlInitialized()) 		  
	      return;
  }
  
  class AnmlInterpreter : public LanguageInterpreter 
  {
    public:
      virtual ~AnmlInterpreter() {}	
      virtual std::string interpret(std::istream& input, const std::string& source);
      
    protected:
      ANML::ANMLTranslator m_anmlTranslator;
  };

  std::string AnmlInterpreter::interpret(std::istream& input, const std::string& source) {
    antlr::RefAST ast;
    // create and invoke parser
    ast = ANMLParser::parse(input,source);
    // create translator
    ANML2NDDL treeParser(m_anmlTranslator);
    // pass AST to translator
    treeParser.anml(ast);
    // return result.
    return m_anmlTranslator.toString();
  }

  void ModuleAnml::initialize(EngineId engine)
  {
	  engine->addLanguageInterpreter("anml", new AnmlInterpreter());
  }
  
  void ModuleAnml::uninitialize(EngineId engine)
  {
	  LanguageInterpreter *old = engine->removeLanguageInterpreter("anml");
	  if (old)
		  delete old;
  }
}
