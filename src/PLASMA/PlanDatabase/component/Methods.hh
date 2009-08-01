/*
 * Methods.hh
 *
 *  Created on: Jul 31, 2009
 *      Author: javier
 */

#ifndef METHODS_HH_
#define METHODS_HH_

#include "Method.hh"

namespace EUROPA {

/*
 * Closes a Plan Database
 */
class PDBClose : public Method
{
public:
    PDBClose();
    virtual ~PDBClose();

    virtual DataRef eval(const std::vector<ConstrainedVariableId>& args) const;

    virtual const std::vector<DataTypeId>& getSignature();
    virtual const DataTypeId& getReturnType();
};

// TODO:
// add 'specify', 'reset', 'constrain', 'free', 'activate', 'merge', 'reject', 'cancel', 'close'
}

#endif /* METHODS_HH_ */
