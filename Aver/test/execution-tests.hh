// k9.backtrack.moderate-transaction.xml

#include "Db.hh"
#include "NddlUtils.hh"

namespace NDDL {

  class Resource;
  typedef Id<Resource> ResourceId;
  typedef ObjectDomain ResourceDomain;
  
  class UnaryResource;
  typedef Id<UnaryResource> UnaryResourceId;
  typedef ObjectDomain UnaryResourceDomain;
  
  class Location;
  typedef Id<Location> LocationId;
  typedef ObjectDomain LocationDomain;
  
  class Target;
  typedef Id<Target> TargetId;
  typedef ObjectDomain TargetDomain;
  
  class Path;
  typedef Id<Path> PathId;
  typedef ObjectDomain PathDomain;
  
  class CHAMP_Accessable;
  typedef Id<CHAMP_Accessable> CHAMP_AccessableId;
  typedef ObjectDomain CHAMP_AccessableDomain;
  
  class OppSci_Accessable;
  typedef Id<OppSci_Accessable> OppSci_AccessableId;
  typedef ObjectDomain OppSci_AccessableDomain;
  
  class Position;
  typedef Id<Position> PositionId;
  typedef ObjectDomain PositionDomain;
  
  class Tracker;
  typedef Id<Tracker> TrackerId;
  typedef ObjectDomain TrackerDomain;
  
  class OpportunisticScience;
  typedef Id<OpportunisticScience> OpportunisticScienceId;
  typedef ObjectDomain OpportunisticScienceDomain;
  
  class CHAMP;
  typedef Id<CHAMP> CHAMPId;
  typedef ObjectDomain CHAMPDomain;
  
  class Energy;
  typedef Id<Energy> EnergyId;
  typedef ObjectDomain EnergyDomain;
  
  class Rover;
  typedef Id<Rover> RoverId;
  typedef ObjectDomain RoverDomain;
  
  class NddlWorld;
  typedef Id<NddlWorld> NddlWorldId;
  typedef ObjectDomain NddlWorldDomain;
  
  
  
  // Plasma.nddl:8 TokenStates
  typedef SymbolDomain TokenStates;
  TokenStates TokenStatesBaseDomain();
  // SKIPPING DECLARATION FOR BUILT-IN CLASS Object
  
  // SKIPPING DECLARATION FOR BUILT-IN CLASS Timeline
  
  
  // Plasma.nddl:20 Resource
  class Resource : public NddlResource {
  public:
    Resource(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Resource(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Resource(const ObjectId& parent, const LabelStr& name);
    Resource(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    class change;
    typedef Id<change> changeId;
    
    // Plasma.nddl:20 Resource
    virtual void constructor(float ic, float ll_min, float ll_max);
    
    // Plasma.nddl:20 Resource
    virtual void constructor(float ic, float ll_min, float ll_max, float p_max, float c_max);
    
    // Plasma.nddl:20 Resource
    virtual void constructor(float ic, float ll_min, float ll_max, float pr_max, float p_max, float cr_max, float c_max);
    
    // Plasma.nddl:20 Resource
    virtual void constructor();
    ConstrainedVariableId initialCapacity; // VariableWriter::declareVariable
    ConstrainedVariableId levelLimitMin; // VariableWriter::declareVariable
    ConstrainedVariableId levelLimitMax; // VariableWriter::declareVariable
    ConstrainedVariableId productionRateMax; // VariableWriter::declareVariable
    ConstrainedVariableId productionMax; // VariableWriter::declareVariable
    ConstrainedVariableId consumptionRateMax; // VariableWriter::declareVariable
    ConstrainedVariableId consumptionMax; // VariableWriter::declareVariable
    
    // Plasma.nddl:31 change
    class change : public NddlResourceTransaction {
    public:
      change(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      change(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Resource::change, Resource.change);
      
      ConstrainedVariableId quantity; // VariableWriter::declareVariable
    };
  };
  
  // Plasma.nddl:83 UnaryResource
  class UnaryResource : public Timeline {
  public:
    UnaryResource(const PlanDatabaseId& planDatabase, const LabelStr& name);
    UnaryResource(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    UnaryResource(const ObjectId& parent, const LabelStr& name);
    UnaryResource(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    virtual void constructor(); // default constructoror
    
    class uses;
    typedef Id<uses> usesId;
    
    // Plasma.nddl:84 uses
    class uses : public NddlToken {
    public:
      uses(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      uses(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(UnaryResource::uses, UnaryResource.uses);
      
    };
  };
  
  
  // k9.model.backtrack.moderate.nddl:3 Types
  typedef SymbolDomain Types;
  Types TypesBaseDomain();
  
  
  // k9.model.backtrack.moderate.nddl:4 Data
  typedef SymbolDomain Data;
  Data DataBaseDomain();
  
  // k9.model.backtrack.moderate.nddl:6 Location
  class Location : public Object {
  public:
    Location(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Location(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Location(const ObjectId& parent, const LabelStr& name);
    Location(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    virtual void constructor(); // default constructoror
    
  };
  
  // k9.model.backtrack.moderate.nddl:9 Target
  class Target : public Timeline {
  public:
    Target(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Target(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Target(const ObjectId& parent, const LabelStr& name);
    Target(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    virtual void constructor(); // default constructoror
    
    class NotTracked;
    typedef Id<NotTracked> NotTrackedId;
    class trackstart;
    typedef Id<trackstart> trackstartId;
    class Tracked;
    typedef Id<Tracked> TrackedId;
    class trackstop;
    typedef Id<trackstop> trackstopId;
    
    // k9.model.backtrack.moderate.nddl:11 NotTracked
    class NotTracked : public NddlToken {
    public:
      NotTracked(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      NotTracked(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Target::NotTracked, Target.NotTracked);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:18 trackstart
    class trackstart : public NddlToken {
    public:
      trackstart(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      trackstart(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Target::trackstart, Target.trackstart);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:33 Tracked
    class Tracked : public NddlToken {
    public:
      Tracked(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      Tracked(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Target::Tracked, Target.Tracked);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:40 trackstop
    class trackstop : public NddlToken {
    public:
      trackstop(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      trackstop(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Target::trackstop, Target.trackstop);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
  };
  
  // k9.model.backtrack.moderate.nddl:79 Path
  class Path : public Object {
  public:
    Path(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Path(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Path(const ObjectId& parent, const LabelStr& name);
    Path(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:79 Path
    virtual void constructor(const LocationId& from, const LocationId& to);
    ConstrainedVariableId m_from; // VariableWriter::declareVariable
    ConstrainedVariableId m_to; // VariableWriter::declareVariable
  };
  
  // k9.model.backtrack.moderate.nddl:88 CHAMP_Accessable
  class CHAMP_Accessable : public Object {
  public:
    CHAMP_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& name);
    CHAMP_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    CHAMP_Accessable(const ObjectId& parent, const LabelStr& name);
    CHAMP_Accessable(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:88 CHAMP_Accessable
    virtual void constructor(const LocationId& from, const TargetId& to);
    ConstrainedVariableId m_from; // VariableWriter::declareVariable
    ConstrainedVariableId m_to; // VariableWriter::declareVariable
  };
  
  // k9.model.backtrack.moderate.nddl:98 OppSci_Accessable
  class OppSci_Accessable : public Object {
  public:
    OppSci_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& name);
    OppSci_Accessable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    OppSci_Accessable(const ObjectId& parent, const LabelStr& name);
    OppSci_Accessable(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:98 OppSci_Accessable
    virtual void constructor(const LocationId& from, const TargetId& to);
    ConstrainedVariableId m_from; // VariableWriter::declareVariable
    ConstrainedVariableId m_to; // VariableWriter::declareVariable
  };
  
  // k9.model.backtrack.moderate.nddl:108 Position
  class Position : public Timeline {
  public:
    Position(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Position(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Position(const ObjectId& parent, const LabelStr& name);
    Position(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:108 Position
    virtual void constructor(const RoverId& rover);
    class At;
    typedef Id<At> AtId;
    class navigate;
    typedef Id<navigate> navigateId;
    ConstrainedVariableId m_rover; // VariableWriter::declareVariable
    
    // k9.model.backtrack.moderate.nddl:115 At
    class At : public NddlToken {
    public:
      At(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      At(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Position::At, Position.At);
      
      ConstrainedVariableId location; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:121 navigate
    class navigate : public NddlToken {
    public:
      navigate(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      navigate(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Position::navigate, Position.navigate);
      
      ConstrainedVariableId from; // VariableWriter::declareVariable
      ConstrainedVariableId to; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean_multiplier; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std_multiplier; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean_multiplier; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std_multiplier; // VariableWriter::declareVariable
    };
  };
  
  // k9.model.backtrack.moderate.nddl:174 Tracker
  class Tracker : public Timeline {
  public:
    Tracker(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Tracker(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Tracker(const ObjectId& parent, const LabelStr& name);
    Tracker(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:174 Tracker
    virtual void constructor(const RoverId& rover);
    class TrackingOff;
    typedef Id<TrackingOff> TrackingOffId;
    class trackloadgroup;
    typedef Id<trackloadgroup> trackloadgroupId;
    class LandmarksDefined;
    typedef Id<LandmarksDefined> LandmarksDefinedId;
    class StartTracking;
    typedef Id<StartTracking> StartTrackingId;
    class TrackingOn;
    typedef Id<TrackingOn> TrackingOnId;
    class trackfreeze;
    typedef Id<trackfreeze> trackfreezeId;
    class TrackingFrozen;
    typedef Id<TrackingFrozen> TrackingFrozenId;
    class trackunfreeze;
    typedef Id<trackunfreeze> trackunfreezeId;
    ConstrainedVariableId m_rover; // VariableWriter::declareVariable
    
    // k9.model.backtrack.moderate.nddl:181 TrackingOff
    class TrackingOff : public NddlToken {
    public:
      TrackingOff(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      TrackingOff(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::TrackingOff, Tracker.TrackingOff);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:186 trackloadgroup
    class trackloadgroup : public NddlToken {
    public:
      trackloadgroup(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      trackloadgroup(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::trackloadgroup, Tracker.trackloadgroup);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:199 LandmarksDefined
    class LandmarksDefined : public NddlToken {
    public:
      LandmarksDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      LandmarksDefined(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::LandmarksDefined, Tracker.LandmarksDefined);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:204 StartTracking
    class StartTracking : public NddlToken {
    public:
      StartTracking(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      StartTracking(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::StartTracking, Tracker.StartTracking);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:217 TrackingOn
    class TrackingOn : public NddlToken {
    public:
      TrackingOn(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      TrackingOn(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::TrackingOn, Tracker.TrackingOn);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:222 trackfreeze
    class trackfreeze : public NddlToken {
    public:
      trackfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      trackfreeze(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::trackfreeze, Tracker.trackfreeze);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:235 TrackingFrozen
    class TrackingFrozen : public NddlToken {
    public:
      TrackingFrozen(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      TrackingFrozen(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::TrackingFrozen, Tracker.TrackingFrozen);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:240 trackunfreeze
    class trackunfreeze : public NddlToken {
    public:
      trackunfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      trackunfreeze(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::trackunfreeze, Tracker.trackunfreeze);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
  };
  
  // k9.model.backtrack.moderate.nddl:300 OpportunisticScience
  class OpportunisticScience : public Timeline {
  public:
    OpportunisticScience(const PlanDatabaseId& planDatabase, const LabelStr& name);
    OpportunisticScience(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    OpportunisticScience(const ObjectId& parent, const LabelStr& name);
    OpportunisticScience(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:300 OpportunisticScience
    virtual void constructor(const RoverId& rover);
    class OppSciIdle;
    typedef Id<OppSciIdle> OppSciIdleId;
    class oppscidefineproc;
    typedef Id<oppscidefineproc> oppscidefineprocId;
    class OppSciProcDefined;
    typedef Id<OppSciProcDefined> OppSciProcDefinedId;
    class oppscisetparams;
    typedef Id<oppscisetparams> oppscisetparamsId;
    class OppSciParamsSet;
    typedef Id<OppSciParamsSet> OppSciParamsSetId;
    class oppscilooknow;
    typedef Id<oppscilooknow> oppscilooknowId;
    class OppSciDoneLookNow;
    typedef Id<OppSciDoneLookNow> OppSciDoneLookNowId;
    class oppscigetstatus;
    typedef Id<oppscigetstatus> oppscigetstatusId;
    ConstrainedVariableId m_rover; // VariableWriter::declareVariable
    
    // k9.model.backtrack.moderate.nddl:307 OppSciIdle
    class OppSciIdle : public NddlToken {
    public:
      OppSciIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      OppSciIdle(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:312 oppscidefineproc
    class oppscidefineproc : public NddlToken {
    public:
      oppscidefineproc(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      oppscidefineproc(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscidefineproc, OpportunisticScience.oppscidefineproc);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:325 OppSciProcDefined
    class OppSciProcDefined : public NddlToken {
    public:
      OppSciProcDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      OppSciProcDefined(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciProcDefined, OpportunisticScience.OppSciProcDefined);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:330 oppscisetparams
    class oppscisetparams : public NddlToken {
    public:
      oppscisetparams(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      oppscisetparams(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscisetparams, OpportunisticScience.oppscisetparams);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:343 OppSciParamsSet
    class OppSciParamsSet : public NddlToken {
    public:
      OppSciParamsSet(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      OppSciParamsSet(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciParamsSet, OpportunisticScience.OppSciParamsSet);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:348 oppscilooknow
    class oppscilooknow : public NddlToken {
    public:
      oppscilooknow(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      oppscilooknow(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscilooknow, OpportunisticScience.oppscilooknow);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:363 OppSciDoneLookNow
    class OppSciDoneLookNow : public NddlToken {
    public:
      OppSciDoneLookNow(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      OppSciDoneLookNow(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciDoneLookNow, OpportunisticScience.OppSciDoneLookNow);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:370 oppscigetstatus
    class oppscigetstatus : public NddlToken {
    public:
      oppscigetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      oppscigetstatus(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscigetstatus, OpportunisticScience.oppscigetstatus);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
  };
  
  // k9.model.backtrack.moderate.nddl:455 CHAMP
  class CHAMP : public Timeline {
  public:
    CHAMP(const PlanDatabaseId& planDatabase, const LabelStr& name);
    CHAMP(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    CHAMP(const ObjectId& parent, const LabelStr& name);
    CHAMP(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:455 CHAMP
    virtual void constructor(const RoverId& rover);
    class IPIdle;
    typedef Id<IPIdle> IPIdleId;
    class ipgetname;
    typedef Id<ipgetname> ipgetnameId;
    class IPHaveName;
    typedef Id<IPHaveName> IPHaveNameId;
    class ipsettarget;
    typedef Id<ipsettarget> ipsettargetId;
    class IPTargetSet;
    typedef Id<IPTargetSet> IPTargetSetId;
    class ipplaceinstrument;
    typedef Id<ipplaceinstrument> ipplaceinstrumentId;
    class IPDonePlaceInstrument;
    typedef Id<IPDonePlaceInstrument> IPDonePlaceInstrumentId;
    class ipgetstatus;
    typedef Id<ipgetstatus> ipgetstatusId;
    ConstrainedVariableId m_rover; // VariableWriter::declareVariable
    
    // k9.model.backtrack.moderate.nddl:462 IPIdle
    class IPIdle : public NddlToken {
    public:
      IPIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      IPIdle(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPIdle, CHAMP.IPIdle);
      
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:467 ipgetname
    class ipgetname : public NddlToken {
    public:
      ipgetname(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      ipgetname(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipgetname, CHAMP.ipgetname);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:483 IPHaveName
    class IPHaveName : public NddlToken {
    public:
      IPHaveName(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      IPHaveName(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPHaveName, CHAMP.IPHaveName);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:490 ipsettarget
    class ipsettarget : public NddlToken {
    public:
      ipsettarget(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      ipsettarget(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipsettarget, CHAMP.ipsettarget);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:506 IPTargetSet
    class IPTargetSet : public NddlToken {
    public:
      IPTargetSet(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      IPTargetSet(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPTargetSet, CHAMP.IPTargetSet);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:513 ipplaceinstrument
    class ipplaceinstrument : public NddlToken {
    public:
      ipplaceinstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      ipplaceinstrument(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipplaceinstrument, CHAMP.ipplaceinstrument);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:529 IPDonePlaceInstrument
    class IPDonePlaceInstrument : public NddlToken {
    public:
      IPDonePlaceInstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      IPDonePlaceInstrument(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPDonePlaceInstrument, CHAMP.IPDonePlaceInstrument);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
    };
    
    // k9.model.backtrack.moderate.nddl:536 ipgetstatus
    class ipgetstatus : public NddlToken {
    public:
      ipgetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      ipgetstatus(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipgetstatus, CHAMP.ipgetstatus);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
      ConstrainedVariableId TYPE; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_duration_std; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_mean; // VariableWriter::declareVariable
      ConstrainedVariableId m_energy_std; // VariableWriter::declareVariable
    };
  };
  
  // k9.model.backtrack.moderate.nddl:669 Energy
  class Energy : public Resource {
  public:
    Energy(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Energy(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Energy(const ObjectId& parent, const LabelStr& name);
    Energy(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:669 Energy
    virtual void constructor(float ic, float ll_min, float ll_max);
  };
  
  // k9.model.backtrack.moderate.nddl:676 Rover
  class Rover : public Object {
  public:
    Rover(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Rover(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Rover(const ObjectId& parent, const LabelStr& name);
    Rover(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9.model.backtrack.moderate.nddl:676 Rover
    virtual void constructor(float ic);
    ConstrainedVariableId m_position; // VariableWriter::declareVariable
    ConstrainedVariableId m_tracker; // VariableWriter::declareVariable
    ConstrainedVariableId m_oppsci; // VariableWriter::declareVariable
    ConstrainedVariableId m_champ; // VariableWriter::declareVariable
    ConstrainedVariableId m_energy; // VariableWriter::declareVariable
    ConstrainedVariableId BATTERY_MIN; // VariableWriter::declareVariable
    ConstrainedVariableId BATTERY_MAX; // VariableWriter::declareVariable
  };
  
  // NddlWorld.nddl:2 NddlWorld
  class NddlWorld : public Timeline {
  public:
    NddlWorld(const PlanDatabaseId& planDatabase, const LabelStr& name);
    NddlWorld(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    NddlWorld(const ObjectId& parent, const LabelStr& name);
    NddlWorld(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    class initialState;
    typedef Id<initialState> initialStateId;
    
    // NddlWorld.nddl:2 NddlWorld
    virtual void constructor(int start, int end, int maxPlannerSteps);
    
    // NddlWorld.nddl:2 NddlWorld
    virtual void constructor();
    ConstrainedVariableId m_horizonStart; // VariableWriter::declareVariable
    ConstrainedVariableId m_horizonEnd; // VariableWriter::declareVariable
    ConstrainedVariableId m_maxPlannerSteps; // VariableWriter::declareVariable
    
    // NddlWorld.nddl:9 initialState
    class initialState : public NddlToken {
    public:
      initialState(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool close = false);
      initialState(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);
      void handleDefaults(bool autoClose = true); // default variable initialization
      DECLARE_TOKEN_FACTORY(NddlWorld::initialState, NddlWorld.initialState);
      
    };
  };

} // namespace NDDL
