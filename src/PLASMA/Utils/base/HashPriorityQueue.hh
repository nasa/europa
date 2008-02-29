/**************************************************************************
     File: HashPriorityQueue.hh
   Author: Paul H. Morris
Copyright: NASA 2003
Purpose:
    Defines a template for handling Priority Queues of various kinds.
    The items in a Priority Queue can be of any type.  It is assumed
    that memory management is handled correctly for each item.  The
    Priority Queue is handled as in Cormen, but the items are organized
    into buckets that have the same key value.  A hashtable is used to
    provide rapid access to the bucket with a given key.
Contents:
    The template definition for a Hash Priority Queue.
    Member function implementations.
Notes:
    All of the Priority Queue operate with respect to pointers to the
    item type.  Key comparisons are done using an integer key that is
    passed to the Priority Queue module.
    The implementation uses a hash table to quickly access the bucket
    for a given key so that new elements can be inserted.
**************************************************************************/

#ifndef _H_HashPriority
#define _H_HashPriority

#include "CommonDefs.hh"

namespace EUROPA {

typedef void Void;
typedef long Long;
typedef unsigned Unsigned;
typedef bool Bool;

  class HPQErr {
  public:
    DECLARE_ERROR(VectorMemoryError);
    DECLARE_ERROR(VectorInternalError);
  };

/**************************************************************************
Template: HashPriorityQueue
Description: 
    A template for Hash Priority Queues of any given type.
Author: Paul H. Morris

Attributes:
-----------
queue  - The PriorityQueue of buckets.
table  - A hashtable used to provide direct access to a bucket from the key.
**************************************************************************/

#define defaultInitialSpace 10

static Unsigned getpower2(Unsigned n)
{
  // Gets smallest m such that m = 2^k and m >= n.
  Unsigned power2 = 1;
  while (power2 < n)
    power2 <<= 1;  // shift left 1 place.
  return power2;
}

template<class TYPE>
class Pbucket;

template<class TYPE>
class HashPriorityQueue {
  Unsigned count;    // Number of buckets in use
  Unsigned tableAllocation;
  Unsigned queueAllocation;
  Pbucket<TYPE>** table;
  Pbucket<TYPE>** queue;
  /*
  BucketPriorityQueue<TYPE>* test;
  */
public:
  HashPriorityQueue (Unsigned n = defaultInitialSpace)
  {
    count = 0;
    queueAllocation = n+1;
    tableAllocation = getpower2(n);
    queue = new Pbucket<TYPE>* [queueAllocation];
    table = new Pbucket<TYPE>* [tableAllocation];
    inithash();
    /*
    test = new BucketPriorityQueue<TYPE>(n);
    */
  }
  virtual ~HashPriorityQueue () { clrbucks(); delete[] table; delete[] queue; }
  TYPE* extractMin();
  Void insert (TYPE* node, Long key);
  Void reset();
private:
  Void inithash();
  Void clrhash();
  Void clrbucks();
  Unsigned getHashLoc(Long key);
  Pbucket<TYPE>* gethash(Long key);
  Void remhash(Long key);
  Void puthash(Pbucket<TYPE>* bucket, Long key);
  Bool growHashTable();
  Pbucket<TYPE>* minqueue();
  Void popqueue();
  Void putqueue(Pbucket<TYPE>* bucket, Long key);
  Void heapify(Unsigned);
  Bool growPriorityQueue();
};

template<class TYPE>
class PbucketLinker;

template<class TYPE>
class PbucketLinker {
  friend class Pbucket<TYPE>;
  TYPE* node;
  PbucketLinker<TYPE>* link;
  PbucketLinker(TYPE* n, PbucketLinker<TYPE>* x) { node=n; link=x; }
};

template<class TYPE>
class Pbucket {
  friend class HashPriorityQueue<TYPE>;
  Long key;
  Pbucket<TYPE>* hashlink;
  PbucketLinker<TYPE>* stack;
  Pbucket(Long x) { key=x; hashlink=NULL; stack=NULL; }

  Void push (TYPE* x) {
    stack = new PbucketLinker<TYPE>(x,stack);
  }

  TYPE* pop() {
    if (!stack)
      return NULL;
    TYPE* x = stack->node;
    PbucketLinker<TYPE>* temp = stack;
    stack = stack->link;
    delete temp;
    return x;
  }
};

template <class TYPE>
Void HashPriorityQueue<TYPE>::reset()
{
  /*
  test->reset();
  */
  clrhash();
  clrbucks();
  count = 0;
}

/**************************************************************************
Function: Insert an item into the Hash Priority Queue.
Method:  Check the hashtable to see if there is an existing bucket
  for the key.  If not, create one, record it in the hashtable, and
  insert it in the priority queue.  Add the item to the bucket.
**************************************************************************/

template <class TYPE>
Void HashPriorityQueue<TYPE>::insert (TYPE* node, Long key) {
  Pbucket<TYPE>* bucket = gethash(key);
  if (!bucket) {
    bucket = new Pbucket<TYPE>(key);
    count++;
    puthash(bucket,key);  // See growHashTable comment.
    putqueue(bucket,key);
  }
  bucket->push(node);
  /*
  test->insert(node,key);
  */
}

/**************************************************************************
Function: Remove and return the minimum element from the Hash Priority Queue.
Method:
**************************************************************************/
template <class TYPE>
TYPE* HashPriorityQueue<TYPE>::extractMin()
{
  TYPE* ans = NULL;
  Pbucket<TYPE>* bucket = minqueue();
  if (bucket) {
    TYPE* node = bucket->pop();
//     checkError (!node, vectorInternalError,
//                 "HashPriorityQueue: Empty Bucket?", majorError, NULL);
    check_error(node, "HashPriorityQueue: Empty Bucket?", HPQErr::VectorInternalError());
    // If this is the last node in the bucket, we need to clean up
    if (bucket->stack == NULL) {
      popqueue();
      remhash(bucket->key);
      count--;
      delete bucket;
    }
    ans = node;
  }
  /*
  TYPE* check = test->extractMin();
  checkError (ans != check,
              vectorInternalError,
              "HashPriorityQueue: Wrong answer", majorError, NULL);
  */
  return ans;
}

/**************************************************************************
Private Functions
**************************************************************************/

template <class TYPE>
Unsigned HashPriorityQueue<TYPE>::getHashLoc(Long key)
{
  Unsigned keyplus = (Unsigned) key;
  Unsigned loc = keyplus & (tableAllocation-1);
  return loc;
}

template <class TYPE>
Pbucket<TYPE>* HashPriorityQueue<TYPE>::gethash(Long key)
{
  Unsigned loc = getHashLoc(key);
  Pbucket<TYPE>* bucket = table[loc];
  while (bucket != NULL && bucket->key != key)
    bucket = bucket->hashlink;
  return bucket;
}

template <class TYPE>
Void HashPriorityQueue<TYPE>::puthash (Pbucket<TYPE>* bucket, Long key)
{
  //if (this->count >= (this->tableAllocation)/2  && !growHashTable())
//     handleError(vectorMemoryError,
//                 "Insufficient memory to grow HashPriorityQueue hashtable.",
//                 majorError, );

  check_error(!(this->count >= (this->tableAllocation)/2  && !growHashTable()), 
              "Insufficient memory to grow HashPriorityQueue hashtable.",
              HPQErr::VectorMemoryError());

  Unsigned loc = getHashLoc(key);
  bucket->hashlink = table[loc];
  table[loc] = bucket;
}

template <class TYPE>
Void HashPriorityQueue<TYPE>::remhash (Long key)
{
  Unsigned loc = getHashLoc(key);
  Pbucket<TYPE>** bucketptr = &table[loc];
  while ((*bucketptr) != NULL && (*bucketptr)->key != key)
    bucketptr = &((*bucketptr)->hashlink);
  Pbucket<TYPE>* bucket = (*bucketptr);
  if (bucket) {
    (*bucketptr) = bucket->hashlink;
    bucket->hashlink = NULL;
  }
}

template <class TYPE>
Void HashPriorityQueue<TYPE>::inithash()
{
  for (Unsigned loc=0; loc < tableAllocation; loc++)
    table[loc] = NULL;
}

template <class TYPE>
Void HashPriorityQueue<TYPE>::clrhash()
{
  if (count > 0)
    inithash();
}

template <class TYPE>
Void HashPriorityQueue<TYPE>::clrbucks()
{
  // We make use of the fact that the queue stores the buckets.
  for (Unsigned i=1; i <= count; i++)
    delete queue[i];
}

template <class TYPE>
Bool HashPriorityQueue<TYPE>::growHashTable() {
  Unsigned oldAllocation = tableAllocation;
  Pbucket<TYPE>** oldTable = table;
  tableAllocation = 2*oldAllocation;
  table = new Pbucket<TYPE>*[tableAllocation];

  check_error(table != 0, 
              "Insufficient memory to grow HashPriorityQueue hashtable.",
              HPQErr::VectorMemoryError());

  inithash();

  // Transfer buckets to new hashtable.  Can find them in the Priority
  // Queue.  (Therefore need to grow hashtable before inserting in
  // queue, and count-1 is then the number of buckets in queue.  Also
  // recall the queue leaves the queue[0] location unused.)

  Unsigned i;
  for (i=1; i<count; i++) {
    Pbucket<TYPE>* bucket = queue[i];
    Unsigned loc = getHashLoc(bucket->key);
    bucket->hashlink = table[loc];
    table[loc] = bucket;
  }

  // Delete the old hashtable.
  delete[] oldTable;
  return true;
}


/**************************************************************************
Functions: PARENT,LEFT,RIGHT for navigation in the heap.
Method:
    See Cormen, Lieserson, and Rivest, Chapter 7.
    In following we will waste a cell (the 0 cell) so that the
    heap array is 1-based and the arithmetic is faster (and matches
    Cormen, Leiserson, and Rivest).
**************************************************************************/

// PARENT(i) == floor(i/2)
// LEFT(i) = 2*i
// RIGHT(i) = 2*i + 1

// but the following bit twiddling definitions, suggested in Cormen,
// Lieserson, and Rivest, are more efficient.

#define PARENT(i) ((i)>>1)
#define LEFT(i) ((i)<<1)
#define RIGHT(i) (((i)<<1)|1)

/**************************************************************************
Function: Insert an item into the Priority Queue.
Method:
    See Cormen, Lieserson, and Rivest, Chapter 7.
**************************************************************************/

template <class TYPE>
Void HashPriorityQueue<TYPE>::putqueue (Pbucket<TYPE>* node, Long key) {
  Unsigned i = count;
//   if (i >= this->queueAllocation && !growPriorityQueue())
//       handleError(vectorMemoryError,
//                   "Insufficient memory to grow PriorityQueue.", majorError, );
  check_error(!(i >= this->queueAllocation && !growPriorityQueue()),
              "Insufficient memory to grow PriorityQueue.",
              HPQErr::VectorMemoryError());
  while (i > 1 && key < this->queue[PARENT(i)]->key) {
      this->queue[i] = this->queue[PARENT(i)];
      i = PARENT(i);
  }
  this->queue[i] = node;
}

/**************************************************************************
Function: Return the minimum element from the Priority Queue.
Method:
    See Cormen, Lieserson, and Rivest, Chapter 7.
**************************************************************************/
template <class TYPE>
Pbucket<TYPE>* HashPriorityQueue<TYPE>::minqueue()
{
  if (this->count < 1)
    return NULL;
  return this->queue[1];
}

/**************************************************************************
Function: Remove the minimum element from the Priority Queue.
Method:
    See Cormen, Lieserson, and Rivest, Chapter 7.
**************************************************************************/

template <class TYPE>
Void HashPriorityQueue<TYPE>::popqueue()
{
  if (this->count < 1)
    return;
  this->queue[1] = this->queue[(this->count)];
  heapify(1);
}

/**************************************************************************
Function: Restore the heap property after extraction.
Method:
    See Cormen, Lieserson, and Rivest, Chapter 7.
**************************************************************************/

template <class TYPE>
Void HashPriorityQueue<TYPE>::heapify(Unsigned i)
{
  if (i > this->count)
    return;
  Unsigned left = LEFT(i);
  Unsigned right = RIGHT(i);
  Unsigned min;
  if (left <= this->count && this->queue[left]->key < this->queue[i]->key)
    min = left;
  else
    min = i;
  if (right <= this->count && this->queue[right]->key < this->queue[min]->key)
    min = right;
  if (min != i) {
    Pbucket<TYPE>* temp = this->queue[i];
    this->queue[i] = this->queue[min];
    this->queue[min] = temp;
    heapify(min);
  }
}

/**************************************************************************
Function: Grow the PriorityQueue's heap array
Method:
    Allocate a new array with twice the capacity of the previous
    array.  Copy each member of the array to the new array, in
    the same place.  Delete the older array and set the queue and
    allocation attributes to correspond to new array.
**************************************************************************/

template <class TYPE>
Bool HashPriorityQueue<TYPE>::growPriorityQueue() {
  Pbucket<TYPE>** oldQueue = queue;
  Unsigned oldAllocation = queueAllocation;
  queueAllocation = 2*oldAllocation;
  if ((queue = new Pbucket<TYPE>*[queueAllocation]) == 0) {
    queueAllocation = oldAllocation + defaultInitialSpace;
    queue = new Pbucket<TYPE>*[queueAllocation];
  }
  if (!queue)
    return false;
  // Copy the old items to the new queue.
  // Note: new item not yet inserted so queue[count] is unused.
  for (Unsigned i=1; i<count; i++)
    queue[i] = oldQueue[i];
  delete oldQueue;
  return true;
}

#undef LEFT
#undef PARENT
#undef RIGHT
  



#undef defaultInitialSpace

} /* namespace EUROPA */

#endif
