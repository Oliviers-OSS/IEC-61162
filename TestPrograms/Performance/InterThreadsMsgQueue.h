/*
 * InterThreadsMsgQueue.h
 *
 *  Created on: 19 avr. 2013
 *      Author: oc
 */

#ifndef _INTER_THREADS_MSG_QUEUE_H_
#define _INTER_THREADS_MSG_QUEUE_H_

#include <semaphore.h>
#include <pthread.h>
#include <stdexcept>
#include <vector>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <time.h>
#include "locks.h"
#include "tools.h"
#include <dbgflags-1/debug_macros.h>

#ifdef FLAG_ITC
#define MODULE_FLAG FLAG_ITC
#endif //FLAG_ITC

template <size_t size, typename message> class MsgQueue {
    sem_t available;
    sem_t used;
    //std::vector<message> queue;
    message* queue;

    class IndexManager {
        pthread_mutex_t mutex;
        unsigned int value;
        unsigned int max;
    public:
        IndexManager(const unsigned int initialValue,const unsigned int maximumValue)
            :mutex(PTHREAD_MUTEX_INITIALIZER)
            ,value(initialValue)
            ,max(maximumValue) {
        }

        ~IndexManager() {
            pthread_mutex_destroy(&mutex);
        }

        operator const unsigned int() {
            return value;
        }

        IndexManager& operator ++() {
            MutexMgr mutexMgr(mutex);
            value++;
            if (value > max) {
                value = 0;
            }
            return (*this);
        }

        IndexManager& operator++(int) {
            IndexManager tmp(*this);
            operator++();
            return tmp;
        }

        IndexManager& operator --() {
            MutexMgr mutexMgr(mutex);
            if (value != 0) {
                value--;
            } else {
                value = max;
            }
            return (*this);
        }

        bool operator==(const IndexManager& rhs) {
            return value==rhs.value;
        }

        bool operator!=(const IndexManager& rhs) {
            return value!=rhs.value;
        }
    } writeIndex,readIndex;

    void addTo(const struct timespec &value1,struct timespec &value2) {
        value2.tv_sec += value1.tv_sec;
        value2.tv_nsec += value1.tv_nsec;
        if (value2.tv_nsec >= 1000000000L) {
            value2.tv_sec++;
            value2.tv_nsec -= 1000000000L;
        }
    }
public:

    MsgQueue()
        :writeIndex(0,size-1)
        ,readIndex(0,size-1) {
        if(sem_init(&available, 0, size) == 0) {
            if(sem_init(&used, 0, 0) == 0) {
                //queue.reserve(size);
                queue = new message[size];
            } else {
                const int error(errno);
                std::string errorMsg;
                sprintf(errorMsg,"sem_init used error %d",error);
                throw std::runtime_error(errorMsg);
            }
        } else {
            const int error(errno);
            std::string errorMsg;
            sprintf(errorMsg,"sem_init available %u error %d",size,error);
            throw std::runtime_error(errorMsg);
        }
    }

    ~MsgQueue() {
        if (sem_destroy(&available) != 0) {
            const int error(errno);
        }
        if (sem_destroy(&used) != 0) {
            const int error(errno);
        }
        delete [] queue;
    }

    int putMessage(const message &msg) {
        int error(EXIT_SUCCESS);
        // wait for a slot
        if (sem_wait(&available) == 0) {
            // then fill it
            queue[writeIndex] = msg;
            //queue.assign(writeIndex,msg);
            ++writeIndex;
            if (sem_post(&used) != 0) {
                error = errno;
                ERROR_MSG("sem_post used error %d (%m)",error);
            }
        } else {
            error = errno;
            ERROR_MSG("sem_wait available error %d (%m)",error);
        }
        return error;
    }

    int getMessage(message &msg) {
        int error(EXIT_SUCCESS);
        // wait for a filled slot
        if (sem_wait(&used) == 0) {
            msg = queue[readIndex];
            ++readIndex;
            if (sem_post(&available) != 0) {
                error = errno;
                ERROR_MSG("sem_post available error %d (%m)",error);
            }
        } else {
            error = errno;
            ERROR_MSG("sem_wait used error %d (%m)",error);
        }
        return error;
    }

    int getMessage(message &msg, const struct timespec &timeout) {
        int error(EXIT_SUCCESS);
        // Optimization: Because clock_get_time calls are costly, try to get the semaphore without timeout first
        if (sem_trywait(&used) == 0) {
            msg = queue[readIndex];
            ++readIndex;
            if (sem_post(&available) != 0) {
                error = errno;
                ERROR_MSG("sem_post available error %d (%m)",error);
            }
        } else {
            // ignore the real error code returned by sem_trywait, we suppose that errno == EAGAIN
            // else the next call to sem_timedwait will report the same error.
            // get current time to be able to convert relative time out to absolute time out
            struct timespec AbsoluteTimeOut;
            if (clock_gettime(CLOCK_REALTIME, &AbsoluteTimeOut) == 0) {
                addTo(timeout,AbsoluteTimeOut); // convert relative time out to absolute time out
                // wait for a filled slot
                if (sem_timedwait(&used,&AbsoluteTimeOut) == 0) {
                    msg = queue[readIndex];
                    ++readIndex;
                    if (sem_post(&available) != 0) {
                        error = errno;
                        ERROR_MSG("sem_post available error %d (%m)",error);
                    }
                } else {
                    error = errno;
                    ERROR_MSG("sem_timedwait used error %d (%m)",error);
                }
            } else {
                error = errno;
                ERROR_MSG("clock_gettime CLOCK_REALTIME error %d (%m)",error);
            }
        }
        return error;
    }

};

#ifdef MODULE_FLAG
#undef MODULE_FLAG
#endif //MODULE_FLAG

#endif /* _INTER_THREADS_MSG_QUEUE_H_ */
