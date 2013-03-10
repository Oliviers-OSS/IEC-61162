/*
 * SourceMultiTCP_tests.h
 *
 *  Created on: 30 avr. 2013
 *      Author: oc
 */

#ifndef _SOURCE_TCP_TESTS_H_
#define _SOURCE_TCP_TESTS_H_

#include "config.h"
#include "../../SourceMultiTCP.h"
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

#define MODULE_FLAG FLAG_SOURCE

class SourceMultiTCP_tests : public CxxTest::TestSuite {

public:
    void test_basic_factory_cnx() {
        int error(EXIT_SUCCESS);
        SourceMultiTCP::address addresses[] = {
                {"192.168.0.10","9001"}
                ,{"192.168.0.10","9002"}
                ,{"192.168.0.10","9003"}
                ,{"192.168.0.10","9004"}
        };
        const size_t number = sizeof(addresses)/sizeof(addresses[0]);
        SourceDevice *device = SourceMultiTCP::Factory(addresses,number,error);
        TS_ASSERT_EQUALS(error,0);
        TS_ASSERT_DIFFERS(device,(SourceDevice *)0);
        delete device;
    }

    void test_basic_factory_readMsg() {
        int error(EXIT_SUCCESS);
        SourceMultiTCP::address addresses[] = {
                {"192.168.0.10","9001"}
                ,{"192.168.0.10","9002"}
                ,{"192.168.0.10","9003"}
                ,{"192.168.0.10","9004"}
        };
        const size_t number = sizeof(addresses)/sizeof(addresses[0]);
        SourceDevice *device = SourceMultiTCP::Factory(addresses,number,error);
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
            DEBUG_DUMP_MEMORY(Message,size);
        }
        delete device;
    }

    void test_basic_factory_read5Msg() {
        int error(EXIT_SUCCESS);
        SourceMultiTCP::address addresses[] = {
                {"192.168.0.10","9001"}
                ,{"192.168.0.10","9002"}
                ,{"192.168.0.10","9003"}
                ,{"192.168.0.10","9004"}
        };
        const size_t number = sizeof(addresses)/sizeof(addresses[0]);
        SourceDevice *device = SourceMultiTCP::Factory(addresses,number,error);
        TS_ASSERT_EQUALS(error,0);
        TS_ASSERT_DIFFERS(device,(SourceDevice *)0);
        if (device) {
            for(unsigned int i=0;i<5;i++) {
                char Message[254];
                size_t size(0);
                error = device->getMessage(Message,sizeof(Message),size);
                TS_ASSERT_EQUALS(error,0);
                TS_ASSERT_DIFFERS(size,0);
                TS_ASSERT_EQUALS(Message[size-2],'\r');
                TS_ASSERT_EQUALS(Message[size-1],'\n');
                DEBUG_DUMP_MEMORY(Message,size);
            }
        }
        delete device;
    }

    void test_basic_factory_fail_service_cnx() {
        int error(EXIT_SUCCESS);
        SourceMultiTCP::address addresses[] = {
                        {"192.168.0.10","9001"}
                        ,{"192.168.0.10","64900"}
                        ,{"192.168.0.10","9003"}
                        ,{"192.168.0.10","9004"}
        };
        const size_t number = sizeof(addresses)/sizeof(addresses[0]);
        SourceDevice *device = SourceMultiTCP::Factory(addresses,number,error);
        TS_ASSERT_EQUALS(device,(SourceDevice *)0);
        TS_ASSERT_DIFFERS(error,0);
        delete device;
    }

    void test_basic_factory_fail_server_cnx() {
        int error(EXIT_SUCCESS);
        SourceMultiTCP::address addresses[] = {
                {"192.168.0.10","9001"}
                ,{"foo","9002"}
                ,{"192.168.0.10","9003"}
                ,{"192.168.0.10","9004"}
        };
        const size_t number = sizeof(addresses)/sizeof(addresses[0]);
        SourceDevice *device = SourceMultiTCP::Factory(addresses,number,error);
        TS_ASSERT_EQUALS(device,(SourceDevice *)0);
        TS_ASSERT_DIFFERS(error,0);
        delete device;
    }
};


#endif /* _SOURCE_TCP_TESTS_H_ */
