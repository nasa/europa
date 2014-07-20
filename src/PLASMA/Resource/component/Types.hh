#ifndef _TYPES_HEADER_FILE_
#define _TYPES_HEADER_FILE_

/**
 * @file Types.hh
 * @author David Rijsman
 * @brief Defines types required to define a graph
 * @date April 2006
 * @ingroup Resource
 */

// #ifdef _MSC_VER
// #  include <map>
// using std::map;
// namespace hash_src = stdext;
// #elif __clang__
// #  include "hash_map.hh"
// namespace hash_src = __gnu_cxx;
// #endif //_MSC_VER
#include <boost/unordered_map.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <list>
#include <sstream>

#include "Error.hh"
#include "Instant.hh"
#include "Transaction.hh"

//#define TRACE_GRAPH 1

#ifdef TRACE_GRAPH

#define graphDebug( msg )  { \
    std::stringstream sstr; \
    sstr << msg; \
    std::cout << sstr.str() << std::endl; \
}

#else

#define graphDebug( msg )

#endif

namespace EUROPA
{
  class Node;
  class Edge;
  class Graph;

  typedef TransactionId NodeIdentity;

  typedef std::pair< NodeIdentity, NodeIdentity > EdgeIdentity;

  typedef std::map< NodeIdentity, Node* > NodeIdentity2Node;
  typedef std::map< EdgeIdentity, Edge* > EdgeIdentity2Edge;

  typedef std::list< Edge* > EdgeList;
  typedef std::list< Node* > NodeList;

  class NodeHash: 
    public std::unary_function<Node*, size_t>
#ifdef _MSC_VER
    , public hash_compare< Node * >
#endif //_MSC_VER
  {
  public:
    size_t operator()(Node* n) const
    {
      boost::hash<long> H;
      return H( (long) n);
    }
  };

  class EdgeHash:
    public std::unary_function<Edge*, size_t>
#ifdef _MSC_VER
    , public hash_compare< Edge * >
#endif //_MSC_VER
  {
  public:
    size_t operator()(Edge* n) const
    {
      boost::hash<long> H;
      return H( (long) n );
    }
    
  };

  class TransactionIdHash:
    public std::unary_function< TransactionId, size_t>
#ifdef _MSC_VER
    , public hash_compare< TransactionId >
#endif //_MSC_VER
  {
  public:
    size_t operator()(TransactionId n) const
    {
      boost::hash<long> H;
      return H( (long) ( (Transaction*) n ) );
    }
    
  };

//TODO: Do we need to keep this _MSC_VER branch?
#ifdef _MSC_VER
  typedef map< Node*, bool > Node2Bool;
  typedef map< Node*, eint > Node2Int;
  typedef map< Node*, eint > Node2Long;
  typedef map< Node*, edouble > Node2Double;

  typedef map< Edge*, edouble > Edge2DoubleMap;
  typedef map< TransactionId, InstantId > TransactionId2InstantId;
#else
typedef boost::unordered_map< Node*, bool, NodeHash > Node2Bool;
typedef boost::unordered_map< Node*, eint, NodeHash > Node2Int;
typedef boost::unordered_map< Node*, eint, NodeHash > Node2Long;
typedef boost::unordered_map< Node*, edouble, NodeHash > Node2Double;

typedef boost::unordered_map< Edge*, edouble, EdgeHash > Edge2DoubleMap;
typedef boost::unordered_map< TransactionId, InstantId, TransactionIdHash > TransactionId2InstantId;
#endif

  std::ostream& operator<<( std::ostream& os, const EdgeIdentity& fei ) ;

} //namespace EUROPA

#endif //_TYPES_HEADER_FILE_
