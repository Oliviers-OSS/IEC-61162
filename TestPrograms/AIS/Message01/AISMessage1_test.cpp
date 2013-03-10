/*
 * AISMessage1_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage1_test.h"
#include <cerrno>
#include <cstdio>

void msgHandler(IEC61162::AISMsg1 msg,void *param) {
    AISMsg1_Tests *caller = static_cast<AISMsg1_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg1_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);
        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_navigationStatus(),test_data->navigationStatus);
        if ((msg->has_rateOfTurn()) && (isnan(test_data->rateOfTurn))){
            TS_ASSERT_DELTA(msg->get_rateOfTurn(),test_data->rateOfTurn,0.1);
        }
        TS_ASSERT_EQUALS(msg->get_speedOverGround(),test_data->speedOverGround);
        TS_ASSERT_EQUALS(msg->get_positionAccuracy(),test_data->positionAccuracy);
        TS_ASSERT_EQUALS(msg->get_longitude(),test_data->longitude);
        TS_ASSERT_EQUALS(msg->get_latitude(),test_data->latitude);
        TS_ASSERT_EQUALS(msg->get_courseOverGround(),test_data->courseOverGround);
        if ((msg->has_trueHeading()) && (isnan(test_data->trueHeading))){
        TS_ASSERT_EQUALS(msg->get_trueHeading(),test_data->trueHeading);
        }
        TS_ASSERT_EQUALS(msg->get_timeStamp(),test_data->timeStamp);
        TS_ASSERT_EQUALS(msg->get_specialManoeuvreIndicator(),test_data->specialManoeuvreIndicator);
        TS_ASSERT_EQUALS(msg->get_RAIMFlag(),test_data->RAIMFlag);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


