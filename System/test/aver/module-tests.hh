// System/test/k9-initial.xml

#include "Db.hh"
#include "NddlUtils.hh"

namespace NDDL {

  class NddlWorld;
  typedef Id<NddlWorld> NddlWorldId;
  typedef ObjectDomain NddlWorldDomain;
  
  class Location;
  typedef Id<Location> LocationId;
  typedef ObjectDomain LocationDomain;
  
  class Path;
  typedef Id<Path> PathId;
  typedef ObjectDomain PathDomain;
  
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
  
  class Rover;
  typedef Id<Rover> RoverId;
  typedef ObjectDomain RoverDomain;
  
  
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
      initialState(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      initialState(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(NddlWorld::initialState, NddlWorld.initialState);
      
    };
  };
  
  // k9-initial.nddl:5 Location
  class Location : public Timeline {
  public:
    Location(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Location(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Location(const ObjectId& parent, const LabelStr& name);
    Location(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
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
    
    // k9-initial.nddl:7 NotTracked
    class NotTracked : public NddlToken {
    public:
      NotTracked(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      NotTracked(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Location::NotTracked, Location.NotTracked);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:12 trackstart
    class trackstart : public NddlToken {
    public:
      trackstart(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      trackstart(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Location::trackstart, Location.trackstart);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:17 Tracked
    class Tracked : public NddlToken {
    public:
      Tracked(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      Tracked(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Location::Tracked, Location.Tracked);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:22 trackstop
    class trackstop : public NddlToken {
    public:
      trackstop(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      trackstop(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Location::trackstop, Location.trackstop);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
    };
  };
  
  // k9-initial.nddl:30 Path
  class Path : public Object {
  public:
    Path(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Path(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Path(const ObjectId& parent, const LabelStr& name);
    Path(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9-initial.nddl:30 Path
    virtual void constructor(const LocationId& from, const LocationId& to);
    ConstrainedVariableId m_from; // VariableWriter::declareVariable
    ConstrainedVariableId m_to; // VariableWriter::declareVariable
  };
  
  // k9-initial.nddl:39 Position
  class Position : public Timeline {
  public:
    Position(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Position(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Position(const ObjectId& parent, const LabelStr& name);
    Position(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    virtual void constructor(); // default constructoror
    
    class At;
    typedef Id<At> AtId;
    class navigate;
    typedef Id<navigate> navigateId;
    
    // k9-initial.nddl:40 At
    class At : public NddlToken {
    public:
      At(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      At(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(const bool& autoClose); // default variable initialization
      DECLARE_TOKEN_FACTORY(Position::At, Position.At);
      
      ConstrainedVariableId location; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:44 navigate
    class navigate : public NddlToken {
    public:
      navigate(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      navigate(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(const bool& autoClose); // default variable initialization
      DECLARE_TOKEN_FACTORY(Position::navigate, Position.navigate);
      
      ConstrainedVariableId from; // VariableWriter::declareVariable
      ConstrainedVariableId to; // VariableWriter::declareVariable
    };
  };
  
  // k9-initial.nddl:77 Tracker
  class Tracker : public Timeline {
  public:
    Tracker(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Tracker(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Tracker(const ObjectId& parent, const LabelStr& name);
    Tracker(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    virtual void constructor(); // default constructoror
    
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
    
    // k9-initial.nddl:79 TrackingOff
    class TrackingOff : public NddlToken {
    public:
      TrackingOff(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      TrackingOff(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::TrackingOff, Tracker.TrackingOff);
      
    };
    
    // k9-initial.nddl:82 trackloadgroup
    class trackloadgroup : public NddlToken {
    public:
      trackloadgroup(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      trackloadgroup(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::trackloadgroup, Tracker.trackloadgroup);
      
    };
    
    // k9-initial.nddl:85 LandmarksDefined
    class LandmarksDefined : public NddlToken {
    public:
      LandmarksDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      LandmarksDefined(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::LandmarksDefined, Tracker.LandmarksDefined);
      
    };
    
    // k9-initial.nddl:88 StartTracking
    class StartTracking : public NddlToken {
    public:
      StartTracking(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      StartTracking(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::StartTracking, Tracker.StartTracking);
      
    };
    
    // k9-initial.nddl:91 TrackingOn
    class TrackingOn : public NddlToken {
    public:
      TrackingOn(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      TrackingOn(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::TrackingOn, Tracker.TrackingOn);
      
    };
    
    // k9-initial.nddl:94 trackfreeze
    class trackfreeze : public NddlToken {
    public:
      trackfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      trackfreeze(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::trackfreeze, Tracker.trackfreeze);
      
    };
    
    // k9-initial.nddl:97 TrackingFrozen
    class TrackingFrozen : public NddlToken {
    public:
      TrackingFrozen(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      TrackingFrozen(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::TrackingFrozen, Tracker.TrackingFrozen);
      
    };
    
    // k9-initial.nddl:100 trackunfreeze
    class trackunfreeze : public NddlToken {
    public:
      trackunfreeze(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      trackunfreeze(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(Tracker::trackunfreeze, Tracker.trackunfreeze);
      
    };
  };
  
  // k9-initial.nddl:105 OpportunisticScience
  class OpportunisticScience : public Timeline {
  public:
    OpportunisticScience(const PlanDatabaseId& planDatabase, const LabelStr& name);
    OpportunisticScience(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    OpportunisticScience(const ObjectId& parent, const LabelStr& name);
    OpportunisticScience(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    virtual void constructor(); // default constructoror
    
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
    
    // k9-initial.nddl:107 OppSciIdle
    class OppSciIdle : public NddlToken {
    public:
      OppSciIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      OppSciIdle(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciIdle, OpportunisticScience.OppSciIdle);
      
    };
    
    // k9-initial.nddl:110 oppscidefineproc
    class oppscidefineproc : public NddlToken {
    public:
      oppscidefineproc(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      oppscidefineproc(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscidefineproc, OpportunisticScience.oppscidefineproc);
      
    };
    
    // k9-initial.nddl:113 OppSciProcDefined
    class OppSciProcDefined : public NddlToken {
    public:
      OppSciProcDefined(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      OppSciProcDefined(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciProcDefined, OpportunisticScience.OppSciProcDefined);
      
    };
    
    // k9-initial.nddl:116 oppscisetparams
    class oppscisetparams : public NddlToken {
    public:
      oppscisetparams(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      oppscisetparams(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscisetparams, OpportunisticScience.oppscisetparams);
      
    };
    
    // k9-initial.nddl:119 OppSciParamsSet
    class OppSciParamsSet : public NddlToken {
    public:
      OppSciParamsSet(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      OppSciParamsSet(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciParamsSet, OpportunisticScience.OppSciParamsSet);
      
    };
    
    // k9-initial.nddl:122 oppscilooknow
    class oppscilooknow : public NddlToken {
    public:
      oppscilooknow(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      oppscilooknow(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscilooknow, OpportunisticScience.oppscilooknow);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:128 OppSciDoneLookNow
    class OppSciDoneLookNow : public NddlToken {
    public:
      OppSciDoneLookNow(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      OppSciDoneLookNow(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::OppSciDoneLookNow, OpportunisticScience.OppSciDoneLookNow);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:134 oppscigetstatus
    class oppscigetstatus : public NddlToken {
    public:
      oppscigetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      oppscigetstatus(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(OpportunisticScience::oppscigetstatus, OpportunisticScience.oppscigetstatus);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
  };
  
  // k9-initial.nddl:141 CHAMP
  class CHAMP : public Timeline {
  public:
    CHAMP(const PlanDatabaseId& planDatabase, const LabelStr& name);
    CHAMP(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    CHAMP(const ObjectId& parent, const LabelStr& name);
    CHAMP(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    virtual void constructor(); // default constructoror
    
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
    
    // k9-initial.nddl:142 IPIdle
    class IPIdle : public NddlToken {
    public:
      IPIdle(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      IPIdle(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPIdle, CHAMP.IPIdle);
      
    };
    
    // k9-initial.nddl:145 ipgetname
    class ipgetname : public NddlToken {
    public:
      ipgetname(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      ipgetname(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipgetname, CHAMP.ipgetname);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:151 IPHaveName
    class IPHaveName : public NddlToken {
    public:
      IPHaveName(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      IPHaveName(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPHaveName, CHAMP.IPHaveName);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:156 ipsettarget
    class ipsettarget : public NddlToken {
    public:
      ipsettarget(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      ipsettarget(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipsettarget, CHAMP.ipsettarget);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:162 IPTargetSet
    class IPTargetSet : public NddlToken {
    public:
      IPTargetSet(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      IPTargetSet(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPTargetSet, CHAMP.IPTargetSet);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:167 ipplaceinstrument
    class ipplaceinstrument : public NddlToken {
    public:
      ipplaceinstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      ipplaceinstrument(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipplaceinstrument, CHAMP.ipplaceinstrument);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:173 IPDonePlaceInstrument
    class IPDonePlaceInstrument : public NddlToken {
    public:
      IPDonePlaceInstrument(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      IPDonePlaceInstrument(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::IPDonePlaceInstrument, CHAMP.IPDonePlaceInstrument);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
    
    // k9-initial.nddl:178 ipgetstatus
    class ipgetstatus : public NddlToken {
    public:
      ipgetstatus(const PlanDatabaseId& planDatabase, const LabelStr& name, const bool& rejectable = false, const bool& autoClose = false);
      ipgetstatus(const TokenId& parent, const LabelStr& relation, const LabelStr& name, const bool& autoClose = false);
      void handleDefaults(); // default variable initialization
      DECLARE_TOKEN_FACTORY(CHAMP::ipgetstatus, CHAMP.ipgetstatus);
      
      ConstrainedVariableId target; // VariableWriter::declareVariable
      ConstrainedVariableId at_loc; // VariableWriter::declareVariable
    };
  };
  
  // k9-initial.nddl:185 Rover
  class Rover : public Object {
  public:
    Rover(const PlanDatabaseId& planDatabase, const LabelStr& name);
    Rover(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Rover(const ObjectId& parent, const LabelStr& name);
    Rover(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    
    // k9-initial.nddl:185 Rover
    virtual void constructor();
    ConstrainedVariableId m_position; // VariableWriter::declareVariable
    ConstrainedVariableId m_tracker; // VariableWriter::declareVariable
    ConstrainedVariableId m_oppsci; // VariableWriter::declareVariable
    ConstrainedVariableId m_champ; // VariableWriter::declareVariable
  };

} // namespace NDDL
