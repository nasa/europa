/*
 * AnmlInterpreter.cc
 *
 */

#include "AnmlInterpreter.hh"

namespace EUROPA {

AnmlInterpreter::AnmlInterpreter(EngineId& engine)
	: m_engine(engine)
{
}

AnmlInterpreter::~AnmlInterpreter()
{
}

std::string AnmlInterpreter::interpret(std::istream& input, const std::string& source)
{
  return "";
}


}
