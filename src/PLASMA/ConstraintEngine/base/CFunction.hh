/*
 * CFunction.hh
 *
 *  Created on: Oct 2, 2009
 *      Author: jbarreir
 */

#ifndef CFUNCTION_HH_
#define CFUNCTION_HH_

#include "Id.hh"

#include <vector>

namespace EUROPA {
class DataType;
typedef Id<DataType> DataTypeId;
class CFunction;
typedef Id<CFunction> CFunctionId;

class CFunction {
public:
  CFunction(const char* name);
  virtual ~CFunction();
  
  const CFunctionId getId() const;
  
  const std::string& getName();
  
  // TODO: this API probably needs to be re-evaluated
  // TODO: CFunction should know how to evaluate itself
  virtual const char* getConstraint() = 0;
  virtual const DataTypeId getReturnType() = 0;
  virtual unsigned int getArgumentCount() = 0;
  
  virtual void checkArgTypes(const std::vector<DataTypeId>& argTypes);
  
 protected:
  CFunctionId m_id;
  std::string m_name;
};

}

#endif /* CFUNCTION_HH_ */
