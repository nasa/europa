#ifndef _H_TemporalNetworkDefs
#define _H_TemporalNetworkDefs

#include "Error.hh"
#include "Id.hh"

/*!< Type definitions to map for ht ones we were using in Europa */
namespace EUROPA {
  typedef bool Bool;
  typedef int Int;
  typedef void Void;

  typedef eint::basis_type Time;//typedef long Time;  
  //Since Time is the storage type within the temporal network, it may be beneficial to leave eint at the module interface
  //door and let the tnet only deal with longs/ints internally
  //turns out this is the case

  class Dnode;
  typedef Id<Dnode> DnodeId;

  class Dedge;
  typedef Id<Dedge> DedgeId;

  class Tnode;
  typedef Id< Tnode > TimepointId;

  class Tspec;
  typedef Id<Tspec> TemporalConstraintId;

  class TemporalNetwork;
  typedef Id<TemporalNetwork> TemporalNetworkId; 

  class TemporalPropagator;
  typedef Id<TemporalPropagator> TemporalPropagatorId;

  class TemporalNetworkListener;
  typedef Id<TemporalNetworkListener> TemporalNetworkListenerId;

  class TimepointWrapper;
  typedef Id<TimepointWrapper> TimepointWrapperId;

  class TemporalDistanceConstraint;
  typedef Id<TemporalDistanceConstraint> TemporalDistanceConstraintId;

  class ConcurrentConstraint;
  typedef Id<ConcurrentConstraint> ConcurrentConstraintId;

  class PrecedesConstraint;
  typedef Id<PrecedesConstraint> PrecedesConstraintId;

  #define noIndex -1;

  class TempNetErr {
  public:
    DECLARE_ERROR(DistanceGraphInconsistentError);
    DECLARE_ERROR(TempNetMemoryError);
    DECLARE_ERROR(TempNetInternalError);
    DECLARE_ERROR(TimeOutOfBoundsError);
    DECLARE_ERROR(TempNetInconsistentError);
    DECLARE_ERROR(TempNetInvalidTimepointError);
    DECLARE_ERROR(TempNetEmptyConstraintError);
    DECLARE_ERROR(TempNetInvalidConstraintError);
    DECLARE_ERROR(TempNetDeletingOriginError);
    DECLARE_ERROR(TempNetNoInconsistencyError);
  };
}
#endif
