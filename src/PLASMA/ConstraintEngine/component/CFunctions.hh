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

#define DECLARE_FUNCTION_TYPE(cname, fname, constraint, type, args)	\
  class cname##Function : public CFunction				\
  {									\
  public:								\
    cname##Function() : CFunction(#fname) {}				\
    virtual ~cname##Function() {}					\
    									\
    virtual const char* getConstraint() { return constraint; }		\
    virtual const DataTypeId getReturnType() { return type::instance(); } \
    virtual unsigned int getArgumentCount() { return args; }		\
    virtual void checkArgTypes(const std::vector<DataTypeId>& argTypes) {} \
  };
// Check args TODO.

DECLARE_FUNCTION_TYPE(Max, max, "maxf", FloatDT, 2);
DECLARE_FUNCTION_TYPE(Min, min, "minf", FloatDT, 2);
DECLARE_FUNCTION_TYPE(Abs, abs, "abs", FloatDT, 1);
DECLARE_FUNCTION_TYPE(Pow, pow, "pow", FloatDT, 2);
DECLARE_FUNCTION_TYPE(Sqrt, sqrt, "sqrt", FloatDT, 1);
DECLARE_FUNCTION_TYPE(Mod, mod, "mod", IntDT, 2);
DECLARE_FUNCTION_TYPE(Rand, rand, "rand", IntDT, 0);
DECLARE_FUNCTION_TYPE(Floor, floor, "floor", IntDT, 2);
DECLARE_FUNCTION_TYPE(Ceil, ceil, "ceil", IntDT, 2);
}

#endif /* CFUNCTIONS_HH_ */
