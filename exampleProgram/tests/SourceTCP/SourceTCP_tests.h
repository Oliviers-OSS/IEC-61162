/*
 * SourceTCP_tests.h
 *
 *  Created on: 30 avr. 2013
 *      Author: oc
 */

#ifndef _SOURCE_TCP_TESTS_H_
#define _SOURCE_TCP_TESTS_H_

#include "config.h"
#include "../../SourceTCP.h"
#define CXXTEST_HAVE_EH
#include <cxxtest/TestSuite.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <dbgflags-1/debug_macros.h>
#include <memory>

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define EOM "\r\n"
#include <dbgflags-1/loggers.h>

class SourceTCP_tests : public CxxTest::TestSuite {

public:
    void test_basic_factory_cnx() {
        int error(EXIT_SUCCESS);
        SourceDevice *device = SourceTCP::Factory("192.168.0.10","9001",error);
        TS_ASSERT_EQUALS(error,0);
        TS_ASSERT_DIFFERS(device,(SourceDevice *)0);
        delete device;
    }

    void test_basic_factory_readMsg() {
        int error(EXIT_SUCCESS);
        SourceDevice *device = SourceTCP::Factory("192.168.0.10","9001",error);
        TS_ASSERT_EQUALS(error,0);
        TS_ASSERT_DIFFERS(device,(SourceDevice *)0);
        if (device) {
            char Message[254];
            size_t size(0);
            error = device->getMessage(Message,sizeof(Message),size);
            TS_ASSERT_EQUALS(error,0);
            TS_ASSERT_DIFFERS(size,0);
            TS_ASSERT_EQUALS(Message[size-2],'\r');
            TS_ASSERT_EQUALS(Message[size-1],'\n');
        }
        delete device;
    }

    void test_basic_factory_fail_service_cnx() {
        int error(EXIT_SUCCESS);
        SourceDevice *device = SourceTCP::Factory("127.0.0.1","64900",error);
        TS_ASSERT_EQUALS(device,(SourceDevice *)0);
        TS_ASSERT_DIFFERS(error,0);
        delete device;
    }

    void test_basic_factory_fail_server_cnx() {
        int error(EXIT_SUCCESS);
        SourceDevice *device = SourceTCP::Factory("foo","64900",error);
        TS_ASSERT_EQUALS(device,(SourceDevice *)0);
        TS_ASSERT_DIFFERS(error,0);
        delete device;
    }
};


#endif /* _SOURCE_TCP_TESTS_H_ */
