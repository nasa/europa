#ifndef H_TemporalNetworkDefs
#define H_TemporalNetworkDefs

#include "Error.hh"
#include "Id.hh"
#include <boost/shared_ptr.hpp>

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
typedef boost::shared_ptr<Dnode> DnodeId;

class Dedge;
typedef boost::shared_ptr<Dedge> DedgeId;

class Tnode;
typedef Tnode Timepoint;
typedef boost::shared_ptr<Tnode> TimepointId;

class Tspec;
typedef Tspec TemporalConstraint;
//typedef Tspec* TemporalConstraintId;
typedef boost::shared_ptr<Tspec> TemporalConstraintId;

class TemporalNetwork;
typedef TemporalNetwork* TemporalNetworkId; 

class TemporalPropagator;
typedef TemporalPropagator* TemporalPropagatorId;

class TemporalNetworkListener;
typedef TemporalNetworkListener* TemporalNetworkListenerId;

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
