#ifndef _H_TransactionInterpreterResources
#define _H_TransactionInterpreterResources

#include "Interpreter.hh"

namespace EUROPA {

  class ReusableObjectFactory : public NativeObjectFactory
  {
  	public:
  	    ReusableObjectFactory(const ObjectTypeId& objType, const LabelStr& signature);
  	    virtual ~ReusableObjectFactory();

  	protected:
    	virtual ObjectId makeNewObject(
	                        const PlanDatabaseId& planDb,
	                        const LabelStr& objectType,
	                        const LabelStr& objectName,
	                        const std::vector<const AbstractDomain*>& arguments) const;
  };

  class ReusableUsesTokenFactory: public NativeTokenFactory
  {
    public:
	  ReusableUsesTokenFactory(const LabelStr& predicateName);

	private:
	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable , bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };

  class CBReusableObjectFactory : public NativeObjectFactory
  {
    public:
        CBReusableObjectFactory(const ObjectTypeId& objType, const LabelStr& signature);
        virtual ~CBReusableObjectFactory();

    protected:
        virtual ObjectId makeNewObject(
                            const PlanDatabaseId& planDb,
                            const LabelStr& objectType,
                            const LabelStr& objectName,
                            const std::vector<const AbstractDomain*>& arguments) const;
  };

  class ReservoirObjectFactory : public NativeObjectFactory
  {
  	public:
  	    ReservoirObjectFactory(const ObjectTypeId& objType, const LabelStr& signature);
  	    virtual ~ReservoirObjectFactory();

  	protected:
    	virtual ObjectId makeNewObject(
	                        const PlanDatabaseId& planDb,
	                        const LabelStr& objectType,
	                        const LabelStr& objectName,
	                        const std::vector<const AbstractDomain*>& arguments) const;
  };

  class ReservoirProduceTokenFactory: public NativeTokenFactory
  {
    public:
	  ReservoirProduceTokenFactory(const LabelStr& predicateName);

	private:
	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable , bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };

  class ReservoirConsumeTokenFactory: public NativeTokenFactory
  {
    public:
	  ReservoirConsumeTokenFactory(const LabelStr& predicateName);

	private:
	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable , bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };

  class UnaryObjectFactory : public NativeObjectFactory
  {
    public:
        UnaryObjectFactory(const ObjectTypeId& objType, const LabelStr& signature);
        virtual ~UnaryObjectFactory();

    protected:
        virtual ObjectId makeNewObject(
                            const PlanDatabaseId& planDb,
                            const LabelStr& objectType,
                            const LabelStr& objectName,
                            const std::vector<const AbstractDomain*>& arguments) const;
  };

  class UnaryUseTokenFactory: public NativeTokenFactory
  {
    public:
        UnaryUseTokenFactory(const LabelStr& predicateName) : NativeTokenFactory(predicateName) {}

    private:
      virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable , bool isFact) const;
      virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };
}

#endif
