#ifndef H_TransactionInterpreterResources
#define H_TransactionInterpreterResources

#include "Interpreter.hh"

namespace EUROPA {

  class ReusableObjectFactory : public NativeObjectFactory
  {
  	public:
  	    ReusableObjectFactory(const ObjectTypeId objType, const std::string& signature);
  	    virtual ~ReusableObjectFactory();

  	protected:
    	virtual ObjectId makeNewObject(
	                        const PlanDatabaseId planDb,
	                        const std::string& objectType,
	                        const std::string& objectName,
	                        const std::vector<const Domain*>& arguments) const;
  };

class ReusableUsesTokenType: public NativeTokenType {
 public:
  ReusableUsesTokenType(const ObjectTypeId ot,const std::string& predicateName);

 private:
  virtual TokenId createInstance(const PlanDatabaseId planDb, const std::string& name,
                                 bool rejectable , bool isFact) const;
  virtual TokenId createInstance(const TokenId master, const std::string& name,
                                 const std::string& relation) const;
};

  class CBReusableObjectFactory : public NativeObjectFactory
  {
    public:
        CBReusableObjectFactory(const ObjectTypeId objType, const std::string& signature);
        virtual ~CBReusableObjectFactory();

    protected:
        virtual ObjectId makeNewObject(
                            const PlanDatabaseId planDb,
                            const std::string& objectType,
                            const std::string& objectName,
                            const std::vector<const Domain*>& arguments) const;
  };

  class ReservoirObjectFactory : public NativeObjectFactory
  {
  	public:
  	    ReservoirObjectFactory(const ObjectTypeId objType, const std::string& signature);
  	    virtual ~ReservoirObjectFactory();

  	protected:
    	virtual ObjectId makeNewObject(
	                        const PlanDatabaseId planDb,
	                        const std::string& objectType,
	                        const std::string& objectName,
	                        const std::vector<const Domain*>& arguments) const;
  };

class ReservoirProduceTokenType: public NativeTokenType {
 public:
  ReservoirProduceTokenType(const ObjectTypeId ot,const std::string& predicateName);

 private:
  virtual TokenId createInstance(const PlanDatabaseId planDb, const std::string& name,
                                 bool rejectable , bool isFact) const;
  virtual TokenId createInstance(const TokenId master, const std::string& name,
                                 const std::string& relation) const;
};

class ReservoirConsumeTokenType: public NativeTokenType {
 public:
  ReservoirConsumeTokenType(const ObjectTypeId ot,const std::string& predicateName);
  
 private:
  virtual TokenId createInstance(const PlanDatabaseId planDb, const std::string& name,
                                 bool rejectable , bool isFact) const;
  virtual TokenId createInstance(const TokenId master, const std::string& name,
                                 const std::string& relation) const;
};

  class UnaryObjectFactory : public NativeObjectFactory
  {
    public:
        UnaryObjectFactory(const ObjectTypeId objType, const std::string& signature);
        virtual ~UnaryObjectFactory();

    protected:
        virtual ObjectId makeNewObject(
                            const PlanDatabaseId planDb,
                            const std::string& objectType,
                            const std::string& objectName,
                            const std::vector<const Domain*>& arguments) const;
  };

class UnaryUseTokenType: public NativeTokenType {
 public:
  UnaryUseTokenType(const ObjectTypeId ot,const std::string& predicateName) : NativeTokenType(ot,predicateName) {}

 private:
  virtual TokenId createInstance(const PlanDatabaseId planDb, const std::string& name,
                                 bool rejectable , bool isFact) const;
  virtual TokenId createInstance(const TokenId master, const std::string& name,
                                 const std::string& relation) const;
};

  class SetCapacity : public Method
  {
  public:
      SetCapacity() : Method("set_capacity") {}
      virtual ~SetCapacity() {}

      virtual DataRef eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const;

      virtual const std::vector<DataTypeId>& getSignature();
      virtual const DataTypeId getReturnType();
  };

  class SetLimit : public Method
  {
  public:
      SetLimit() : Method("set_limit") {}
      virtual ~SetLimit() {}

      virtual DataRef eval(EvalContext& context, const std::vector<ConstrainedVariableId>& args) const;

      virtual const std::vector<DataTypeId>& getSignature();
      virtual const DataTypeId getReturnType();
  };
}

#endif
