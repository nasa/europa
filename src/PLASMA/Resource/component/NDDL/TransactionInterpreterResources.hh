#ifndef _H_TransactionInterpreterResources
#define _H_TransactionInterpreterResources

#include "TransactionInterpreter.hh"

namespace EUROPA {
  class ResourceObjectFactory : public NativeObjectFactory 
  { 
  	public: 
  	    ResourceObjectFactory(const LabelStr& signature);
  	    virtual ~ResourceObjectFactory(); 
  	
  	protected: 
    	virtual ObjectId makeNewObject( 
	                        const PlanDatabaseId& planDb, 
	                        const LabelStr& objectType, 
	                        const LabelStr& objectName, 
	                        const std::vector<const AbstractDomain*>& arguments) const;
  };   
  
  class ResourceChangeTokenFactory: public NativeTokenFactory 
  { 
    public: 
	  ResourceChangeTokenFactory(const LabelStr& predicateName) : NativeTokenFactory(predicateName) {}
	  
	private: 
	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };   

  class ReusableObjectFactory : public NativeObjectFactory 
  { 
  	public: 
  	    ReusableObjectFactory(const LabelStr& signature);
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
	  ReusableUsesTokenFactory(const LabelStr& predicateName) : NativeTokenFactory(predicateName) {}
	  
	private: 
	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable , bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };   
  
  class ReservoirObjectFactory : public NativeObjectFactory 
  { 
  	public: 
  	    ReservoirObjectFactory(const LabelStr& signature);
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
	  ReservoirProduceTokenFactory(const LabelStr& predicateName) : NativeTokenFactory(predicateName) {}
	  
	private: 
	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable , bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };   
  
  class ReservoirConsumeTokenFactory: public NativeTokenFactory 
  { 
    public: 
	  ReservoirConsumeTokenFactory(const LabelStr& predicateName) : NativeTokenFactory(predicateName) {}
	  
	private: 
	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable , bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;
  };     
  
  class UnaryObjectFactory : public NativeObjectFactory 
  { 
    public: 
        UnaryObjectFactory(const LabelStr& signature);
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
