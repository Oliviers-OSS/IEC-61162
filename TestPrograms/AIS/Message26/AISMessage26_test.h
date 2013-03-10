/*
 * AISMessage26.h
 *
 *  Created on: 09 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_26_TEST_H_
#define _AIS_MESSAGE_26_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const unsigned char bin1[] = {0x35,0x69,0xAF,0x05,0xA7,0x44,0x25,0xE9,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x34,0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x40};

const struct AISMsg26_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI sourceID;
    bool destinationIndicator;
    bool binaryDataFlag;
    IEC61162::MMSI destinationID;
    IEC61162::Word applicationIdentifier;
    IEC61162::Byte binaryData[(128/8)+(224/8)+(224/8)+(224/8)+(224/8)]; //1024 bits
    size_t binaryDataSize; // size in bits
    bool communicationStateSelectorFlag;
    IEC61162::DWord communicationState;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
         {"!AIVDM,1,1,,A,J@TwvJN6p`jGDKkdKm@5B7nm=lrG63>j?eOucGSw?ED,2*3A"EOM
//                 Name   Value   Description
//                 Packet Type AIVDM
//                 CHANNEL A
//                 Message Type    26  Multiple Slot Binary Message
//                 Repeat Indicator    1
//                 Source ID   38796905
//                 Destination Indicator   1   Addressed message (to individual station(s))
//                 Binary Data Flag    1   Binary data coded as defined by using the   16-bit Application identifier
//                 Destination ID  565748517
//                 App Identifier  51 BC
//                 Binary Data EC 6F 54 05 48 7D B5
         }
        ,{"!AIVDM,1,1,,B,J>hMDggogu69dW8,2*4B"EOM
//                Name    Value   Description
//                Packet Type AIVDM
//                CHANNEL B
//                Message Type    26  Multiple Slot Binary Message
//                Repeat Indicator    0   Default
//                Source ID   990336190
//                Destination Indicator   1   Addressed message (to individual station(s))
//                Binary Data Flag    1   Binary data coded as defined by using the   16-bit Application identifier
//                Destination ID  1039135842
//                App Identifier  B2 72
//                Binary Data 00
        }
        ,{"!AIVDM,1,1,,B,JKj@W60EULcouqI7,0*75"EOM
//                Packet Type AIVDM
//                CHANNEL B
//                Message Type    26  Multiple Slot Binary Message
//                Repeat Indicator    1
//                Source ID   790898456
//                Destination Indicator   0   Broadcast Geographical Area Message (default)
//                Binary Data Flag    0   Unstructured binary data
//                Binary Data CA F7 F7 96 47
        }
};

void msgHandler(IEC61162::AISMsg26 msg,void *param);

class AISMsg26_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg26_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg26 msg,void *param);

    AISMsg26_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message26.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg26_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg26_Data);
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

#endif /* _AIS_MESSAGE_26_TEST_H_ */
