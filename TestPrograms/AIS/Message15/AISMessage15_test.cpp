/*
 * AISMessage15_test.cpp
 *
 *  Created on: 11 jun. 2013
 *      Author: oc
 */



#include "AISMessage15_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg15 msg,void *param) {
    AISMsg15_Tests *caller = static_cast<AISMsg15_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg15_Data *test_data = caller->currentMsg;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_sourceID(),test_data->sourceID);
        if ((msg->has_destinationID1()) || (test_data->destinationID1 != 0)) {
            TS_ASSERT_EQUALS(msg->get_DestinationID1(),test_data->destinationID1);
        } else {
            //TS_FAIL("destinationID1");
        }

        if ((msg->has_messageID1_1()) || (test_data->messageID1_1 != 0)) {
            TS_ASSERT_EQUALS(msg->get_messageID1_1(),test_data->messageID1_1);
        }

        if ((msg->has_slotOffset1_1()) || (test_data->slotOffset1_1 != 0)) {
            TS_ASSERT_EQUALS(msg->get_slotOffset1_1(),test_data->slotOffset1_1);
        }

        if ((msg->has_messageID1_2()) || (test_data->messageID1_2 != 0)) {
            TS_ASSERT_EQUALS(msg->get_messageID1_2(),test_data->messageID1_2);
        }

        if ((msg->has_slotOffset1_2()) || (test_data->slotOffset1_2 != 0)) {
            TS_ASSERT_EQUALS(msg->get_slotOffset1_2(),test_data->slotOffset1_2);
        }

        if ((msg->has_destinationID2()) || (test_data->destinationID2 != 0)) {
            TS_ASSERT_EQUALS(msg->get_destinationID2(),test_data->destinationID2);
        }

        if ((msg->has_messageID2_1()) || (test_data->messageID2_1 != 0)) {
            TS_ASSERT_EQUALS(msg->get_messageID2_1(),test_data->messageID2_1);
        }

        if ((msg->has_slotOffset2_1()) || (test_data->slotOffset2_1 != 0)) {
            TS_ASSERT_EQUALS(msg->get_slotOffset2_1(),test_data->slotOffset2_1);
        }

    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


