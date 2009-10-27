/*
 * Methods.cc
 *
 *  Created on: Jul 31, 2009
 *      Author: javier
 */

#include "Methods.hh"
#include "DataTypes.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"

namespace EUROPA {

// TODO: keep using pdbClient?
const DbClientId& getCtxPDB(EvalContext& context)
{
    // TODO: Add this behavior to EvalContext instead?
    DbClient* dbClient = (DbClient*)context.getElement("DbClient");
    if (dbClient != NULL)
        return dbClient->getId();

    PlanDatabase* pdb = (PlanDatabase*)context.getElement("PlanDatabase");
    check_error(pdb != NULL,"Could not find Plan Database in eval context");
    return pdb->getClient();
}

TokenId varToToken(const ConstrainedVariableId& v)
{
    StateVarId stateVar = v;
    return stateVar->getParentToken();
}

ObjectId varToObject(const ConstrainedVariableId& v)
{
    return ObjectId(v->derivedDomain().getSingletonValue());
}

DataRef PDBClose::eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const
{
    getCtxPDB(context)->close();
    return DataRef::null;
}

const std::vector<DataTypeId>& PDBClose::getSignature()
{
    static std::vector<DataTypeId> signature;
    return signature;
}

const DataTypeId& PDBClose::getReturnType()
{
    return VoidDT::instance();
}

DataRef VariableMethod::eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const
{
    ConstrainedVariableId var = args[0];
    return eval(context,var,args);
}

const std::vector<DataTypeId>& VariableMethod::getSignature()
{
    // TODO : implement this
    static std::vector<DataTypeId> signature;
    return signature;
}

const DataTypeId& VariableMethod::getReturnType()
{
    return VoidDT::instance();
}

DataRef SpecifyVariable::eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const
{
    const AbstractDomain& ad = args[1]->lastDomain();
    if (ad.isSingleton())
        getCtxPDB(context)->specify(var,ad.getSingletonValue());
    else
        getCtxPDB(context)->restrict(var,ad);

    return DataRef::null;
}

DataRef ResetVariable::eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const
{
    getCtxPDB(context)->reset(var);
    return DataRef::null;
}

DataRef CloseVariable::eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const
{
    getCtxPDB(context)->close(var);
    return DataRef::null;
}

DataRef ObjectMethod::eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const
{
    ObjectId obj = varToObject(args[0]);
    return eval(context,obj,args);
}

const std::vector<DataTypeId>& ObjectMethod::getSignature()
{
    // TODO : implement this
    static std::vector<DataTypeId> signature;
    return signature;
}

const DataTypeId& ObjectMethod::getReturnType()
{
    return VoidDT::instance();
}

DataRef ConstrainToken::eval(EvalContext& context, ObjectId& obj, const std::vector<ConstrainedVariableId>& args) const
{
    TokenId pred = varToToken(args[1]);
    TokenId succ = (args.size()==3 ? varToToken(args[2]) : pred);
    getCtxPDB(context)->constrain(obj,pred,succ);

    return DataRef::null;
}

DataRef FreeToken::eval(EvalContext& context, ObjectId& obj, const std::vector<ConstrainedVariableId>& args) const
{
    TokenId pred = varToToken(args[1]);
    TokenId succ = (args.size()==3 ? varToToken(args[2]) : pred);
    getCtxPDB(context)->free(obj,pred,succ);

    return DataRef::null;
}

DataRef TokenMethod::eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const
{
    TokenId tok = varToToken(args[0]);
    return eval(context,tok,args);
}

const std::vector<DataTypeId>& TokenMethod::getSignature()
{
    // TODO : implement this
    static std::vector<DataTypeId> signature;
    return signature;
}

const DataTypeId& TokenMethod::getReturnType()
{
    return VoidDT::instance();
}

DataRef ActivateToken::eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const
{
    if(!tok->isActive()) //Temporary.  Pull out when we scrub test input files. DbClientTransactionPlayer is doing the same
        getCtxPDB(context)->activate(tok);

    return DataRef::null;
}

DataRef MergeToken::eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const
{
    TokenId activeToken = varToToken(args[1]);
    getCtxPDB(context)->merge(tok,activeToken);
    return DataRef::null;
}

DataRef RejectToken::eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const
{
    getCtxPDB(context)->reject(tok);
    return DataRef::null;
}

DataRef CancelToken::eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const
{
    getCtxPDB(context)->cancel(tok);
    return DataRef::null;
}

DataRef CloseClass::eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const
{
    LabelStr className(args[0]->derivedDomain().getSingletonValue());
    getCtxPDB(context)->close(className.c_str());
    return DataRef::null;
}

const std::vector<DataTypeId>& CloseClass::getSignature()
{
    // TODO : implement this
    static std::vector<DataTypeId> signature;
    return signature;
}

const DataTypeId& CloseClass::getReturnType()
{
    return VoidDT::instance();
}

}
