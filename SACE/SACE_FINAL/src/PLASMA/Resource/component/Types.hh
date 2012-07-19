#ifndef _TYPES_HEADER_FILE_
#define _TYPES_HEADER_FILE_

/**
 * @file Types.hh
 * @author David Rijsman
 * @brief Defines types required to define a graph
 * @date April 2006
 * @ingroup Resource
 */

#include <ext/hash_map>
using namespace __gnu_cxx;

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <list>
#include <sstream>

#include "Error.hh"
#include "Instant.hh"
#include "Transaction.hh"

//#define TRACE_GRAPH = 1

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
    {
    public:
      size_t operator()(Node* n) const
      {
	hash<long> H;
	return H( (long) n);
      }

    };

    class EdgeHash:
      public std::unary_function<Edge*, size_t>
    {
    public:
      size_t operator()(Edge* n) const
      {
	hash<long> H;
	return H( (long) n );
      }

    };

    class TransactionIdHash:
      public std::unary_function< TransactionId, size_t>
    {
    public:
      size_t operator()(TransactionId n) const
      {
	hash<long> H;
	return H( (long) ( (Transaction*) n ) );
      }

    };

    typedef hash_map< Node*, bool, NodeHash > Node2Bool;
    typedef hash_map< Node*, int, NodeHash > Node2Int;
    typedef hash_map< Node*, long, NodeHash > Node2Long;
    typedef hash_map< Node*, double, NodeHash > Node2Double;

    typedef hash_map< Edge*, double, EdgeHash > Edge2DoubleMap;
    typedef hash_map< TransactionId, InstantId, TransactionIdHash > TransactionId2InstantId;


    std::ostream& operator<<( std::ostream& os, const EdgeIdentity& fei ) ;
}

#endif //_TYPES_HEADER_FILE_
