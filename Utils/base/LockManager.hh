#ifndef _H_LockManager
#define _H_LockManager

/**
 * @file   LockManager.hh
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @brief Provides a locking mechanism of use of EUROPA2 in a multi-threaded environment.
 * @date   Thu Jan 13 17:35:14 2005
 * @ingroup Utils
 */

#include "LabelStr.hh"
#include "Error.hh"

#include <iostream>
#include <map>
#include <pthread.h>
#include <string>

namespace EUROPA {

  /**
   *@class LockManager
   *@brief Provides a general mechanism for acquiring locks and lock checking.
   *
   *The LockManager class uses a Singleton pattern to represent a single entry point for multi-threaded locking/unlocking
   *and lock checking.  The default implementation does nothing.  There is a second implementation, ThreadedLockManager, that
   *manages actual mutexes.  Currently, lock checking is provided in the Id class, and will error out if an Id is dereferenced without
   *the current thread having a lock.
   *
   *Instantiating the LockManager or ThreadedLockManager classes causes the new instance to be registered as the current LockManager.
   */
  class LockManager {
  public:
    /**
     *@brief Constructor. Deletes any existing LockManager and registers itself as the current LockManager.
     */
    LockManager();

    /**
     *@brief Destructor.
     */
    virtual ~LockManager();

    /**
     *@brief "Connect" the current thread to the LockManager.  This implementation does nothing.
     *@param user the permission level of the thread.
     */
    virtual void connect(const LabelStr& user = "ANONYMOUS"){}

    /**
     *@brief "Disconnect" the current thread from the LockManager.  This implementation does nothing.
     */
    virtual void disconnect(){}

    /**
     *@brief Obtain a lock.  This implementation does nothing.
     */
    virtual void lock(){}

    /**
     *@brief Release a lock.  This implementation does nothing.
     */
    virtual void unlock(){}

    /**
     *@brief Check to see if the current thread has a lock.  This implementation always returns true.
     *@return true if the current thread has a lock.  This implementation always returns true.
     */
    virtual bool hasLock() const {return true;}

    /**
     *@brief Check to see if the current thread is connected.  This implementation always returns true.
     *@return true if the curren thread is connected.  This implementation always returns true.
     */
    virtual bool isConnected() const {return true;}

    /**
     *@brief Get the current permission level of the current thread.  This implementation always returns "ANONYMOUS".
     *@return the current permission level of the current thread.  This implementation always returns "ANONYMOUS".
     */
    virtual const LabelStr& getCurrentUser() const {
      static const LabelStr sl_user("ANONYMOUS"); 
      return sl_user;
    }

    /**
     *@brief Get the current LockManager instance, or allocate a new default LockManager.
     *@return the current LockManager instance.
     */
    static LockManager& instance();
  protected:
  private:
    static LockManager* s_instance; /*!< The current LockManager */
  };


  /**
   *@class ThreadedLockManager
   *@brief A class that adds locking semantics to the LockManager interface.
   *
   *The default LockManager provides an interface but no actual thread safety.  The ThreadedLockManager provides thread safety through the
   *use of a mutex.
   */
  class ThreadedLockManager : public LockManager {
  public:

    /**
     *@brief Constructor.  Sets the current locking thread to no_thread and calls the LockManager constructor.
     */
    ThreadedLockManager();

    /**
     *@brief Destructor.  Disconnects if connected, releasing any locks that may be held and calls the LockManager destructor.
     */
    ~ThreadedLockManager();

    /**
     *@brief Connect the current thread to the ThreadedLockManager, allowing it to acquire locks and associating a permission level with it.
     *@param user the permission level of the current thread.
     */
    void connect(const LabelStr& user = "ANONYMOUS");

    /**
     *@brief Disconnect the current thread from the ThreadedLockManager, releasing any locks it may have.
     */
    void disconnect();

    /**
     *@brief Acquire a lock.  If a lock can't be acquired immediately, this call will block until it can be.
     */
    virtual void lock();

    /**
     *@brief Release a lock, allowing other threads to acquire a lock.
     */
    virtual void unlock();

    /**
     *@brief Check to see if the current thread is the one that has the lock.
     *@return true if the current thread has the lock, false otherwise.
     */
    bool hasLock() const;
    
    /**
     *@brief Check to see if the current thread is connected.
     *@return true if the current thread is connected, false otherwise.
     */
    bool isConnected() const;

    /**
     *@brief Get the permission level of the current thread.
     *@return a LabelStr representing the permission level with which the current thread connected.
     */
    const LabelStr& getCurrentUser() const;
  protected:
  private:

    /**
     *@brief This function actually acquires the lock.
     */
    void m_lock();

    /**
     *@brief This function actually releases the lock.
     */
    void m_unlock();

    static pthread_mutex_t s_lockMutex; /*!< The mutex that gets locked and unlocked. */
    static const pthread_t s_noThread; /*!< A constant value representing an invalid thread. */
    pthread_t m_lockingThread; /*!< The thread that currently has the lock. */
    std::map<pthread_t, LabelStr> m_validThreads; /*!< A map from connected threads to permission levels. */
  };

  /**
   *@class RecursiveLockManager
   *@brief A class that adds recursive locking semantics to the ThreadedLockManager class.
   *
   *The ThreadedLockManager provides thread safety through the use of a mutex, but it is an error for
   *a single thread to call lock() when it already has a lock.  The RecursiveLockManager keeps track
   *of the number of times lock() has been called and releases the lock only when unlock() has been
   *called an equal number of times.
   */

  class RecursiveLockManager : public ThreadedLockManager {
  public:
    /**
     *@brief Constructor.  Sets the lock count to 0 and calls the ThreadedLockManager constructor.
     */
    RecursiveLockManager();

    /**
     *@brief  Acquire a lock or just increment the lock count.
     */
    void lock();
    
    /**
     *@brief Release a lock or just decrement the lock count.
     */
    void unlock();
  protected:
  private:
    int m_lockCount; /*!< The number of times lock() has been called */
  };
}

#endif
