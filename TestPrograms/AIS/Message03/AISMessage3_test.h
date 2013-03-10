/*
 * AISMessage3.h
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

const struct AISMsg3_Data {
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
        {"!AIVDM,1,1,,B,33I3:M5P00OcJVtKclMdo?wN0000,0*0D"EOM,0,227592820,5,-729,0,0,-4.496695,48.38249,329.2,NAN,47,0,0,0}
        ,{"!AIVDM,1,1,,B,33Gkm25000OcVU8KcoW4>17l0000,0*34"EOM,0,226293000,5,0,0,0,-4.45583166666667,48.3838333333333,108,35,58,0,0,0}
        //,{"!AIVDM,1,1,,B,33Gg8@?P?wOaW>vKR=hv4?vb05@C,0*48
};

void msgHandler(IEC61162::AISMsg3 msg,void *param);

class AISMsg3_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg3_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg3 msg,void *param);

    AISMsg3_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message3.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg3_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg3_Data);
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



#endif /* _AIS_MESSAGE_1_TEST_H_ */
