
#include "InterpreterResources.hh"
#include "NddlResource.hh"

namespace EUROPA {

  ReusableObjectFactory::ReusableObjectFactory(const ObjectTypeId& objType, const LabelStr& signature)
    : NativeObjectFactory(objType,signature)
  {
  }

  ReusableObjectFactory::~ReusableObjectFactory()
  {
  }

  ObjectId ReusableObjectFactory::makeNewObject(
						const PlanDatabaseId& planDb,
						const LabelStr& objectType,
						const LabelStr& objectName,
						const std::vector<const Domain*>& arguments) const
  {
    Id<NDDL::NddlReusable>  instance = (new NDDL::NddlReusable(planDb, objectType, objectName,true))->getId();

    std::vector<edouble> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back(arguments[i]->getSingletonValue());

    if (argValues.size() == 0)
      instance->constructor();
    else if (argValues.size() == 2)
      instance->constructor(argValues[0],argValues[1]);
    else if (argValues.size() == 3)
      instance->constructor(argValues[0],argValues[1],argValues[2]);
    else if (argValues.size() == 4)
      instance->constructor(argValues[0],argValues[1],argValues[2],argValues[3]);
    else {
      std::ostringstream os;
      os << "Unexpected number of args in Reusable constructor:" << argValues.size();
      check_runtime_error(ALWAYS_FAILS,os.str());
    }

    instance->handleDefaults(false /*don't close the object yet*/);
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString());

    return instance;
  }

  ReusableUsesTokenType::ReusableUsesTokenType(const ObjectTypeId& ot,const LabelStr& predicateName)
      : NativeTokenType(ot,predicateName)
  {
      addArg(FloatDT::instance(),"quantity");
      addArg(IntDT::instance(),"time");
  }

  TokenId ReusableUsesTokenType::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reusable.uses");
    return (new NDDL::NddlReusable::uses(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId ReusableUsesTokenType::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reusable.uses");
    return (new NDDL::NddlReusable::uses(master,name,relation,true))->getId();
  }


  CBReusableObjectFactory::CBReusableObjectFactory(const ObjectTypeId& objType, const LabelStr& signature)
    : NativeObjectFactory(objType,signature)
  {
  }

  CBReusableObjectFactory::~CBReusableObjectFactory()
  {
  }

  ObjectId CBReusableObjectFactory::makeNewObject(
                        const PlanDatabaseId& planDb,
                        const LabelStr& objectType,
                        const LabelStr& objectName,
                        const std::vector<const Domain*>& arguments) const
  {
    Id<NDDL::NddlCBReusable>  instance = (new NDDL::NddlCBReusable(planDb, objectType, objectName,true))->getId();

    std::vector<edouble> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back(arguments[i]->getSingletonValue());

    if (argValues.size() == 0)
      instance->constructor();
    else if (argValues.size() == 2)
      instance->constructor(argValues[0],argValues[1]);
    else if (argValues.size() == 3)
      instance->constructor(argValues[0],argValues[1],argValues[2]);
    else if (argValues.size() == 4)
      instance->constructor(argValues[0],argValues[1],argValues[2],argValues[3]);
    else {
      std::ostringstream os;
      os << "Unexpected number of args in CBReusable constructor:" << argValues.size();
      check_runtime_error(ALWAYS_FAILS,os.str());
    }

    instance->handleDefaults(false /*don't close the object yet*/);
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString());

    return instance;
  }

  ReservoirObjectFactory::ReservoirObjectFactory(const ObjectTypeId& objType, const LabelStr& signature)
    : NativeObjectFactory(objType,signature)
  {
  }

  ReservoirObjectFactory::~ReservoirObjectFactory()
  {
  }

  ObjectId ReservoirObjectFactory::makeNewObject(
						const PlanDatabaseId& planDb,
						const LabelStr& objectType,
						const LabelStr& objectName,
						const std::vector<const Domain*>& arguments) const
  {
    Id<NDDL::NddlReservoir>  instance = (new NDDL::NddlReservoir(planDb, objectType, objectName,true))->getId();

    std::vector<edouble> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back(arguments[i]->getSingletonValue());

    if (argValues.size() == 0)
      instance->constructor();
    else if (argValues.size() == 3)
      instance->constructor(argValues[0],argValues[1],argValues[2]);
    else if (argValues.size() == 5)
      instance->constructor(argValues[0],argValues[1],argValues[2],argValues[3],argValues[4]);
    else if (argValues.size() == 7)
      instance->constructor(argValues[0],argValues[1],argValues[2],argValues[3],argValues[4],argValues[5],argValues[6]);
    else {
      std::ostringstream os;
      os << "Unexpected number of args in Reservoir constructor:" << argValues.size();
      check_runtime_error(ALWAYS_FAILS,os.str());
    }

    instance->handleDefaults(false /*don't close the object yet*/);
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString());

    return instance;
  }

  ReservoirProduceTokenType::ReservoirProduceTokenType(const ObjectTypeId& ot,const LabelStr& predicateName)
      : NativeTokenType(ot,predicateName)
  {
      addArg(FloatDT::instance(),"quantity");
      addArg(IntDT::instance(),"time");
  }

  TokenId ReservoirProduceTokenType::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.produce");
    return (new NDDL::NddlReservoir::produce(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId ReservoirProduceTokenType::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.produce");
    return (new NDDL::NddlReservoir::produce(master,name,relation,true))->getId();
  }

  ReservoirConsumeTokenType::ReservoirConsumeTokenType(const ObjectTypeId& ot,const LabelStr& predicateName)
      : NativeTokenType(ot,predicateName)
  {
      addArg(FloatDT::instance(),"quantity");
      addArg(IntDT::instance(),"time");
  }

  TokenId ReservoirConsumeTokenType::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.consume");
    return (new NDDL::NddlReservoir::consume(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId ReservoirConsumeTokenType::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.consume");
    return (new NDDL::NddlReservoir::consume(master,name,relation,true))->getId();
  }


  UnaryObjectFactory::UnaryObjectFactory(const ObjectTypeId& objType, const LabelStr& signature)
    : NativeObjectFactory(objType,signature)
  {
  }

  UnaryObjectFactory::~UnaryObjectFactory()
  {
  }

  ObjectId UnaryObjectFactory::makeNewObject(
                        const PlanDatabaseId& planDb,
                        const LabelStr& objectType,
                        const LabelStr& objectName,
                        const std::vector<const Domain*>& arguments) const
  {
    Id<NDDL::NddlUnary>  instance = (new NDDL::NddlUnary(planDb, objectType, objectName,true))->getId();

    std::vector<edouble> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back(arguments[i]->getSingletonValue());

    if (argValues.size() == 0)
      instance->constructor();
    else if (argValues.size() == 1)
      instance->constructor(argValues[0]);
    else {
      std::ostringstream os;
      os << "Unexpected number of args in Unary constructor:" << argValues.size();
      check_runtime_error(ALWAYS_FAILS,os.str());
    }

    instance->handleDefaults(false /*don't close the object yet*/);
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString());

    return instance;
  }

  TokenId UnaryUseTokenType::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Unary.Use");
    return (new NDDL::NddlUnary::use(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId UnaryUseTokenType::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Unary.Use");
    return (new NDDL::NddlUnary::use(master,name,relation,true))->getId();
  }

  DataRef SetCapacity::eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const
  {
	  check_runtime_error(args.size()==4,"SetCapacity expects 4 parameters");

	  ConstrainedVariableId resVar = args[0];
	  check_runtime_error(resVar->isSingleton(),"SetCapacity: resource parameter must be singleton");
	  Resource* res = dynamic_cast<Resource*>(resVar->getSingletonValue().asObject());
	  check_runtime_error(res != NULL,"SetCapacity can only be applied to a Resource, not to:" + resVar->toLongString());

	  ConstrainedVariableId timeVar = args[1];
	  check_runtime_error(timeVar->isSingleton(),"SetCapacity: time parameter must be singleton");
	  eint time = timeVar->getSingletonValue().asInt();

	  ConstrainedVariableId lbVar = args[2];
	  check_runtime_error(lbVar->isSingleton(),"SetCapacity: lowerBound parameter must be singleton");
	  edouble lb = lbVar->getSingletonValue().asDouble();

	  ConstrainedVariableId ubVar = args[3];
	  check_runtime_error(ubVar->isSingleton(),"SetCapacity: upperBound parameter must be singleton");
	  edouble ub = ubVar->getSingletonValue().asDouble();

	  res->setCapacity(time,lb,ub);

	  return DataRef::null;
  }

  const std::vector<DataTypeId>& SetCapacity::getSignature()
  {
	  // TODO : implement this
	  static std::vector<DataTypeId> signature;
	  return signature;
  }

  const DataTypeId& SetCapacity::getReturnType()
  {
	  return VoidDT::instance();
  }

  DataRef SetLimit::eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const
  {
	  check_runtime_error(args.size()==4,"SetLimit expects 4 parameters");

	  ConstrainedVariableId resVar = args[0];
	  check_runtime_error(resVar->isSingleton(),"SetLimit: resource parameter must be singleton");
	  Resource* res = dynamic_cast<Resource*>(resVar->getSingletonValue().asObject());
	  check_runtime_error(res != NULL,"SetLimit can only be applied to a Resource, not to:" + resVar->toLongString());

	  ConstrainedVariableId timeVar = args[1];
	  check_runtime_error(timeVar->isSingleton(),"SetLimit: time parameter must be singleton");
	  eint time = timeVar->getSingletonValue().asInt();

	  ConstrainedVariableId lbVar = args[2];
	  check_runtime_error(lbVar->isSingleton(),"SetLimit: lowerBound parameter must be singleton");
	  edouble lb = lbVar->getSingletonValue().asDouble();

	  ConstrainedVariableId ubVar = args[3];
	  check_runtime_error(ubVar->isSingleton(),"SetLimit: upperBound parameter must be singleton");
	  edouble ub = ubVar->getSingletonValue().asDouble();

	  res->setCapacity(time,lb,ub);

	  return DataRef::null;
  }

  const std::vector<DataTypeId>& SetLimit::getSignature()
  {
	  // TODO : implement this
	  static std::vector<DataTypeId> signature;
	  return signature;
  }

  const DataTypeId& SetLimit::getReturnType()
  {
	  return VoidDT::instance();
  }
}
