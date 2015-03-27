#ifndef EDGE_ITERATOR_HEADER_
#define EDGE_ITERATOR_HEADER_

/**
 * @file EdgeIterator.hh
 * @author David Rijsman
 * @brief Defines the public interface for an edge iterator
 * @date April 2006
 * @ingroup Resource
 */

#include "Types.hh"
#include "Node.hh"

namespace EUROPA
{
    class EdgeIterator
    {
    public:
      inline bool ok() const;
      inline Edge* operator*() const;
    protected:
      /**
       * @brief Edge iterator
       * @par edges The edges to iterate over
       * @par mustBeEnabled If true iterates over enabled edges otherwise iterates over all edges
       */
      EdgeIterator( const EdgeList& edges, bool mustBeEnabled ):
	m_Iterator( edges.begin() ),
	m_End( edges.end() ),
	m_Enabled( mustBeEnabled )
      {
      }
    protected:
      EdgeList::const_iterator m_Iterator;
      EdgeList::const_iterator m_End;
      bool m_Enabled;
    };

    bool EdgeIterator::ok() const
    {
      return m_Iterator != m_End;
    }


    Edge* EdgeIterator::operator*() const
    {
      if( ok() )
	return (*m_Iterator);

      return 0;
    }
    class EdgeOutIterator:
      public EdgeIterator
    {
    public:
      /**
       * @brief Out edge iterator
       * @par node The node to iterate from
       * @par mustBeEnabled If true iterates over enabled edges otherwise iterates over all edges
       */
      EdgeOutIterator( const Node& node, bool mustBeEnabled = true ):
	EdgeIterator( node.getOutEdges(), mustBeEnabled )
      {
	if( m_Enabled )
	  {
	    while( m_Iterator != m_End && ( !(*m_Iterator)->isEnabled() || !(*m_Iterator)->getTarget()->isEnabled() ) )
	      ++m_Iterator;
	  }
      }

      EdgeOutIterator& operator++()
      {
	++m_Iterator;

	if( m_Enabled )
	  {
	    while( m_Iterator != m_End && ( !(*m_Iterator)->isEnabled() || !(*m_Iterator)->getTarget()->isEnabled() ) )
	      ++m_Iterator;
	  }
        return *this;
      }

    };

    class EdgeInIterator:
      public EdgeIterator
    {
    public:
      /**
       * @brief In-edge iterator
       * @par node The node to iterate from
       * @par mustBeEnabled If true iterates over enabled edges otherwise iterates over all edges
       */
      EdgeInIterator( const Node& node, bool mustBeEnabled = true  ):
	EdgeIterator( node.getInEdges(), mustBeEnabled )
      {
	if( m_Enabled )
	  {
	    while( m_Iterator != m_End && ( !(*m_Iterator)->isEnabled() || !(*m_Iterator)->getSource()->isEnabled() ) )
	      ++m_Iterator;
	  }
      }

      EdgeInIterator& operator++()
      {
	++m_Iterator;

	if( m_Enabled )
	  {
	    while( m_Iterator != m_End && ( !(*m_Iterator)->isEnabled() || !(*m_Iterator)->getSource()->isEnabled() ) )
	      ++m_Iterator;
	  }
        return *this;
      }

    };
}

#endif // EDGE_ITERATOR_HEADER_

