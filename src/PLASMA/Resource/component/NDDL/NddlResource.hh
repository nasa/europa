#ifndef NDDL_RESOURCE_HH
#define NDDL_RESOURCE_HH

#include "NddlDefs.hh"
#include "Resource.hh"
#include "Reservoir.hh"
#include "InstantTokens.hh"
#include "Reusable.hh"
#include "DurativeTokens.hh"
#include "Transaction.hh"

namespace NDDL {

  typedef EUROPA::ObjectDomain UnaryDomain;
  typedef EUROPA::ObjectDomain ResuableDomain;
  typedef EUROPA::ObjectDomain ReservoirDomain;

  class NddlUnaryToken : public EUROPA::UnaryToken {
  public:
    NddlUnaryToken(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, const bool& rejectable = false, const bool& isFact=false, const bool& close = false);
    NddlUnaryToken(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, const bool& close = false);

    StateVarId state;
    ObjectVarId object;
    TempVarId tStart;
    TempVarId tEnd;
    TempVarId tDuration;
  protected:
    virtual void handleDefaults(const bool&);
  private:
    void commonInit(const bool& autoClose);
  };

  class NddlUnary : public EUROPA::Reusable {
  public:
    NddlUnary(const PlanDatabaseId& planDatabase,
		 const LabelStr& type,
		 const LabelStr& name,
		 bool open);
    NddlUnary(const ObjectId& parent,
		 const LabelStr& type,
		 const LabelStr& name,
		 bool open);

    virtual ~NddlUnary(){}

    virtual void close();

    virtual void constructor(double c_max);
    virtual void constructor();

    void handleDefaults(bool autoClose = true);

    ConstrainedVariableId consumptionMax;

    class use : public EUROPA::ReusableToken {
    public:
      use(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, bool rejectable, bool isFact, bool close);
      use(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, bool close);

      StateVarId state;
      ObjectVarId object;
      TempVarId tStart;
      TempVarId tEnd;
      TempVarId tDuration;

      virtual void close();
    protected:
      virtual void handleDefaults(bool autoClose = true);
    private:
    };
  };

  class NddlReusable : public EUROPA::Reusable {
  public:
    NddlReusable(const PlanDatabaseId& planDatabase,
		 const LabelStr& type,
		 const LabelStr& name,
		 bool open);
    NddlReusable(const ObjectId& parent,
		 const LabelStr& type,
		 const LabelStr& name,
		 bool open);

    virtual ~NddlReusable(){}

    virtual void close();

    virtual void constructor(double c, double ll_min);
    virtual void constructor(double c, double ll_min, double cr_max);
    virtual void constructor(double c, double ll_min, double c_max, double cr_max);
    virtual void constructor();

    void handleDefaults(bool autoClose = true);

    ConstrainedVariableId capacity;
    ConstrainedVariableId levelLimitMin;
    ConstrainedVariableId consumptionRateMax;
    ConstrainedVariableId consumptionMax;

    class uses : public EUROPA::ReusableToken {
    public:
      uses(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, bool rejectable, bool isFact, bool close);
      uses(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, bool close);

      StateVarId state;
      ObjectVarId object;
      TempVarId tStart;
      TempVarId tEnd;
      TempVarId tDuration;

      ConstrainedVariableId quantity;
      virtual void close();
    protected:
      virtual void handleDefaults(bool autoClose = true);
    private:
    };
  };

  class NddlCBReusable : public EUROPA::CBReusable {
  public:
    NddlCBReusable(const PlanDatabaseId& planDatabase,
         const LabelStr& type,
         const LabelStr& name,
         bool open);
    NddlCBReusable(const ObjectId& parent,
         const LabelStr& type,
         const LabelStr& name,
         bool open);

    virtual ~NddlCBReusable(){}

    virtual void close();

    virtual void constructor(float c, float ll_min);
    virtual void constructor(float c, float ll_min, float cr_max);
    virtual void constructor(float c, float ll_min, float c_max, float cr_max);
    virtual void constructor();

    void handleDefaults(bool autoClose = true);

    ConstrainedVariableId capacity;
    ConstrainedVariableId levelLimitMin;
    ConstrainedVariableId consumptionRateMax;
    ConstrainedVariableId consumptionMax;
  };

  class NddlReservoir : public EUROPA::Reservoir {
  public:
    NddlReservoir(const PlanDatabaseId& planDatabase,
		  const LabelStr& type,
		  const LabelStr& name,
		  bool open);

    NddlReservoir(const ObjectId parent,
		  const LabelStr& type,
		  const LabelStr& name,
		  bool open);

    virtual ~NddlReservoir(){}

    virtual void close();
    
    virtual void constructor(double ic, double ll_min, double ll_max);
    
    virtual void constructor(double ic, double ll_min, double ll_max, double p_max, double c_max);
    
    virtual void constructor(double ic, double ll_min, double ll_max, double pr_max, double p_max, double cr_max, double c_max);
    
    virtual void constructor();

    void handleDefaults(bool autoClose = true); // default variable initialization

    ConstrainedVariableId initialCapacity;
    ConstrainedVariableId levelLimitMin;
    ConstrainedVariableId levelLimitMax;
    ConstrainedVariableId productionRateMax;
    ConstrainedVariableId productionMax;
    ConstrainedVariableId consumptionRateMax;
    ConstrainedVariableId consumptionMax;

    class produce : public EUROPA::ProducerToken {
    public:
      produce(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, bool rejectable, bool isFact, bool close);
      produce(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, bool close);

      /* Access to primitives of a token as public members. */
      StateVarId state;
      ObjectVarId object;
      TempVarId tStart;
      TempVarId tEnd;
      TempVarId tDuration;
      TempVarId time;

      ConstrainedVariableId quantity; /*!< Add member specific for a resource */

      virtual void close();

    protected:
      virtual void handleDefaults(bool autoClose = true);

    private:
      //void commonInit();
    };

    class consume : public EUROPA::ConsumerToken {
    public:
      consume(const PlanDatabaseId& planDatabase, const LabelStr& predicateName, bool rejectable, bool isFact, bool close);
      consume(const TokenId& master, const LabelStr& predicateName, const LabelStr& relation, bool close);

      /* Access to primitives of a token as public members. */
      StateVarId state;
      ObjectVarId object;
      TempVarId tStart;
      TempVarId tEnd;
      TempVarId tDuration;
      TempVarId time;

      ConstrainedVariableId quantity; /*!< Add member specific for a resource */

      virtual void close();

    protected:
      virtual void handleDefaults(bool autoClose = true);

    private:
      //void commonInit();
    };

  protected:
  private:
  };

} // namespace NDDL

#endif // NDDL_RESOURCE_HH
