
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

/**************************************************************************
     File: queues.cc
   Author: Paul H. Morris
Purpose:
    Define queue mechanisms for dispatchability processing.
Contents:
    The implementation of the Dqueue and BucketQueue classes.
Updates:
    20000407 - PHM - Initial version
Notes:
  .
**************************************************************************/

#include "DistanceGraph.hh"
//#include "Debug.hh"

namespace EUROPA {

/* Dqueue functions */

void Dqueue::reset()
{
  this->first = DnodeId::noId();
  Dnode::unmarkAll();
}

void Dqueue::addToQueue (DnodeId node)
{
  // FIFO queue add to last.
  if (!node->isMarked()) {
    // If not already in queue...
    if (this->first.isNoId())  // queue is empty
      this->first = node;
    else                      // splice in node
      this->last->link = node;
    node->link = DnodeId::noId();
    this->last = node;
    node->mark();
  }
}

DnodeId Dqueue::popFromQueue ()
{
  // FIFO queue pop from first.
  DnodeId node = this->first;
  if (node.isNoId())
    return node;
  this->first = node->link;
  node->unmark();
  return node;
}

Bool Dqueue::isEmpty()
{
  return (this->first.isNoId());
}


/* BucketQueue functions */

BucketQueue::BucketQueue (int n)
{
  buckets = new DnodePriorityQueue();
}

BucketQueue::~BucketQueue ()
{
  if(buckets != 0)
    delete buckets;
}

void BucketQueue::reset()
{
  if(buckets != 0)
    delete buckets;

  buckets = new DnodePriorityQueue();
  Dnode::unmarkAll();
}

DnodeId BucketQueue::popMinFromQueue()
{

  DnodeId node;
  while (!buckets->empty()){
    node = buckets->top();
    buckets->pop();
    if (node->isMarked()){
      node->unmark();
      return node;
    }
  }

  return DnodeId::noId();
}

void BucketQueue::insertInQueue(DnodeId node, long key)
{
  if(node.isNoId())
    return;

  if(node->isMarked() && node->getKey() > -key ){
    return;
  }

  node->setKey(-key); // Reverse since we want effective lowest priority first
  node->mark();
  this->buckets->push(node);

  //debugMsg("BucketQueue:insertInQueue", "Enqueueing " << node << " with key " << -key);
}

void BucketQueue::insertInQueue(DnodeId node)
{
  if(node.isNoId())
    return;

  insertInQueue(node, node->distance - node->potential);
}

} /* namespace Europa */
