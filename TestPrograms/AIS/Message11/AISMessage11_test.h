/*
 * AISMessage11.h
 *
 *  Created on: 09 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_11_TEST_H_
#define _AIS_MESSAGE_11_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg11_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI userID;
    unsigned short UTC_year;
    unsigned char UTC_month;
    unsigned char UTC_day;
    unsigned char UTC_hour;
    unsigned char UTC_minute;
    unsigned char UTC_second;
    bool positionAccuracy;
    float longitude;
    float latitude;
    unsigned char typeOfPositionDevice;
    bool transMissionControlForLongRangeBroadcastMessage;
    bool RAIMFlag;
    unsigned long communicationState;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
        {"!AIVDM,1,1,,B,;3IPuJ1uUWehW0HSQPHj5i100000,0*42"EOM,0,228081000,2009,6,15,13,48,39,0,5.36413333333333,43.3108333333333,1,0,0
//                Packet Type AIVDM
//                CHANNEL B
//                Message Type    11  UTC And Date Response
//                Repeat Indicator    0   Default
//                User ID 228081000
//                Year (UTC)  2009
//                Month (UTC) 6
//                Day (UTC)   15
//                Hour (UTC)  13
//                Minute (UTC)    48
//                Second (UTC)    39
//                Position Accuracy   0   An unaugmented GNSS fix with accuracy > 10 m
//                Longitude   5.36413333333333    East
//                Latitude    43.3108333333333    North
//                Electronic Position Fixing Device Type  1   GPS
//                RAIM flag   0   RAIM not in use (default)
//                Communication State 0   Sync state: UTC Direct; Slot Timeout: This was the last transmission in this slot; Slot offset: 0
//                Communication Sync State    0   Sync state: UTC Direct
//                Communication Slot Timeout  0   Slot Timeout: This was the last transmission in this slot
//                Communication Sub Message   0
//                Communication Utc Hour  No value
//                Communication Utc Minute    No value
//                Communication Time Stamp    No value
//                Communication Slot Number   No value
//                Communication Received Stations No value
//                Communication Slot Offset   0
        }
        ,{"!AIVDM,1,1,,A,;9115QQuUWelW0IMuRHB9SA00000,0*3A"EOM,0,605046150,2009,6,15,13,52,39,0,5.56360166666667,42.438635,1,0,0
//                Packet Type AIVDM
//                CHANNEL A
//                Message Type    11  UTC And Date Response
//                Repeat Indicator    0   Default
//                User ID 605046150
//                Year (UTC)  2009
//                Month (UTC) 6
//                Day (UTC)   15
//                Hour (UTC)  13
//                Minute (UTC)    52
//                Second (UTC)    39
//                Position Accuracy   0   An unaugmented GNSS fix with accuracy > 10 m
//                Longitude   5.56360166666667    East
//                Latitude    42.438635   North
//                Electronic Position Fixing Device Type  1   GPS
//                RAIM flag   0   RAIM not in use (default)
//                Communication State 0   Sync state: UTC Direct; Slot Timeout: This was the last transmission in this slot; Slot offset: 0
//                Communication Sync State    0   Sync state: UTC Direct
//                Communication Slot Timeout  0   Slot Timeout: This was the last transmission in this slot
//                Communication Sub Message   0
//                Communication Utc Hour  No value
//                Communication Utc Minute    No value
//                Communication Time Stamp    No value
//                Communication Slot Number   No value
//                Communication Received Stations No value
//                Communication Slot Offset   0
        }


};

void msgHandler(IEC61162::AISMsg11 msg,void *param);

class AISMsg11_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg11_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg11 msg,void *param);

    AISMsg11_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message11.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg11_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg11_Data);
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

#endif /* _AIS_MESSAGE_11_TEST_H_ */
