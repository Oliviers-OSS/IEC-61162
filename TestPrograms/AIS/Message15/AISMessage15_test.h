/*
 * AISMessage15.h
 *
 *  Created on: 11 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_15_TEST_H_
#define _AIS_MESSAGE_15_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg15_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI sourceID;
    IEC61162::MMSI destinationID1;
    IEC61162::Byte messageID1_1;
    IEC61162::Word slotOffset1_1;
    IEC61162::Byte messageID1_2;
    IEC61162::Word slotOffset1_2;
    IEC61162::MMSI destinationID2;
    IEC61162::Byte messageID2_1;
    IEC61162::Word slotOffset2_1;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
         {"!AIVDM,1,1,,A,?42LShi:Fu?PD00,2*21"EOM,0,271000515,311883000,5,0,0
//                Packet Type AIVDM
//                CHANNEL A
//                Message Type    15  Interrogation
//                Repeat Indicator    0   Default
//                Source ID   271000515
//                Destination ID #1   311883000
//                Message Type #11    5
//                Slot Offset11   0
//                Message Type #12    0
         }
        ,{"!AIVDM,1,1,,A,?42LShiDC940D00,2*7D"EOM,0,271000515,353576000,5,0,0
//                Packet Type AIVDM
//                CHANNEL A
//                Message Type    15  Interrogation
//                Repeat Indicator    0   Default
//                Source ID   271000515
//                Destination ID #1   353576000
//                Message Type #11    5
//                Slot Offset11   0
//                Message Type #12    0
         }
         //"!AIVDM,1,1,,A,?42LShi:Fu?PD00,2*21"
         //"!AIVDM,1,1,,A,?42LShhmU=fPD00,2*75"
         //"!AIVDM,1,1,,A,?42LShiDC940D00,2*7D"
};

void msgHandler(IEC61162::AISMsg15 msg,void *param);

class AISMsg15_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg15_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg15 msg,void *param);

    AISMsg15_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message15.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg15_Tests() {
    }

    void test_emptyMsg(void) {
        const char *currentMsg = "";
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,ENODATA);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_tooBigMsg(void) {
        const char *currentMsg = "012345615890123456158901234561589012345615890123456158901234561589012345615890123456158901234561589";
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
        const char *currentMsg = "!AIVDM,1,1,,B,33P8RP@P0gOrtRVM4UGH;?v600iQ,0*10"EOM;
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg15_Data);
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

#endif /* _AIS_MESSAGE_15_TEST_H_ */
