/*
 * InterThreadsMsgQueue_tests.cpp
 *
 *  Created on: 28 avr. 2013
 *      Author: oc
 */

#include "InterThreadsMsgQueue_tests.h"
#include <cstdlib>

void *reader_test_basic(void *param) {
    const size_t n(100);
    MsgQueue<n,unsigned int> *msgQueue = (MsgQueue<n,unsigned int> *)param;
    unsigned int i;
    for(i=0;i<n;i++) {
        unsigned int r(0);
        msgQueue->getMessage(r);
        TS_ASSERT_EQUALS(r,i);
    }
    return NULL;
}

void *reader_test_basic2(void *param) {
    const size_t n(100);
    const unsigned int limit(n*10000);
    MsgQueue<n,unsigned int> *msgQueue = (MsgQueue<n,unsigned int> *)param;
    unsigned int i;
    for(i=0;i<limit;i++) {
        unsigned int r(0);
        msgQueue->getMessage(r);
        TS_ASSERT_EQUALS(r,i);
    }
    return NULL;
}

void *reader_test_basic3(void *param) {
    const size_t n(100);
    const unsigned int limit(n*10000);
    MsgQueue<n,unsigned int> *msgQueue = (MsgQueue<n,unsigned int> *)param;
    unsigned int i;
    usleep(200000);
    for(i=0;i<limit;i++) {
        unsigned int r(0);
        msgQueue->getMessage(r);
        TS_ASSERT_EQUALS(r,i);
    }
    return NULL;
}

void *reader_test_basic_string(void *param) {
    const size_t n(100);
    const unsigned int limit(n*10000);
    MsgQueue<n,std::string> *msgQueue = (MsgQueue<n,std::string> *)param;
    unsigned int i;
    usleep(200000);
    for(i=0;i<limit;i++) {
        std::string str;
        msgQueue->getMessage(str);
        const unsigned int r =  strtoul(str.c_str(),NULL,0);
        TS_ASSERT_EQUALS(r,i);
    }
    return NULL;
}

void *reader_test_basic3_timeOut(void *param) {
    const size_t n(100);
    const unsigned int limit(n*10000);
    struct timespec timeout = {1,0};
    MsgQueue<n,unsigned int> *msgQueue = (MsgQueue<n,unsigned int> *)param;
    unsigned int i(0);
    int error(EXIT_SUCCESS);
    usleep(200000);
    do {
        unsigned int r(0);
        error = msgQueue->getMessage(r,timeout);
        if (EXIT_SUCCESS == error) {
            TS_ASSERT_EQUALS(r,i);
        }
        i++;
    } while(EXIT_SUCCESS == error);
    TS_ASSERT_EQUALS(error,ETIMEDOUT);
    return NULL;
}
