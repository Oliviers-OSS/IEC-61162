/*
 * AISMessage22.h
 *
 *  Created on: 09 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_22_TEST_H_
#define _AIS_MESSAGE_22_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg22_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI stationID;
    IEC61162::Word channelA;
    IEC61162::Word channelB;
    IEC61162::Byte txRxMode;
    bool power;
    float longitude1;
    float latitude1;
    IEC61162::MMSI addressedStationID1;
    float longitude2;
    float latitude2;
    IEC61162::MMSI addressedStationID2;
    bool addressedOrBroadcastMsgIndicator;
    bool channelABandwith;
    bool channelBBandwith;
    IEC61162::Byte transitionalZoneSize;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
         {"!AIVDM,1,1,,B,F030p?j2N2P73FiiNesU3FR10000,0*32"EOM,0,3160127,2087,2088,0,0,-0.05174833333333,0.04841666666667,0,-0.05649833333333,0.04576666666667,0,0,0,0,2
//                 Packet Type    AIVDM
//                 CHANNEL B
//                 Message Type    22  Channel management
//                 Repeat Indicator    0   Default
//                 Station ID  3160127
//                 Channel A   2087
//                 Channel B   2088
//                 TX/RX Mode  0   TxA/TxB, RxA/RxB (default)
//                 Power   0   High (default)
//                 Longitude #1    -0.05174833333333   West
//                 Latitude #1 0.04841666666667    North
//                 Longitude #2    -0.05649833333333   West
//                 Latitude #2 0.04576666666667    North
//                 Addressed or Broadcast Message Indicator    0   Broadcast Geographical Area Message (default)
//                 Channel A Band Width    0   As specified by channel number (default)
//                 Channel B Band Width    0   As specified by channel number (default)
//                 Transitional Zone Size  2
         }
         ,{"!AIVDM,1,1,,B,F02:oq22N2P3D73EB6`>6bT20000,0*47"EOM,0,2275300,2087,2088,0,0,0.181,0.091,0,0.181,0.091,0,0,0,0,4
//             Packet Type AIVDM
//             CHANNEL B
//             Message Type    22  Channel management
//             Repeat Indicator    0   Default
//             Station ID  2275300
//             Channel A   2087
//             Channel B   2088
//             TX/RX Mode  0   TxA/TxB, RxA/RxB (default)
//             Power   0   High (default)
//             Longitude #1    0.181   East
//             Latitude #1 0.091   North
//             Longitude #2    0.181   East
//             Latitude #2 0.091   North
//             Addressed or Broadcast Message Indicator    0   Broadcast Geographical Area Message (default)
//             Channel A Band Width    0   As specified by channel number (default)
//             Channel B Band Width    0   As specified by channel number (default)
//             Transitional Zone Size  4
         }
         // !AIVDM,1,1,,B,F030p2j2N2P6Ubib@=4q35b1P000,0*61

};

void msgHandler(IEC61162::AISMsg22 msg,void *param);

class AISMsg22_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg22_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg22 msg,void *param);

    AISMsg22_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message22.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg22_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg22_Data);
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

#endif /* _AIS_MESSAGE_22_TEST_H_ */
