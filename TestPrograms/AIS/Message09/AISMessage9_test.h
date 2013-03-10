/*
 * AISMessage9.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_9_TEST_H_
#define _AIS_MESSAGE_9_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg9_Data {
    const char *rawMsg;
    unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI userID;
    unsigned short altitude;
    float speedOverGround; //SOG
    bool positionAccuracy;
    float longitude;
    float latitude;
    float courseOverGround; //COG
    unsigned char timeStamp;
    bool altitudeSensor;
    bool DTE;
    bool assignedModeFlag;
    bool RAIMFlag;
    bool communicationStateSelectionFlag;
    unsigned long communicationState;
} data[] = {
        // http://www.maritec.co.za/aisvdmvdodecoding.php
        {"!AIVDM,1,1,,B,91b55vRAirOn<94M097lV@@20<6=,0*5D"EOM,0,111232506,583,12.2,0,-2.14308833333333,50.685065,117.7,1,0,1,0,0,0,49549
//                Packet Type AIVDM
//                CHANNEL B
//                Message Type    9   Standard SAR Aircraft Position Report
//                Repeat Indicator    0   Default
//                Source ID   111232506
//                Altitude    583
//                Speed Over Ground (SOG) 12.2
//                Position Accuracy   0   An unaugmented GNSS fix with accuracy > 10 m
//                Longitude   -2.14308833333333   West
//                Latitude    50.685065   North
//                Course Over Ground (COG)    117.7
//                Time Stamp  1
//                Reserved for regional   0   Not available (default)
//                DTE 1   Not available (default)
//                Assigned Mode Flag  0   Station operating in autonomous and continuos mode (default)
//                RAIM flag   0   RAIM not in use (default)
//                Communication state Selector Flag   0   SOTDMA Communication State follows
//                Communication State 49536   Sync state: UTC Direct; Slot Timeout: 3 frames respectively are left until slot change; Received stations: 384
//                Communication Sync State    0   Sync state: UTC Direct
//                Communication Slot Timeout  3   Slot Timeout: 3 frames respectively are left until slot change
//                Communication Sub Message   384
//                Communication Utc Hour  No value
//                Communication Utc Minute    No value
//                Communication Time Stamp    No value
//                Communication Slot Number   No value
//                Communication Received Stations 384
//                Communication Slot Offset   No value
        }
        ,{"!AIVDM,1,1,,B,91b55vRAivOnAWTM05?CNUP20<6F,0*67"EOM,0,111232506,583,12.6,0,-2.124395,50.6834083333333,89,22,0,1,0,0,0,49558
//            Packet Type AIVDM
//            CHANNEL B
//            Message Type    9   Standard SAR Aircraft Position Report
//            Repeat Indicator    0   Default
//            Source ID   111232506
//            Altitude    583
//            Speed Over Ground (SOG) 12.6
//            Position Accuracy   0   An unaugmented GNSS fix with accuracy > 10 m
//            Longitude   -2.124395   West
//            Latitude    50.6834083333333    North
//            Course Over Ground (COG)    89
//            Time Stamp  22
//            Reserved for regional   0   Not available (default)
//            DTE 1   Not available (default)
//            Assigned Mode Flag  0   Station operating in autonomous and continuos mode (default)
//            RAIM flag   0   RAIM not in use (default)
//            Communication state Selector Flag   0   SOTDMA Communication State follows
//            Communication State 49558   Sync state: UTC Direct; Slot Timeout: 3 frames respectively are left until slot change; Received stations: 406
//            Communication Sync State    0   Sync state: UTC Direct
//            Communication Slot Timeout  3   Slot Timeout: 3 frames respectively are left until slot change
//            Communication Sub Message   406
//            Communication Utc Hour  No value
//            Communication Utc Minute    No value
//            Communication Time Stamp    No value
//            Communication Slot Number   No value
//            Communication Received Stations 406
//            Communication Slot Offset   No value
                }
};

void msgHandler(IEC61162::AISMsg9 msg,void *param);

class AISMsg9_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg9_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg9 msg,void *param);

    AISMsg9_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message9.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg9_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg9_Data);
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

#endif /* _AIS_MESSAGE_9_TEST_H_ */
