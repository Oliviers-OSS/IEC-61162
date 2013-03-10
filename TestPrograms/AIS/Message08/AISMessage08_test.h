/*
 * AISMessage08.h
 *
 *  Created on: 09 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_08_TEST_H_
#define _AIS_MESSAGE_08_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const unsigned char bin1[] = {0x35,0x69,0xAF,0x05,0xA7,0x44,0x25,0xE9,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x34,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x40};
const unsigned char bin2[] = {0xC3,0x2C,0xF5,0xC3,0x1D,0xB9,0x22,0x60,0xE5,0x07,0xC5,0x12,0xC0,0x00};

const struct AISMsg08_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI sourceID;
    const unsigned int applicationID;
    const size_t size;
    const unsigned char *bytes;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
        {"!AIVDM,1,1,,B,802?LF@j2d<duL<MfB9Pq@O54d00,0*10"EOM,0,2350169,12810,sizeof(bin2),bin2
//                Packet Type AIVDM
//                CHANNEL B
//                Message Type    8   Binary Broadcast Message
//                Repeat Indicator    0   Default
//                Source ID   2350169
//                Application ID  12810
//                Binary Data C3 2C F5 C3 1D B9 22 60 E5 07 C5 12 C0 00
        }
         ,{"!AIVDM,1,1,,A,802R5Ph0BkEachFWA2GaOwwwwwwwwwwwwkBwwwwwwwwwwwwwwwwwwwwwwwu,2*57"EOM,0,2655619,75,sizeof(bin1),bin1}

         //,{"!AIVDM,1,1,,A,8B;iOQQggDLpN`?f67B1aGwagvL,2*53"
           //      ,{"!AIVDM,1,1,,B,8U2rTFRqkr?LOpvMR`iMeGO2iR?mv<eN,0*16"
             //            ,{"!AIVDM,2,2,8,B,Utwp,2*39"
};

void msgHandler(IEC61162::AISMsg8 msg,void *param);

class AISMsg08_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg08_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg8 msg,void *param);

    AISMsg08_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message8.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg08_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg08_Data);
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

#endif /* _AIS_MESSAGE_08_TEST_H_ */
