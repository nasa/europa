
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
  this->first = NULL;
  Dnode::unmarkAll();
}

void Dqueue::addToQueue (Dnode* node)
{
  // FIFO queue add to last.
  if (!node->isMarked()) {
    // If not already in queue...
    if (this->first == NULL)  // queue is empty
      this->first = node;
    else                      // splice in node
      this->last->link = node;
    node->link = NULL;
    this->last = node;
    node->mark();
  }
}

Dnode* Dqueue::popFromQueue ()
{
  // FIFO queue pop from first.
  Dnode* node = this->first;
  if (node == NULL)
    return node;
  this->first = node->link;
  node->unmark();
  return node;
}

Bool Dqueue::isEmpty()
{
  return (this->first == NULL);
}


/* BucketQueue functions */


BucketQueue::BucketQueue (int) : buckets() {
}

BucketQueue::~BucketQueue ()
{
}

void BucketQueue::reset()
{
  buckets = DnodePriorityQueue();
  Dnode::unmarkAll();
}

Dnode* BucketQueue::popMinFromQueue()
{
  Dnode* node = NULL;
	
  while (!buckets.empty()){
    const Bucket& b = buckets.top();
    node = const_cast<Dnode*>(b.node);
    buckets.pop();

    if (node->isMarked()){
      node->unmark();
      return node;
    }
  }
	
  return NULL;
}

void BucketQueue::insertInQueue(Dnode* node, eint::basis_type key)
{
	if(node == NULL)
		return;

	if(node->isMarked() && node->getKey() > -key )
		return;

	node->setKey(-key); // Reverse since we want effective lowest priority first
	node->mark();
	Bucket b(node,-key);
	this->buckets.push(b);

	//debugMsg("BucketQueue:insertInQueue", "Enqueueing " << node << " with key " << -key);
}

void BucketQueue::insertInQueue(Dnode* node)
{
  if(node == NULL)
    return;

  insertInQueue(node, node->distance - node->potential);
}

} /* namespace Europa */
