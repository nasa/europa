
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

#ifndef _H_DispGraph
#define _H_DispGraph

#include "DistanceGraph.hh"

namespace EUROPA {

class DispatchNode;

typedef int Referent;  // Expected to be an index into a caller array.

/**
     * @class  DispatchGraph
     * @author Paul H. Morris
     * @brief Specialization of DistanceGraph for dispatching activities 
     * from a Simple Temporal Network 
     * in the case when the timing of external events is not
     * under the full control of agent we have planned for.
     *
     * Efficiency is paramount since we have to do a full propagation
     * from each node.  We use the algorithm of
     * Tsamardinos, Muscettola, Morris in AAAI-98.
     * This is a fairly straight port of the Lisp version (file filter.lisp)
     * used in the RAX experiment.
     *
     * The definition of class Dnode must come before class DispatchGraph
     * so that DispatchGraph::createNode may be defined in the DispatchGraph
     * class definition.
     *
     * @ingroup TemporalNetwork
    */

class DispatchGraph : public DistanceGraph {
  std::vector<DispatchNode*> sccLeaders;  // Strongly Connected Component leaders.
  DispatchNode** reversePostorder;   // A scratch list for ordering nodes.
public:
  DispatchNode* createNode(Referent name);
  void createEdge(DispatchNode* from, DispatchNode* to, Time length);
  void filter( void (*keepEdge)(DispatchNode*, DispatchNode*, Time) );
  // Constructor & Destructor
  DispatchGraph () {}
  // Destructor inherited from DistanceGraph is ok.
private:
  void findSccs( void (*keepEdge)(DispatchNode*, DispatchNode*, Time) );
  void buildReversePostorder (std::vector<DnodeId>& nodes);
  void predGraphDfs (DispatchNode* node, int& position);
  void predGraphTraceScc (DispatchNode* node, DispatchNode* scc[],
                          size_t& sccSize, int nodeCount);
  void processScc (DispatchNode* scc[], size_t sccSize,
                   void (*keepEdge)(DispatchNode*, DispatchNode*, Time));
  void sccMoveFluids (DispatchNode* node, DispatchNode* leader);
  void sccMoveDirectional (DispatchNode* node, DispatchNode* leader,
                           Time offset,
                           DedgeId* Dnode::*ins,
                           int Dnode::*inCount,
                           DedgeId* Dnode::*outs,
                           int Dnode::*outCount,
                           int Dnode::*outSize,
                           DnodeId Dedge::*to,
                           DnodeId Dedge::*from);
  void findKeptEdges (DispatchNode* source,
                      void (*keepEdge)(DispatchNode*, DispatchNode*, Time)
                      );
protected:                          // Overridden virtual functions
  DnodeId makeNode();
};

 /**
     * @class  DispatchNode
     * @author Paul H. Morris
     * @brief Node is a Dispatch Graph
     * @ingroup TemporalNetwork
    */
class DispatchNode : public Dnode {
  friend class DispatchGraph;
  friend int compareNodes (const DispatchNode** node1,
                           const DispatchNode** node2);
  Referent name;     // Used by caller to match nodes with caller objects.
  Bool isSccMember;  // Used during SCC construction.
  Time minDistance;  // Minimum over node and ancestors, excepting source.
public:
  Referent getRef() { return name; }
};

} /* namespace Europa */

#endif
