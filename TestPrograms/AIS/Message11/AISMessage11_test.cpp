/*
 * AISMessage11_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage11_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg11 msg,void *param) {
    AISMsg11_Tests *caller = static_cast<AISMsg11_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg11_Data *test_data = caller->currentMsg;

        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_UTC_year(),test_data->UTC_year);
        TS_ASSERT_EQUALS(msg->get_UTC_month(),test_data->UTC_month);
        TS_ASSERT_EQUALS(msg->get_UTC_day(),test_data->UTC_day);
        TS_ASSERT_EQUALS(msg->get_UTC_hour(),test_data->UTC_hour);
        TS_ASSERT_EQUALS(msg->get_UTC_minute(),test_data->UTC_minute);
        TS_ASSERT_EQUALS(msg->get_UTC_second(),test_data->UTC_second);
        TS_ASSERT_EQUALS(msg->get_positionAccuracy(),test_data->positionAccuracy);
        TS_ASSERT_EQUALS(msg->get_longitude(),test_data->longitude);
        TS_ASSERT_EQUALS(msg->get_latitude(),test_data->latitude);
        TS_ASSERT_EQUALS(msg->get_typeOfPositionDevice(),test_data->typeOfPositionDevice);
        TS_ASSERT_EQUALS(msg->get_transMissionControlForLongRangeBroadcastMessage(),test_data->transMissionControlForLongRangeBroadcastMessage);
        TS_ASSERT_EQUALS(msg->get_RAIMFlag(),test_data->RAIMFlag);
        TS_ASSERT_EQUALS(msg->get_communicationState(),test_data->communicationState);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


