/*
 * AISMessage1.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_1_TEST_H_
#define _AIS_MESSAGE_1_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg1_Data {
    const char *rawMsg;
    unsigned char repeatIndicator;
    IEC61162::MMSI userID;
    unsigned char navigationStatus;
    float rateOfTurn; // ROT
    float speedOverGround; //SOG
    bool positionAccuracy;
    float longitude;
    float latitude;
    float courseOverGround; //COG
    unsigned short trueHeading;
    unsigned char timeStamp;
    unsigned char specialManoeuvreIndicator;
    bool RAIMFlag;
    unsigned long communicationState;
} data[] = {
         {"!AIVDM,1,1,,A,1P000Oh1IT1svTP2r:43grwb0Eq4,0*01"EOM,2,127,0,1.1,61.2,0,27.0833333333333,5.08333333333333,95.9,351,53,0,0,89668}
        ,{"!AIVDM,1,1,,A,15MgK45P3@G?fl0E`JbR0OwT0@MS,0*4E"EOM,0,366730000,IEC61162::moored,NAN,20.8,0,-122.392531666667,37.8038033333333,51.3,511,50,0,0,67427} // ROT not available
        ,{"!AIVDM,1,1,,B,18M2Et0003:u9hEhETTaM10>051L,0*7C"EOM,0,567318000,0,0,0.3,0,153.169083333333,-27.3729816666667,242,32,7,0,0,20572}
};

void msgHandler(IEC61162::AISMsg1 msg,void *param);

class AISMsg1_Tests : public CxxTest::TestSuite /*, sigc::trackable*/
{
    IEC61162::AISMessage msg;
    const AISMsg1_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg1 msg,void *param);

    AISMsg1_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message1.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg1_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg1_Data);
        currentMsg = data;
        while(nbMsg > 0) {
            nbMsg--;
            const int error = msg.parse(currentMsg->rawMsg);
            TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
            TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
            currentMsg++;
        }
    }

    void test_AISMultiSentenceMsg() {
        const char *sentence1 = "!AIVDM,2,1,7,A,1P000Oh1IT1svT,0*28"EOM;
        const char *sentence2 = "!AIVDM,2,2,7,A,P2r:43grwb0Eq4,0*0C"EOM;
        currentMsg = data;
        int error = msg.parse(sentence1);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
        error = msg.parse(sentence2);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_AISMultiSentenceMsgReverseOrder() {
        const char *sentence2 = "!AIVDM,2,1,7,A,1P000Oh1IT1svT,0*28"EOM;
        const char *sentence1 = "!AIVDM,2,2,7,A,P2r:43grwb0Eq4,0*0C"EOM;
        currentMsg = data;
        int error = msg.parse(sentence1);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
        error = msg.parse(sentence2);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }
};

#endif /* _AIS_MESSAGE_1_TEST_H_ */
