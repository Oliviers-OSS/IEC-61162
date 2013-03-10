/*
 * BasicFunctions.h
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */

#ifndef BASICFUNCTIONS_H_
#define BASICFUNCTIONS_H_

#include "config.h"
#include <cxxtest/TestSuite.h>
#include <sigc++/sigc++.h>
#include <cerrno>
#include <cstdlib>
#include <IEC61162/IEC-61162.h>
#include <cmath>

#define EOM "\r\n"
#include <dbgflags-1/loggers.h>
#include "../../../libIEC61162/AISMessages.cpp"

class AISFragmentsMgr_Tests : public CxxTest::TestSuite {

public:

    void test_basic() {
        AISFragmentsMgr fragmentsMgr;
        const char *sentence1[] = {"0123456789012345678901234567890,1*A","ABCDEFGHIJKLMNOPQRSTUVWXYZ,2*B","abcdedfghijklmnopqrstuvwxyz,3*C"};
        const char *fullPayload = fragmentsMgr.addSegment('A',9,3,1,sentence1[0]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',9,3,2,sentence1[1]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',9,3,3,sentence1[2]);
        TS_ASSERT(fullPayload != 0)
        TS_ASSERT(strcmp(fullPayload,"0123456789012345678901234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdedfghijklmnopqrstuvwxyz,3") == 0);
    }

    void test_basic_bad_order() {
        AISFragmentsMgr fragmentsMgr;
        const char *sentence1[] = {"0123456789012345678901234567890,1*A","ABCDEFGHIJKLMNOPQRSTUVWXYZ,2*B","abcdedfghijklmnopqrstuvwxyz,3*C"};
        const char *fullPayload = fragmentsMgr.addSegment('A',1,3,1,sentence1[0]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',1,3,3,sentence1[2]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',1,3,2,sentence1[1]);
        TS_ASSERT(fullPayload != 0)
        TS_ASSERT(strcmp(fullPayload,"0123456789012345678901234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdedfghijklmnopqrstuvwxyz,3") == 0);
    }

    void test_basic_reverse_order() {
        AISFragmentsMgr fragmentsMgr;
        const char *sentence1[] = {"0123456789012345678901234567890,1*A","ABCDEFGHIJKLMNOPQRSTUVWXYZ,2*B","abcdedfghijklmnopqrstuvwxyz,3*C"};
        const char *fullPayload = fragmentsMgr.addSegment('A',6,3,3,sentence1[2]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',6,3,2,sentence1[1]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',6,3,1,sentence1[0]);
        TS_ASSERT(fullPayload != 0)
        TS_ASSERT(strcmp(fullPayload,"0123456789012345678901234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdedfghijklmnopqrstuvwxyz,3") == 0);
    }

    void test_mix() {
        AISFragmentsMgr fragmentsMgr;
        const char *sentence1[] = {"0123456789012345678901234567890,1*A","ABCDEFGHIJKLMNOPQRSTUVWXYZ,2*B","abcdedfghijklmnopqrstuvwxyz,3*C"};
        const char *fullPayload = fragmentsMgr.addSegment('A',9,3,1,sentence1[0]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',1,3,1,sentence1[0]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',9,3,2,sentence1[1]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',1,3,3,sentence1[2]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',9,3,3,sentence1[2]);
        TS_ASSERT(fullPayload != 0)
        TS_ASSERT(strcmp(fullPayload,"0123456789012345678901234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdedfghijklmnopqrstuvwxyz,3") == 0);
        fullPayload = fragmentsMgr.addSegment('A',1,3,2,sentence1[1]);
        TS_ASSERT(fullPayload != 0)
        TS_ASSERT(strcmp(fullPayload,"0123456789012345678901234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdedfghijklmnopqrstuvwxyz,3") == 0);
    }

    void test_end_of_old_then_new() {
        AISFragmentsMgr fragmentsMgr;
        const char *sentence1[] = {"0123456789012345678901234567890,1*A","ABCDEFGHIJKLMNOPQRSTUVWXYZ,2*B","abcdedfghijklmnopqrstuvwxyz,3*C"};
        const char *sentence2[] = {"AzertyuioP,1*a","QsdfghjklM,2*b","WxcvbN,3*c","0147852369,4*d"};
        const char *fullPayload = fragmentsMgr.addSegment('A',9,3,2,sentence1[0]);
        TS_ASSERT(0 == fullPayload);
        fullPayload = fragmentsMgr.addSegment('A',9,3,3,sentence1[2]);
        TS_ASSERT(0 == fullPayload)
        fullPayload = fragmentsMgr.addSegment('A',9,4,1,sentence2[0]);
        TS_ASSERT(0 == fullPayload)
        fullPayload = fragmentsMgr.addSegment('A',9,4,3,sentence2[2]);
        TS_ASSERT(0 == fullPayload)
        fullPayload = fragmentsMgr.addSegment('A',9,4,2,sentence2[1]);
        TS_ASSERT(0 == fullPayload)
        fullPayload = fragmentsMgr.addSegment('A',9,4,4,sentence2[3]);
        TS_ASSERT(fullPayload != 0)
        TS_ASSERT(strcmp(fullPayload,"AzertyuioP" "QsdfghjklM" "WxcvbN" "0147852369,4") == 0);
    }

    // check bug correction: if a message was using the same id than a previous one
    // BEFORE the timeout then they were mixed and the number of segments incremented
    // so it will never be completed.
    void test_two_complete_msg_same_slot() {
        AISFragmentsMgr fragmentsMgr;
        const char *sentence[2] = {"0123456789,1*A","ABCDEFGHIJKLMNOPQRSTUVWXYZ,2*B"};
        const char *fullPayload = fragmentsMgr.addSegment('A',1,2,1,sentence[0]);
        TS_ASSERT_EQUALS((char*)0,fullPayload); //wait for the next segment
        fullPayload = fragmentsMgr.addSegment('A',1,2,2,sentence[1]);
        TS_ASSERT_EQUALS(fullPayload,"0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ,2");
        fullPayload = fragmentsMgr.addSegment('A',1,2,1,sentence[0]);
        TS_ASSERT_EQUALS((char*)0,fullPayload); // must wait for the next segment
        fullPayload = fragmentsMgr.addSegment('A',1,2,2,sentence[1]);
        TS_ASSERT_EQUALS(fullPayload,"0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ,2"); // completed again
    }

    // check bug correction: two messages in 2 parts in the same slot and we only receive the same part
    // of each of them, ex: only the first part of both...
    void test_two_complete_msg_same_slot_same_part() {
        AISFragmentsMgr fragmentsMgr;
        const char *sentence[2] = {"0123456789,1*A","ABCDEFGHIJKLMNOPQRSTUVWXYZ,2*B"};
        const char *fullPayload = fragmentsMgr.addSegment('A',1,2,1,sentence[0]);
        TS_ASSERT_EQUALS((char*)0,fullPayload); //wait for the next segment
        fullPayload = fragmentsMgr.addSegment('A',1,2,1,sentence[1]);
        TS_ASSERT_EQUALS((char*)0,fullPayload); // MUST still waiting for the next segment
    }
};

#endif /* BASICFUNCTIONS_H_ */
