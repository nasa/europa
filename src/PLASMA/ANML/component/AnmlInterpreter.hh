/*
 * AnmlInterpreter.hh
 *
 *  Created on: Jul 22, 2011
 *      Author: jbarreir
 */

#ifndef ANMLINTERPRETER_HH_
#define ANMLINTERPRETER_HH_

#include "Interpreter.hh"

namespace EUROPA {

class AnmlInterpreter : public LanguageInterpreter
{
public:
    AnmlInterpreter(EngineId& engine);
    virtual ~AnmlInterpreter();
    virtual std::string interpret(std::istream& input, const std::string& source);

protected:
    EngineId m_engine;
};

}

#endif /* ANMLINTERPRETER_HH_ */
