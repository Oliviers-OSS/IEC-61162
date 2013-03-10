/*
 * AISMessage6.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_6_TEST_H_
#define _AIS_MESSAGE_6_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg6_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI sourceID;
    IEC61162::Byte sequenceNumber;
    IEC61162::MMSI destinationID;
    bool retransmitFlag;
    IEC61162::BinaryData binaryData;
} data[] = {
        // http://www.maritec.co.za/aisvdmvdodecoding.php
        // http://www.aggsoft.com/ais-decoder.htm
         { "!AIVDM,1,1,9,B,61c2;qLPH1m@wsm6ARhp<ji6ATHd<C8f=Bhk>34k;S8i=3ToDjhi=3Di<2pp=34k>4D,2*01"EOM
//                 Packet Type    AIVDM
//                 CHANNEL B
//                 Message Type    6   Addressed Binary Message
//                 Repeat Indicator    0   Default
//                 Source ID   112233445
//                 Sequence Number 3
//                 Destination ID  135792468
//                 Retransmit Flag 0   Not retransmission (default)
//                 Application ID  65469
//                 Binary Data 46 46 2C 38 33 2C 46 46 46 2C 31 32 2E
                 ,0
                 ,112233445
                 ,3
                 ,135792468
                 ,0
         }
         ,{"!AIVDM,1,1,,A,6h2E:p66B2SR04<0@00000000000,0*4C"EOM
//                 Packet Type    AIVDM
//                 CHANNEL A
//                 Message Type    6   Addressed Binary Message
//                 Repeat Indicator    3   Do not repeat any more
//                 Source ID   2444000
//                 Sequence Number 1
//                 Destination ID  563219000
//                 Retransmit Flag 1   Retransmitted
//                 Application ID  67
//                 Binary Data 00 40 00 00 00 00 00 00 00 00
                 ,3,2444000,1,563219000,1
         }
         //"!AIVDM,1,1,,B,6028nC@0R=T@@Wj=0h0@0`0,2*05"
         //"!AIVDM,1,1,,A,6028nC@0R=T@@Wj=0h0@0`0,2*06"
         //"!AIVDM,1,1,,A,6028nC@0R=T@@Wj=0h0@0`0,2*06"

};

void msgHandler(IEC61162::AISMsg6 msg,void *param);

class AISMsg6_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg6_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg6 msg,void *param);

    AISMsg6_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message6.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg6_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg6_Data);
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

#endif /* _AIS_MESSAGE_5_TEST_H_ */
