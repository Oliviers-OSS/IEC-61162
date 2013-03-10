/*
 * AISMessage5.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_5_TEST_H_
#define _AIS_MESSAGE_5_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

const struct AISMsg5_Data {
    const char *rawMsg;
    unsigned char repeatIndicator;
    IEC61162::MMSI userID;
    unsigned char AISVersionIndicator;
    unsigned long IMONumber;
    char callSign[8];
    char name[21];
    unsigned char typeOfShipAndCargoType;
    IEC61162::ReferencePoint overallDimensionReferenceForPosition;
    unsigned char typeOfPositionDevice;
    IEC61162::EstimatedTimeOfArrival ETA;
    float maximumPresentStaticDraught;
    char destination[21];
    bool DTE;
} data[] = {
        // http://www.maritec.co.za/aisvdmvdodecoding.php
         {"!AIVDM,2,1,0,A,58wt8Ui`g??r21`7S=:22058<v05Htp000000015>8OA;0sk,0*7B"EOM,0,603916439,0,439303422,"ZA83R","ARCO AVON",69,{113,31,17,11},0,{3,23,19,45},13.2,"HOUSTON",0}
        ,{"!AIVDM,2,2,0,A,eQ8823mDm3kP00000000000,2*5D"EOM,0,603916439,0,439303422,"ZA83R","ARCO AVON",69,{113,31,17,11},0,{3,23,19,45},13.2,"HOUSTON",0}

         ,{"!AIVDM,2,1,0,B,53HO91P00000IKKSG:0DpEb1=Dr3>2222222220t1@I535fbS52C1H11,0*1B"EOM,0,227002630,0,0,"FV6852 ","ENEZ SUN 3          ",60,{10,25,5,3},1,{6,29,10,35},2,"ILE DE SEIN         ",0}
         ,{"!AIVDM,2,2,0,B,H4iBC`888888880,2*53"EOM,0,227002630,0,0,"FV6852 ","ENEZ SUN 3          ",60,{10,25,5,3},1,{6,29,10,35},2,"ILE DE SEIN         ",0}

        ,{"!AIVDM,2,1,7,B,53IfnN01rO`DHhEJ2210E@E:0H48E:222222220Q000005Im0<PTQDm8,0*57"EOM,0,228308600,0,8027781,"FLEV   ","PETER FABER         ",33,{0,0,0,0},1,{5,19,21,0},5,"BREST PILOT         ",0}
        ,{"!AIVDM,2,2,7,B,42C3m8888888880,2*03"EOM,0,228308600,0,8027781,"FLEV   ","PETER FABER         ",33,{0,0,0,0},1,{5,19,21,0},5,"BREST PILOT         ",0}

         //!AIVDM,2,1,4,B,53mqCP01iI2`h4Ho801T4l4l4P0000000000001I@HWD55hn0HTSm51D,0*2D
         //!AIVDM,2,2,4,B,Q0C@00000000000,2*41

};

void msgHandler(IEC61162::AISMsg5 msg,void *param);

class AISMsg5_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg5_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg5 msg,void *param);

    AISMsg5_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message5.connect(sigc::ptr_fun(msgHandler));
    }

    ~AISMsg5_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg5_Data);
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

#endif /* _AIS_MESSAGE_5_TEST_H_ */
