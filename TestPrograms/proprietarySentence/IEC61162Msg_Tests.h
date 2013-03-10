/*
 * IEC61162Msg_Tests.h
 *
 *  Created on: 11 avr. 2013
 *      Author: oc
 */

#ifndef IEC61162MSG_TESTS_H_
#define IEC61162MSG_TESTS_H_

#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>

#define EOM "\r\n"

void proprietaryMsgHandler(IEC61162::raw_sentence msg,void *param);

class AISMsg1_Tests : public CxxTest::TestSuite /*, sigc::trackable*/
{
    IEC61162::Message msg;
    const char *currentMsg;
    int lastHandlerTestResult;
public:

    friend void proprietaryMsgHandler(IEC61162::raw_sentence msg,void *param);

    AISMsg1_Tests()
        :msg(this),currentMsg(0),lastHandlerTestResult(EXIT_SUCCESS) {
        msg.proprietary_sentence.connect(sigc::ptr_fun(proprietaryMsgHandler));
    }

    ~AISMsg1_Tests() {
    }

    void test_emptyMsg(void) {
        currentMsg = "";
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,ENODATA);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_tooBigMsg(void) {
        currentMsg = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EMSGSIZE);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_badMessage(void) {
        currentMsg = "ABCDEFGHIJKL";
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EPROTO);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_noChecksumMsg(void) {
        currentMsg = "ABCDEFGHIJKL"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EBADMSG);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_badCheckSum(void) {
        currentMsg = "!AIVDM,1,1,,B,33P8RP@P0gOrtRVM4UGH;?v600iQ,0*10"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EBADMSG);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_unknownStartSequence(void) {
        currentMsg = "#AIVDM,1,1,,B,33P8RP@P0gOrtRVM4UGH;?v600iQ,0*1A"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EPROTO);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_unhandledAISMsg(void) {
        currentMsg = "!AIVDM,1,1,,B,13Hj5J7000Od<fdKQJ3Iw`S>28FK,0*3F"EOM;
        int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
        currentMsg = "$AIALR,000002.00,025,A,V,AIS: External EPFS lost*0F"EOM;
        error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_unknownTalker(void) {
        currentMsg = "!FOODM,1,1,,B,13Hj5J7000Od<fdKQJ3Iw`S>28FK,0*27"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EINVAL);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }

    void test_propryetaryMsg(void) {
        currentMsg = "$PGRMM,WGS 84*06"EOM;
        const int error = msg.parse(currentMsg);
        TS_ASSERT_EQUALS(error,EXIT_SUCCESS);
        TS_ASSERT_EQUALS(lastHandlerTestResult,EXIT_SUCCESS);
    }
};

#endif /* IEC61162MSG_TESTS_H_ */
