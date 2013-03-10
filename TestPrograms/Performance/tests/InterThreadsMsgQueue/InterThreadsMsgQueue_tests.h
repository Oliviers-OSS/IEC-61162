/*
 * InterThreadsMsgQueue_tests.h
 *
 *  Created on: 27 avr. 2013
 *      Author: oc
 */

#ifndef INTER_THREADS_MSGQUEUE_TESTS_H_
#define INTER_THREADS_MSGQUEUE_TESTS_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <cerrno>
#include <cstdlib>
#include <pthread.h>
#include <string>
#include <dbgflags-1/debug_macros.h>
#include "../../InterThreadsMsgQueue.h"

#define EOM "\r\n"
#include <dbgflags-1/loggers.h>

void *reader_test_basic(void *param);
void *reader_test_basic2(void *param);
void *reader_test_basic3(void *param);
void *reader_test_basic_string(void *param);
void *reader_test_basic3_timeOut(void *param);

class InterThreadsMsgQueue_tests : public CxxTest::TestSuite {

public:
    void test_basic_mono() {
        const size_t n(4);
        MsgQueue<n,unsigned int> msgQueue;
        unsigned int i;
        for(i=0;i<n;i++) {
            msgQueue.putMessage(i);
        }
        for(i=0;i<n;i++) {
            unsigned int r(0);
            msgQueue.getMessage(r);
            TS_ASSERT_EQUALS(r,i);
        }
    }

    void test_basic_reader_writer() {
        const size_t n(100);
        MsgQueue<n,unsigned int> msgQueue;
        pthread_t reader;
        pthread_attr_t attr;
        int error = pthread_attr_init(&attr);
        if (0 == error) {
            error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            if (0 == error) {
                error = pthread_create(&reader,&attr,reader_test_basic,(void*)&msgQueue);
                if (0 == error) {
                    unsigned int i;
                    for(i=0;i<n;i++) {
                        msgQueue.putMessage(i);
                    }
                    error = pthread_join(reader,NULL);
                    if (error != 0) {
                        ERROR_MSG("pthread_join error %d",error);
                    }
                } else {
                    ERROR_MSG("pthread_attr_setdetachstate error %d",error);
                }
            } else {
                ERROR_MSG("pthread_attr_init error %d",error);
            }
            error = pthread_attr_destroy(&attr);
            if (error != 0) {
                ERROR_MSG("pthread_attr_destroy error %d",error);
            }
        } else {
            ERROR_MSG("pthread_attr_init error %d",error);
        }
    }

    void test_basic_reader_writer2() {
        const size_t n(100);
        MsgQueue<n,unsigned int> msgQueue;
        pthread_t reader;
        pthread_attr_t attr;
        int error = pthread_attr_init(&attr);
        if (0 == error) {
            error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            if (0 == error) {
                error = pthread_create(&reader,&attr,reader_test_basic2,(void*)&msgQueue);
                if (0 == error) {
                    unsigned int i;
                    const unsigned int limit(n*10000);
                    for(i=0;i<limit;i++) {
                        msgQueue.putMessage(i);
                    }
                    error = pthread_join(reader,NULL);
                    if (error != 0) {
                        ERROR_MSG("pthread_join error %d",error);
                    }
                } else {
                    ERROR_MSG("pthread_attr_setdetachstate error %d",error);
                }
            } else {
                ERROR_MSG("pthread_attr_init error %d",error);
            }
            error = pthread_attr_destroy(&attr);
            if (error != 0) {
                ERROR_MSG("pthread_attr_destroy error %d",error);
            }
        } else {
            ERROR_MSG("pthread_attr_init error %d",error);
        }
    }

    void test_basic_reader_writer3() {
        const size_t n(100);
        MsgQueue<n,unsigned int> msgQueue;
        pthread_t reader;
        pthread_attr_t attr;
        int error = pthread_attr_init(&attr);
        if (0 == error) {
            error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            if (0 == error) {
                error = pthread_create(&reader,&attr,reader_test_basic3,(void*)&msgQueue);
                if (0 == error) {
                    unsigned int i;
                    const unsigned int limit(n*10000);
                    for(i=0;i<limit;i++) {
                        msgQueue.putMessage(i);
                    }
                    error = pthread_join(reader,NULL);
                    if (error != 0) {
                        ERROR_MSG("pthread_join error %d",error);
                    }
                } else {
                    ERROR_MSG("pthread_attr_setdetachstate error %d",error);
                }
            } else {
                ERROR_MSG("pthread_attr_init error %d",error);
            }
            error = pthread_attr_destroy(&attr);
            if (error != 0) {
                ERROR_MSG("pthread_attr_destroy error %d",error);
            }
        } else {
            ERROR_MSG("pthread_attr_init error %d",error);
        }
    }

    void test_basic_reader_writer_string() {
        const size_t n(100);
        MsgQueue<n,std::string> msgQueue;
        pthread_t reader;
        pthread_attr_t attr;
        int error = pthread_attr_init(&attr);
        if (0 == error) {
            error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            if (0 == error) {
                error = pthread_create(&reader,&attr,reader_test_basic_string,(void*)&msgQueue);
                if (0 == error) {
                    unsigned int i;
                    const unsigned int limit(n*10000);
                    for(i=0;i<limit;i++) {
                        std::stringstream str;
                        str << i;
                        msgQueue.putMessage(str.str());
                    }
                    error = pthread_join(reader,NULL);
                    if (error != 0) {
                        ERROR_MSG("pthread_join error %d",error);
                    }
                } else {
                    ERROR_MSG("pthread_attr_setdetachstate error %d",error);
                }
            } else {
                ERROR_MSG("pthread_attr_init error %d",error);
            }
            error = pthread_attr_destroy(&attr);
            if (error != 0) {
                ERROR_MSG("pthread_attr_destroy error %d",error);
            }
        } else {
            ERROR_MSG("pthread_attr_init error %d",error);
        }
    }

    void test_basic_timeout() {
        const size_t n(4);
        MsgQueue<n,unsigned int> msgQueue;
        struct timespec timeout = {1,0};
        unsigned int msg(-1);
        const int error = msgQueue.getMessage(msg,timeout);
        TS_ASSERT_EQUALS(error,ETIMEDOUT);
    }

    void test_basic_timeout2() {
        const size_t n(4);
        MsgQueue<n,unsigned int> msgQueue;
        struct timespec timeout = {0,500};
        unsigned int msg(-1);
        const int error = msgQueue.getMessage(msg,timeout);
        TS_ASSERT_EQUALS(error,ETIMEDOUT);
    }

    void test_basic_timeout3() {
        const size_t n(4);
        MsgQueue<n,unsigned int> msgQueue;
        struct timespec timeout = {0,500};
        const unsigned int refMsg(1);
        unsigned int msg(0);
        msgQueue.putMessage(refMsg);
        const int error = msgQueue.getMessage(msg,timeout);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(msg,refMsg);
    }

    void test_basic_reader_writer3_timeOut() {
        const size_t n(100);
        MsgQueue<n,unsigned int> msgQueue;
        pthread_t reader;
        pthread_attr_t attr;
        int error = pthread_attr_init(&attr);
        if (0 == error) {
            error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            if (0 == error) {
                error = pthread_create(&reader,&attr,reader_test_basic3_timeOut,(void*)&msgQueue);
                if (0 == error) {
                    unsigned int i;
                    const unsigned int limit(n*10000);
                    for(i=0;i<limit;i++) {
                        msgQueue.putMessage(i);
                    }
                    error = pthread_join(reader,NULL);
                    if (error != 0) {
                        ERROR_MSG("pthread_join error %d",error);
                    }
                } else {
                    ERROR_MSG("pthread_attr_setdetachstate error %d",error);
                }
            } else {
                ERROR_MSG("pthread_attr_init error %d",error);
            }
            error = pthread_attr_destroy(&attr);
            if (error != 0) {
                ERROR_MSG("pthread_attr_destroy error %d",error);
            }
        } else {
            ERROR_MSG("pthread_attr_init error %d",error);
        }
    }
};


#endif /* INTERTHREADSMSGQUEUE_TESTS_H_ */
