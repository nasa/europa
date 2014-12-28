#include "FlowProfileGraph.hh"

#include "Debug.hh"
#include "Edge.hh"
#include "EdgeIterator.hh"
#include "MaxFlow.hh"
#include "Node.hh"
#include "Number.hh"
#include "ConstrainedVariable.hh"
#include "Domain.hh"

namespace EUROPA {
FlowProfileGraph::FlowProfileGraph(const TransactionId ,
                                   const TransactionId ,
                                   bool lowerLevel)
    : m_lowerLevel(lowerLevel), m_recalculate(false) {}

FlowProfileGraphImpl::FlowProfileGraphImpl(const TransactionId source, 
                                           const TransactionId sink, bool lowerLevel)
    : FlowProfileGraph(source, sink, lowerLevel), m_maxflow(NULL), m_graph( 0 ),
      m_source( 0 ), m_sink( 0 ) {
  m_graph = new Graph();
  m_source = m_graph->createNode( source );
  m_sink = m_graph->createNode( sink );
  m_maxflow = new MaximumFlowAlgorithm( m_graph, m_source, m_sink );
}

FlowProfileGraphImpl::~FlowProfileGraphImpl()
{
  delete m_maxflow;
  m_maxflow = 0;

  delete m_graph;
  m_graph = 0;

  m_source = 0;
  m_sink = 0;
}

void FlowProfileGraphImpl::enableAt( const TransactionId t1, const TransactionId t2 )
{
  debugMsg("FlowProfileGraph:enableAt","Transaction "
           << t1->time()->toString() << " and transaction "
           << t2->time()->toString() << " lower level: "
           << std::boolalpha << m_lowerLevel );


  if( 0 == m_graph->getNode( t1 ) )
    return;

  if( 0 == m_graph->getNode( t2 ) )
    return;

  m_recalculate = true;

  m_graph->createEdge( t1, t2, Edge::getMaxCapacity() );
  m_graph->createEdge( t2, t1, Edge::getMaxCapacity() );
}

void FlowProfileGraphImpl::enableAtOrBefore( const TransactionId t1, const TransactionId t2 )
{
  debugMsg("FlowProfileGraph:enableAtOrBefore","Transaction "
           << t1->time()->toString() << " and transaction "
           << t2->time()->toString() << " lower level: "
           << std::boolalpha << m_lowerLevel );


  if( 0 == m_graph->getNode( t1 ) )
    return;

  if( 0 == m_graph->getNode( t2 ) )
    return;

  m_recalculate = true;

  m_graph->createEdge( t1, t2, 0 );
  m_graph->createEdge( t2, t1, Edge::getMaxCapacity() );
}

bool FlowProfileGraphImpl::isEnabled(  const TransactionId transaction ) const
{
  Node* node = m_graph->getNode( transaction );

  return 0 == node ? false : node->isEnabled();
}

void FlowProfileGraphImpl::enableTransaction( const TransactionId t, const InstantId i, TransactionId2InstantId contributions )
{
  debugMsg("FlowProfileGraph:enableTransaction","Transaction ("
           << t->getId() << ") "
           << t->time()->toString() << " lower level: "
           << std::boolalpha << m_lowerLevel );

  TransactionId source = TransactionId::noId();
  TransactionId target = TransactionId::noId();

  edouble edgeCapacity = 0;

  if( ( m_lowerLevel && t->isConsumer() )
      ||
      (!m_lowerLevel && !t->isConsumer() ) )
  {
    // connect to the source of the graph
    source = m_source->getIdentity();
    target = t;

    edgeCapacity = t->quantity()->lastDomain().getUpperBound();
  }
  else
  {
    // connect to the sink of the graph
    source = t;
    target = m_sink->getIdentity();

    edgeCapacity = t->quantity()->lastDomain().getLowerBound();
  }

  if( 0 == edgeCapacity )
  {
    debugMsg("FlowProfileGraph:enableTransaction","Transaction "
             << t << " starts contributing at "
             << i->getTime() << " lower level " << std::boolalpha << m_lowerLevel );

    contributions[ t ] = i;

    return;
  }

  check_error( TransactionId::noId() != source );
  check_error( TransactionId::noId() != target );

  m_recalculate = true;

  m_graph->createNode( t, true );
  m_graph->createEdge( source, target, edgeCapacity );
  m_graph->createEdge( target, source, 0 );
}

void FlowProfileGraphImpl::removeTransaction( const TransactionId id )
{
  debugMsg("FlowProfileGraph:removeTransaction","Transaction ("
           << id->getId() << ") lower level: "
           << std::boolalpha << m_lowerLevel );

  m_recalculate = true;

  m_graph->removeNode( id );
}

void FlowProfileGraphImpl::reset()
{
  m_recalculate = true;

  m_graph->setDisabled();

  m_sink->setEnabled();
  m_source->setEnabled();
}


edouble FlowProfileGraphImpl::getResidualFromSource()
{
  edouble residual = 0.0;

  if( m_recalculate )
  {
    m_maxflow->execute();

    m_recalculate = false;
  }

  EdgeOutIterator ite( *m_source );

  for( ; ite.ok(); ++ite )
  {
    Edge* edge = *ite;

    residual += m_maxflow->getResidual( edge );
  }

  return residual;
}

void FlowProfileGraphImpl::disable(  const TransactionId id )
{
  debugMsg("FlowProfileGraph:disable","Transaction ("
           << id->getId() << ") lower level: "
           << std::boolalpha << m_lowerLevel );

  Node* node = m_graph->getNode( id );

  check_error( 0 != node );
  check_error( node->isEnabled() );

  node->setDisabled();
}

void FlowProfileGraphImpl::pushFlow( const TransactionId id )
{
  Node* node = m_graph->getNode( id );

  check_error( 0 != node );
  check_error( node->isEnabled() );

  if( !m_recalculate )
  {
    debugMsg("FlowProfileGraph:pushFlow","Transaction ("
             << id->getId() << ") lower level: "
             << std::boolalpha << m_lowerLevel );

    m_maxflow->pushFlowBack( node );
  }
  else
  {
    debugMsg("FlowProfileGraph:pushFlow","Transaction ("
             << id->getId() << ") lower level: "
             << std::boolalpha << m_lowerLevel
             << " skipping pushing flow back because a recalculation is required.");

  }
}

void FlowProfileGraphImpl::restoreFlow()
{
  m_maxflow->execute( false );
}


edouble FlowProfileGraphImpl::disableReachableResidualGraph( TransactionId2InstantId contributions, const InstantId instant )
{
  debugMsg("FlowProfileGraph:disableReachableResidualGraph","Lower level: "
           << std::boolalpha << m_lowerLevel );

  edouble residual = 0.0;

  if( m_recalculate )
  {
    debugMsg("FlowProfileGraph:disableReachableResidualGraph","Lower level: "
             << std::boolalpha << m_lowerLevel << ", recalculate invoked.");

    m_maxflow->execute();

    Node2Bool visited;

    visited[ m_source ] = true;

    visitNeighbors( m_source, residual, visited, contributions, instant );
  }

  return residual;
}

void FlowProfileGraphImpl::visitNeighbors(const Node* node, edouble& residual,
                                      Node2Bool& visited, 
                                      TransactionId2InstantId contributions,
                                      const InstantId instant) {
  EdgeOutIterator ite( *node );

  for( ; ite.ok(); ++ite )
  {
    Edge* edge = *ite;

    Node* target = edge->getTarget();

    if( false == visited[ target ] )
    {
      if( 0 != m_maxflow->getResidual( edge ) )
      {
        visited[ target ] = true;

        if( target != m_source && target != m_sink )
        {
          debugMsg("FlowProfileGraph:visitNeighbors",
                   "Disabling node with transaction ("
                   << target->getIdentity()->getId() << ") lower level " <<
                   std::boolalpha << m_lowerLevel  << " due to " << *edge);

          target->setDisabled();

          const TransactionId t = target->getIdentity();

          debugMsg("FlowProfileGraph::visitNeighbors","Transaction "
                   << t << " starts contributing at "
                   << instant->getTime() << " lower level " << std::boolalpha << 
                   m_lowerLevel );

          contributions[ t ] = instant;

          int sign = t->isConsumer() ? -1 : +1;

          if( ( m_lowerLevel && t->isConsumer() )
              ||
              (!m_lowerLevel && !t->isConsumer() ) )
          {
            debugMsg("FlowProfileGraph:visitNeighbors","Adding "
                     << sign * t->quantity()->lastDomain().getUpperBound() << 
                     " to the level.");

            residual += sign * t->quantity()->lastDomain().getUpperBound();
          }
          else
          {
            debugMsg("FlowProfileGraph:visitNeighbors","Adding "
                     << sign* t->quantity()->lastDomain().getLowerBound() << 
                     " to the level.");

            residual += sign * t->quantity()->lastDomain().getLowerBound();
          }

          visitNeighbors( target, residual, visited, contributions, instant );
        }
      }
    }
  }
}
}
