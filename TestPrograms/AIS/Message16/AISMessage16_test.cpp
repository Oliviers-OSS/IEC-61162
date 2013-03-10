/*
 * AISMessage16_test.cpp
 *
 *  Created on: 9 juin. 2013
 *      Author: oc
 */



#include "AISMessage16_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg16 msg,void *param) {
    AISMsg16_Tests *caller = static_cast<AISMsg16_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg16_Data *test_data = caller->currentMsg;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_sourceID(),test_data->sourceID);
        TS_ASSERT_EQUALS(msg->get_destinationIdA(),test_data->destinationIdA);
        TS_ASSERT_EQUALS(msg->get_offsetA(),test_data->offsetA);
        TS_ASSERT_EQUALS(msg->get_incrementA(),test_data->incrementA);
        TS_ASSERT_EQUALS(msg->get_destinationIdB(),test_data->destinationIdB);
        TS_ASSERT_EQUALS(msg->get_offsetB(),test_data->offsetB);
        TS_ASSERT_EQUALS(msg->get_incrementB(),test_data->incrementB);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


