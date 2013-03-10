/*
 * FastMutex.h
 *
 *  Created on: 9 mai 2013
 *      Author: oc
 */

#ifndef _FAST_MUTEX_H_
#define _FAST_MUTEX_H_

#include "globals.h"
#include <pthread.h>

#ifdef FLAG_LOCKS
#define MODULE_FLAG FLAG_LOCKS
#endif

class FastMutex {
    pthread_mutex_t mutex;

    FastMutex(const FastMutex &src) {
        // copy not allowed
    }
    FastMutex& operator =(const FastMutex &src) {
        // copy not allowed
        assert(0);
        return *this;
    }
public:
    FastMutex() {
         pthread_mutexattr_t mutexattr;
         pthread_mutexattr_init(&mutexattr);
         int error = pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_FAST_NP);
         if (unlikely(error != 0)) {
             ERROR_MSG("pthread_mutexattr_settype error %d",error);
         }
         error = pthread_mutex_init(&mutex,&mutexattr);
         if (unlikely(error != 0)) {
             ERROR_MSG("pthread_mutex_init error %d",error);
         }
    }

    ~FastMutex() {
         const int error = pthread_mutex_destroy(&mutex);
         if (error != 0) {
             ERROR_MSG("pthread_mutex_destroy error %d",error);
         }
    }

    pthread_mutex_t* operator &() {
        return &mutex;
    }

    int get() {
        const int error(pthread_mutex_lock(&mutex));
        if (unlikely(error != 0)) {
            ERROR_MSG("pthread_mutex_lock error %d",error);
        }
        return error;
    }

    int release() {
        const int error(pthread_mutex_unlock(&mutex));
        if (unlikely(error != 0)) {
            ERROR_MSG("pthread_mutex_unlock error %d",error);
        }
        return error;
    }
};

#undef MODULE_FLAG

#endif /* _FAST_MUTEX_H_ */
