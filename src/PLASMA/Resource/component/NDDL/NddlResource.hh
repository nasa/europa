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
  NddlUnaryToken(const EUROPA::PlanDatabaseId planDatabase,
                 const std::string& predicateName, const bool& rejectable = false,
                 const bool& isFact=false, const bool& close = false);
  NddlUnaryToken(const EUROPA::TokenId master,
                 const std::string& predicateName,
                 const std::string& relation, const bool& close = false);

  EUROPA::StateVarId state;
  EUROPA::ObjectVarId object;
  EUROPA::TempVarId tStart;
  EUROPA::TempVarId tEnd;
  EUROPA::TempVarId tDuration;
 protected:
  virtual void handleDefaults(const bool&);
 private:
  void commonInit(const bool& autoClose);
};

class NddlUnary : public EUROPA::Reusable {
 public:
  NddlUnary(const EUROPA::PlanDatabaseId planDatabase,
            const std::string& type,
            const std::string& name,
            bool open);
  NddlUnary(const EUROPA::ObjectId parent,
            const std::string& type,
            const std::string& name,
            bool open);

  virtual ~NddlUnary(){}

  virtual void close();

  virtual void constructor(const std::vector<const EUROPA::Domain*>& args) {
    Reusable::constructor(args);
  }
  virtual void constructor(EUROPA::edouble c_max);
  virtual void constructor();

  void handleDefaults(bool autoClose = true);

  EUROPA::ConstrainedVariableId consumptionMax;

  class use : public EUROPA::ReusableToken {
   public:
    use(const EUROPA::PlanDatabaseId planDatabase, const std::string& predicateName,
        bool rejectable, bool isFact, bool close);
    use(const EUROPA::TokenId master, const std::string& predicateName,
        const std::string& relation, bool close);

    EUROPA::StateVarId state;
    EUROPA::ObjectVarId object;
    EUROPA::TempVarId tStart;
    EUROPA::TempVarId tEnd;
    EUROPA::TempVarId tDuration;

    virtual void close();
   protected:
    virtual void handleDefaults(bool autoClose = true);
   private:
  };
};

class NddlReusable : public EUROPA::Reusable {
 public:
  NddlReusable(const EUROPA::PlanDatabaseId planDatabase,
               const std::string& type,
               const std::string& name,
               bool open);
  NddlReusable(const EUROPA::ObjectId parent,
               const std::string& type,
               const std::string& name,
               bool open);

  virtual ~NddlReusable(){}

  virtual void close();

  virtual void constructor(const std::vector<const EUROPA::Domain*>& args) {
    Reusable::constructor(args);
  }
  virtual void constructor(EUROPA::edouble c, EUROPA::edouble ll_min);
  virtual void constructor(EUROPA::edouble c, EUROPA::edouble ll_min,
                           EUROPA::edouble cr_max);
  virtual void constructor(EUROPA::edouble c, EUROPA::edouble ll_min,
                           EUROPA::edouble c_max, EUROPA::edouble cr_max);
  virtual void constructor();

  void handleDefaults(bool autoClose = true);

  EUROPA::ConstrainedVariableId capacity;
  EUROPA::ConstrainedVariableId levelLimitMin;
  EUROPA::ConstrainedVariableId consumptionRateMax;
  EUROPA::ConstrainedVariableId consumptionMax;

  class uses : public EUROPA::ReusableToken {
   public:
    uses(const EUROPA::PlanDatabaseId planDatabase, const std::string& predicateName,
         bool rejectable, bool isFact, bool close);
    uses(const EUROPA::TokenId master, const std::string& predicateName,
         const std::string& relation, bool close);

    EUROPA::StateVarId state;
    EUROPA::ObjectVarId object;
    EUROPA::TempVarId tStart;
    EUROPA::TempVarId tEnd;
    EUROPA::TempVarId tDuration;

    EUROPA::ConstrainedVariableId quantity;
    virtual void close();
   protected:
    virtual void handleDefaults(bool autoClose = true);
   private:
  };
};

class NddlCBReusable : public EUROPA::CBReusable {
 public:
  NddlCBReusable(const EUROPA::PlanDatabaseId planDatabase,
                 const std::string& type,
                 const std::string& name,
                 bool open);
  NddlCBReusable(const EUROPA::ObjectId parent,
                 const std::string& type,
                 const std::string& name,
                 bool open);

  virtual ~NddlCBReusable(){}

  virtual void close();

  virtual void constructor(const std::vector<const EUROPA::Domain*>& args) {
    CBReusable::constructor(args);
  }
  virtual void constructor(EUROPA::edouble c, EUROPA::edouble ll_min);
  virtual void constructor(EUROPA::edouble c, EUROPA::edouble ll_min,
                           EUROPA::edouble cr_max);
  virtual void constructor(EUROPA::edouble c, EUROPA::edouble ll_min,
                           EUROPA::edouble c_max, EUROPA::edouble cr_max);
  virtual void constructor();

  void handleDefaults(bool autoClose = true);

  EUROPA::ConstrainedVariableId capacity;
  EUROPA::ConstrainedVariableId levelLimitMin;
  EUROPA::ConstrainedVariableId consumptionRateMax;
  EUROPA::ConstrainedVariableId consumptionMax;
};

class NddlReservoir : public EUROPA::Reservoir {
 public:
  NddlReservoir(const EUROPA::PlanDatabaseId planDatabase,
                const std::string& type,
                const std::string& name,
                bool open);

  NddlReservoir(const EUROPA::ObjectId parent,
                const std::string& type,
                const std::string& name,
                bool open);

  virtual ~NddlReservoir(){}

  virtual void close();

  virtual void constructor(const std::vector<const EUROPA::Domain*>& args) {
    Reservoir::constructor(args);
  }
  virtual void constructor(EUROPA::edouble ic, EUROPA::edouble ll_min,
                           EUROPA::edouble ll_max);

  virtual void constructor(EUROPA::edouble ic, EUROPA::edouble ll_min,
                           EUROPA::edouble ll_max, EUROPA::edouble p_max,
                           EUROPA::edouble c_max);

  virtual void constructor(EUROPA::edouble ic, EUROPA::edouble ll_min,
                           EUROPA::edouble ll_max, EUROPA::edouble pr_max,
                           EUROPA::edouble p_max, EUROPA::edouble cr_max,
                           EUROPA::edouble c_max);

  virtual void constructor();

  void handleDefaults(bool autoClose = true); // default variable initialization

  EUROPA::ConstrainedVariableId initialCapacity;
  EUROPA::ConstrainedVariableId levelLimitMin;
  EUROPA::ConstrainedVariableId levelLimitMax;
  EUROPA::ConstrainedVariableId productionRateMax;
  EUROPA::ConstrainedVariableId productionMax;
  EUROPA::ConstrainedVariableId consumptionRateMax;
  EUROPA::ConstrainedVariableId consumptionMax;

  class produce : public EUROPA::ProducerToken {
   public:
    produce(const EUROPA::PlanDatabaseId planDatabase,
            const std::string& predicateName, bool rejectable, bool isFact, bool close);
    produce(const EUROPA::TokenId master, const std::string& predicateName,
            const std::string& relation, bool close);

    /* Access to primitives of a token as public members. */
    EUROPA::StateVarId state;
    EUROPA::ObjectVarId object;
    EUROPA::TempVarId tStart;
    EUROPA::TempVarId tEnd;
    EUROPA::TempVarId tDuration;
    EUROPA::TempVarId time;

    EUROPA::ConstrainedVariableId quantity; /*!< Add member specific for a resource */

    virtual void close();

   protected:
    virtual void handleDefaults(bool autoClose = true);

   private:
    //void commonInit();
  };

  class consume : public EUROPA::ConsumerToken {
   public:
    consume(const EUROPA::PlanDatabaseId planDatabase,
            const std::string& predicateName, bool rejectable, bool isFact, bool close);
    consume(const EUROPA::TokenId master, const std::string& predicateName,
            const std::string& relation, bool close);

    /* Access to primitives of a token as public members. */
    EUROPA::StateVarId state;
    EUROPA::ObjectVarId object;
    EUROPA::TempVarId tStart;
    EUROPA::TempVarId tEnd;
    EUROPA::TempVarId tDuration;
    EUROPA::TempVarId time;

    EUROPA::ConstrainedVariableId quantity; /*!< Add member specific for a resource */

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
