#ifndef _TYPES_HEADER_FILE_
#define _TYPES_HEADER_FILE_

/**
 * @file SAVH_Types.hh
 * @author David Rijsman
 * @brief Defines types required to define a graph 
 * @date April 2006
 * @ingroup Resource
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <list>
#include <sstream>

#include "Error.hh"
#include "SAVH_Transaction.hh"

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
  namespace SAVH 
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

    typedef std::map< Node*, long > Node2Long;
    typedef std::map< Node*, double > Node2Double;
  
    typedef std::map< Edge*, double > Edge2DoubleMap;


    std::ostream& operator<<( std::ostream& os, const EdgeIdentity& fei ) ;
  }
}

#endif //_TYPES_HEADER_FILE_
