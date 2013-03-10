/*
 * AISAlarm21.h
 *
 *  Created on: 07 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_ALARM_TEST_H_
#define _AIS_ALARM_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISAlarm_Data {
    const char *rawMsg;
    unsigned char hour;
    unsigned char minutes;
    unsigned char seconds;
    IEC61162::Byte id;
    char condition;
    char state;
    const char *description;
} data[] = {
        {"$AIALR,142904.00,001,A,V,AIS: Tx malfunction*56"EOM,14,29,4,1,'A','V',"AIS: Tx malfunction"}
        ,{"$AIALR,142905.00,002,A,V,AIS: Antenna VSWR exceeds limit*59"EOM,14,29,5,2,'A','V',"AIS: Antenna VSWR exceeds limit"}
        ,{"$AIALR,142906.00,003,A,V,AIS: Rx channel 1 malfunction*02"EOM,14,29,6,3,'A','V',"AIS: Rx channel 1 malfunction"}
        ,{"$AIALR,142906.00,004,A,V,AIS: Rx channel 2 malfunction*06"EOM,14,29,6,4,'A','V',"AIS: Rx channel 2 malfunction"}
        ,{"$AIALR,142907.00,006,A,V,AIS: General failure*2A"EOM,14,29,7,6,'A','V',"AIS: General failure"}
        ,{"$AIALR,142907.00,025,A,V,AIS: External EPFS lost*04"EOM,14,29,7,25,'A','V',"AIS: External EPFS lost"}
        ,{"$AIALR,142908.00,026,A,V,AIS: No sensor position in use*7B"EOM,14,29,8,26,'A','V',"AIS: No sensor position in use"}
        ,{"$AIALR,142908.00,029,A,V,AIS: No valid SOG information*7E"EOM,14,29,8,29,'A','V',"AIS: No valid SOG information"}
        ,{"$AIALR,142908.00,030,A,V,AIS: No valid COG information*66"EOM,14,29,8,30,'A','V',"AIS: No valid COG information"}
};

void msgHandler(IEC61162::Alarm msg,void *param);

class AISAlarm_Test : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISAlarm_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::Alarm msg,void *param);

    AISAlarm_Test()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.alarm.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISAlarm_Test() {
    }

    void test_emptyMsg(void) {
        const char *currentMsg = "";
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,ENODATA);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_tooBigMsg(void) {
        const char *currentMsg = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EMSGSIZE);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_badMessage(void) {
        const char *currentMsg = "ABCDEFGHIJKL";
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EPROTO);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_noChecksumMsg(void) {
        const char *currentMsg = "ABCDEFGHIJKL"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EBADMSG);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_badCheckSum(void) {
        const char *currentMsg = "$AIALR,142904.00,001,A,V,AIS: Tx malfunction*02"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EBADMSG);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_unknownStartSequence(void) {
        const char *currentMsg = "#AIVDM,1,1,,B,33P8RP@P0gOrtRVM4UGH;?v600iQ,0*1A"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EPROTO);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_unknownTalker(void) {
        const char *currentMsg = "!FOODM,1,1,,B,13Hj5J7000Od<fdKQJ3Iw`S>28FK,0*27"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EINVAL);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_AISMsg(void) {
        register unsigned int nbMsg = sizeof(data) / sizeof(AISAlarm_Data);
        currentMsg = data;
        while(nbMsg > 0) {
            nbMsg--;
            int error = msg.parse(currentMsg->rawMsg);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
            currentMsg++;
        }
    }
};

#endif /* _AIS_ALARM_TEST_H_ */
