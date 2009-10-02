/*
 * CFunctions.hh
 *
 *  Created on: Oct 2, 2009
 *      Author: jbarreir
 */

#ifndef CFUNCTIONS_HH_
#define CFUNCTIONS_HH_

#include "CFunction.hh"
#include "DataTypes.hh"

namespace EUROPA {

class IsSingleton : public CFunction
{
public:
	IsSingleton() : CFunction("isSingleton") {}
	virtual ~IsSingleton() {}

	virtual const char* getConstraint() { return "testSingleton"; }
	virtual const DataTypeId getReturnType() { return BoolDT::instance(); }
	virtual unsigned int getArgumentCount() { return 1; }

	// TODO: implement this
	virtual void checkArgTypes(const std::vector<DataTypeId>& argTypes) {}
};

class IsSpecified : public CFunction
{
public:
	IsSpecified() : CFunction("isSpecified") {}
	virtual ~IsSpecified() {}

	virtual const char* getConstraint() { return "testSpecified"; }
	virtual const DataTypeId getReturnType() { return BoolDT::instance(); }
	virtual unsigned int getArgumentCount() { return 1; }

	// TODO: implement this
	virtual void checkArgTypes(const std::vector<DataTypeId>& argTypes) {}
};

}

#endif /* CFUNCTIONS_HH_ */
