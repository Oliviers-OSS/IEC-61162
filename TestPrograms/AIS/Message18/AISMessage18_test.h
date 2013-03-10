/*
 * AISMessage18.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_18_TEST_H_
#define _AIS_MESSAGE_18_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg18_Data {
    const char *rawMsg;
    unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI userID;
    float speedOverGround; //SOG
    bool positionAccuracy;
    float longitude;
    float latitude;
    float courseOverGround; //COG
    IEC61162::Word trueHeading;
    IEC61162::Byte timeStamp;
    bool classBUnitFlag;
    bool classBDisplayFlag;
    bool classBDSCFlag;
    bool classBBandFlag;
    bool classBMessage22Flag;
    bool modeFlag;
    bool RAIMFlag;
    bool communicationStateSelectorFlag;
    unsigned long communicationState;
} data[] = {
        // http://www.maritec.co.za/aisvdmvdodecoding.php
        {"!AIVDM,1,1,,B,B3P;Qgh0BGrBLh6lnGQugwTUoP06,0*3F"EOM,0,235069887,7.3,0,-4.990985,47.7156133333333,201.1,511,9,0,0,0,0,0,0,1,1,393222}
        ,{"!AIVDM,1,1,,B,B3P:m5P08orj9t6rTo@Swwe5kP06,0*05"EOM,0,235058454,3.5,0,-4.55809166666667,48.3411,57.5,511,26,0,0,0,0,0,0,0,1,393222}
        ,{"!AIVDM,1,1,,B,B3P;Qgh0AWrB:jVlkAf2KwT5kP06,0*03"EOM,0,235069887,7.0,0,-4.99481666666667,47.7103383333333,208.6,511,8,0,0,0,0,0,0,0,1,393222}
};

void msgHandler(IEC61162::AISMsg18 msg,void *param);

class AISMsg18_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg18_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg18 msg,void *param);

    AISMsg18_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message18.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg18_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg18_Data);
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

#endif /* _AIS_MESSAGE_18_TEST_H_ */
