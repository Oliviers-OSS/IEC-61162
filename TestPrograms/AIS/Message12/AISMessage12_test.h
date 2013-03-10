/*
 * AISMessage12.h
 *
 *  Created on: 10 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_12_TEST_H_
#define _AIS_MESSAGE_12_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"



const struct AISMsg12_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI sourceID;
    IEC61162::Byte sequenceNumber;
    IEC61162::MMSI destinationID;
    bool retransmissionFlag;
    const char *safetyRelatedText;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
         {"!AIVDM,1,1,,B,<9NWrahn9mNPD5CDP@<CP13;f0,4*44"EOM,0,636091047,0,227137000,0,"TEST PLS ACK."
//                 Packet Type    AIVDM
//                 CHANNEL B
//                 Message Type    12  Addressed Safety Related Message
//                 Repeat Indicator    0   Default
//                 Source ID   636091047
//                 Sequence Number 0
//                 Destination ID  227137000
//                 Retransmit Flag 0   Not retransmission (default)
//                 Safety-Related Text TEST PLS ACK.
         }
        ,{"!AIVDM,1,1,,A,<9NWrahuc<7PDCDP?><IP@<CP13;f0,4*3F"EOM,0,636091047,0,258683000,0,"TST ONLY PLS ACK."
//                Packet Type AIVDM
//                CHANNEL A
//                Message Type    12  Addressed Safety Related Message
//                Repeat Indicator    0   Default
//                Source ID   636091047
//                Sequence Number 0
//                Destination ID  258683000
//                Retransmit Flag 0   Not retransmission (default)
//                Safety-Related Text TST ONLY PLS ACK.
        }
        ,{"!AIVDM,1,1,,A,<9NWrahuc<7RDCDP?><IP@<CP13;f0,4*3D"EOM,0,636091047,0,258683000,1,"TST ONLY PLS ACK."
//                Name    Value   Description
//                Packet Type AIVDM
//                CHANNEL A
//                Message Type    12  Addressed Safety Related Message
//                Repeat Indicator    0   Default
//                Source ID   636091047
//                Sequence Number 0
//                Destination ID  258683000
//                Retransmit Flag 1   Retransmitted
//                Safety-Related Text TST ONLY PLS ACK.
        }
        ,{"!AIVDM,1,1,,B,<9NWrahuc<7RDCDP?><IP@<CP13;f0,4*3E"EOM,0,636091047,0,258683000,1,"TST ONLY PLS ACK."
//                Name    Value   Description
//                Packet Type AIVDM
//                CHANNEL B
//                Message Type    12  Addressed Safety Related Message
//                Repeat Indicator    0   Default
//                Source ID   636091047
//                Sequence Number 0
//                Destination ID  258683000
//                Retransmit Flag 1   Retransmitted
//                Safety-Related Text TST ONLY PLS ACK.
        }
};

void msgHandler(IEC61162::AISMsg12 msg,void *param);

class AISMsg12_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg12_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg12 msg,void *param);

    AISMsg12_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message12.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg12_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg12_Data);
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

#endif /* _AIS_MESSAGE_12_TEST_H_ */
