#ifndef H_EUROPA_MUTEX
#define H_EUROPA_MUTEX

#include <pthread.h>

namespace EUROPA {
class MutexGrabber {
 public:
  MutexGrabber(pthread_mutex_t& m);
  MutexGrabber(const MutexGrabber& o); //make a move constructor for C++11
  ~MutexGrabber();
  void release();
 protected:
  pthread_mutex_t& m_mutex;        
  bool m_needsRelease;
};
}

#endif
