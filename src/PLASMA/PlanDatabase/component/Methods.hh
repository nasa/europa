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
    PDBClose() : Method("pdb_close") {}
    virtual ~PDBClose() {}

    virtual DataRef eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const;

    virtual const std::vector<DataTypeId>& getSignature();
    virtual const DataTypeId& getReturnType();
};

class VariableMethod : public Method
{
public:
    VariableMethod(const char* name) : Method(name) {}
    virtual ~VariableMethod() {}

    virtual DataRef eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const;

    virtual const std::vector<DataTypeId>& getSignature();
    virtual const DataTypeId& getReturnType();

protected:
    virtual DataRef eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const = 0;
};

class SpecifyVariable : public VariableMethod
{
public:
    SpecifyVariable() : VariableMethod("specify") {}
    virtual ~SpecifyVariable() {}

protected:
    virtual DataRef eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const;
};

class ResetVariable : public VariableMethod
{
public:
    ResetVariable() : VariableMethod("reset") {}
    virtual ~ResetVariable() {}

protected:
    virtual DataRef eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const;
};

class CloseVariable : public VariableMethod
{
public:
    CloseVariable() : VariableMethod("close") {}
    virtual ~CloseVariable() {}

protected:
    virtual DataRef eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const;
};


class ObjectMethod : public Method
{
public:
    ObjectMethod(const char* name) : Method(name) {}
    virtual ~ObjectMethod() {}

    virtual DataRef eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const;

    virtual const std::vector<DataTypeId>& getSignature();
    virtual const DataTypeId& getReturnType();

protected:
    virtual DataRef eval(EvalContext& context, ObjectId& obj, const std::vector<ConstrainedVariableId>& args) const = 0;
};

class ConstrainToken : public ObjectMethod
{
public:
    ConstrainToken() : ObjectMethod("constrain") {}
    virtual ~ConstrainToken() {}

protected:
    virtual DataRef eval(EvalContext& context, ObjectId& obj, const std::vector<ConstrainedVariableId>& args) const;
};

class FreeToken : public ObjectMethod
{
public:
    FreeToken() : ObjectMethod("free") {}
    virtual ~FreeToken() {}

protected:
    virtual DataRef eval(EvalContext& context, ObjectId& obj, const std::vector<ConstrainedVariableId>& args) const;
};


class TokenMethod : public Method
{
public:
    TokenMethod(const char* name) : Method(name)  {}
    virtual ~TokenMethod() {}

    virtual DataRef eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const;

    virtual const std::vector<DataTypeId>& getSignature();
    virtual const DataTypeId& getReturnType();

protected:
    virtual DataRef eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const = 0;
};

class ActivateToken : public TokenMethod
{
public:
    ActivateToken() : TokenMethod("activate") {}
    virtual ~ActivateToken() {}

protected:
    virtual DataRef eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const;
};

class MergeToken : public TokenMethod
{
public:
    MergeToken() : TokenMethod("merge") {}
    virtual ~MergeToken() {}

protected:
    virtual DataRef eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const;
};

class RejectToken : public TokenMethod
{
public:
    RejectToken() : TokenMethod("reject") {}
    virtual ~RejectToken() {}

protected:
    virtual DataRef eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const;
};

class CancelToken : public TokenMethod
{
public:
    CancelToken() : TokenMethod("cancel") {}
    virtual ~CancelToken() {}

protected:
    virtual DataRef eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const;
};

}

#endif /* METHODS_HH_ */
