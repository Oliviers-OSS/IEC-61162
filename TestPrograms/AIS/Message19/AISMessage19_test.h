/*
 * AISMessage19.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_19_TEST_H_
#define _AIS_MESSAGE_19_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg19_Data {
    const char *rawMsg;
    unsigned char repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI userID;
    float speedOverGround;
    bool positionAccuracy;
    float longitude;
    float latitude;
    float courseOverGround; //COG
    IEC61162::Word trueHeading;
    IEC61162::Byte timeStamp;
    char name[21];
    IEC61162::Byte typeOfShipAndCargoType;
    IEC61162::ReferencePoint dimensionOfShipReferenceForPosition;
    IEC61162::Byte typeOfPositionDevice;
    bool RAIMFlag;
    bool DTE;
    bool  assignedModeFlag;
} data[] = {
        // http://www.maritec.co.za/aisvdmvdodecoding.php
        // !AIVDM,1,1,0,B,C8u:8C@t7@TnGCKfm6Po`e6N`:Va0L2J;06HV50JV?SjBPL311RP,0*17
        {"!AIVDM,1,1,0,B,C8u:8C@t7@TnGCKfm6Po`e6N`:Va0L2J;06HV50JV?SjBPL311RP,0*29"EOM
                ,0 //repeatIndicator
                ,601000013 //userID
                /*,15*/
                ,2.9 //speedOverGround
                ,0 //positionAccuracy
                ,32.19953 //longitude
                ,-29.83748 //latitude
                ,89 //courseOverGround
                ,90 //trueHeading
                ,12 //timeStamp
                /*,15*/
                ,"TEST NAME CLSB MSG19" //name
                ,37 //typeOfShipAndCargoType
                ,{7,6,2,3} //dimensionOfShipReferenceForPosition
                ,1
                ,0
                ,1
                ,0
                /*,0*/}
        //!AIVDM,1,1,,B,C0004Vt0`2T93s5t>7iOswqPbLBa1Sgk111111111110N0p11QPP,0*79
        ,{"!AIVDM,1,1,,B,C0004W<0`2O<6GURuPJfGwb0bLBa1SiQ111111111110Q0P2110P,0*30"EOM
                ,0,1180,16,0,139.101518333333,38.77121,278.9,511,20,"UNIT 180",66,{8,4,2,2},0,0,1,0
//                Packet Type AIVDM
//                CHANNEL B
//                Message Type    19  Extended Class B Equipment Position Report
//                Repeat Indicator    0   Default
//                User ID 1180
//                Reserved for regional   192
//                Speed Over Ground (SOG) 16
//                Position Accuracy   0   An unaugmented GNSS fix with accuracy > 10 m
//                Longitude   139.101518333333    East
//                Latitude    38.77121    North
//                Course Over Ground (COG)    278.9
//                True Heading (HDG)  511 Not available (default)
//                Time Stamp  20
//                Reserved for regional   0   Not available (default)
//                Name    UNIT 180
//                Ship Type   66  Passenger, Reserved for future use
//                Dimension: reference for pos. A 8
//                Dimension: reference for pos. B 4
//                Dimension: reference for pos. C 2
//                Dimension: reference for pos. D 2
//                Vessel length   12
//                Vessel beam 4
//                Electronic Position Fixing Device Type  0   Undefined (default)
//                RAIM flag   0   RAIM not in use (default)
//                DTE 1   Not available (default)
//                Assigned Mode Flag  0   Station operating in autonomous and continuos mode (default)
        }

};

void msgHandler(IEC61162::AISMsg19 msg,void *param);

class AISMsg19_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg19_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg19 msg,void *param);

    AISMsg19_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message19.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg19_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg19_Data);
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

#endif /* _AIS_MESSAGE_19_TEST_H_ */
