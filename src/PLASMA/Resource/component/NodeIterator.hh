#ifndef NODE_ITERATOR_HEADER_FILE__
#define NODE_ITERATOR_HEADER_FILE__

/**
 * @file Types.hh
 * @author David Rijsman
 * @brief Defines the public interface for a node iterator
 * @date April 2006
 * @ingroup Resource
 */

#include "Types.hh"

namespace EUROPA
{
    /**
     * @brief
     */
    class NodeIterator
    {
    public:
      /**
       * @brief
       */
      NodeIterator( const Graph& graph, bool mustBeEnabled = true ):
	m_Iterator( graph.m_Nodes.begin() ),
	m_End( graph.m_Nodes.end() ),
	m_Enabled( mustBeEnabled )
      {
	if( m_Enabled )
	  {
	    while( m_Iterator != m_End && !(*m_Iterator).second->isEnabled() )
	      ++m_Iterator;
	  }
      }
      /**
       * @brief
       */
      inline bool ok() const;
      /**
       * @brief
       */
      inline void operator++();
      /**
       * @brief
       */
      inline Node* operator*() const;
    private:
      NodeIdentity2Node::const_iterator m_Iterator;
      NodeIdentity2Node::const_iterator m_End;
      bool m_Enabled;
    };

    bool NodeIterator::ok() const
    {
      return m_Iterator != m_End;
    }

    void NodeIterator::operator++()
    {
      ++m_Iterator;

      if( m_Enabled )
	{
	  while( m_Iterator != m_End && !(*m_Iterator).second->isEnabled() )
	    ++m_Iterator;
	}
    }

    Node* NodeIterator::operator*() const
    {
      if( ok() )
	return (*m_Iterator).second;

      return 0;
    }
}
#endif //NODE_ITERATOR_HEADER_FILE__
