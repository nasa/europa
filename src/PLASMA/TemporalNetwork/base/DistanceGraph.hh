
//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

#ifndef H_DistGraph
#define H_DistGraph

#include "TemporalNetworkDefs.hh"
#include "Entity.hh"

#include <climits>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <limits>
#include <boost/scoped_ptr.hpp>

namespace EUROPA {

// We require the magnitude bounds to be symmetric under negation.
// This is so that the legal values will be closed under negation.
// The following achieves that without provoking over or under flow.

#define TIME_MAX cast_basis(MAX_FINITE_TIME)
#define TIME_MIN cast_basis(MIN_FINITE_TIME)

// Following gives granularity (min separation) of Time type.
// Note that x <= y is equivalent to x < (y + TIME_TICK).
// This is used in isDistanceLessThanOrEqual.
#define TIME_TICK 1

// Following used to prevent overflow and underflow of temporal
// distance.  Note that adding an acceptable edge length to an
// acceptable distance is guaranteed not to produce
// overflow/underflow.  This allows us to only check the distance
// values that get stored in nodes rather than all distance values
// that arise in propagation (most of which are discarded).

#define MAX_LENGTH (TIME_MAX/2)       // Largest length allowed for edge
#define MIN_LENGTH (TIME_MIN/2)       // Smallest length allowed for edge

//#define POS_INFINITY (TIME_MAX - MAX_LENGTH)
//#define NEG_INFINITY (TIME_MIN - MIN_LENGTH)

#define MAX_DISTANCE (TIME_MAX/2)   // Largest propagated distance for node
#define MIN_DISTANCE (TIME_MIN/2)   // Smallest propagated distance for node

#define POS_INFINITY ((TIME_MAX/2) + 1)
#define NEG_INFINITY ((TIME_MIN/2) - 1)

#ifndef nullptr
#define nullptr 0       // Use for null pointers.
#endif


class Dnode;
class Dedge;

// Queue classes are implemented in queues.cc
class Dqueue;         // For use in Bellman-Ford algorithm.
class BucketQueue;    // For use in Dijkstra algorithm.

 /**
     * @class  DistanceGraph
     * @author Paul H. Morris (with mods by Conor McGann)
     * @date   Mon Dec 27 2004

     * @brief  A distance graph is a directed graph that has a
     *         weight associated with each edge. A directed graph
     *         is a graph whose edges are ordered pairs of nodes. That is
     *         each edge can be followed from one node to another node.
     *
     *  Class defines a primitive distance graph mechanism that implements
     *  standard algorithms such as Bellman-Ford, Dijstra etc. for answering
     *  queries such as finding the shortest path between nodes.
     *
     *  The single-source shortest-path problem is the problem of finding the
     *  shortest paths from a specifc source nodes to every over in a weighted
     *  directed graph. Dijkstra's algorithum solves this if all weights are nonnegative.
     *  The Bellman-Ford algorithum handles any weights.
     *
     * @ingroup TemporalNetwork
    */

class DistanceGraph {
  std::set<DedgeId> edges;
  Int dijkstraGeneration;
protected:
  std::vector<DnodeId> nodes; //TODO: should this be a ptr_container instead?
  boost::scoped_ptr<Dqueue> dqueue;
  boost::scoped_ptr<BucketQueue> bqueue;
  std::list<Dedge*> edgeNogoodList;

  Void attachEdge(std::vector<Dedge*>& edgeArray, Int& size, Int& count, Dedge& edge);
  Void detachEdge(std::vector<Dedge*>& edgeArray, Int& count, const Dedge& edge);

public:

  void addNode(DnodeId node);
  /**
   * @brief Create a new node and add it to the network
   * @return the new network node
   */
  DnodeId createNode();

  /**
   * @brief Remove node from network
   *
   * Detach the node's edges from all other nodes, and erase the edges.
   *          Cleanup the node's fields. Remove the node from the network's global list of edges.
   *          Call the node destructor (takes care of the edge arrays).
   * Note: this API dangerous, because whomever calls deleteNode will be holding a
   * dangling reference.
   * @param node node to remove from the network
   */
  Void deleteNode(Dnode& node);

  /**
  * @brief Add edge to the network
  * @param from start of the edge
  * @param to end of the edge
  * @param length length of the edge
  */
  Void addEdgeSpec(Dnode& from, Dnode& to, Time length);

 /**
  * @brief  Remove the constraint length from the edge lengthSpecs.
  *  If no more constraints, delete the edge, else update
  *  the edge length as the min of the lengthSpecs.
  * @param from start of the edge
  * @param to end of the edge
  * @param length length of the edge
  */
  Void removeEdgeSpec(Dnode& from, Dnode& to, Time length);

 /**
  * @brief Constructor
  */
  DistanceGraph ();

 /**
  * @brief Destructor
  */
  virtual ~DistanceGraph ();

 /**
   * @brief Textbook Bellman Ford algorithm propagation to determine network consistency.
   *
   * Broken down into initialization + incremental propagation.
   * Propagates "potentials" from initial zero values in all nodes,
   * as in Johnson's algorithm.
   * We also allow for subclasses to use specialized cycle detection
   * by providing a (here no-op) cycle detection check.
   * Changed the propagation to be more Dijkstra-like in that
   *    it uses a PriorityQueue ordered by the amount of change in potential
   *    from the previous value.  This cuts down on the amount of wasted
   *    propagation of values that are later superseded.  Also rewrote
   *    full-prop version as entirely separate function.
   */
  Bool bellmanFord();


  /**
   * @brief incremental Bellman Ford propagation algorithm
   */

  Bool incBellmanFord();

   /**
   * @brief Standard algorithm for finding the shortest paths from a single node to all other nodes in a
   *        weighted graph.
   * @param source start node
   * @param destination terminal node (optional)
   */
  Void dijkstra(Dnode& source, Dnode* destination = NULL);

   /**
   * @brief Incremental version of Dijkstra's algorithum
   */
  Void incDijkstra(Int generation, DnodeId destination);

  /***
   * @brief Bounded propagation bidirectional version of Dijkstra's
   * algorithm. Propagation limited to reach nodes within a bound
   * distance from source with given min/max potential.
   */
  Void boundedDijkstraForward (Dnode& source,
                               Time bound,
                               Time minPotential) {
    boundedDijkstra (source, bound, minPotential, +1);
  }

  Void boundedDijkstraBackward (Dnode& source,
                                Time bound,
                                Time maxPotential) {
    boundedDijkstra (source, bound, maxPotential, -1);
  }
private:
  Void boundedDijkstra (Dnode& source,
                        Time bound,
                        Time destPotential,
                        int direction);
public:

   /**
   * @brief compute distance from node to all other nodes in network
   * @param node start node.
   */
  Time getDistance(const Dnode& node);

   /**
   * @brief Determine if distance between nodes is less than bound
   * @param from start node.
   * @param to end node.
   * @param bound time bound
   * @return true iff distance (from, to) < bound
   */
   Bool isDistanceLessThan (Dnode& from, Dnode& to, Time bound);


   /**
   * @brief test if node is a member of the network.
   * @return true iff node is valid, false otherwise.
   */
  Bool isValid(const Dnode& node);

  /**
   * @brief Produce a string representation of the network
   * @return String containing the edges in the network.
   */
  std::string toString() const;

protected:

  /**
   * @brief Identify edge instance that connects nodes
   * @param from start of edge
   * @param to end of edge
   * @return return the edge that connects from to to if such an edge exists.
   *         returns a null edge otherwise.
   */
  Dedge* findEdge(Dnode& from, Dnode& to);

  /**
   * @brief test if network contain node
   * @param node to test membership
   * @return true iff network contains node, false otherwise.
   */
  bool hasNode(const Dnode& node) const;

  Dqueue& initializeDqueue();

  BucketQueue& initializeBqueue();

  /**
   * @brief If a subclass does not need EdgeSpec maintenance, it can call
   * createEdge directly.  (The DispatchGraph uses this feature.)
   * @param from from node
   * @param to to node
   * @param length duration of edge
   */
   Dedge* createEdge(Dnode& from, Dnode& to, Time length);

  /**
   * @brief virtual method to allows specialized Dnodes in subclasses
   */
  virtual DnodeId makeNode();

  /**
   * @brief Virtual method allows subclasses to provide specialized methods for
   * detecting cycles in graphs.
   * return false always. Specialization should return true if cycle is detected.
   */
  virtual Bool cycleDetected (const Dnode&) { return false; }

  /**
   * @brief Allow subclass to take action when a node is updated
   * @param node node updated
   */
  virtual void handleNodeUpdate(const Dnode& node);

private:
  
  DistanceGraph(const DistanceGraph&);
  DistanceGraph& operator=(const DistanceGraph&);
  Void deleteEdge(Dedge& edge);
  Void eraseEdge(Dedge& edge);
  Void preventNodeMarkOverflow();
  Void preventGenerationOverflow();
  Void updateNogoodList(Dnode&);
  Bool isAllZeroPropagationPath(Dnode& node, Dnode& targ, Time potential);
  Bool isPropagationPath(Dnode& node, Dnode& targ, Time potential);
};


 /**
     * @class  Dnode
     * @author Paul H. Morris (with mods by Conor McGann)
     * @date   Mon Dec 27 2004
     * @brief  Node in a distance graph.
     * @ingroup TemporalNetwork
    */

class Dnode : public Entity {
  friend class DistanceGraph;
  friend class BucketQueue;
  friend class Dqueue;

protected:

  // std::vector<DedgeId> inArray;
  std::vector<Dedge*> inArray;
  Int inArraySize;
  Int inCount;
  // std::vector<DedgeId> outArray;
  std::vector<Dedge*> outArray;
  Int outArraySize;
  Int outCount;
  // std::map<DnodeId,DedgeId> edgemap;
  std::map<Dnode*, Dedge*> edgemap;  //TODO: Why is this at the node level and not in the graph?
  Time distance;      // Distance from any source of propagation.
  Time potential;     // Distance from Johnson-type external source.
  Int depth;  // Depth of propagation for testing against the BF limit.
  Time key; // Used for priority ordering */
private:
  Dnode(const Dnode&);
  Dnode& operator=(const Dnode&);
public:

  Dnode* link;        // For creating linked-list of nodes (for Dqueue)
protected:
  Dedge* predecessor;      // For reconstructing negative cycles.
private:
  Int markLocal;               // Used for obsoletable marking of nodes.
  static Int markGlobal;       // Global obsolescence number for marks.
  Int generation;     // Used for obsoleting Dijkstra-calculated distances.
public:

  Dnode() : inArray(), inArraySize(0), inCount(0), outArray(),
            outArraySize(0), outCount(0), edgemap(), distance(0), potential(0), depth(0),
            key(0), link(), predecessor(), markLocal(0), generation(0) {
  }
  virtual ~Dnode() {
  }

  Time getTimeKey() { return distance - potential; }  // Used in Dijkstra
  static Void unmarkAll();
  Void mark ();
  Bool isMarked();
  Void unmark ();

  /* Key accessors */
  inline Time getKey() const {return key;}
  inline void setKey(Time t) {key = t;}

  inline Dedge* getPredecessor() const { return predecessor;}
  inline void setPredecessor(Dedge* edge) {predecessor = edge;}
};

 /**
     * @class  Dedge
     * @author Paul H. Morris (with mods by Conor McGann)
     * @date   Mon Dec 27 2004
     * @brief  Directed Edge in distance graph
     * @ingroup TemporalNetwork
    */

class Dedge {
  friend class DistanceGraph;
  std::vector<Time> lengthSpecs;

public:
  Dnode& from;
  Dnode& to;
  Time length;
  /**
   * @brief constructor
   */
  Dedge (Dnode& _from, Dnode& _to):lengthSpecs(), from(_from), to(_to), length(0) {}
  /**
   * @brief destructor
   */
  ~Dedge(){}
};

 /**
 * @class  Bucket
 * @author Paul H. Morris (with mods by Conor McGann)
 * @date   Mon Dec 27 2004
 * @brief  Utility class. Bucket elements are units stored by the BucketQueue
 * @ingroup TemporalNetwork
 */
class Bucket 
{
public:
  const Dnode* node;
  Time key;

private:
  Bucket (const Dnode* n, Time distance) : node(n), key(distance) {}

  friend class BucketQueue;
};

class BucketComparator
{
public:
	bool operator()(const Bucket& lhs,const Bucket& rhs) const
	{
		return lhs.key < rhs.key;
	}
};


typedef std::priority_queue<Bucket,std::vector<Bucket>,BucketComparator> DnodePriorityQueue;

/**
 * @class  BucketQueue
 * @author Paul H. Morris (with mods by Conor McGann)
 * @date   Mon Dec 27 2004
 * @brief  Utility class. An ordered linked-list of buckets
 * designed to give an efficient implementation of Dijkstra's algorithum for
 * finding the shortest path between nodes (where all weights are non negative).
 * @ingroup TemporalNetwork
*/
class BucketQueue {
private:
  BucketQueue(const BucketQueue&);
  BucketQueue& operator=(const BucketQueue&);
  DnodePriorityQueue buckets;
public:

  /**
   * @brief constructor
   */
  BucketQueue (Int n);

  /**
   * @brief deconstructor
   */
  ~BucketQueue ();

  /**
   * @brief delete any buckets in the queue.
   */
  Void reset();

  /**
   * @brief  Search through nodes in distance order, ignoring unmarked nodes.
   * Return first marked node found.  Pop all the nodes until
   * it is found.
   */
  Dnode* popMinFromQueue();

  /**
   * @brief insert node into queue
   * @param node node to insert
   * @param key key attached to node
   */
  Void insertInQueue(Dnode* node, eint::basis_type key);

  /**
   * @brief insert node into queue. Use (distance - potential) as key
   * @param node node to insert.
   */
  Void insertInQueue(Dnode* node);

  /**
   * @brief test if bucket is empty
   * @return true iff bucket is empty, false otherwise.
   */
  Bool isEmpty();
};

/**
 * @class Dqueue
 * @author Paul H. Morris
 * @brief Utility class. Dqueue is a straightforward adaption of the Queue
 * container class. It is used by the Bellman-Ford algorithum
 * for finding the shortest path between two nodes.
 * @ingroup TemporalNetwork
 */
class Dqueue {
  Dnode* first;
  Dnode* last;
public:
  Dqueue() : first(), last() {}
  /**
   * @brief remove all nodes from the queue
   */
  Void reset();

  /**
   * @brief insert node into queue
   * @param node node to insert
   */
  Void addToQueue (Dnode* node);

  /**
   * @brief pop first element from queue following first in first out strategy (FIFO).
   * @return head of the queue
   */
  Dnode* popFromQueue ();

  /**
   * @brief test if queue is empty
   * @return true iff queue is empty, false otherwise.
   */
  Bool isEmpty();
};

} /* namespace Europa */

#endif
