/*
 * AISMessage7_test.cpp
 *
 *  Created on: 11 jun. 2013
 *      Author: oc
 */



#include "AISMessage7_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg7 msg,void *param) {
    AISMsg7_Tests *caller = static_cast<AISMsg7_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg7_Data *test_data = caller->currentMsg;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_sourceID(),test_data->sourceID);
        if ((msg->has_destinationID1()) && (test_data->destinationID1 != 0)) {
            TS_ASSERT_EQUALS(msg->get_destinationID1(),test_data->destinationID1);
            TS_ASSERT_EQUALS(msg->get_sequenceNumberForID1(),test_data->sequenceNumberForID1);
        } else {
            //TS_FAIL("destinationID1");
        }

        if ((msg->has_destinationID2()) && (test_data->destinationID2 != 0)) {
            TS_ASSERT_EQUALS(msg->get_destinationID2(),test_data->destinationID2);
            TS_ASSERT_EQUALS(msg->get_sequenceNumberForID2(),test_data->sequenceNumberForID2);
        } else {
            //TS_FAIL("destinationID2");
        }

        if ((msg->has_destinationID3()) && (test_data->destinationID3 != 0)) {
            TS_ASSERT_EQUALS(msg->get_destinationID3(),test_data->destinationID3);
            TS_ASSERT_EQUALS(msg->get_sequenceNumberForID3(),test_data->sequenceNumberForID3);
        } else {
            //TS_FAIL("destinationID3");
        }

        if ((msg->has_destinationID4()) && (test_data->destinationID4 != 0)) {
            TS_ASSERT_EQUALS(msg->get_destinationID4(),test_data->destinationID4);
            TS_ASSERT_EQUALS(msg->get_sequenceNumberForID4(),test_data->sequenceNumberForID4);
        } else {
            if ((!msg->has_destinationID4()) && (test_data->destinationID4 == 0)) {

            } else {
                //TS_FAIL("destinationID3");
            }
        }
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


