/*
 * Method.hh
 *
 *  Created on: Jul 31, 2009
 *      Author: javier
 */

#ifndef METHOD_HH_
#define METHOD_HH_

#include "ConstraintEngineDefs.hh"
#include "PDBInterpreter.hh"
#include <vector>

namespace EUROPA {

class Method;
typedef Id<Method> MethodId;

class Method
{
public:
    Method(const char* name);
    virtual ~Method();

    const MethodId getId() const;

    const std::string& getName() const;

    virtual DataRef eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const = 0;

    virtual const std::vector<DataTypeId>& getSignature() = 0;
    virtual const DataTypeId getReturnType() = 0;

    virtual std::string toString() const;

protected:
    MethodId m_id;
    std::string m_name;
};

}

#endif /* METHOD_HH_ */
