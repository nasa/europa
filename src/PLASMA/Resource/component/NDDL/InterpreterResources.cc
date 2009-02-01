
#include "InterpreterResources.hh"
#include "NddlResource.hh"

namespace EUROPA {

  ResourceObjectFactory::ResourceObjectFactory(const LabelStr& signature)
    : NativeObjectFactory("Resource",signature)
  {
  }

  ResourceObjectFactory::~ResourceObjectFactory()
  {
  }

  ObjectId ResourceObjectFactory::makeNewObject(
						const PlanDatabaseId& planDb,
						const LabelStr& objectType,
						const LabelStr& objectName,
						const std::vector<const AbstractDomain*>& arguments) const
  {
    Id<NDDL::NddlResource>  instance = (new NDDL::NddlResource(planDb, objectType, objectName,true))->getId();

    std::vector<float> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back((float)(arguments[i]->getSingletonValue()));

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
      os << "Unexpected number of args in Resource constructor:" << argValues.size();
      check_runtime_error(ALWAYS_FAILS,os.str());
    }

    instance->handleDefaults(false /*don't close the object yet*/);
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native " << m_className.toString() << ":" << objectName.toString() << " type:" << objectType.toString());

    return instance;
  }

  ResourceChangeTokenFactory::ResourceChangeTokenFactory(const LabelStr& predicateName)
      : NativeTokenFactory(predicateName)
  {
      addArg("float","quantity");
  }

  TokenId ResourceChangeTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Resource.change");
    return (new NDDL::NddlResource::change(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId ResourceChangeTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Resource.change");
    return (new NDDL::NddlResource::change(master,name,relation,true))->getId();
  }

  ReusableObjectFactory::ReusableObjectFactory(const LabelStr& signature)
    : NativeObjectFactory("Reusable",signature)
  {
  }

  ReusableObjectFactory::~ReusableObjectFactory()
  {
  }

  ObjectId ReusableObjectFactory::makeNewObject(
						const PlanDatabaseId& planDb,
						const LabelStr& objectType,
						const LabelStr& objectName,
						const std::vector<const AbstractDomain*>& arguments) const
  {
    Id<NDDL::NddlReusable>  instance = (new NDDL::NddlReusable(planDb, objectType, objectName,true))->getId();

    std::vector<float> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back((float)(arguments[i]->getSingletonValue()));

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

  ReusableUsesTokenFactory::ReusableUsesTokenFactory(const LabelStr& predicateName)
      : NativeTokenFactory(predicateName)
  {
      addArg("float","quantity");
  }

  TokenId ReusableUsesTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reusable.uses");
    return (new NDDL::NddlReusable::uses(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId ReusableUsesTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reusable.uses");
    return (new NDDL::NddlReusable::uses(master,name,relation,true))->getId();
  }


  CBReusableObjectFactory::CBReusableObjectFactory(const LabelStr& signature)
    : NativeObjectFactory("CBReusable",signature)
  {
  }

  CBReusableObjectFactory::~CBReusableObjectFactory()
  {
  }

  ObjectId CBReusableObjectFactory::makeNewObject(
                        const PlanDatabaseId& planDb,
                        const LabelStr& objectType,
                        const LabelStr& objectName,
                        const std::vector<const AbstractDomain*>& arguments) const
  {
    Id<NDDL::NddlCBReusable>  instance = (new NDDL::NddlCBReusable(planDb, objectType, objectName,true))->getId();

    std::vector<float> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back((float)(arguments[i]->getSingletonValue()));

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

  ReservoirObjectFactory::ReservoirObjectFactory(const LabelStr& signature)
    : NativeObjectFactory("Reservoir",signature)
  {
  }

  ReservoirObjectFactory::~ReservoirObjectFactory()
  {
  }

  ObjectId ReservoirObjectFactory::makeNewObject(
						const PlanDatabaseId& planDb,
						const LabelStr& objectType,
						const LabelStr& objectName,
						const std::vector<const AbstractDomain*>& arguments) const
  {
    Id<NDDL::NddlReservoir>  instance = (new NDDL::NddlReservoir(planDb, objectType, objectName,true))->getId();

    std::vector<float> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back((float)(arguments[i]->getSingletonValue()));

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

  ReservoirProduceTokenFactory::ReservoirProduceTokenFactory(const LabelStr& predicateName)
      : NativeTokenFactory(predicateName)
  {
      addArg("float","quantity");
  }

  TokenId ReservoirProduceTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.produce");
    return (new NDDL::NddlReservoir::produce(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId ReservoirProduceTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.produce");
    return (new NDDL::NddlReservoir::produce(master,name,relation,true))->getId();
  }

  ReservoirConsumeTokenFactory::ReservoirConsumeTokenFactory(const LabelStr& predicateName)
      : NativeTokenFactory(predicateName)
  {
      addArg("float","quantity");
  }

  TokenId ReservoirConsumeTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.consume");
    return (new NDDL::NddlReservoir::consume(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId ReservoirConsumeTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Reservoir.consume");
    return (new NDDL::NddlReservoir::consume(master,name,relation,true))->getId();
  }


  UnaryObjectFactory::UnaryObjectFactory(const LabelStr& signature)
    : NativeObjectFactory("Unary",signature)
  {
  }

  UnaryObjectFactory::~UnaryObjectFactory()
  {
  }

  ObjectId UnaryObjectFactory::makeNewObject(
                        const PlanDatabaseId& planDb,
                        const LabelStr& objectType,
                        const LabelStr& objectName,
                        const std::vector<const AbstractDomain*>& arguments) const
  {
    Id<NDDL::NddlUnary>  instance = (new NDDL::NddlUnary(planDb, objectType, objectName,true))->getId();

    std::vector<float> argValues;
    for (unsigned int i=0;i<arguments.size();i++)
      argValues.push_back((float)(arguments[i]->getSingletonValue()));

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

  TokenId UnaryUseTokenFactory::createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Unary.Use");
    return (new NDDL::NddlUnary::use(planDb,name,rejectable,isFact,true))->getId();
  }

  TokenId UnaryUseTokenFactory::createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const
  {
    debugMsg("XMLInterpreter:NativeObjectFactory","Created Native Unary.Use");
    return (new NDDL::NddlUnary::use(master,name,relation,true))->getId();
  }

}
