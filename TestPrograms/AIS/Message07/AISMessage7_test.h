/*
 * AISMessage7.h
 *
 *  Created on: 09 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_7_TEST_H_
#define _AIS_MESSAGE_7_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"



const struct AISMsg7_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI sourceID;
    IEC61162::MMSI destinationID1;
    IEC61162::Byte sequenceNumberForID1;
    IEC61162::MMSI destinationID2;
    IEC61162::Byte sequenceNumberForID2;
    IEC61162::MMSI destinationID3;
    IEC61162::Byte sequenceNumberForID3;
    IEC61162::MMSI destinationID4;
    IEC61162::Byte sequenceNumberForID4;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
         {"!AIVDM,1,1,,A,73HqmP0n<sf0,0*61"EOM,0,227440000,227340000,0,0,0,0,0,0
//                 Name    Value   Description
//                 Packet Type AIVDM
//                 CHANNEL A
//                 Message Type    7   Binary Acknowledge Message
//                 Repeat Indicator    0   Default
//                 Source ID   227440000
//                 Destination ID #1   227340000
//                 Sequence Number #1  0
         }
        ,{"!AIVDM,1,1,,B,73HqmP0n<sf1,0*63"EOM,0,227440000,227340000,1,0,0,0,0,0
//             Packet Type AIVDM
//             CHANNEL B
//             Message Type    7   Binary Acknowledge Message
//             Repeat Indicator    0   Default
//             Source ID   227440000
//             Destination ID #1   227340000
//             Sequence Number #1  1
         }
};

void msgHandler(IEC61162::AISMsg7 msg,void *param);

class AISMsg7_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg7_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg7 msg,void *param);

    AISMsg7_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message7.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg7_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg7_Data);
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

#endif /* _AIS_MESSAGE_7_TEST_H_ */
