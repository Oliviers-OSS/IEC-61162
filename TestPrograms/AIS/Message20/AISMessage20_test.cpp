/*
 * AISMessage20_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage20_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg20 msg,void *param) {
    AISMsg20_Tests *caller = static_cast<AISMsg20_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg20_Data *test_data = caller->currentMsg;
        const size_t n = strlen(test_data->rawMsg);
        const size_t payloadSize = n - 12;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_sourceID(),test_data->sourceID);

        if (msg->has_offsetNumber1()) {
            TS_ASSERT_EQUALS(msg->get_offsetNumber1(),test_data->offsetNumber1);
        }
        if (msg->has_numberOfSlot1()) {
            TS_ASSERT_EQUALS(msg->get_numberOfSlot1(),test_data->numberOfSlot1);
        }
        if (msg->has_timeOut1()) {
            TS_ASSERT_EQUALS(msg->get_timeOut1(),test_data->timeOut1);
        }
        if (msg->has_increment1()) {
            TS_ASSERT_EQUALS(msg->get_increment1(),test_data->increment1);
        }

        if (msg->has_offsetNumber2()) {
            TS_ASSERT_EQUALS(msg->get_offsetNumber2(),test_data->offsetNumber2);
        }
        if (msg->has_numberOfSlot2()) {
            TS_ASSERT_EQUALS(msg->get_numberOfSlot2(),test_data->numberOfSlot2);
        }
        if (msg->has_timeOut2()) {
            TS_ASSERT_EQUALS(msg->get_timeOut2(),test_data->timeOut2);
        }
        if (msg->has_increment2()) {
            TS_ASSERT_EQUALS(msg->get_increment2(),test_data->increment2);
        }

        if (msg->has_offsetNumber3()) {
            TS_ASSERT_EQUALS(msg->get_offsetNumber3(),test_data->offsetNumber3);
        }
        if (msg->has_numberOfSlot3()) {
            TS_ASSERT_EQUALS(msg->get_numberOfSlot3(),test_data->numberOfSlot3);
        }
        if (msg->has_timeOut3()) {
            TS_ASSERT_EQUALS(msg->get_timeOut3(),test_data->timeOut3);
        }
        if (msg->has_increment3()) {
            TS_ASSERT_EQUALS(msg->get_increment3(),test_data->increment3);
        }

        if (msg->has_offsetNumber4()) {
            TS_ASSERT_EQUALS(msg->get_offsetNumber4(),test_data->offsetNumber4);
        }
        if (msg->has_numberOfSlot4()) {
            TS_ASSERT_EQUALS(msg->get_numberOfSlot4(),test_data->numberOfSlot4);
        }
        if (msg->has_timeOut4()) {
            TS_ASSERT_EQUALS(msg->get_timeOut4(),test_data->timeOut4);
        }
        if (msg->has_increment4()) {
            TS_ASSERT_EQUALS(msg->get_increment4(),test_data->increment4);
        }
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


