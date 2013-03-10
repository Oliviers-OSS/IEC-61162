/*
 * AISMessage1_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage4_test.h"
#include <cerrno>
#include <cstdio>

void msgHandler(IEC61162::AISMsg4 msg,void *param) {
    AISMsg4_Tests *caller = static_cast<AISMsg4_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg4_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);
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
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


