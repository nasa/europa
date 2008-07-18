#ifndef _H_EUROPA_MUTEX
#define _H_EUROPA_MUTEX

#include <pthread.h>

namespace EUROPA {
    class MutexGrabber
    {
      public:
        MutexGrabber(pthread_mutex_t& m);
        ~MutexGrabber();
        
      protected:
          pthread_mutex_t& m_mutex;        
    };
}

#endif
