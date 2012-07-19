#ifndef _H_ANMLPSInterpreter
#define _H_ANMLPSInterpreter

#include "PSEngineImpl.hh"
#include "ANMLTranslator.hh"

namespace EUROPA {

  class ANMLPSInterpreter : public PSLanguageInterpreter {
  public:
    ANMLPSInterpreter() {}
    std::string interpret(const std::string& script);
  private:
    ANML::ANMLTranslator m_anmlTranslator;
  };
}

#endif
