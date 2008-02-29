#ifndef H_EntityIterator
#define H_EntityIterator

/**
 * @Author Conor McGann
 */


namespace EUROPA {

  class Iterator;
  typedef Id<Iterator> IteratorId;

  /**
   * @brief Utility class to factor out iteration over candidate types
   */
  class Iterator {
  public:
    Iterator() : m_id(this) {}
    /**
     * @brief Virtual destructor to be overriden in subclasses.
     */
    virtual ~Iterator(){ m_id.remove();}

    /**
     * @brief Tests if we have exhausted all the items.
     */
    virtual bool done() const = 0;

    /**
     * @brief Returns the next entity and increments the visited index.
     * @return The next entity
     */
    virtual const EntityId next()  = 0;

    /**
     * @brief Returns the number of times that next has been successfully called.
     * @see next()
     */
    virtual unsigned int visited() const = 0;

    const IteratorId& getId() {return m_id;}
 
  private:
    IteratorId m_id;
  };

  /**
   * @brief Implements an iterator pattern over COLLECTION of ENTITY
   */
  template< class ITERATOR > class EntityIterator : public Iterator {
  public:
    EntityIterator(const ITERATOR& begin_it, const ITERATOR& end_it)
      : m_iterator(begin_it), m_end(end_it), m_visited(0){}

    virtual ~EntityIterator(){}

    /**
     * @brief Tests if we have exhausted all the items.
     */
    bool done() const {return m_iterator == m_end;}

    /**
     * @brief Advances the iterator to the next item. Errors out if done()
     */
    const EntityId next() {
      checkError(!done(), "Cannot advance an item since the iterator is done.");
      m_visited++;
      const EntityId item = *m_iterator;
      ++m_iterator;
      return(item);
    }

    unsigned int visited() const {return m_visited;}
  private:
    ITERATOR m_iterator;
    const ITERATOR m_end;
    unsigned int m_visited;
  };
}

#endif
