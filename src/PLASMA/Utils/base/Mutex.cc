
#include "Mutex.hh"

namespace EUROPA {
        
MutexGrabber::MutexGrabber(pthread_mutex_t& m)
    : m_mutex(m)
{
    pthread_mutex_lock( &m_mutex );    
}
        
MutexGrabber::~MutexGrabber()
{
    pthread_mutex_unlock( &m_mutex );    
}

}

