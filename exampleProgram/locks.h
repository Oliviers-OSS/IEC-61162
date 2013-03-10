/*
 * File:   locks.h
 * Author: oc
 *
 * Created on January 15, 2011, 9:02 AM
 */

#ifndef _LOCKS_H_
#define	_LOCKS_H_

//#include "globals.h"
#include "debug.h"
//#include "FastMutex.h"
#include <pthread.h>
#include <ostream>
#include <string>
#include <dbgflags-1/debug_macros.h>

#ifdef FLAG_LOCKS
#define MODULE_FLAG FLAG_LOCKS
#endif

class Lock {
    public:
        Lock() {
        }
        virtual ~Lock() {
        }
        virtual int getRead() = 0;
        virtual int releaseRead() = 0;
        virtual int getWrite() = 0;
        virtual int releaseWrite() = 0;
};

#ifdef _GNU_SOURCE

class rwLock : public Lock{
    pthread_rwlock_t rwlock;
public:
    rwLock() {
        pthread_rwlock_init(&rwlock,NULL);
    }
    virtual ~rwLock() {
        pthread_rwlock_destroy(&rwlock);
    }
    virtual int getRead() {
         const int error(pthread_rwlock_rdlock(&rwlock));
         DEBUG_VAR(error,"%d");
         return error;
    }
    virtual int releaseRead() {
        const int error(pthread_rwlock_unlock(&rwlock));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int getWrite() {
        const int error(pthread_rwlock_wrlock(&rwlock));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int releaseWrite(){
        const int error(pthread_rwlock_unlock(&rwlock));
        DEBUG_VAR(error,"%d");
        return error;
    }
};

#else /* _GNU_SOURCE */

class mutexLock : public Lock {
    pthread_mutex_t mutex_;
public:
    mutexLock() {
        pthread_mutex_init(&mutex_,NULL);
    }
    virtual ~mutexLock() {
        pthread_mutex_destroy(&mutex_);
    }
    virtual int getRead() {
        const int error(pthread_mutex_lock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int releaseRead() {
        const int error(pthread_mutex_unlock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int getWrite() {
        const int error(pthread_mutex_lock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
    virtual int releaseWrite(){
        const int error(pthread_mutex_unlock(&mutex_));
        DEBUG_VAR(error,"%d");
        return error;
    }
};

#endif /* _GNU_SOURCE */

class readLockMgr {
    Lock *managedLock;
public:
    readLockMgr(Lock *l)
       :managedLock(l) {
          managedLock->getRead();
    }
    readLockMgr(Lock &l)
       :managedLock(&l) {
          managedLock->getRead();
    }
    ~readLockMgr() {
        managedLock->releaseRead();
    }
};

class writeLockMgr {
    Lock *managedLock;
public:
    writeLockMgr(Lock *l)
       :managedLock(l) {
          managedLock->getWrite();
    }
    writeLockMgr(Lock &l)
       :managedLock(&l) {
          managedLock->getWrite();
    }
    ~writeLockMgr() {
        managedLock->releaseWrite();
    }
};

class MutexMgr {
    pthread_mutex_t *managedMutex;
    int lock() {
      const int error = pthread_mutex_lock(managedMutex);
      if (error != 0) {
          throw exception(error,"pthread_mutex_lock error %d",error);
      }
      return error;
  }
public:

    class exception : public std::exception {
        int error;
        std::string msg;
    public:
        exception(int code,const std::string &message) throw()
            :error(code),msg(message) {
        }
        exception(int code,const char *format,...);
        virtual ~exception() throw() {
        }
        virtual const char* what() const throw() {
            return msg.c_str();
        }
        virtual int code() const throw() {
            return error;
        }
    };

    MutexMgr(pthread_mutex_t &mutex)
        :managedMutex(&mutex) {
        lock();
    }

#ifdef _FAST_MUTEX_H_
    MutexMgr(FastMutex &mutex)
           :managedMutex(&mutex) {
           lock();
       }
#endif // _FAST_MUTEX_H_

    ~MutexMgr() {
        const int error = pthread_mutex_unlock(managedMutex);
        if (error != 0) {
            // don't throw exception from the destructor, just log the error;
            ERROR_MSG("pthread_mutex_unlock %d",error);
        }
    }
};

inline std::ostream& operator<< (std::ostream &o,const MutexMgr::exception &ex) {
  return o << "MutexMgr::Exception : " << ex.what() << " error code = " << ex.code();
}

#ifdef MODULE_FLAG
#undef MODULE_FLAG
#endif

#endif	/* _LOCKS_H_ */

