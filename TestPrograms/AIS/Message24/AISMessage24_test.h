/*
 * AISMessage24.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef _AIS_MESSAGE_24_TEST_H_
#define _AIS_MESSAGE_24_TEST_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

struct AISMsg24B_Data {
    unsigned char typeOfShipAndCargoType;
    IEC61162::VendorID vendorID;
    char callSign[8];
    IEC61162::ReferencePoint dimension;
    IEC61162::MMSI motherShip;
};

const struct AISMsg24_Data {
    // Msg 24 common part
    const char *rawMsg;
    unsigned char repeatIndicator;
    IEC61162::MMSI userID;
    unsigned char partNumber;
    // Msg 24 A
    char name[21];
    // Msg 24 B
    unsigned char typeOfShipAndCargoType;
    IEC61162::VendorID vendorID;
    char callSign[8];
    IEC61162::ReferencePoint dimension;
    IEC61162::MMSI motherShip;

    // 24A
    AISMsg24_Data(const char *m,const unsigned char ri,const unsigned long id,const unsigned char pn,const char *n)
            :rawMsg(m),repeatIndicator(ri),userID(id),partNumber(pn)
            ,typeOfShipAndCargoType(0)
            ,motherShip(0) {
        strcpy(name,n);
        strcpy(callSign,"@@@@@@@");
    }
    // 24B
    AISMsg24_Data(const char *m,const unsigned char ri,const unsigned long id,const unsigned char pn
            ,const unsigned char t, const char *vid,const char *cs
            ,unsigned long A,unsigned long B,unsigned char C,unsigned char D)
        :rawMsg(m),repeatIndicator(ri),userID(id),partNumber(pn)
        ,typeOfShipAndCargoType(t),dimension(A,B,C,D),motherShip(0) {
        name[0]='\0';
        strcpy(callSign,cs);
    }

    AISMsg24_Data(const char *m,const unsigned char ri,const unsigned long id,const unsigned char pn
            ,const unsigned char t, const char *vid,const char *cs
            ,const unsigned long ms)
        :rawMsg(m),repeatIndicator(ri),userID(id),partNumber(pn)
        ,typeOfShipAndCargoType(t),dimension(0,0,0,0),motherShip(ms) {
        name[0]='\0';
        strcpy(callSign,cs);
    }

    ~AISMsg24_Data() {
    }

} data[] = {
        // http://www.maritec.co.za/aisvdmvdodecoding.php
        //{"!AIVDM,1,1,,B,H39SF`Pd488Dh45800000000000,2*35"EOM,0,211343010,0,"KABBELAAR",0,"","",0,0,0,0,0} // 24A
        //,{"!AIVDM,1,1,,B,H39SF`TUDBE584747lioq01P1220,0*16"EOM,0,211343010,1,"",37,"TRUEHDG","DG4179",12,1,2,2,13,4} // 24B
        //,{"!AIVDM,1,1,,B,H3P:m5PmE<e85@0000000000000,2*32"EOM,0,235058454,0,"MUSKRAT",0,"","",0,0,0,0,0} // 24A
        //,{"!AIVDM,1,1,,B,H3fJqRTT4I138D05923o001@0220,0*71"EOM,0,250001802,1,"",36,"DYACHT","EIBC7",10,0,2,2,10,4} // 24B
        //,{"!AIVDM,1,1,,B,H3P;Qgh`5UL4hdE800000000000,2*50"EOM,0,235069887,0,"JAYWALKER",0,"","",0,0,0,0,0} // 24A
        {"!AIVDM,1,1,,B,H39SF`Pd488Dh45800000000000,2*35"EOM,0,211343010,0,"KABBELAAR"} // 24A
        ,{"!AIVDM,1,1,,B,H39SF`TUDBE584747lioq01P1220,0*16"EOM,0,211343010,1,37,"TRUEHDG","DG4179",12,1,2,2/*,13,4*/} // 24B
        ,{"!AIVDM,1,1,,B,H3P:m5PmE<e85@0000000000000,2*32"EOM,0,235058454,0,"MUSKRAT"} // 24A
        ,{"!AIVDM,1,1,,B,H3fJqRTT4I138D05923o001@0220,0*71"EOM,0,250001802,1,36,"DYACHT","EIBC7",10,0,2,2,/*10,4*/} // 24B
        ,{"!AIVDM,1,1,,B,H3P;Qgh`5UL4hdE800000000000,2*50"EOM,0,235069887,0,"JAYWALKER"} // 24A
        ,{"!AIVDO,1,1,,B,H1c2;qA@PU>0U>060<h5=>0:1Dp,2*7D"EOM,0,112233445,0,"THIS IS A CLASS B UN"} // 24A
        ,{"!AIVDO,1,1,,B,H1c2;qDTijklmno31<<C970`43<1,0*28"EOM,0,112233445,1,36,"1234567","CALLSIG",5,4,3,12} // 24B
};

void msgHandler(IEC61162::AISMsg24 msg,void *param);
void msgHandlerA(IEC61162::AISMsg24A msg,void *param);
void msgHandlerB(IEC61162::AISMsg24B msg,void *param);

class AISMsg24_Tests : public CxxTest::TestSuite
{
    IEC61162::AISMessage msg;
    const AISMsg24_Data *currentMsg;
    int lastHandlerTestResult;
public:

    friend void msgHandler(IEC61162::AISMsg24 msg,void *param);
    friend void msgHandlerA(IEC61162::AISMsg24A msg,void *param);
    friend void msgHandlerB(IEC61162::AISMsg24B msg,void *param);

    AISMsg24_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.message24.connect(sigc::ptr_fun(msgHandler));
        msg.message24A.connect(sigc::ptr_fun(msgHandlerA));
        msg.message24B.connect(sigc::ptr_fun(msgHandlerB));
    }

    ~AISMsg24_Tests() {
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
        register unsigned int nbMsg = sizeof(data) / sizeof(AISMsg24_Data);
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

#endif /* _AIS_MESSAGE_24_TEST_H_ */
