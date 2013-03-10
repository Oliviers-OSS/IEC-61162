/*
 * AISMessage17.h
 *
 *  Created on: 09 jun. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_17_TEST_H_
#define _AIS_MESSAGE_17_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"



const struct AISMsg17_Data {
    const char *rawMsg;
    IEC61162::Byte repeatIndicator; //valid values are 0..3, number of time this msg has been repeated, 3 means do not repeat any more, default 0.
    IEC61162::MMSI sourceID;
    float longitude;
    float latitude;
    IEC61162::Byte *data;
    IEC61162::Word sizeInBits;
} data[] = {
        // http://www.aggsoft.com/ais-decoder.htm
        // http://www.maritec.co.za/aisvdmvdodecoding.php
         {"!AIVDM,1,1,,A,A04757QAv0agH2JdGlLP7Oqa0@TGw9H170,4*5A"EOM,0,4310302,0.13989333333333,0.03561833333333,0,0
//                 Packet Type AIVDM
//                 CHANNEL A
//                 Message Type    17  GNSS Broadcast Binary Message
//                 Repeat Indicator    0   Default
//                 Source ID   4310302
//                 Longitude   0.13989333333333    East
//                 Latitude    0.03561833333333    North
//                 Broadcast Message Type  9
//                 Station ID  684
//                 Z Count 3048
//                 Sequence Number 7
//                 N   4
//                 Health  0
//                 DGNSS Data  1D FE 69 01 09 17 FC 96 01 1C 00
         }
         ,{"!AIVDM,1,1,,A,Atonutv>pfTMFHS44Ae74Dt,2*1C"EOM,3,863878643,-0.19304666666667,-0.078115,0,0
//                 Packet Type    AIVDM
//                 CHANNEL A
//                 Message Type    17  GNSS Broadcast Binary Message
//                 Repeat Indicator    3   Do not repeat any more
//                 Source ID   863878643
//                 Longitude   -0.19304666666667   West
//                 Latitude    -0.078115   West
//                 Broadcast Message Type  34
//                 Station ID  196
//                 Z Count 547
//                 Sequence Number 3
//                 N   8
//                 Health  7
//                 DGNSS Data  11 4F 00
         }
                 ,{"!AIVDM,1,1,,A,AleoS?PkjWPTJL7?RS;BWh@g<Nb=F4MNjefAP75G,0*60"EOM
//                         Packet Type    AIVDM
//                         CHANNEL A
//                         Message Type    17  GNSS Broadcast Binary Message
//                         Repeat Indicator    3   Do not repeat any more
//                         Source ID   316531518
//                         Longitude   0.08838833333333    East
//                         Latitude    -0.02682    West
//                         Broadcast Message Type  48
//                         Station ID  463
//                         Z Count 4422
//                         Sequence Number 2
//                         N   26
//                         Health  2
//                         DGNSS Data  9F 04 2F 31 EA 80
                 }
                         ,{"!AIVDM,1,1,,B,AL5wswHrgN?loWtruTKN7@BQ50,4*51"EOM
//                             Packet Type AIVDM
//                             CHANNEL B
//                             Message Type    17  GNSS Broadcast Binary Message
//                             Repeat Indicator    1
//                             Source ID   811596797
//                             Longitude   0.10025166666667    East
//                             Latitude    -0.09572166666667   West
//                             Broadcast Message Type  31
//                             Station ID  826
//                             Z Count 7880
//                             Sequence Number 6
//                             N   27
//                             Health  6
//                             DGNSS Data  1D 04 A1 14 00
                         }


};

void msgHandler(IEC61162::AISMsg17 msg,void *param);

class AISMsg17_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg17_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg17 msg,void *param);

    AISMsg17_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message17.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg17_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg17_Data);
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

#endif /* _AIS_MESSAGE_17_TEST_H_ */
