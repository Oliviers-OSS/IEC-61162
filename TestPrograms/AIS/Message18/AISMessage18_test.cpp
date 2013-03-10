/*
 * AISMessage18_test.cpp
 *
 *  Created on: 12 avr. 2013
 *      Author: oc
 */



#include "AISMessage18_test.h"
#include <cerrno>
#include <cstdio>
#include <cstring>

bool operator ==(const IEC61162::ReferencePoint &r1,const IEC61162::ReferencePoint &r2) {
    return ((r1.A == r2.A) && (r1.B == r2.B) && (r1.C == r2.C) && (r1.D == r2.D));
}

bool operator ==(const IEC61162::EstimatedTimeOfArrival &e1,const IEC61162::EstimatedTimeOfArrival &e2) {
    return ((e1.month == e2.month) && (e1.day == e2.day)&& (e1.hour == e2.hour) && (e1.minute == e2.minute));
}

void msgHandler(IEC61162::AISMsg18 msg,void *param) {
    AISMsg18_Tests *caller = static_cast<AISMsg18_Tests *>(param);
    if (caller) {
        // check the decoded message's content against the references data
        const struct AISMsg18_Data *test_data = caller->currentMsg;
        char buffer[255];
        sprintf(buffer,"checking message %s...\n",test_data->rawMsg);
        TS_TRACE(buffer);
        TS_ASSERT_EQUALS(msg->get_repeatIndicator(),test_data->repeatIndicator);
        TS_ASSERT_EQUALS(msg->get_userID(),test_data->userID);
        TS_ASSERT_EQUALS(msg->get_speedOverGround(),test_data->speedOverGround);
        TS_ASSERT_EQUALS(msg->get_positionAccuracy(),test_data->positionAccuracy);
        TS_ASSERT_EQUALS(msg->get_longitude(),test_data->longitude);
        TS_ASSERT_EQUALS(msg->get_latitude(),test_data->latitude);
        TS_ASSERT_EQUALS(msg->get_courseOverGround(),test_data->courseOverGround);
        TS_ASSERT_EQUALS(msg->get_trueHeading(),test_data->trueHeading);
        TS_ASSERT_EQUALS(msg->get_timeStamp(),test_data->timeStamp);
        //TS_ASSERT_EQUALS(msg->get_classBUnitFlag(),test_data->classBUnitFlag);
        //TS_ASSERT_EQUALS(msg->get_classBDisplayFlag(),test_data->classBDisplayFlag);
        //TS_ASSERT_EQUALS(msg->get_classBDSCFlag(),test_data->classBDSCFlag);
        //TS_ASSERT_EQUALS(msg->get_classBBandFlag(),test_data->classBBandFlag);
        //TS_ASSERT_EQUALS(msg->get_classBMessage22Flag(),test_data->classBMessage22Flag);
        TS_ASSERT_EQUALS(msg->get_modeFlag(),test_data->modeFlag);
        TS_ASSERT_EQUALS(msg->get_RAIMFlag(),test_data->RAIMFlag);
        TS_ASSERT_EQUALS(msg->get_communicationStateSelectorFlag(),test_data->communicationStateSelectorFlag);
        TS_ASSERT_EQUALS(msg->get_communicationState(),test_data->communicationState);
    } else {
        TS_FAIL("caller parameter is NULL");
    }
}


