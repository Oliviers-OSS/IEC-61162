/*
 * Configuration_tests.h
 *
 *  Created on: 30 avr. 2013
 *      Author: oc
 */

#ifndef _DELIMITED_FILE_TESTS_H_
#define _DELIMITED_FILE_TESTS_H_

#include "config.h"
#define CXXTEST_HAVE_EH
#include <cxxtest/TestSuite.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
//#include <dbgflags-1/loggers.h>
//#include <dbgflags-1/debug_macros.h>
#include "../../debug.h"
#include "../../Configuration.h"

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define EOM "\r\n"

class Configuration_tests : public CxxTest::TestSuite {

public:

    void test_basic_full_short_equals() {
        const char *argv[] = {
                "program","-s=tcp://127.0.0.1:40000","-d=/tmp","-l=debug","-m=0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(argv[1]+3,theConfiguration.getSource());
        TS_ASSERT_EQUALS(argv[2]+3,theConfiguration.getDatabase());
        const int traceLevel = setlogmask(0);
        TS_ASSERT_EQUALS(traceLevel,LOG_UPTO(LOG_DEBUG));
        TS_ASSERT_EQUALS(debugFlags.mask,0x1234);
    }

    void test_basic_full_short() {
        const char *argv[] = {
                "program","-s","tcp://127.0.0.1:40000","-d","/tmp","-l","debug","-m","0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(argv[2],theConfiguration.getSource());
        TS_ASSERT_EQUALS(argv[4],theConfiguration.getDatabase());
        const int traceLevel = setlogmask(0);
        TS_ASSERT_EQUALS(traceLevel,LOG_UPTO(LOG_DEBUG));
        TS_ASSERT_EQUALS(debugFlags.mask,0x1234);
    }

    void test_basic_full_long_equals() {
        const char *argv[] = {
                "program","--source=tcp://127.0.0.1:40000","--database=/tmp","--log-level=debug","--log-mask=0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(argv[1]+strlen("--source="),theConfiguration.getSource());
        TS_ASSERT_EQUALS(argv[2]+strlen("--database="),theConfiguration.getDatabase());
        const int traceLevel = setlogmask(0);
        TS_ASSERT_EQUALS(traceLevel,LOG_UPTO(LOG_DEBUG));
        TS_ASSERT_EQUALS(debugFlags.mask,0x1234);
    }

    void test_basic_full_long() {
        const char *argv[] = {
                "program","--source","tcp://127.0.0.1:40000","--database","/tmp","--log-level","debug","--log-mask","0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(argv[2],theConfiguration.getSource());
        TS_ASSERT_EQUALS(argv[4],theConfiguration.getDatabase());
        const int traceLevel = setlogmask(0);
        TS_ASSERT_EQUALS(traceLevel,LOG_UPTO(LOG_DEBUG));
        TS_ASSERT_EQUALS(debugFlags.mask,0x1234);
    }

    void test_basic_missing_args1() {
        const char *argv[] = {
                "program","--source","--database","/tmp","--log-level","debug","--log-mask","0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EINVAL);
    }

    void test_basic_missing_args2() {
        const char *argv[] = {
                "program","-s=","-d=/tmp","-l=debug","-m=0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EINVAL);
    }

    void test_basic_missing_args3() {
        const char *argv[] = {
                "program","-s","-d=/tmp","-l=debug","-m=0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EINVAL);
    }

    void test_basic_missing_args4() {
        const char *argv[] = {
                "program","--source=file://toto/tmp","--database","-l","debug","-m=0x1234"
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EINVAL);
    }

    void test_basic_missing_args5() {
        const char *argv[] = {
                "program","--source=file://toto/tmp","--database=/tmp/db","-l","debug","-m="
        };
        int argc = sizeof(argv)/sizeof(argv[0]);
        int error = theConfiguration.init(argc,(char **)argv);

        TS_ASSERT_EQUALS(error,EINVAL);
    }
};


#endif /* _DELIMITED_FILE_TESTS_H_ */
