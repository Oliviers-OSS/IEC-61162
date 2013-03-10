/*
 * InternalDatabase_tests.h
 *
 *  Created on: 30 avr. 2013
 *      Author: oc
 */

#ifndef _INTERNAL_DATABASE_TESTS_H_
#define _INTERNAL_DATABASE_TESTS_H_

#include "config.h"
#define CXXTEST_HAVE_EH
#include <cxxtest/TestSuite.h>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
//#include <dbgflags-1/loggers.h>
//#include <dbgflags-1/debug_macros.h>
#include "../../debug.h"
//#include "../../Configuration.h"
#include "../../InternalDatabaseWriter.h"

#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#define EOM "\r\n"

class InternalDatabase_tests : public CxxTest::TestSuite {

public:

    void test_init() {
        try {
            InternalDatabaseWriter database;
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_Msg1() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message1.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage1));
            const int error = aisMessageParser.parse("!AIVDM,1,1,,A,1P000Oh1IT1svTP2r:43grwb0Eq4,0*01"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_Msg3() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message3.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage3));
            const int error = aisMessageParser.parse("!AIVDM,1,1,,B,33I3:M5P00OcJVtKclMdo?wN0000,0*0D"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_Msg5() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message5.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage5));
            int error = aisMessageParser.parse("!AIVDM,2,1,0,B,53HO91P00000IKKSG:0DpEb1=Dr3>2222222220t1@I535fbS52C1H11,0*1B"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            error = aisMessageParser.parse("!AIVDM,2,2,0,B,H4iBC`888888880,2*53"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            //
            error = aisMessageParser.parse("!AIVDM,2,1,0,B,53HO91P00000IKKSG:0DpEb1=Dr3>2222222220t1@I535fbS52C1H11,0*1B"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            error = aisMessageParser.parse("!AIVDM,2,2,0,B,H4iBC`888888880,2*53"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_Msg24A() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message24A.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24A));
            int error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`Pd488Dh45800000000000,2*35"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_Msg24B() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message24B.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24B));
            int error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`TUDBE584747lioq01P1220,0*16"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_Msg24() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message24A.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24A));
            aisMessageParser.message24B.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24B));
            int error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`Pd488Dh45800000000000,2*35"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`TUDBE584747lioq01P1220,0*16"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`Pd488Dh45800000000000,2*35"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`TUDBE584747lioq01P1220,0*16"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_Msg27() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message27.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage27));
            int error = aisMessageParser.parse("!AIVDM,1,1,,A,KoVTqBp1g8WOr`Co9SPkJMncREIE,0*4B"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_transaction() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message24A.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24A));
            int error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`Pd488Dh45800000000000,2*35"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            TS_ASSERT(database.isTransactionInProgress());
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }

    void test_transactionTimeOut() {
        try {
            InternalDatabaseWriter database;
            IEC61162::AISMessage aisMessageParser;
            aisMessageParser.message24A.connect(sigc::mem_fun(database, &InternalDatabaseWriter::onMessage24A));
            int error = aisMessageParser.parse("!AIVDM,1,1,,B,H39SF`Pd488Dh45800000000000,2*35"EOM);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            TS_ASSERT(database.isTransactionInProgress());
            const time_t timeOut(theConfiguration.getTransactionMaxTime());
            TS_TRACE("time out test in progress, please wait...");
            sleep(timeOut + 1);
            TS_ASSERT_EQUALS(database.numberOfOperationsInCurrentTransaction,NO_TRANSACTION_IN_PROGRESS);
        } catch(InternalDatabaseWriter::exception &e) {
            TS_FAIL("InternalDatabaseWriter::exception caught");
        }
    }
};


#endif /* _INTERNAL_DATABASE_TESTS_H_ */
