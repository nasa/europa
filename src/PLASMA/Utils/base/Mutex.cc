
#include "Mutex.hh"

namespace EUROPA {
        
MutexGrabber::MutexGrabber(pthread_mutex_t& m)
    : m_mutex(m), m_needsRelease(true) {
  pthread_mutex_lock( &m_mutex );    
}

MutexGrabber::MutexGrabber(const MutexGrabber& o)
    : m_mutex(o.m_mutex), m_needsRelease(true) {
  const_cast<MutexGrabber&>(o).m_needsRelease = false;
}

void MutexGrabber::release() {
  if(m_needsRelease) {
    pthread_mutex_unlock( &m_mutex );    
    m_needsRelease = false;
  }
}
MutexGrabber::~MutexGrabber() {
  release();
}

}

