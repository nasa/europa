#include "LockManager.hh"
#include "LabelStr.hh"

#ifdef __MINGW32__
  bool operator < (const ptw32_handle_t& left, const ptw32_handle_t& right) {
    return left.p < right.p;
  }

  bool operator > (const ptw32_handle_t& left, const ptw32_handle_t& right) {
    return left.p > right.p;
  }
#endif

namespace EUROPA {

  LockManager* LockManager::s_instance = NULL;
  pthread_mutex_t ThreadedLockManager::s_lockMutex = PTHREAD_MUTEX_INITIALIZER;

  LockManager::LockManager() {
    if(s_instance != NULL)
      delete s_instance;
    s_instance = this;
  }

  LockManager::~LockManager() {
    check_error(s_instance == this, "Expected the current LockManager instance to be this one.");
    s_instance = NULL;
  }

  LockManager& LockManager::instance() {
    if(s_instance == NULL)
      s_instance = new LockManager();
    return *s_instance;
  }

  ThreadedLockManager::ThreadedLockManager() : LockManager() {
    m_hasLockingThread = false;
  }

  ThreadedLockManager::~ThreadedLockManager() {
    if(isConnected())
      disconnect();
  }

  void ThreadedLockManager::connect(const LabelStr& user) {
    m_lock();
    pthread_t thread = pthread_self();
    if(m_validThreads.find(thread) == m_validThreads.end())
      m_validThreads[thread] = user;
    check_error(isConnected(), "Thread not connected at end of connect() call.");
    m_unlock();
  }

  void ThreadedLockManager::disconnect() {
    if(hasLock())
      unlock();
    m_lock();
    pthread_t thread = pthread_self();
    check_error(isConnected(), "Attempted to disconnect unconnected thread.");
    m_validThreads.erase(thread);
    m_unlock();
  }

  void ThreadedLockManager::lock() {
    m_lock();
  }
 
  void ThreadedLockManager::unlock() {
    m_unlock();
  }

  bool ThreadedLockManager::hasLock() const {
    return m_hasLockingThread && pthread_equal(m_lockingThread, pthread_self()) != 0;
  }
  
  bool ThreadedLockManager::isConnected() const {
    return (m_validThreads.find(pthread_self()) != m_validThreads.end());
  }

  const LabelStr& ThreadedLockManager::getCurrentUser() const {
    check_error(isConnected() && hasLock(), "Attempted to get the current user without connection and lock.");
    std::map<pthread_t, LabelStr>::const_iterator it = m_validThreads.find(pthread_self());
    return (*it).second;
  }

  void ThreadedLockManager::m_lock() {
    check_error(!hasLock(), "Attempted to acquire a lock multiply.");
    pthread_mutex_lock(&s_lockMutex);
    m_lockingThread = pthread_self();
    m_hasLockingThread = true;
    check_error(hasLock(), "Failed to acquire lock.");
  }

  void ThreadedLockManager::m_unlock() {
    check_error(hasLock(), "Attempted to release a lock without having one.");
    m_hasLockingThread = false;
    pthread_mutex_unlock(&s_lockMutex);
    check_error(!hasLock(), "Failed to release lock.");
  }

  RecursiveLockManager::RecursiveLockManager() : ThreadedLockManager(), m_lockCount(0) {}
  
  void RecursiveLockManager::lock() {
    check_error(m_lockCount >= 0, "Released too many locks.");
    if(!hasLock())
      ThreadedLockManager::lock();
    ++m_lockCount;
  }

  void RecursiveLockManager::unlock() {
    check_error(m_lockCount > 0, "Released too many locks.");
    --m_lockCount;
    if(m_lockCount == 0)
      ThreadedLockManager::unlock();
  }
}
